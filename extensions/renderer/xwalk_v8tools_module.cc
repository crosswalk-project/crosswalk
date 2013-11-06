// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"

#include "base/logging.h"
#include "content/public/renderer/render_view.h"
#include "ipc/ipc_message.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "xwalk/extensions/renderer/xwalk_v8_utils.h"

using content::RenderView;
using WebKit::WebFrame;
using WebKit::WebView;

namespace xwalk {
namespace extensions {

namespace {

void ForceSetPropertyCallback(const v8::FunctionCallbackInfo<v8::Value>& info) {
  // FIXME(cmarcelo): is this the default?
  info.GetReturnValue().SetUndefined();

  if (info.Length() != 3 || !info[0]->IsObject() || !info[1]->IsString()) {
    return;
  }
  info[0].As<v8::Object>()->ForceSet(info[1], info[2]);
}

void LifecycleTrackerCleanup(v8::Isolate* isolate,
                             v8::Persistent<v8::Object>* tracker,
                             void*) {
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Object> local_tracker =
      v8::Local<v8::Object>::New(isolate, *tracker);
  v8::Handle<v8::Value> function =
      local_tracker->Get(v8::String::New("destructor"));

  if (function.IsEmpty() || !function->IsFunction()) {
    DLOG(WARNING) << "Destructor function not set for LifecycleTracker.";
    tracker->Dispose();
    return;
  }

  v8::Handle<v8::Context> context = v8::Context::New(isolate);
  WebKit::WebScopedMicrotaskSuppression suppression;

  v8::TryCatch try_catch;
  v8::Handle<v8::Function>::Cast(function)->Call(context->Global(), 0, NULL);
  if (try_catch.HasCaught())
    LOG(WARNING) << "Exception when running LifecycleTracker destructor: "
        << ExceptionToString(try_catch);

  tracker->Dispose();
}

void LifecycleTracker(const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::Persistent<v8::Object> tracker(isolate, v8::Object::New());
  tracker.MakeWeak<void>(NULL, &LifecycleTrackerCleanup);

  info.GetReturnValue().Set(tracker);
}

RenderView* GetCurrentRenderView() {
  WebFrame* frame = WebFrame::frameForCurrentContext();
  DCHECK(frame) << "There should be an active frame here";

  if (!frame)
    return NULL;

  WebView* view = frame->view();
  if (!view)
    return NULL;

  return RenderView::FromWebView(view);
}

// Get the 'window' object according to the given view id, and meanwhile
// set its opener to the current render view.
void GetWindowObject(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() != 1)
    return;

  if (!args[0]->IsInt32())
    return;

  int new_view_id = args[0]->Int32Value();
  if (new_view_id == MSG_ROUTING_NONE)
    return;

  RenderView* cur_view = GetCurrentRenderView();
  if (!cur_view)
    return;

  RenderView* new_view = RenderView::FromRoutingID(new_view_id);
  if (!new_view)
    return;

  WebFrame* opener = cur_view->GetWebView()->mainFrame();
  WebFrame* frame = new_view->GetWebView()->mainFrame();
  frame->setOpener(opener);

  v8::Local<v8::Value> window = frame->mainWorldScriptContext()->Global();
  args.GetReturnValue().Set(window);
}

}  // namespace

XWalkV8ToolsModule::XWalkV8ToolsModule() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::ObjectTemplate> object_template = v8::ObjectTemplate::New();
  object_template->Set("forceSetProperty",
                        v8::FunctionTemplate::New(ForceSetPropertyCallback));
  object_template->Set("lifecycleTracker",
                       v8::FunctionTemplate::New(LifecycleTracker));

  object_template->Set("getWindowObject",
                       v8::FunctionTemplate::New(GetWindowObject));

  object_template_.Reset(isolate, object_template);
}

XWalkV8ToolsModule::~XWalkV8ToolsModule() {
  object_template_.Dispose();
  object_template_.Clear();
}

v8::Handle<v8::Object> XWalkV8ToolsModule::NewInstance() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::ObjectTemplate> object_template =
      v8::Handle<v8::ObjectTemplate>::New(isolate, object_template_);
  return handle_scope.Close(object_template->NewInstance());
}

}  // namespace extensions
}  // namespace xwalk
