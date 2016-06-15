// Copyright 2012 the V8 project authors. All rights reserved.
//
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "xwalk/extensions/xesh/xesh_v8_runner.h"

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "xwalk/extensions/renderer/xwalk_extension_module.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"
#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"

using xwalk::extensions::XWalkExtensionModule;
using xwalk::extensions::XWalkNativeModule;
using xwalk::extensions::XWalkV8ToolsModule;

namespace {
// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}
}  // namespace

XEShV8Runner::XEShV8Runner()
    : shutdown_event_(false, false) {
}

XEShV8Runner::~XEShV8Runner() {
  shutdown_event_.Signal();
}

void XEShV8Runner::Shutdown() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = GetV8Context();

  context->Exit();
  v8_context_.Reset();
  v8::V8::Dispose();
}

void XEShV8Runner::Initialize(int argc,
                              char** argv,
                              base::SingleThreadTaskRunner* io_task_runner,
                              const IPC::ChannelHandle& handle) {
  client_channel_ =
      IPC::SyncChannel::Create(handle, IPC::Channel::MODE_CLIENT, &client_,
                               io_task_runner, true, &shutdown_event_);

  client_.Initialize(client_channel_.get());

  v8::V8::Initialize();
  v8::V8::InitializeICU();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8_context_.Reset(isolate, v8::Context::New(isolate));
  if (v8_context_.IsEmpty()) {
    fprintf(stderr, "Error creating v8::Context.\n");
    exit(1);
  }

  v8::Local<v8::Context> context = GetV8Context();
  context->Enter();

  CreateModuleSystem();
  RegisterAccessors();
}

std::string XEShV8Runner::ExecuteString(std::string statement) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script = v8::Script::Compile(
      v8::String::NewFromUtf8(isolate, statement.c_str()),
      v8::String::NewFromUtf8(isolate, "(xesh)"));

  if (script.IsEmpty()) {
    // Print errors that happened during compilation.
    return ReportException(&try_catch);
  }

  v8::Handle<v8::Value> result = script->Run();
  if (result.IsEmpty()) {
    // Print errors that happened during execution.
    return ReportException(&try_catch);
  }

  if (!result->IsUndefined()) {
    // If all went well and the result wasn't undefined then print
    // the returned value.
    v8::String::Utf8Value str(result);
    return std::string(ToCString(str));
  }

  return std::string();
}

std::string XEShV8Runner::ReportException(v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
  v8::String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = ToCString(exception);
  v8::Handle<v8::Message> message = try_catch->Message();

  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // return the exception.
    return base::StringPrintf("%s\n", exception_string);
  }

  // Print (filename):(line number): (message).
  v8::String::Utf8Value filename(message->GetScriptResourceName());
  const char* filename_string = ToCString(filename);
  int linenum = message->GetLineNumber();
  std::string result = base::StringPrintf("%s:%i: %s\n", filename_string,
      linenum, exception_string);

  // Print line of source code.
  v8::String::Utf8Value sourceline(message->GetSourceLine());
  const char* sourceline_string = ToCString(sourceline);
  base::StringAppendF(&result, "%s\n", sourceline_string);
  // Print wavy underline (GetUnderline is deprecated).
  int start = message->GetStartColumn();
  for (int i = 0; i < start; i++) {
    base::StringAppendF(&result, " ");
  }
  int end = message->GetEndColumn();
  for (int i = start; i < end; i++) {
    base::StringAppendF(&result, "^");
  }
  base::StringAppendF(&result, "\n");
  v8::String::Utf8Value stack_trace(try_catch->StackTrace());
  if (stack_trace.length() > 0) {
    const char* stack_trace_string = ToCString(stack_trace);
    base::StringAppendF(&result, "%s\n", stack_trace_string);
  }

  return result;
}

void XEShV8Runner::RegisterAccessors() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Context> context = GetV8Context();

  context->Global()->Set(
      v8::String::NewFromUtf8(isolate, "print"),
      v8::FunctionTemplate::New(isolate, PrintCallback)->GetFunction());
  context->Global()->SetAccessor(
      v8::String::NewFromUtf8(isolate, "quit"),
      QuitCallback);
}

// static
void XEShV8Runner::PrintCallback(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  // FIXME(jeez): Convert Object to JSON so we can 'pretty-print' them.
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}

// static
void XEShV8Runner::QuitCallback(v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  fflush(stdout);
  fflush(stderr);
  exit(0);
}

void XEShV8Runner::CreateModuleSystem() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = GetV8Context();

  XWalkModuleSystem* module_system = new XWalkModuleSystem(context);
  XWalkModuleSystem::SetModuleSystemInContext(
      std::unique_ptr<XWalkModuleSystem>(module_system),
      context);

  // FIXME(jeez): Register the 'internal' native module.
  // FIXME(jeez): Register the 'window' module (for setTimeout(), etc).
  module_system->RegisterNativeModule("v8tools",
      std::unique_ptr<XWalkNativeModule>(new XWalkV8ToolsModule));

  CreateExtensionModules(module_system);
  module_system->Initialize();
}

void XEShV8Runner::CreateExtensionModules(XWalkModuleSystem* module_system) {
  const XWalkExtensionClient::ExtensionAPIMap& extensions =
      client_.extension_apis();
  XWalkExtensionClient::ExtensionAPIMap::const_iterator it =
      extensions.begin();
  for (; it != extensions.end(); ++it) {
    XWalkExtensionClient::ExtensionCodePoints* codepoint = it->second;
    if (codepoint->api.empty())
      continue;
    std::unique_ptr<XWalkExtensionModule> module(
        new XWalkExtensionModule(&client_, module_system, it->first,
                                 codepoint->api));
    module_system->RegisterExtensionModule(std::move(module),
                                           codepoint->entry_points);
  }
}
