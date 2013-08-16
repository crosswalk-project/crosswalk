// Copyright (C) 2000-2011 Carsten Haitzler and various contributors.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_TIZEN_PLUG_H_
#define XWALK_RUNTIME_BROWSER_UI_TIZEN_PLUG_H_

#include <string>
#include "base/memory/scoped_ptr.h"
#include "base/message_pump_libevent.h"
#include "ui/gfx/size.h"

namespace xwalk {

class TizenIndicator;

// Copied from EFL 1.7, in src/lib/ecore_ipc/ecore_ipc.c.
struct ecore_ipc_msg_header {
  int major;
  int minor;
  int ref;
  int ref_to;
  int response;
  int size;
};

// Implementation of the socket protocol for sharing memory used by Elementary
// "plugs" in EFL. This class implements the low level protocol and is used by
// TizenIndicator to update its image.
class TizenPlug : public base::MessagePumpLibevent::Watcher {
 public:
  explicit TizenPlug(TizenIndicator* indicator);
  virtual ~TizenPlug();

  // base::MessagePumpLibevent::Watcher implementation.
  void OnFileCanReadWithoutBlocking(int fd) OVERRIDE;
  void OnFileCanWriteWithoutBlocking(int fd) OVERRIDE;

  void StartWatching();
  void StopWatching();
  bool Connect();

  gfx::Size GetSize() const;

 private:
  size_t GetHeaderSize(unsigned int header_instructions);
  bool UpdateMessageHeader(unsigned int header_instructions,
                           uint8_t* header_payload);
  bool GetHeader();
  bool ShmLoad();
  void ShmUnload();
  bool OpResize(const uint8_t* payload, size_t size);
  bool OpUpdate();
  bool OpUpdateDone();
  bool OpShmRef(const uint8_t* payload, size_t size);
  bool ProcessPayload();
  void SetImageInTizenIndicator();
  void SetSizeFromEnvironment();
  void ResizeTizenIndicator();

  TizenIndicator* indicator_;
  base::MessagePumpLibevent::FileDescriptorWatcher fd_watcher_;
  int width_;
  int height_;
  int alpha_;
  bool updated_;
  int shm_size_;
  int shm_fd_;
  int fd_;
  std::string shm_name_;
  struct ecore_ipc_msg_header current_msg_header_;
  void* shm_mem_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_TIZEN_PLUG_H_
