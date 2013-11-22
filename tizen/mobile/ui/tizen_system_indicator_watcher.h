// Copyright (C) 2000-2011 Carsten Haitzler and various contributors.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_MOBILE_UI_TIZEN_SYSTEM_INDICATOR_WATCHER_H_
#define XWALK_TIZEN_MOBILE_UI_TIZEN_SYSTEM_INDICATOR_WATCHER_H_

#include <string>
#include "base/memory/scoped_ptr.h"
#include "base/memory/shared_memory.h"
#include "base/message_loop/message_pump_libevent.h"
#include "ui/gfx/size.h"
#include "xwalk/tizen/mobile/ui/tizen_plug_message_writer.h"

namespace xwalk {

class TizenSystemIndicator;

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

  void OnMouseDown();
  void OnMouseUp();
  void OnMouseMove(int x, int y);

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
  TizenPlugMessageWriter* writer_;
  base::MessagePumpLibevent::FileDescriptorWatcher fd_watcher_;
  scoped_ptr<base::SharedMemory> shared_memory_;

  int width_;
  int height_;
  int alpha_;
  bool updated_;
  int fd_;
  std::string shm_name_;
  struct EcoreIPCMsgHeader current_msg_header_;

  DISALLOW_COPY_AND_ASSIGN(TizenSystemIndicatorWatcher);
};

}  // namespace xwalk

#endif  // XWALK_TIZEN_MOBILE_UI_TIZEN_SYSTEM_INDICATOR_WATCHER_H_
