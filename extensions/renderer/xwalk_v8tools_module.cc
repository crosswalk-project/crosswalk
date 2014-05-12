// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"

#include "base/logging.h"
#include "content/public/renderer/render_view.h"
#include "ipc/ipc_message.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "xwalk/extensions/renderer/xwalk_v8_utils.h"

using content::RenderView;
using blink::WebFrame;
using blink::WebLocalFrame;
using blink::WebView;

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

void LifecycleTrackerCleanup(
    const v8::WeakCallbackData<v8::Object, v8::Persistent<v8::Object> >& data) {
  v8::Isolate* isolate = data.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Object> tracker = data.GetValue();
  v8::Handle<v8::Value> function =
      tracker->Get(v8::String::NewFromUtf8(isolate, "destructor"));

  if (function.IsEmpty() || !function->IsFunction()) {
    DLOG(WARNING) << "Destructor function not set for LifecycleTracker.";
    data.GetParameter()->Reset();
    delete data.GetParameter();
    return;
  }

  v8::Handle<v8::Context> context = v8::Context::New(isolate);
  blink::WebScopedMicrotaskSuppression suppression;

  v8::TryCatch try_catch;
  v8::Handle<v8::Function>::Cast(function)->Call(context->Global(), 0, NULL);
  if (try_catch.HasCaught())
    LOG(WARNING) << "Exception when running LifecycleTracker destructor: "
        << ExceptionToString(try_catch);

  data.GetParameter()->Reset();
  delete data.GetParameter();
}

void LifecycleTracker(const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::Persistent<v8::Object>* tracker =
      new v8::Persistent<v8::Object>(isolate, v8::Object::New(isolate));
  tracker->SetWeak(tracker, &LifecycleTrackerCleanup);

  info.GetReturnValue().Set(*tracker);
}

RenderView* GetCurrentRenderView() {
  WebLocalFrame* frame = WebLocalFrame::frameForCurrentContext();
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

  // TODO(cmarcelo): Use Template::Set() function that takes isolate, once we
  // update the Chromium (and V8) version.
  object_template->Set(v8::String::NewFromUtf8(isolate, "forceSetProperty"),
                       v8::FunctionTemplate::New(isolate, ForceSetPropertyCallback));
  object_template->Set(v8::String::NewFromUtf8(isolate, "lifecycleTracker"),
                       v8::FunctionTemplate::New(isolate, LifecycleTracker));

  object_template->Set(v8::String::NewFromUtf8(isolate, "getWindowObject"),
                       v8::FunctionTemplate::New(isolate, GetWindowObject));

  object_template_.Reset(isolate, object_template);
}

XWalkV8ToolsModule::~XWalkV8ToolsModule() {
  object_template_.Reset();
}

v8::Handle<v8::Object> XWalkV8ToolsModule::NewInstance() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::EscapableHandleScope handle_scope(isolate);
  v8::Handle<v8::ObjectTemplate> object_template =
      v8::Local<v8::ObjectTemplate>::New(isolate, object_template_);
  return handle_scope.Escape(object_template->NewInstance());
}

}  // namespace extensions
}  // namespace xwalk
