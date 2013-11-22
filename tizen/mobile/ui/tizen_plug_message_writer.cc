// Copyright (C) 2000-2011 Carsten Haitzler and various contributors.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/mobile/ui/tizen_plug_message_writer.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>

namespace xwalk {

// This MessageWriter is used to build and write messages using the protocol
// implemented in EFL 1.7 on the Tizen Mobile 2.1 platform.
//
// Messages contain a header and a payload. The instructions are used to build
// the new header based on the previous header. This allows the system to save
// bytes when a certain header field wasn't updated.
//
// This class is used to construct the next message based on the previous one
// and the new instructions.
//
// See src/lib/ecore_ipc/ecore_ipc.c in ecore source
// (http://git.enlightenment.org/legacy/ecore.git).

TizenPlugMessageWriter::TizenPlugMessageWriter(int* fd)
  : fd_(fd) {
  memset(&prev_, 0, sizeof(struct EcoreIPCMsgHeader));
}

TizenPlugMessageWriter::~TizenPlugMessageWriter() {
}

void TizenPlugMessageWriter::SendEvent(int minor, const void* data, int size) {
  Send(*fd_, kPlugProtocolVersion, minor, 0, 0, 0, data, size);
}

void TizenPlugMessageWriter::Send(int fd, int major, int minor, int ref,
                                  int ref_to, int response, const void* data,
                                  int size) {
  struct EcoreIPCMsgHeader msg;
  Instruction instruction = DLT_ZERO;
  int s = 4;
  unsigned char dat[sizeof(struct EcoreIPCMsgHeader)];
  int* head = reinterpret_cast<int*>(dat);

  memset(&msg, 0, sizeof(msg));

  if (size < 0)
    size = 0;

  msg.major    = major;
  msg.minor    = minor;
  msg.ref      = ref;
  msg.ref_to   = ref_to;
  msg.response = response;
  msg.size     = size;

  *head  = AttachInstructionData(msg.major, prev_.major, instruction,
                                 &s, dat);
  *head |= AttachInstructionData(msg.minor, prev_.minor, instruction,
                                 &s, dat) << (4 * 1);
  *head |= AttachInstructionData(msg.ref, prev_.ref, instruction,
                                 &s, dat) << (4 * 2);
  *head |= AttachInstructionData(msg.ref_to, prev_.ref_to, instruction,
                                 &s, dat) << (4 * 3);
  *head |= AttachInstructionData(msg.response, prev_.response, instruction,
                                 &s, dat) << (4 * 4);
  *head |= AttachInstructionData(msg.size, prev_.size, instruction,
                                 &s, dat) << (4 * 5);
  *head = htonl(*head);

  prev_ = msg;  // Store previous message.

  if (!WriteSafe(reinterpret_cast<const uint8_t*>(&dat), s)) {
    perror("Failed to write msg_header");
    return;
  }

  if (size <= 0)
    return;

  if (!WriteSafe(reinterpret_cast<const uint8_t*>(data), size))
    perror("Failed to write msg");
}

int TizenPlugMessageWriter::ProcessNextInstruction(int out, int prev,
                                                   Instruction* inst) {
  int int_max = static_cast<int>(0xffffffff);
  // 0 byte.
  if (out == 0) {            *inst = DLT_ZERO;   return 0; }
  if (out == int_max) {      *inst = DLT_ONE;    return 0; }
  if (out == prev) {         *inst = DLT_SAME;   return 0; }
  if (out == prev << 1) {    *inst = DLT_SHL;    return 0; }
  if (out == prev >> 1) {    *inst = DLT_SHR;    return 0; }
  // 1 byte.
  int dlt = out - prev;
  if (!(dlt & 0xffffff00)) { *inst = DLT_ADD8;   return dlt & 0xff; }
  dlt = prev - out;
  if (!(dlt & 0xffffff00)) { *inst = DLT_DEL8;   return dlt & 0xff; }
  dlt = out - prev;
  if (!(dlt & 0x00ffffff)) { *inst = DLT_ADDU8;  return (dlt >> 24) & 0xff; }
  dlt = prev - out;
  if (!(dlt & 0x00ffffff)) { *inst = DLT_DELU8;  return (dlt >> 24) & 0xff; }
  // 2 bytes.
  dlt = out - prev;
  if (!(dlt & 0xffff0000)) { *inst = DLT_ADD16;  return dlt & 0xffff; }
  dlt = prev - out;
  if (!(dlt & 0xffff0000)) { *inst = DLT_DEL16;  return dlt & 0xffff; }
  dlt = out - prev;
  if (!(dlt & 0x0000ffff)) { *inst = DLT_ADDU16; return (dlt >> 16) & 0xffff; }
  dlt = prev - out;
  if (!(dlt & 0x0000ffff)) { *inst = DLT_DELU16; return (dlt >> 16) & 0xffff; }
  // 4 bytes.
  *inst = DLT_SET;
  return out;
}

Instruction TizenPlugMessageWriter::AttachInstructionData(int msg, int prev,
                                                          Instruction inst,
                                                          int* s,
                                                          unsigned char* dat) {
  int d = ProcessNextInstruction(msg, prev, &inst);
  if (inst >= DLT_SET) {
    uint32_t v = htonl(d);
    uint8_t* dd = reinterpret_cast<uint8_t*>(&v);
    *(dat + *s + 0) = dd[0];
    *(dat + *s + 1) = dd[1];
    *(dat + *s + 2) = dd[2];
    *(dat + *s + 3) = dd[3];
    *s += 4;
  } else if (inst >= DLT_ADD16) {
      uint16_t v = htons(d);
      uint8_t* dd = reinterpret_cast<uint8_t*>(&v);
      *(dat + *s + 0) = dd[0];
      *(dat + *s + 1) = dd[1];
      *s += 2;
  } else if (inst >= DLT_ADD8) {
      *(dat + *s + 0) = static_cast<uint8_t>(d);
      *s += 1;
  }
  return inst;
}

bool TizenPlugMessageWriter::WriteSafe(const uint8_t* buffer, size_t len) {
  size_t todo = len;
  while (todo) {
     ssize_t r = write(*fd_, buffer, todo);
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

}  // namespace xwalk
