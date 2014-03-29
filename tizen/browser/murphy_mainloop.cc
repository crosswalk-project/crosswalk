// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/browser/murphy_mainloop.h"

#include <errno.h>
#include <murphy/common/log.h>
#include <sys/socket.h>

#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_libevent.h"
#include "content/public/browser/browser_thread.h"

//
// The Murphy resource libraries don't have interfaces that would allow one
// to take externally care of the IPC necessary for communicating with the
// server (the policy decision making entity). Instead these libraries always
// operate within the context of a Murphy mainloop abstraction which by design
// can be set up to be pumped by an external event loop. The MurphyMainloop
// class below creates a Murphy mainloop and takes care of the details of
// adapting it to be pumped by the local MessageLoop infrastructure.
//
// The Murphy mainloop needs abstractions for I/O watches, timers, and deferred
// callbacks from the hosting environment. If necessary it can get along with
// just I/O watches and timers and emulate deferred callbacks by low-delay
// timers. Murphy needs timers to be both cancellable and modifiable and it
// deferred callbacks that can be disabled and re-enabled.
//
// The available infrastructure here offers the basic building blocks for all of
// these but none of them are a perfect fit. Here is the summary of the tweaks
// we need (in the hope) to get the Murphy mainloop running:
//
// 1) You can only do async I/O in the content::BrowserThread::IO thread.
//
//    We're running in content::BrowserThread::UI so we can't manipulate
//    FileDescriptorWatchers there. To overcome this we need to relay all
//    operations on FileDescriptorWatchers to content::BrowserThread::IO
//    by PostTask. This will result in getting all the I/O events delivered
//    to content::BrowserThread::IO. However, since we don't want to set up
//    a (potentially) complex locking scheme to protect access to the mainloop
//    we also relay FileDescriptorWatcher notifications back to the UI thread
//    for processing. Finally to make sure there are no pending events in
//    flight by the time we destroy an I/O watch we essentially drive the
//    destructor through a full UI -> IO -> UI thread relay.
//
// 2) FileDescriptorWatchers don't have HUP events.
//
//    As a POSIX-specific hack we MSG_PEEK and generate a syntetic HUP event
//    if we got 0 bytes.
//
// 3) You can't cancel a PostTask'd or PostDelayedTask'd task.
//
//    This prevents us from having a straightforward mapping of deferred
//    callbacks and timers to these. To overcome this we use a 'timeout finger-
//    print' which is used to ignore timeout callbacks that should have been
//    cancelled if only there was a mechanism for it. The fingerprint in simply
//    a monotonically increasing integer which is stored in the timer and also
//    associated with a pending timeout. Whenever the timer is reconfigured
//    (delay updated, or timer disabled) the fingerpring is updated. Timeout
//    callbacks with mismatching fingerprints are ignored.
//
//
// Additionally to the basic mainloop adaptation we also set up the Murphy
// debugging and logging infrastructure to use the native logging infra as
// a backend. You can control the Murphy logging and debugging by two
// environment variables:
//
//    XWALK_MURPHY_LOG:
//        A comma separated list of log levels, defaults to 'info,warning,error'
//
//    XWALK_MURPHY_DEBUG:
//        A comma-separated list of Murphy debug sites, for instance
//        '@mainloop.c,@resource.c,mrp_resource_set_create'. Setting this to
//        '*' will turn all debug sites on.
//

// Environment variable names to used to control debugging and logging
#define ENVVAR_DBG "XWALK_MURPHY_DEBUG"
#define ENVVAR_LOG "XWALK_MURPHY_LOG"

namespace {

static void xwalklogger(void* data, mrp_log_level_t level, const char* file,
    int line, const char* func, const char* format,
    va_list ap) {
  va_list cp;
  char msg[1024], locbuf[1024];
  const char* loc;

  MRP_UNUSED(data);
  MRP_UNUSED(file);
  MRP_UNUSED(line);

  va_copy(cp, ap);

  if (level != MRP_LOG_DEBUG) {
    loc = "";
  } else {
    snprintf(locbuf, sizeof(locbuf), "[%s] ", func ? func : "<unknown>");
    loc = locbuf;
  }

  if (vsnprintf(msg, sizeof(msg), format, cp) < (ssize_t)sizeof(msg)) {
    switch (level) {
      case MRP_LOG_INFO:    LOG(INFO)    << loc << msg; break;
      case MRP_LOG_WARNING: LOG(WARNING) << loc << msg; break;
      case MRP_LOG_ERROR:   LOG(ERROR)   << loc << msg; break;
      case MRP_LOG_DEBUG:  DLOG(INFO)    << loc << msg; break;
      default:                                          break;
    }
  }

  va_end(cp);
}

// Helper function to check if we have a pending HUP on an fd.
static int pending_hup(int fd) {
  char buf;
  int  len, saved_errno;

  saved_errno = errno;
  len = recv(fd, &buf, 1, MSG_PEEK);
  errno = saved_errno;

  return len == 0;
}

}  // namespace

namespace tizen {

// An I/O watch abstraction for Murphy mainloop integration.
//
// As stated above, FileDescriptorWatcher's can only be manipulated
// in BrowserThread::IO. As we execute in BrowserThread::UI we need
// to relay watch manipulation operations to the I/O thread and
// relay events back to the UI thread.
class IoWatch : public base::MessagePumpLibevent::Watcher {
 public:
  // Constructor. Saves context and relays the watch setup to the I/O thread.
  IoWatch(MurphyMainloop* mainloop, int fd, mrp_io_event_t events,
      void (*cb)(void* glue_data, void* id, int fd, mrp_io_event_t events,
      void* user_data), void* user_data)
      : mainloop_(mainloop),
        fd_(fd),
        events_(events),
        cb_(cb),
        user_data_(user_data),
        dead_(false) {
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
        base::Bind(&IoWatch::StartWatch,
        base::Unretained(this)));
  }

  // Request destruction. Mark dead, relay watch cleanup to the I/O thread.
  // The destruction sequence is finished once StopWatch relays WatchStopped
  // back to the UI thread.
  void Delete() {
    if (dead_)
      return;
    else
      dead_ = true;

    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
        base::Bind(&IoWatch::StopWatch,
        base::Unretained(this)));
  }

 private:
  // Perform watch setup. Run in the I/O thread.
  void StartWatch() {
    base::MessageLoopForIO::Mode mode;

    if (events_ & (MRP_IO_EVENT_IN | MRP_IO_EVENT_OUT))
      mode = base::MessageLoopForIO::WATCH_READ_WRITE;
    else if (events_ & MRP_IO_EVENT_IN)
      mode = base::MessageLoopForIO::WATCH_READ;
    else if (events_ & MRP_IO_EVENT_OUT)
      mode = base::MessageLoopForIO::WATCH_WRITE;
    else
      mode = base::MessageLoopForIO::WATCH_READ;  // Hmm... not quite right.

    const bool success = base::MessageLoopForIO::current()->WatchFileDescriptor(
        fd_, true, mode, &w_, this);
    CHECK(success) << "Failed to add I/O watch for fd " << fd_;
  }

  // Perform watch cleanup. Run in I/O thread.
  void StopWatch() {
    if (dead_)
      return;
    w_.StopWatchingFileDescriptor();

    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
        base::Bind(&IoWatch::WatchStopped,
        base::Unretained(this)));
  }

  // Finish the destruction sequence by self-deleting. Run in UI thread.
  void WatchStopped() {
    delete this;
  }

  // Watch readability event handler. Run in I/O thread. Relays dispatching
  // to UI thread.
  virtual void OnFileCanReadWithoutBlocking(int fd) OVERRIDE {
    if (dead_)
      return;

    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
        base::Bind(&IoWatch::DispatchReadable,
        base::Unretained(this)));
  }

  // Watch writability event handler. Run in I/O thread. Relays dispatching
  // to UI thread.
  virtual void OnFileCanWriteWithoutBlocking(int fd) OVERRIDE {
    if (dead_)
      return;

    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
        base::Bind(&IoWatch::DispatchWritable,
        base::Unretained(this)));
  }

  // Dispatch readability events the the mainloop.
  void DispatchReadable() {
    mrp_io_event_t events;
    if (dead_)
      return;

    if ((events_ & MRP_IO_EVENT_HUP) && pending_hup(fd_))
      events = MRP_IO_EVENT_HUP;
    else
      events = MRP_IO_EVENT_IN;

    mrp_debug("dispatching crosswalk event 0x%x for fd %d", events, fd_);
    cb_(reinterpret_cast<void*>(mainloop_), reinterpret_cast<void*>(this),
        fd_, events, user_data_);
  }

  // Dispatch writability events the the mainloop.
  void DispatchWritable() {
    mrp_io_event_t events;
    if (dead_)
      return;

    events = MRP_IO_EVENT_OUT;
    mrp_debug("dispatching crosswalk event 0x%x for fd %d", events, fd_);
    cb_(reinterpret_cast<void*>(mainloop_), reinterpret_cast<void*>(this),
        fd_, events, user_data_);
  }

  // Associated mainloop data: fd, event mask, callback, and user data.
  MurphyMainloop* mainloop_;
  int fd_;
  mrp_io_event_t events_;
  void (*cb_)(void* glue_data, void* id, int fd, mrp_io_event_t events,
      void* user_data);
  void* user_data_;

  // FileDescriptorWatcher we use for I/O monitoring
  base::MessagePumpLibevent::FileDescriptorWatcher w_;

  // flag used to mark initiated destruction sequence
  bool dead_;

  // A self-destructing object, so we have a private destructor
  virtual ~IoWatch() {
    mrp_debug("Destructing I/O watch");
  }
};

// A Timer abstraction for Murphy mainloop integration.
//
// Since posted tasks cannot be cancelled, we use the stamped Timeout
// object to filter out callbacks for timers that have been cancelled
// (or reconfigured).
class Timer;

class Timeout : public base::RefCounted<Timeout> {
 public:
  explicit Timeout(Timer* t)
      : timer_(t),
        stamp_(0),
        pending_(false) {
    AddRef();
  }

  // Clear Timeout for destruction. After this point all events are ignored.
  // We're just waiting for the last reference to be Release()'d which is
  // immediate if we have no pending timeouts. Otherwise it ought to happen
  // once the last timeout has expired.
  void Delete() {
    timer_ = NULL;
    Release();
  }

  // Arm or rearm the timeout with the given delay.
  void Arm(int delay) {
    base::TimeDelta delta(base::TimeDelta::FromMilliseconds(delay));
    InvalidatePending();
    content::BrowserThread::PostDelayedTask(content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&Timeout::Expired,
        this, stamp_), delta);

    pending_ = true;
  }

  // Disable timeouts.
  void Disable() {
    InvalidatePending();
  }

 private:
  // Invalidate any possible pending Timeouts.
  inline void InvalidatePending() {
    if (pending_)
      stamp_++;
  }

  // Timeout handler callback. Filter cancelled timeouts, deliver valid
  // timeouts to the parent Timer.
  void Expired(unsigned int stamp);

  ~Timeout() {
    mrp_debug("destructing Timeout %p", this);
  }

  // We're refcounted and have a private destructor...
  friend class base::RefCounted<Timeout>;

  // Timer we're serving.
  Timer* timer_;

  // Fingerprint stamp.
  unsigned int stamp_;

  // Flag to mark if we have pending timeouts.
  bool pending_;
};

class Timer {
 public:
  Timer(MurphyMainloop* mainloop, int delay,
      void (*cb)(void* glue_data, void* id, void* user_data),
      void* user_data)
      : mainloop_(mainloop),
        cb_(cb),
        user_data_(user_data),
        delay_(delay),
        enabled_(true),
        timeout_(NULL) {
    timeout_ = new Timeout(this);
    ReArm();
  }

  // Upon timer destruction, initiate timeout destruction sequence and release
  // its initial reference.
  ~Timer() {
    timeout_->Delete();
  }

  // Change timer delay.
  void SetDelay(int delay) {
    delay_ = delay;
    ReArm();
  }

  // Enable timer.
  void Enable() {
    if (!enabled_) {
      enabled_ = true;
      ReArm();
    }
  }

  // Disable timer.
  void Disable() {
    enabled_ = false;
    timeout_->Disable();
  }

 private:
  // Re-arm the timer with the current delay if it is enabled.
  void ReArm() {
    if (!enabled_ || delay_ == -1)
      return;

    mrp_debug("rearming timer %p (delay %d)", this, delay_);

    timeout_->Arm(delay_);
  }

  // Timeout handler callback.
  void DispatchTimer() {
    if (enabled_) {
      mrp_debug("dispatching crosswalk timeout event %p", this);
      cb_(reinterpret_cast<void*>(mainloop_), reinterpret_cast<void*>(this),
          user_data_);
    }

    if (enabled_)
      ReArm();
  }

 private:
  // Let Timeout invoke DispatchTimer.
  friend class Timeout;

  // Associated mainloop data: timer callback and user data.
  MurphyMainloop* mainloop_;
  void (*cb_)(void* glue_data, void* id, void* user_data);
  void* user_data_;

  // Our timeout in milliseconds.
  int delay_;

  // Whether we're enabled.
  bool enabled_;

  // Our associated timeout.
  Timeout* timeout_;
};

void Timeout::Expired(unsigned int stamp) {
  if (timer_ == NULL || stamp != stamp_)
    return;

  pending_ = false;

  timer_->DispatchTimer();
}

MurphyMainloop::MurphyMainloop(const char* log, const char* debug)
    : mainloop_(NULL) {
  setupLogger(log, debug);

  CHECK(setupMainloop());
}

MurphyMainloop::~MurphyMainloop() {}


// Crosswalk mainloop abstraction operations
void* MurphyMainloop::AddIo(void* glue_data, int fd, mrp_io_event_t events,
    void (*cb)(void* glue_data, void* id, int fd, mrp_io_event_t events,
    void* user_data), void* user_data) {

  MurphyMainloop* self = static_cast<MurphyMainloop*>(glue_data);
  mrp_debug("adding I/O Watch for fd %d", fd);

  return new IoWatch(self, fd, events, cb, user_data);
}

// static
void MurphyMainloop::DelIo(void* glue_data, void* id) {
  IoWatch* w = static_cast<IoWatch*>(id);

  mrp_debug("deleting I/O Watch %p", id);
  w->Delete();
}

// static
void* MurphyMainloop::AddTimer(void* glue_data, unsigned int msecs,
    void (*cb)(void* glue_data, void* id, void* user_data),
    void* user_data) {
  MurphyMainloop* self = static_cast<MurphyMainloop*>(glue_data);
  mrp_debug("adding Timer with %u msecs, %p user data", msecs, user_data);

  return new Timer(self, static_cast<int>(msecs), cb, user_data);
}

// static
void MurphyMainloop::DelTimer(void* glue_data, void* id) {
  Timer* t = reinterpret_cast<Timer*>(id);
  MRP_UNUSED(glue_data);

  mrp_debug("deleting Timer %p", id);
  delete t;
}

// static
void MurphyMainloop::ModTimer(void* glue_data, void* id, unsigned int msecs) {
  Timer* t = reinterpret_cast<Timer*>(id);
  MRP_UNUSED(glue_data);

  mrp_debug("modifying Timer %p to %u msecs", t, msecs);
  t->SetDelay(msecs);
}

// static
void* MurphyMainloop::AddDefer(void* glue_data,
    void (*cb)(void* glue_data, void* id, void* user_data),
    void* user_data) {
  mrp_debug("adding deferred callback (cb:%p, user data:%p)", cb, user_data);
  return AddTimer(glue_data, 0, cb, user_data);
}

// static
void MurphyMainloop::DelDefer(void* glue_data, void* id) {
  MRP_UNUSED(glue_data);

  mrp_debug("deleting deferred callback %p", id);

  DelTimer(glue_data, id);
}

// static
void MurphyMainloop::ModDefer(void* glue_data, void* id, int enabled) {
  MRP_UNUSED(glue_data);
  Timer* t = reinterpret_cast<Timer*>(id);

  mrp_debug("%sabling deferred callback %p", enabled ? "en" : "dis", id);

  if (enabled)
    t->Enable();
  else
    t->Disable();
}

// static
void MurphyMainloop::Unregister(void* data) {
  MRP_UNUSED(data);

  mrp_debug("unrgistering mainloop with data %p", data);
}

bool MurphyMainloop::setupMainloop() {
  mainloop_ = mrp_mainloop_create();

  static mrp_superloop_ops_t ops = {
    &MurphyMainloop::AddIo,
    &MurphyMainloop::DelIo,
    &MurphyMainloop::AddTimer,
    &MurphyMainloop::DelTimer,
    &MurphyMainloop::ModTimer,
    &MurphyMainloop::AddDefer,
    &MurphyMainloop::DelDefer,
    &MurphyMainloop::ModDefer,
    &MurphyMainloop::Unregister
  };

  if (mrp_set_superloop(mainloop_, &ops, this)) {
    return true;
  } else {
    mrp_log_error("Failed to set up superloop.");
    return false;
  }
}

// Murphy crosswalk logging backend
void MurphyMainloop::setupLogger(const char* logcfg, const char* dbgcfg) {
  static bool registered = false;
  const char *dbg, *log, *p, *n;
  char        site[1024];
  size_t      l;

  if (registered)
    return;

  if (mrp_log_register_target("xwalk", xwalklogger, NULL))
    mrp_log_set_target("xwalk");

  // configure logging, environment variable overrides argument
  if ((log = getenv(ENVVAR_LOG)) == NULL)
    log = "none";

  mrp_log_enable(mrp_log_parse_levels(log));

  // configure debugging, environment variable overrides argument
  if ((dbg = getenv(ENVVAR_DBG)) == NULL)
    dbg = dbgcfg ? dbgcfg : "off";

  if (strcmp(dbg, "off")) {
    mrp_log_info("Enabling Murphy debugging (%s).", dbg);
    mrp_debug_enable(true);

    p = dbg;
    while (p != NULL) {
      n = strchr(p, ',');
      l = n ? n - p : strlen(p);

      if (l < sizeof(site) - 1) {
        strncpy(site, p, l);
        site[l] = '\0';
        mrp_log_info("Enabling Murphy debug site '%s'.", site);
        mrp_debug_set_config(site);
      }
      p = n ? n + 1 : NULL;
    }
  }
  registered = true;
}

}  // namespace tizen
