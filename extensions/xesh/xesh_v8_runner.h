// Copyright 2012 the V8 project authors. All rights reserved.
//
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_XESH_XESH_V8_RUNNER_H_
#define XWALK_EXTENSIONS_XESH_XESH_V8_RUNNER_H_

#include <string>
#include "v8/include/v8.h"
#include "base/message_loop/message_loop.h"
#include "base/synchronization/waitable_event.h"
#include "base/task_runner_util.h"
#include "base/threading/thread.h"
#include "ipc/ipc_sync_channel.h"
#include "xwalk/extensions/renderer/xwalk_extension_client.h"

namespace xwalk {
namespace extensions {
class XWalkExtensionClient;
class XWalkModuleSystem;
}
}

using xwalk::extensions::XWalkExtensionClient;
using xwalk::extensions::XWalkModuleSystem;

// Creates and manages the lifetime of the JS side of XWalkExtension's
// Framework and all v8 related entities. That means managing
// XWalkExtensionClient, XWalkModuleSystem and the v8::Context.
// This class will live on the v8 thread.
class XEShV8Runner {
 public:
  XEShV8Runner();

  ~XEShV8Runner();

  void Initialize(int argc, char** argv, base::MessageLoopProxy* io_loop_proxy,
      const IPC::ChannelHandle& handle);
  void Shutdown();

  // Executes a string within the current v8 context.
  std::string ExecuteString(std::string statement);

  static const char* GetV8Version() {
    return v8::V8::GetVersion();
  }

 private:
  v8::Handle<v8::Context> GetV8Context() {
    return v8::Local<v8::Context>::New(v8::Isolate::GetCurrent(), v8_context_);
  }

  void CreateModuleSystem();
  void CreateExtensionModules(XWalkModuleSystem* module_system);
  void RegisterAccessors();
  std::string ReportException(v8::TryCatch* try_catch);

  static void PrintCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void QuitCallback(v8::Local<v8::String> property,
      const v8::PropertyCallbackInfo<v8::Value>& info);

  XWalkExtensionClient client_;
  scoped_ptr<IPC::SyncChannel> client_channel_;
  base::WaitableEvent shutdown_event_;

  v8::Persistent<v8::Context> v8_context_;
};

#endif  // XWALK_EXTENSIONS_XESH_XESH_V8_RUNNER_H_
