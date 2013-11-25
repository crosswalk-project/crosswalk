// Copyright (C) 2000-2011 Carsten Haitzler and various contributors.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_MOBILE_UI_TIZEN_PLUG_MESSAGE_WRITER_H_
#define XWALK_TIZEN_MOBILE_UI_TIZEN_PLUG_MESSAGE_WRITER_H_

#include "base/basictypes.h"

// Should match the MAJOR version in ecore_evas_extn.c. We are using
// the same version used by EFL 1.7 available in Tizen Mobile 2.1.
const int kPlugProtocolVersion = 0x1011;

namespace xwalk {

// Copied from EFL 1.7, in src/lib/ecore_ipc/ecore_ipc.c.
struct EcoreIPCMsgHeader {
  int major;
  int minor;
  int ref;
  int ref_to;
  int response;
  int size;
};

// Copied from EFL 1.7, in src/lib/ecore_evas/ecore_evas_extn.c.
enum EvasButtonFlags {
  EVAS_BUTTON_NONE = 0,
  EVAS_BUTTON_DOUBLE_CLICK = (1 << 0),
  EVAS_BUTTON_TRIPLE_CLICK = (1 << 1)
};

enum EvasEventFlags {
  EVAS_EVENT_FLAG_NONE = 0,
  EVAS_EVENT_FLAG_ON_HOLD = (1 << 0),
  EVAS_EVENT_FLAG_ON_SCROLL = (1 << 1)
};

struct IPCDataEvMouseUp {
  int button;
  EvasButtonFlags flags;
  int mask;
  unsigned int timestamp;
  EvasEventFlags event_flags;

  IPCDataEvMouseUp() :
    button(1),
    flags(EVAS_BUTTON_NONE),
    mask(0),
    timestamp(0),
    event_flags(EVAS_EVENT_FLAG_NONE) {}
};

struct IPCDataEvMouseDown {
  int button;
  EvasButtonFlags flags;
  int mask;
  unsigned int timestamp;
  EvasEventFlags event_flags;

  IPCDataEvMouseDown() :
    button(1),
    flags(EVAS_BUTTON_NONE),
    mask(0),
    timestamp(0),
    event_flags(EVAS_EVENT_FLAG_NONE) {}
};

struct IPCDataEvMouseMove {
  int x, y;
  EvasButtonFlags flags;
  int mask;
  unsigned int timestamp;
  EvasEventFlags event_flags;

  IPCDataEvMouseMove() :
    x(0),
    y(0),
    flags(EVAS_BUTTON_NONE),
    mask(0),
    timestamp(0),
    event_flags(EVAS_EVENT_FLAG_NONE) {}
};

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

class TizenPlugMessageWriter {
 public:
  explicit TizenPlugMessageWriter(int* fd);
  virtual ~TizenPlugMessageWriter();

  void SendEvent(int minor, const void* data, int size);
 private:
  void Send(int fd, int major, int minor, int ref, int ref_to, int resp,
            const void* data, int size);

  int ProcessNextInstruction(int out, int prev, Instruction* inst);
  Instruction AttachInstructionData(int msg, int prev, Instruction inst,
                                    int* s, unsigned char* dat);

  bool WriteSafe(const uint8_t* buffer, size_t len);

  EcoreIPCMsgHeader prev_;
  int* fd_;
};

}  // namespace xwalk

#endif  // XWALK_TIZEN_MOBILE_UI_TIZEN_PLUG_MESSAGE_WRITER_H_
