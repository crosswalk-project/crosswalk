// Copyright (C) 2000-2011 Carsten Haitzler and various contributors.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_TIZEN_SYSTEM_INDICATOR_WATCHER_H_
#define XWALK_RUNTIME_BROWSER_UI_TIZEN_SYSTEM_INDICATOR_WATCHER_H_

#include <string>
#include "base/memory/scoped_ptr.h"
#include "base/memory/shared_memory.h"
#include "base/message_pump_libevent.h"
#include "ui/gfx/size.h"

namespace xwalk {

class TizenSystemIndicator;

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
// "Plugs" in EFL. This class implements the low level protocol and is used by
// TizenSystemIndicator to update its image.
class TizenSystemIndicatorWatcher : public base::MessagePumpLibevent::Watcher {
 public:
  explicit TizenSystemIndicatorWatcher(TizenSystemIndicator* indicator);
  virtual ~TizenSystemIndicatorWatcher();

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
  bool MapSharedMemory();
  bool OnResize(const uint8_t* payload, size_t size);
  bool OnUpdate();
  bool OnUpdateDone();
  bool OnShmRef(const uint8_t* payload, size_t size);
  bool ProcessPayload();
  void UpdateIndicatorImage();
  void SetSizeFromEnvVar();
  void ResizeIndicator();

  TizenSystemIndicator* indicator_;
  base::MessagePumpLibevent::FileDescriptorWatcher fd_watcher_;
  scoped_ptr<base::SharedMemory> shared_memory_;

  int width_;
  int height_;
  int alpha_;
  bool updated_;
  int fd_;
  std::string shm_name_;
  struct ecore_ipc_msg_header current_msg_header_;

  DISALLOW_COPY_AND_ASSIGN(TizenSystemIndicatorWatcher);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_TIZEN_SYSTEM_INDICATOR_WATCHER_H_
