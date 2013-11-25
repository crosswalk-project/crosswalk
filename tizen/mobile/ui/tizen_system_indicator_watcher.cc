// Copyright (C) 2000-2011 Carsten Haitzler and various contributors.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/mobile/ui/tizen_system_indicator_watcher.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "content/public/browser/browser_thread.h"
#include "ipc/unix_domain_socket_util.h"
#include "base/strings/string_tokenizer.h"
#include "base/environment.h"
#include "xwalk/tizen/mobile/ui/tizen_system_indicator.h"

using content::BrowserThread;

namespace {

const char kServiceName[] = "elm_indicator_portrait";
const char kServiceNumber[] = "0";

// Environment variable format is x, y, width, height.
const char kTizenSystemIndicatorGeometryVar[] = "ILLUME_IND";

// Copied from EFL 1.7, in src/lib/ecore_evas/ecore_evas_extn.c.
enum PlugOperation {
  OP_RESIZE,
  OP_SHOW,
  OP_HIDE,
  OP_FOCUS,
  OP_UNFOCUS,
  OP_UPDATE,
  OP_UPDATE_DONE,
  OP_LOCK_FILE,
  OP_SHM_REF,
  OP_EV_MOUSE_IN,
  OP_EV_MOUSE_OUT,
  OP_EV_MOUSE_UP,
  OP_EV_MOUSE_DOWN,
  OP_EV_MOUSE_MOVE,
  OP_EV_MOUSE_WHEEL,
  OP_EV_MULTI_UP,
  OP_EV_MULTI_DOWN,
  OP_EV_MULTI_MOVE,
  OP_EV_KEY_UP,
  OP_EV_KEY_DOWN,
  OP_EV_HOLD
};

}  // namespace

namespace xwalk {

TizenSystemIndicatorWatcher::TizenSystemIndicatorWatcher(TizenSystemIndicator*
                                                         indicator)
  : indicator_(indicator),
    width_(-1),
    height_(-1),
    alpha_(-1),
    updated_(false),
    fd_(-1) {
  writer_ = new TizenPlugMessageWriter(&fd_);
  memset(&current_msg_header_, 0, sizeof(current_msg_header_));
  SetSizeFromEnvVar();
}

TizenSystemIndicatorWatcher::~TizenSystemIndicatorWatcher() {}

void TizenSystemIndicatorWatcher::OnFileCanReadWithoutBlocking(int fd) {
  if (!GetHeader()) {
    PLOG(ERROR) << "Error while getting header";
    StopWatching();
    return;
  }

  if (!ProcessPayload()) {
    PLOG(ERROR) << "Error while processing payload";
    StopWatching();
  }
}

void TizenSystemIndicatorWatcher::OnFileCanWriteWithoutBlocking(int fd) {
}

void TizenSystemIndicatorWatcher::StartWatching() {
  base::MessageLoopForIO::current()->WatchFileDescriptor(
     fd_, true, base::MessageLoopForIO::WATCH_READ, &fd_watcher_, this);
}

void TizenSystemIndicatorWatcher::StopWatching() {
  BrowserThread::PostTask(
      BrowserThread::IO,
      FROM_HERE,
      base::Bind(base::IgnoreResult(&base::MessagePumpLibevent::
                                    FileDescriptorWatcher::
                                    StopWatchingFileDescriptor),
                 base::Unretained(&fd_watcher_)));
}

bool TizenSystemIndicatorWatcher::Connect() {
  base::FilePath path(file_util::GetHomeDir()
                      .Append(".ecore")
                      .Append(kServiceName)
                      .Append(kServiceNumber));
  return IPC::CreateClientUnixDomainSocket(path, &fd_);
}

void TizenSystemIndicatorWatcher::OnMouseDown() {
  struct IPCDataEvMouseDown ipc;
  writer_->SendEvent(OP_EV_MOUSE_DOWN, &ipc, sizeof(ipc));
}

void TizenSystemIndicatorWatcher::OnMouseUp() {
  struct IPCDataEvMouseUp ipc;
  writer_->SendEvent(OP_EV_MOUSE_UP, &ipc, sizeof(ipc));
}

void TizenSystemIndicatorWatcher::OnMouseMove(int x, int y) {
  struct IPCDataEvMouseMove ipc;
  ipc.x = x;
  ipc.y = y;

  writer_->SendEvent(OP_EV_MOUSE_MOVE, &ipc, sizeof(ipc));
}

gfx::Size TizenSystemIndicatorWatcher::GetSize() const {
  return gfx::Size(width_, height_);
}

namespace {

// Headers contain a set of "instructions" and a payload. The instructions are
// used to build the new header based on a previous header. This allows the
// system to save bytes when a certain header field wasn't updated.
//
// This utility class is used to construct the next header based on the previous
// one and the new instructions.
//
// This parser is compatible with the protocol implemented in EFL 1.7 on the
// Tizen Mobile 2.1 platform. See src/lib/ecore_ipc/ecore_ipc.c in ecore source.
class HeaderParser {
 public:
  HeaderParser(unsigned int instructions,
               uint8_t* payload,
               struct EcoreIPCMsgHeader* prev_header,
               struct EcoreIPCMsgHeader* next_header)
      : instructions_(instructions),
        payload_(payload),
        prev_(prev_header),
        next_(next_header) {}

  enum Instruction {
    DLT_ZERO,
    DLT_ONE,
    DLT_SAME,
    DLT_SHL,
    DLT_SHR,
    DLT_ADD8,      // 1 bytes.
    DLT_DEL8,
    DLT_ADDU8,
    DLT_DELU8,
    DLT_ADD16,     // 2 bytes.
    DLT_DEL16,
    DLT_ADDU16,
    DLT_DELU16,
    DLT_SET,       // 4 bytes.
    DLT_R1,
    DLT_R2
  };

  void Parse() {
    next_->major = ProcessNextInstruction(prev_->major);
    next_->minor = ProcessNextInstruction(prev_->minor);
    next_->ref = ProcessNextInstruction(prev_->ref);
    next_->ref_to = ProcessNextInstruction(prev_->ref_to);
    next_->response = ProcessNextInstruction(prev_->response);
    next_->size = ProcessNextInstruction(prev_->size);
    if (next_->size < 0)
      next_->size = 0;
  }

 private:
  // The instruction determine how much data we are going to read from the
  // payload.
  int ExtractInstructionData(Instruction instruction) {
    if (instruction >= DLT_SET) {
      uint32_t v;
      uint8_t* dv = reinterpret_cast<uint8_t*>(&v);
      dv[0] = *(payload_++);
      dv[1] = *(payload_++);
      dv[2] = *(payload_++);
      dv[3] = *(payload_++);
      return static_cast<int>(ntohl(v));
    }
    if (instruction >= DLT_ADD16) {
      uint16_t v;
      uint8_t* dv = reinterpret_cast<uint8_t*>(&v);
      dv[0] = *(payload_++);
      dv[1] = *(payload_++);
      return static_cast<int>(ntohs(v));
    }
    if (instruction >= DLT_ADD8) {
      uint8_t v;
      v = *(payload_++);
      return static_cast<int>(v);
    }
    return 0;
  }

  // Takes the previous value for the field, reads the next instruction and
  // calculates the value based on the instruction data and the previous value.
  int ProcessNextInstruction(int prev) {
    Instruction instruction = Instruction(instructions_ & 0xf);
    instructions_ >>= 4;
    int data = ExtractInstructionData(instruction);

    switch (instruction) {
      case DLT_ZERO:   return 0;
      case DLT_ONE:    return 0xffffffff;
      case DLT_SAME:   return prev;
      case DLT_SHL:    return prev << 1;
      case DLT_SHR:    return prev >> 1;
      case DLT_ADD8:   return prev + data;
      case DLT_DEL8:   return prev - data;
      case DLT_ADDU8:  return prev + (data << 24);
      case DLT_DELU8:  return prev - (data << 24);
      case DLT_ADD16:  return prev + data;
      case DLT_DEL16:  return prev - data;
      case DLT_ADDU16: return prev + (data << 16);
      case DLT_DELU16: return prev - (data << 16);
      case DLT_SET:    return data;
      case DLT_R1:     return 0;
      case DLT_R2:     return 0;
    }
    return 0;
  }

  unsigned int instructions_;
  uint8_t* payload_;
  struct EcoreIPCMsgHeader* prev_;
  struct EcoreIPCMsgHeader* next_;
};

bool ReadSafe(int fd, uint8_t* buffer, size_t len) {
  size_t todo = len;
  while (todo) {
     ssize_t r = read(fd, buffer, todo);
     if (r == 0)
       return false;
     if (r < 0) {
       if (errno == EAGAIN || errno == EINTR)
         continue;
       return false;
     }
     todo -= r;
     buffer += r;
  }
  return true;
}

}  // namespace

size_t TizenSystemIndicatorWatcher::GetHeaderSize(unsigned int
                                                  header_instructions) {
  // Header size will depend on the instructions we read from the header. Each
  // instruction is stored in a nibble.
  int header_size = 0;
  for (int i = 0; i < 6; i++) {
    int instruction = (header_instructions >> (4 * i)) & 0xf;
    if (instruction >= HeaderParser::DLT_SET)
      header_size += 4;
    else if (instruction >= HeaderParser::DLT_ADD16)
      header_size += 2;
    else if (instruction >= HeaderParser::DLT_ADD8)
      header_size += 1;
  }
  return header_size;
}

bool TizenSystemIndicatorWatcher::GetHeader() {
  unsigned int header_instructions;
  if (!ReadSafe(fd_, reinterpret_cast<uint8_t*>(&header_instructions),
                sizeof(header_instructions))) {
    if (errno != EAGAIN)
      PLOG(ERROR) << "Failed to read header_instructions";
    return false;
  }

  header_instructions = ntohl(header_instructions);
  size_t header_size = GetHeaderSize(header_instructions);

  if (header_size == 0)
    return true;

  scoped_ptr<unsigned char[]> header_payload(new unsigned char[header_size]);

  if (!header_payload) {
    PLOG(ERROR) << "Failed to allocate memory for header_payload";
    return false;
  }

  if (!ReadSafe(fd_, header_payload.get(), header_size)) {
    PLOG(ERROR) << "Failed to read header_payload";
    return false;
  }

  struct EcoreIPCMsgHeader next_msg_header;
  HeaderParser parser(header_instructions, header_payload.get(),
                      &current_msg_header_, &next_msg_header);
  parser.Parse();

  if (next_msg_header.major != kPlugProtocolVersion) {
    LOG(WARNING) << "Incorrect protocol version " << next_msg_header.major
                 << " expected " << kPlugProtocolVersion;
    return false;
  }

  current_msg_header_ = next_msg_header;
  return true;
}

bool TizenSystemIndicatorWatcher::MapSharedMemory() {
  // We call shm_open() ourselves since the base::SharedMemory prefixes the name
  // we pass to it, making it not work nicely with a name that doesn't follow
  // the convention, e.g. shared memory created by other app. See
  // SharedMemory::FilePathForMemoryName() in shared_memory_posix.cc.
  int shared_fd = shm_open(shm_name_.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
  if (shared_fd < 0) {
    PLOG(ERROR) << "Failed to open shared memory for System Indicator.";
    return false;
  }

  // SharedMemory will take ownership of the file descriptor.
  const bool auto_close = true;
  const bool read_only = true;
  shared_memory_.reset(new base::SharedMemory(
      base::FileDescriptor(shared_fd, auto_close), read_only));

  int size_in_bytes = width_ * height_ * 4;
  if (!shared_memory_->Map(size_in_bytes)) {
    LOG(ERROR) << "Failed to map shared memory for System Indicator.";
    shared_memory_.reset();
    return false;
  }

  return true;
}

bool TizenSystemIndicatorWatcher::OnResize(const uint8_t* payload,
                                           size_t size) {
  memcpy(&width_, payload, sizeof(width_));
  memcpy(&height_, payload + sizeof(width_), sizeof(height_));

  if (!shared_memory_ && !MapSharedMemory()) {
    LOG(WARNING) << "Failed to load shared memory.";
    return false;
  }

  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&TizenSystemIndicatorWatcher::ResizeIndicator,
                 base::Unretained(this)));

  return true;
}

bool TizenSystemIndicatorWatcher::OnUpdate() {
  updated_ = true;
  return true;
}

bool TizenSystemIndicatorWatcher::OnUpdateDone() {
  if (!updated_) {
    LOG(WARNING) << "OnUpdateDone received without previous OnUpdate!";
    return true;
  }

  if (!shared_memory_ && !MapSharedMemory()) {
    LOG(WARNING) << "Failed to load shared memory.";
    return false;
  }

  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&TizenSystemIndicatorWatcher::UpdateIndicatorImage,
                 base::Unretained(this)));

  updated_ = false;
  return true;
}

bool TizenSystemIndicatorWatcher::OnShmRef(const uint8_t* payload,
                                           size_t size) {
  if (size <= 1)
    return false;

  shm_name_ = std::string(reinterpret_cast<const char*>(payload), size);
  // Extra information about the shared memory is passed in the header.
  width_ = current_msg_header_.ref;
  height_ = current_msg_header_.ref_to;
  alpha_ = current_msg_header_.response;

  return true;
}

bool TizenSystemIndicatorWatcher::ProcessPayload() {
  PlugOperation op_code = PlugOperation(current_msg_header_.minor);
  size_t payload_size = current_msg_header_.size;

  scoped_ptr<unsigned char[]> payload(new unsigned char[1024]);
  if (!payload) {
    PLOG(ERROR) << "Could not allocate memory to payload";
    return false;
  }

  if (!ReadSafe(fd_, payload.get(), payload_size)) {
    PLOG(ERROR) << "Failed to read op payload";
    return false;
  }

  bool ok = false;
  switch (op_code) {
    case OP_RESIZE:
      ok = OnResize(payload.get(), payload_size);
      break;

    case OP_UPDATE:
      // We are always updating the entire area, so we ignore the
      // x, y, w, h passed on the payload.
      ok = OnUpdate();
      break;

    case OP_UPDATE_DONE:
      ok = OnUpdateDone();
      break;

    case OP_SHM_REF:
      ok = OnShmRef(payload.get(), payload_size);
      break;

    case OP_SHOW:
    case OP_HIDE:
    case OP_FOCUS:
    case OP_UNFOCUS:
    case OP_LOCK_FILE:
    case OP_EV_MOUSE_IN:
    case OP_EV_MOUSE_OUT:
    case OP_EV_MOUSE_UP:
    case OP_EV_MOUSE_DOWN:
    case OP_EV_MOUSE_MOVE:
    case OP_EV_MOUSE_WHEEL:
    case OP_EV_MULTI_UP:
    case OP_EV_MULTI_DOWN:
    case OP_EV_MULTI_MOVE:
    case OP_EV_KEY_UP:
    case OP_EV_KEY_DOWN:
    case OP_EV_HOLD:
      // Not implemented yet.
      ok = true;
      break;
  }

  return ok;
}

void TizenSystemIndicatorWatcher::UpdateIndicatorImage() {
  SkBitmap bitmap;

  bitmap.setConfig(SkBitmap::kARGB_8888_Config, width_, height_);
  bitmap.setPixels(shared_memory_->memory());

  const gfx::ImageSkia img_skia = gfx::ImageSkia::CreateFrom1xBitmap(bitmap);
  indicator_->SetImage(img_skia);
}

void TizenSystemIndicatorWatcher::SetSizeFromEnvVar() {
  std::string preferred_size;
  scoped_ptr<base::Environment> env(base::Environment::Create());

  env->GetVar(kTizenSystemIndicatorGeometryVar, &preferred_size);

  if (preferred_size.empty())
    return;

  base::StringTokenizer t(preferred_size, ", ");

  // Environment variable format is: x, y, width, height.
  t.GetNext();
  t.GetNext();

  if (t.GetNext())
    width_ = std::atoi(t.token().c_str());

  if (t.GetNext())
    height_ = std::atoi(t.token().c_str());
}

void TizenSystemIndicatorWatcher::ResizeIndicator() {
  indicator_->PreferredSizeChanged();
  UpdateIndicatorImage();
}

}  // namespace xwalk
