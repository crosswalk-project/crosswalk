// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_process_singleton.h"

#include <errno.h>
#include <fcntl.h>
#if defined(TOOLKIT_GTK)
#include <gdk/gdk.h>
#endif
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
#include <set>
#include <string>

#include "base/base_paths.h"
#include "base/basictypes.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/posix/eintr_wrapper.h"
#include "base/process_util.h"
#include "base/rand_util.h"
#include "base/safe_strerror_posix.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/stringprintf.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/platform_thread.h"
#include "base/time.h"
#include "base/timer.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/net_util.h"
#include "ui/base/l10n/l10n_util.h"

using content::BrowserThread;

const int ProcessSingleton::kTimeoutInSeconds;

namespace {

const char kStartToken[] = "START";
const char kACKToken[] = "ACK";
const char kShutdownToken[] = "SHUTDOWN";
const char kTokenDelimiter = '\0';
const int kMaxMessageLength = 32 * 1024;
const int kMaxACKMessageLength = arraysize(kShutdownToken) - 1;

const char kLockDelimiter = '-';

// Set a file descriptor to be non-blocking.
// Return 0 on success, -1 on failure.
int SetNonBlocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (-1 == flags)
    return flags;
  if (flags & O_NONBLOCK)
    return 0;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Set the close-on-exec bit on a file descriptor.
// Returns 0 on success, -1 on failure.
int SetCloseOnExec(int fd) {
  int flags = fcntl(fd, F_GETFD, 0);
  if (-1 == flags)
    return flags;
  if (flags & FD_CLOEXEC)
    return 0;
  return fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

// Close a socket and check return value.
void CloseSocket(int fd) {
  int rv = HANDLE_EINTR(close(fd));
  DCHECK_EQ(0, rv) << "Error closing socket: " << safe_strerror(errno);
}

// Write a message to a socket fd.
bool WriteToSocket(int fd, const char *message, size_t length) {
  DCHECK(message);
  DCHECK(length);
  size_t bytes_written = 0;
  do {
    ssize_t rv = HANDLE_EINTR(
        write(fd, message + bytes_written, length - bytes_written));
    if (rv < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // The socket shouldn't block, we're sending so little data.  Just give
        // up here, since NotifyOtherProcess() doesn't have an asynchronous api.
        LOG(ERROR) << "ProcessSingleton would block on write(), so it gave up.";
        return false;
      }
      PLOG(ERROR) << "write() failed";
      return false;
    }
    bytes_written += rv;
  } while (bytes_written < length);

  return true;
}

// Wait a socket for read for a certain timeout in seconds.
// Returns -1 if error occurred, 0 if timeout reached, > 0 if the socket is
// ready for read.
int WaitSocketForRead(int fd, int timeout) {
  fd_set read_fds;
  struct timeval tv;

  FD_ZERO(&read_fds);
  FD_SET(fd, &read_fds);
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  return HANDLE_EINTR(select(fd + 1, &read_fds, NULL, NULL, &tv));
}

// Read a message from a socket fd, with an optional timeout in seconds.
// If |timeout| <= 0 then read immediately.
// Return number of bytes actually read, or -1 on error.
ssize_t ReadFromSocket(int fd, char *buf, size_t bufsize, int timeout) {
  if (timeout > 0) {
    int rv = WaitSocketForRead(fd, timeout);
    if (rv <= 0)
      return rv;
  }

  size_t bytes_read = 0;
  do {
    ssize_t rv = HANDLE_EINTR(read(fd, buf + bytes_read, bufsize - bytes_read));
    if (rv < 0) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        PLOG(ERROR) << "read() failed";
        return rv;
      } else {
        // It would block, so we just return what has been read.
        return bytes_read;
      }
    } else if (!rv) {
      // No more data to read.
      return bytes_read;
    } else {
      bytes_read += rv;
    }
  } while (bytes_read < bufsize);

  return bytes_read;
}

// Set up a sockaddr appropriate for messaging.
void SetupSockAddr(const std::string& path, struct sockaddr_un* addr) {
  addr->sun_family = AF_UNIX;
  CHECK(path.length() < arraysize(addr->sun_path))
      << "Socket path too long: " << path;
  base::strlcpy(addr->sun_path, path.c_str(), arraysize(addr->sun_path));
}

// Set up a socket appropriate for messaging.
int SetupSocketOnly() {
  int sock = socket(PF_UNIX, SOCK_STREAM, 0);
  PCHECK(sock >= 0) << "socket() failed";

  int rv = SetNonBlocking(sock);
  DCHECK_EQ(0, rv) << "Failed to make non-blocking socket.";
  rv = SetCloseOnExec(sock);
  DCHECK_EQ(0, rv) << "Failed to set CLOEXEC on socket.";

  return sock;
}

// Set up a socket and sockaddr appropriate for messaging.
void SetupSocket(const std::string& path, int* sock, struct sockaddr_un* addr) {
  *sock = SetupSocketOnly();
  SetupSockAddr(path, addr);
}

// Read a symbolic link, return empty string if given path is not a symbol link.
base::FilePath ReadLink(const base::FilePath& path) {
  base::FilePath target;
  if (!file_util::ReadSymbolicLink(path, &target)) {
    // The only errno that should occur is ENOENT.
    if (errno != 0 && errno != ENOENT)
      PLOG(ERROR) << "readlink(" << path.value() << ") failed";
  }
  return target;
}

// Unlink a path. Return true on success.
bool UnlinkPath(const base::FilePath& path) {
  int rv = unlink(path.value().c_str());
  if (rv < 0 && errno != ENOENT)
    PLOG(ERROR) << "Failed to unlink " << path.value();

  return rv == 0;
}

// Create a symlink. Returns true on success.
bool SymlinkPath(const base::FilePath& target, const base::FilePath& path) {
  if (!file_util::CreateSymbolicLink(target, path)) {
    // Double check the value in case symlink suceeded but we got an incorrect
    // failure due to NFS packet loss & retry.
    int saved_errno = errno;
    if (ReadLink(path) != target) {
      // If we failed to create the lock, most likely another instance won the
      // startup race.
      errno = saved_errno;
      PLOG(ERROR) << "Failed to create " << path.value();
      return false;
    }
  }
  return true;
}

// Extract the hostname and pid from the lock symlink.
// Returns true if the lock existed.
bool ParseLockPath(const base::FilePath& path,
                   std::string* hostname,
                   int* pid) {
  std::string real_path = ReadLink(path).value();
  if (real_path.empty())
    return false;

  std::string::size_type pos = real_path.rfind(kLockDelimiter);

  // If the path is not a symbolic link, or doesn't contain what we expect,
  // bail.
  if (pos == std::string::npos) {
    *hostname = "";
    *pid = -1;
    return true;
  }

  *hostname = real_path.substr(0, pos);

  const std::string& pid_str = real_path.substr(pos + 1);
  if (!base::StringToInt(pid_str, pid))
    *pid = -1;

  return true;
}

void DisplayProfileInUseError(const std::string& lock_path,
                              const std::string& hostname,
                              int pid) {
  PLOG(ERROR) << "Profile in use.";
}

bool IsXwalkProcess(pid_t pid) {
  base::FilePath other_xwalk_path(base::GetProcessExecutablePath(pid));
  return (!other_xwalk_path.empty());
}

// A helper class to hold onto a socket.
class ScopedSocket {
 public:
  ScopedSocket() : fd_(-1) { Reset(); }
  ~ScopedSocket() { Close(); }
  int fd() { return fd_; }
  void Reset() {
    Close();
    fd_ = SetupSocketOnly();
  }
  void Close() {
    if (fd_ >= 0)
      CloseSocket(fd_);
    fd_ = -1;
  }
 private:
  int fd_;
};

bool ConnectSocket(ScopedSocket* socket,
                   const base::FilePath& socket_path) {
  base::FilePath socket_target;
  if (file_util::ReadSymbolicLink(socket_path, &socket_target)) {
    // Now we know the directory was (at that point) created by the profile
    // owner. Try to connect.
    sockaddr_un addr;
    SetupSockAddr(socket_path.value(), &addr);
    int ret = HANDLE_EINTR(connect(socket->fd(),
                                   reinterpret_cast<sockaddr*>(&addr),
                                   sizeof(addr)));
    if (ret != 0)
      return false;
    // Success!
    return true;
  } else if (errno == EINVAL) {
    // It exists, but is not a symlink (or some other error we detect
    // later). Just connect to it directly; this is an older version of Chrome.
    sockaddr_un addr;
    SetupSockAddr(socket_path.value(), &addr);
    int ret = HANDLE_EINTR(connect(socket->fd(),
                                   reinterpret_cast<sockaddr*>(&addr),
                                   sizeof(addr)));
    return (ret == 0);
  } else {
    // File is missing, or other error.
    if (errno != ENOENT)
      PLOG(ERROR) << "readlink failed";
    return false;
  }
}

}  // namespace

///////////////////////////////////////////////////////////////////////////////
// ProcessSingleton::LinuxWatcher
// A helper class for a Linux specific implementation of the process singleton.
// This class sets up a listener on the singleton socket and handles parsing
// messages that come in on the singleton socket.
class ProcessSingleton::LinuxWatcher
    : public base::MessageLoopForIO::Watcher,
      public base::MessageLoop::DestructionObserver,
      public base::RefCountedThreadSafe<ProcessSingleton::LinuxWatcher,
                                        BrowserThread::DeleteOnIOThread> {
 public:
  // A helper class to read message from an established socket.
  class SocketReader : public base::MessageLoopForIO::Watcher {
   public:
    SocketReader(ProcessSingleton::LinuxWatcher* parent,
                 base::MessageLoop* ui_message_loop,
                 int fd)
        : parent_(parent),
          ui_message_loop_(ui_message_loop),
          fd_(fd),
          bytes_read_(0) {
      DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
      // Wait for reads.
      base::MessageLoopForIO::current()->WatchFileDescriptor(
          fd, true, base::MessageLoopForIO::WATCH_READ, &fd_reader_, this);
      // If we haven't completed in a reasonable amount of time, give up.
      timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(kTimeoutInSeconds),
                   this, &SocketReader::CleanupAndDeleteSelf);
    }

    virtual ~SocketReader() {
      CloseSocket(fd_);
    }

    // MessageLoopForIO::Watcher impl.
    virtual void OnFileCanReadWithoutBlocking(int fd) OVERRIDE;
    virtual void OnFileCanWriteWithoutBlocking(int fd) OVERRIDE {
      // SocketReader only watches for accept (read) events.
      NOTREACHED();
    }

    // Finish handling the incoming message by optionally sending back an ACK
    // message and removing this SocketReader.
    void FinishWithACK(const char *message, size_t length);

   private:
    void CleanupAndDeleteSelf() {
      DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));

      parent_->RemoveSocketReader(this);
      // We're deleted beyond this point.
    }

    base::MessageLoopForIO::FileDescriptorWatcher fd_reader_;

    // The ProcessSingleton::LinuxWatcher that owns us.
    ProcessSingleton::LinuxWatcher* const parent_;

    // A reference to the UI message loop.
    base::MessageLoop* const ui_message_loop_;

    // The file descriptor we're reading.
    const int fd_;

    // Store the message in this buffer.
    char buf_[kMaxMessageLength];

    // Tracks the number of bytes we've read in case we're getting partial
    // reads.
    size_t bytes_read_;

    base::OneShotTimer<SocketReader> timer_;

    DISALLOW_COPY_AND_ASSIGN(SocketReader);
  };

  // We expect to only be constructed on the UI thread.
  explicit LinuxWatcher(ProcessSingleton* parent)
      : ui_message_loop_(base::MessageLoop::current()),
        parent_(parent) {
  }

  // Start listening for connections on the socket.  This method should be
  // called from the IO thread.
  void StartListening(int socket);

  // This method determines if we should use the same process and if we should,
  // opens a new browser tab.  This runs on the UI thread.
  // |reader| is for sending back ACK message.
  void HandleMessage(const std::string& current_dir,
                     const std::vector<std::string>& argv,
                     SocketReader* reader);

  // MessageLoopForIO::Watcher impl.  These run on the IO thread.
  virtual void OnFileCanReadWithoutBlocking(int fd) OVERRIDE;
  virtual void OnFileCanWriteWithoutBlocking(int fd) OVERRIDE {
    // ProcessSingleton only watches for accept (read) events.
    NOTREACHED();
  }

  // MessageLoop::DestructionObserver
  virtual void WillDestroyCurrentMessageLoop() OVERRIDE {
    fd_watcher_.StopWatchingFileDescriptor();
  }

 private:
  friend struct BrowserThread::DeleteOnThread<BrowserThread::IO>;
  friend class base::DeleteHelper<ProcessSingleton::LinuxWatcher>;

  virtual ~LinuxWatcher() {
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
    STLDeleteElements(&readers_);

    base::MessageLoopForIO* ml = base::MessageLoopForIO::current();
    ml->RemoveDestructionObserver(this);
  }

  // Removes and deletes the SocketReader.
  void RemoveSocketReader(SocketReader* reader);

  base::MessageLoopForIO::FileDescriptorWatcher fd_watcher_;

  // A reference to the UI message loop (i.e., the message loop we were
  // constructed on).
  base::MessageLoop* ui_message_loop_;

  // The ProcessSingleton that owns us.
  ProcessSingleton* const parent_;

  std::set<SocketReader*> readers_;

  DISALLOW_COPY_AND_ASSIGN(LinuxWatcher);
};

void ProcessSingleton::LinuxWatcher::OnFileCanReadWithoutBlocking(int fd) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  // Accepting incoming client.
  sockaddr_un from;
  socklen_t from_len = sizeof(from);
  int connection_socket = HANDLE_EINTR(accept(
      fd, reinterpret_cast<sockaddr*>(&from), &from_len));
  if (-1 == connection_socket) {
    PLOG(ERROR) << "accept() failed";
    return;
  }
  int rv = SetNonBlocking(connection_socket);
  DCHECK_EQ(0, rv) << "Failed to make non-blocking socket.";
  SocketReader* reader = new SocketReader(this,
                                          ui_message_loop_,
                                          connection_socket);
  readers_.insert(reader);
}

void ProcessSingleton::LinuxWatcher::StartListening(int socket) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  // Watch for client connections on this socket.
  base::MessageLoopForIO* ml = base::MessageLoopForIO::current();
  ml->AddDestructionObserver(this);
  ml->WatchFileDescriptor(socket, true, base::MessageLoopForIO::WATCH_READ,
                          &fd_watcher_, this);
}

void ProcessSingleton::LinuxWatcher::HandleMessage(
    const std::string& current_dir, const std::vector<std::string>& argv,
    SocketReader* reader) {
  DCHECK(ui_message_loop_ == base::MessageLoop::current());
  DCHECK(reader);

  if (parent_->notification_callback_.Run(CommandLine(argv),
                                          base::FilePath(current_dir))) {
    // Send back "ACK" message to prevent the client process from starting up.
    reader->FinishWithACK(kACKToken, arraysize(kACKToken) - 1);
  } else {
    LOG(WARNING) << "Not handling interprocess notification as browser"
                    " is shutting down";
    // Send back "SHUTDOWN" message, so that the client process can start up
    // without killing this process.
    reader->FinishWithACK(kShutdownToken, arraysize(kShutdownToken) - 1);
    return;
  }
}

void ProcessSingleton::LinuxWatcher::RemoveSocketReader(SocketReader* reader) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  DCHECK(reader);
  readers_.erase(reader);
  delete reader;
}

///////////////////////////////////////////////////////////////////////////////
// ProcessSingleton::LinuxWatcher::SocketReader
//

void ProcessSingleton::LinuxWatcher::SocketReader::OnFileCanReadWithoutBlocking(
    int fd) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  DCHECK_EQ(fd, fd_);
  while (bytes_read_ < sizeof(buf_)) {
    ssize_t rv = HANDLE_EINTR(
        read(fd, buf_ + bytes_read_, sizeof(buf_) - bytes_read_));
    if (rv < 0) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        PLOG(ERROR) << "read() failed";
        CloseSocket(fd);
        return;
      } else {
        // It would block, so we just return and continue to watch for the next
        // opportunity to read.
        return;
      }
    } else if (!rv) {
      // No more data to read.  It's time to process the message.
      break;
    } else {
      bytes_read_ += rv;
    }
  }

  // Validate the message.  The shortest message is kStartToken\0x\0x
  const size_t kMinMessageLength = arraysize(kStartToken) + 4;
  if (bytes_read_ < kMinMessageLength) {
    buf_[bytes_read_] = 0;
    LOG(ERROR) << "Invalid socket message (wrong length):" << buf_;
    CleanupAndDeleteSelf();
    return;
  }

  std::string str(buf_, bytes_read_);
  std::vector<std::string> tokens;
  base::SplitString(str, kTokenDelimiter, &tokens);

  if (tokens.size() < 3 || tokens[0] != kStartToken) {
    LOG(ERROR) << "Wrong message format: " << str;
    CleanupAndDeleteSelf();
    return;
  }

  // Stop the expiration timer to prevent this SocketReader object from being
  // terminated unexpectly.
  timer_.Stop();

  std::string current_dir = tokens[1];
  // Remove the first two tokens.  The remaining tokens should be the command
  // line argv array.
  tokens.erase(tokens.begin());
  tokens.erase(tokens.begin());

  // Return to the UI thread to handle opening a new browser tab.
  ui_message_loop_->PostTask(FROM_HERE, base::Bind(
      &ProcessSingleton::LinuxWatcher::HandleMessage,
      parent_,
      current_dir,
      tokens,
      this));
  fd_reader_.StopWatchingFileDescriptor();

  // LinuxWatcher::HandleMessage() is in charge of destroying this SocketReader
  // object by invoking SocketReader::FinishWithACK().
}

void ProcessSingleton::LinuxWatcher::SocketReader::FinishWithACK(
    const char *message, size_t length) {
  if (message && length) {
    // Not necessary to care about the return value.
    WriteToSocket(fd_, message, length);
  }

  if (shutdown(fd_, SHUT_WR) < 0)
    PLOG(ERROR) << "shutdown() failed";

  BrowserThread::PostTask(
      BrowserThread::IO,
      FROM_HERE,
      base::Bind(&ProcessSingleton::LinuxWatcher::RemoveSocketReader,
                 parent_,
                 this));
  // We will be deleted once the posted RemoveSocketReader task runs.
}

///////////////////////////////////////////////////////////////////////////////
// ProcessSingleton
//
ProcessSingleton::ProcessSingleton(
    const NotificationCallback& notification_callback)
    : notification_callback_(notification_callback),
      current_pid_(base::GetCurrentProcId()),
      watcher_(new LinuxWatcher(this)) {
  base::FilePath tmp_dir;
  file_util::GetTempDir(&tmp_dir);
  socket_path_ = tmp_dir.Append(FILE_PATH_LITERAL("SingletonSocket"));
  lock_path_ = tmp_dir.Append(FILE_PATH_LITERAL("SingletonLock"));

  kill_callback_ = base::Bind(&ProcessSingleton::KillProcess,
                              base::Unretained(this));
}

ProcessSingleton::~ProcessSingleton() {
}

ProcessSingleton::NotifyResult ProcessSingleton::NotifyOtherProcess() {
  return NotifyOtherProcessWithTimeout(*CommandLine::ForCurrentProcess(),
                                       kTimeoutInSeconds,
                                       true);
}

ProcessSingleton::NotifyResult ProcessSingleton::NotifyOtherProcessWithTimeout(
    const CommandLine& cmd_line,
    int timeout_seconds,
    bool kill_unresponsive) {
  DCHECK_GE(timeout_seconds, 0);

  ScopedSocket socket;
  for (int retries = 0; retries <= timeout_seconds; ++retries) {
    // Try to connect to the socket.
    if (ConnectSocket(&socket, socket_path_))
      break;

    // If we're in a race with another process, they may be in Create() and have
    // created the lock but not attached to the socket.  So we check if the
    // process with the pid from the lockfile is currently running and is a
    // xwalk browser.  If so, we loop and try again for |timeout_seconds|.

    std::string hostname;
    int pid;
    if (!ParseLockPath(lock_path_, &hostname, &pid)) {
      // No lockfile exists.
      return PROCESS_NONE;
    }

    if (hostname.empty()) {
      // Invalid lockfile.
      UnlinkPath(lock_path_);
      return PROCESS_NONE;
    }

    if (hostname != net::GetHostName()) {
      // Locked by process on another host.
      DisplayProfileInUseError(lock_path_.value(), hostname, pid);
      return PROFILE_IN_USE;
    }

    if (!IsXwalkProcess(pid)) {
      // Orphaned lockfile (no process with pid, or non-xwalk process.)
      UnlinkPath(lock_path_);
      return PROCESS_NONE;
    }

    if (IsSameXwalkInstance(pid)) {
      // Orphaned lockfile (pid is part of same xwalk instance we are, even
      // though we haven't tried to create a lockfile yet).
      UnlinkPath(lock_path_);
      return PROCESS_NONE;
    }

    if (retries == timeout_seconds) {
      // Retries failed.  Kill the unresponsive xwalk process and continue.
      if (!kill_unresponsive || !KillProcessByLockPath())
        return PROFILE_IN_USE;
      return PROCESS_NONE;
    }

    base::PlatformThread::Sleep(base::TimeDelta::FromSeconds(1));
  }

  timeval timeout = {timeout_seconds, 0};
  setsockopt(socket.fd(), SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

  // Found another process, prepare our command line
  // format is "START\0<current dir>\0<argv[0]>\0...\0<argv[n]>".
  std::string to_send(kStartToken);
  to_send.push_back(kTokenDelimiter);

  base::FilePath current_dir;
  if (!PathService::Get(base::DIR_CURRENT, &current_dir))
    return PROCESS_NONE;
  to_send.append(current_dir.value());

  const std::vector<std::string>& argv = cmd_line.argv();
  for (std::vector<std::string>::const_iterator it = argv.begin();
      it != argv.end(); ++it) {
    to_send.push_back(kTokenDelimiter);
    to_send.append(*it);
  }

  // Send the message
  if (!WriteToSocket(socket.fd(), to_send.data(), to_send.length())) {
    // Try to kill the other process, because it might have been dead.
    if (!kill_unresponsive || !KillProcessByLockPath())
      return PROFILE_IN_USE;
    return PROCESS_NONE;
  }

  if (shutdown(socket.fd(), SHUT_WR) < 0)
    PLOG(ERROR) << "shutdown() failed";

  // Read ACK message from the other process. It might be blocked for a certain
  // timeout, to make sure the other process has enough time to return ACK.
  char buf[kMaxACKMessageLength + 1];
  ssize_t len =
      ReadFromSocket(socket.fd(), buf, kMaxACKMessageLength, timeout_seconds);

  // Failed to read ACK, the other process might have been frozen.
  if (len <= 0) {
    if (!kill_unresponsive || !KillProcessByLockPath())
      return PROFILE_IN_USE;
    return PROCESS_NONE;
  }

  buf[len] = '\0';
  if (strncmp(buf, kShutdownToken, arraysize(kShutdownToken) - 1) == 0) {
    // The other process is shutting down, it's safe to start a new process.
    return PROCESS_NONE;
  } else if (strncmp(buf, kACKToken, arraysize(kACKToken) - 1) == 0) {
#if defined(TOOLKIT_GTK)
    // Notify the window manager that we've started up; if we do not open a
    // window, GTK will not automatically call this for us.
    gdk_notify_startup_complete();
#endif
    // Assume the other process is handling the request.
    return PROCESS_NOTIFIED;
  }

  NOTREACHED() << "The other process returned unknown message: " << buf;
  return PROCESS_NOTIFIED;
}

ProcessSingleton::NotifyResult ProcessSingleton::NotifyOtherProcessOrCreate() {
  return NotifyOtherProcessWithTimeoutOrCreate(
      *CommandLine::ForCurrentProcess(),
      kTimeoutInSeconds);
}

ProcessSingleton::NotifyResult
ProcessSingleton::NotifyOtherProcessWithTimeoutOrCreate(
    const CommandLine& command_line,
    int timeout_seconds) {
  NotifyResult result = NotifyOtherProcessWithTimeout(command_line,
                                                      timeout_seconds, true);
  if (result != PROCESS_NONE)
    return result;
  if (Create())
    return PROCESS_NONE;
  // If the Create() failed, try again to notify. (It could be that another
  // instance was starting at the same time and managed to grab the lock before
  // we did.)
  // This time, we don't want to kill anything if we aren't successful, since we
  // aren't going to try to take over the lock ourselves.
  result = NotifyOtherProcessWithTimeout(command_line, timeout_seconds, false);
  if (result != PROCESS_NONE)
    return result;

  return LOCK_ERROR;
}

bool ProcessSingleton::Create() {
  int sock;
  sockaddr_un addr;

  // The symlink lock is pointed to the hostname and process id, so other
  // processes can find it out.
  base::FilePath symlink_content(base::StringPrintf(
      "%s%c%u",
      net::GetHostName().c_str(),
      kLockDelimiter,
      current_pid_));

  // Create symbol link before binding the socket, to ensure only one instance
  // can have the socket open.
  if (!SymlinkPath(symlink_content, lock_path_)) {
      // If we failed to create the lock, most likely another instance won the
      // startup race.
      return false;
  }

  UnlinkPath(socket_path_);
  SetupSocket(socket_path_.value(), &sock, &addr);

  if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    PLOG(ERROR) << "Failed to bind() " << socket_path_.value();
    CloseSocket(sock);
    return false;
  }

  if (listen(sock, 5) < 0)
    NOTREACHED() << "listen failed: " << safe_strerror(errno);

  DCHECK(BrowserThread::IsMessageLoopValid(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::IO,
      FROM_HERE,
      base::Bind(&ProcessSingleton::LinuxWatcher::StartListening,
                 watcher_.get(),
                 sock));

  return true;
}

void ProcessSingleton::Cleanup() {
  UnlinkPath(socket_path_);
  UnlinkPath(lock_path_);
}

bool ProcessSingleton::IsSameXwalkInstance(pid_t pid) {
  pid_t cur_pid = current_pid_;
  while (pid != cur_pid) {
    pid = base::GetParentProcessId(pid);
    if (pid < 0)
      return false;
    if (!IsXwalkProcess(pid))
      return false;
  }
  return true;
}

bool ProcessSingleton::KillProcessByLockPath() {
  std::string hostname;
  int pid;
  ParseLockPath(lock_path_, &hostname, &pid);

  if (!hostname.empty() && hostname != net::GetHostName()) {
    DisplayProfileInUseError(lock_path_.value(), hostname, pid);
    return false;
  }
  UnlinkPath(lock_path_);

  if (IsSameXwalkInstance(pid))
    return true;

  if (pid > 0) {
    kill_callback_.Run(pid);
    return true;
  }

  LOG(ERROR) << "Failed to extract pid from path: " << lock_path_.value();
  return true;
}

void ProcessSingleton::KillProcess(int pid) {
  // TODO(james.su@gmail.com): Is SIGKILL ok?
  int rv = kill(static_cast<base::ProcessHandle>(pid), SIGKILL);
  // ESRCH = No Such Process (can happen if the other process is already in
  // progress of shutting down and finishes before we try to kill it).
  DCHECK(rv == 0 || errno == ESRCH) << "Error killing process: "
                                    << safe_strerror(errno);
}
