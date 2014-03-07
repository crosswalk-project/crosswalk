// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/xwalk/adb_impl.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/json/string_escape.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/waitable_event.h"
#include "base/time/time.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/net/adb_client_socket.h"

namespace {

// This class is bound in the callback to AdbQuery and isn't freed until the
// callback is run, even if the function that creates the buffer times out.
class ResponseBuffer : public base::RefCountedThreadSafe<ResponseBuffer> {
 public:
  ResponseBuffer() : ready_(true, false) {}

  void OnResponse(int result, const std::string& response) {
    // TODO(Peter Wang): Need to implement for android xwalk.
    (void) response;
    (void) result;
  }

  Status GetResponse(
      std::string* response, const base::TimeDelta& timeout) {
    // TODO(Peter Wang): Need to implement for android xwalk.
    (void) response;
    (void) timeout;

    return Status(kOk);
  }

 private:
  friend class base::RefCountedThreadSafe<ResponseBuffer>;
  ~ResponseBuffer() {}

  std::string response_;
  int result_;
  base::WaitableEvent ready_;
};

void ExecuteCommandOnIOThread(
    const std::string& command, scoped_refptr<ResponseBuffer> response_buffer,
    int port) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) command;
  (void) response_buffer;
  (void) port;
}

}  // namespace

AdbImpl::AdbImpl(
    const scoped_refptr<base::SingleThreadTaskRunner>& io_task_runner,
    int port)
    : io_task_runner_(io_task_runner), port_(port) {
  // TODO(Peter Wang): Need to implement for android xwalk.
}

AdbImpl::~AdbImpl() {}

Status AdbImpl::GetDevices(std::vector<std::string>* devices) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) devices;

  return Status(kOk);
}

Status AdbImpl::ForwardPort(
    const std::string& device_serial, int local_port,
    const std::string& remote_abstract) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) local_port;
  (void) remote_abstract;

  return Status(kOk);
}

Status AdbImpl::SetCommandLineFile(const std::string& device_serial,
                                   const std::string& command_line_file,
                                   const std::string& exec_name,
                                   const std::string& args) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) device_serial;
  (void) command_line_file;
  (void) exec_name;
  (void) args;

  return Status(kOk);
}

Status AdbImpl::CheckAppInstalled(
    const std::string& device_serial, const std::string& package) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) device_serial;
  (void) package;

  return Status(kOk);
}

Status AdbImpl::ClearAppData(
    const std::string& device_serial, const std::string& package) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) device_serial;
  (void) package;

  return Status(kOk);
}

Status AdbImpl::Launch(
    const std::string& device_serial, const std::string& package,
    const std::string& activity) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) device_serial;
  (void) package;
  (void) activity;

  return Status(kOk);
}

Status AdbImpl::ForceStop(
    const std::string& device_serial, const std::string& package) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) device_serial;
  (void) package;

  return Status(kOk);
}

Status AdbImpl::GetPidByName(const std::string& device_serial,
                             const std::string& process_name,
                             int* pid) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) device_serial;
  (void) process_name;
  (void) pid;

  return Status(kOk);
}

Status AdbImpl::ExecuteCommand(
    const std::string& command, std::string* response) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) command;
  (void) response;

  return Status(kOk);
}

Status AdbImpl::ExecuteHostCommand(
    const std::string& device_serial,
    const std::string& host_command, std::string* response) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) device_serial;
  (void) host_command;
  (void) response;

  return Status(kOk);
}

Status AdbImpl::ExecuteHostShellCommand(
    const std::string& device_serial,
    const std::string& shell_command,
    std::string* response) {
  // TODO(Peter Wang): Need to implement for android xwalk.
  (void) device_serial;
  (void) shell_command;
  (void) response;

  return Status(kOk);
}

