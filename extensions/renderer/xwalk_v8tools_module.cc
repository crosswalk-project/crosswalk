// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"

#include "base/logging.h"
#include "content/public/renderer/render_view.h"
#include "ipc/ipc_message.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "xwalk/extensions/renderer/xwalk_v8_utils.h"

using content::RenderView;
using blink::WebFrame;
using blink::WebLocalFrame;
using blink::WebView;

namespace xwalk {
namespace extensions {

namespace {

// ================
// forceSetProperty
// ================
void ForceSetPropertyCallback(const v8::FunctionCallbackInfo<v8::Value>& info) {
  // FIXME(cmarcelo): is this the default?
  info.GetReturnValue().SetUndefined();

  if (info.Length() != 3 || !info[0]->IsObject() || !info[1]->IsString()) {
    return;
  }
  info[0].As<v8::Object>()->DefineOwnProperty(
      info.GetIsolate()->GetCurrentContext(),
      info[1].As<v8::String>(), info[2]).FromJust();
}

// ================
// lifecycleTracker
// ================
struct LifecycleTrackerWrapper {
  v8::Global<v8::Object> handle;
  v8::Global<v8::Function> destructor;
};

void LifecycleTrackerCleanup2(
    const v8::WeakCallbackInfo<LifecycleTrackerWrapper>& data) {
  LifecycleTrackerWrapper* wrapper = data.GetParameter();
  if (!wrapper->destructor.IsEmpty()) {
    v8::HandleScope handle_scope(data.GetIsolate());
    v8::Local<v8::Context> context = v8::Context::New(data.GetIsolate());
    v8::Context::Scope scope(context);
    v8::Local<v8::Function> destructor =
        wrapper->destructor.Get(data.GetIsolate());
    CHECK(destructor->IsFunction());
    v8::MicrotasksScope microtasks(
      data.GetIsolate(), v8::MicrotasksScope::kDoNotRunMicrotasks);
    v8::TryCatch try_catch(data.GetIsolate());
    destructor->Call(context->Global(), 0, nullptr);
    if (try_catch.HasCaught()) {
      LOG(WARNING) << "Exception when running LifecycleTracker destructor: "
                   << ExceptionToString(try_catch);
    }
  }
  delete wrapper;
}

void LifecycleTrackerCleanup1(
    const v8::WeakCallbackInfo<LifecycleTrackerWrapper>& data) {
  data.GetParameter()->handle.Reset();
  // Behave like Chromium's extensions::GCCallback and, instead of calling
  // v8::WeakCallbackInfo::SetSecondPassCallback(), run the actual callback
  // function as a main loop task: if we run the code here or as a second-pass
  // callback we are stuck inbetween Blink's GC prologue and epilogue that
  // forbid script execution and crash Crosswalk in debug mode when certain
  // objects (such as `console') are referenced.
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&LifecycleTrackerCleanup2,
                            data));
}

void LifecycleTrackerDestructorGetter(
    v8::Local<v8::Name> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  CHECK(info.Data()->IsExternal());
  LifecycleTrackerWrapper* wrapper = static_cast<LifecycleTrackerWrapper*>(
      info.Data().As<v8::External>()->Value());
  if (wrapper->destructor.IsEmpty()) {
    info.GetReturnValue().Set(v8::Null(info.GetIsolate()));
  } else {
    info.GetReturnValue().Set(wrapper->destructor);
  }
}

void LifecycleTrackerDestructorSetter(
    v8::Local<v8::Name> property,
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<void>& info) {
  CHECK(info.Data()->IsExternal());
  LifecycleTrackerWrapper* wrapper = static_cast<LifecycleTrackerWrapper*>(
      info.Data().As<v8::External>()->Value());
  if (value->IsNull()) {
    // Remove the existing destructor.
    wrapper->destructor.Reset();
  } else if (value->IsFunction()) {
    // Set a new destructor.
    wrapper->destructor.Reset(info.GetIsolate(), value.As<v8::Function>());
  } else {
    // Invalid type, throw an exception.
    info.GetIsolate()->ThrowException(
        v8::Exception::TypeError(v8::String::NewFromUtf8(
            info.GetIsolate(),
            "A lifecycleTracker's destructor must be a function or null.")));
  }
}

void LifecycleTracker(const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(info.GetIsolate());

  v8::Local<v8::Object> tracker_object = v8::Object::New(isolate);
  // By the time the weak callback is called (it is a phantom callback),
  // |tracker| will have been destroyed, so we need a wrapper structure to keep
  // the data we need.
  LifecycleTrackerWrapper* wrapper = new LifecycleTrackerWrapper;
  wrapper->handle.Reset(isolate, tracker_object);
  wrapper->handle.SetWeak(wrapper, LifecycleTrackerCleanup1,
                          v8::WeakCallbackType::kParameter);
  tracker_object->SetAccessor(
      isolate->GetCurrentContext(),
      v8::String::NewFromUtf8(isolate, "destructor"),
      LifecycleTrackerDestructorGetter, LifecycleTrackerDestructorSetter,
      v8::External::New(isolate, wrapper), v8::AccessControl::DEFAULT,
      v8::PropertyAttribute::DontDelete);
  info.GetReturnValue().Set(wrapper->handle);
}

// ===============
// getWindowObject
// ===============
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
  v8::Handle<v8::ObjectTemplate> object_template =
      v8::ObjectTemplate::New(isolate);

  // TODO(cmarcelo): Use Template::Set() function that takes isolate, once we
  // update the Chromium (and V8) version.
  object_template->Set(v8::String::NewFromUtf8(isolate, "forceSetProperty"),
                       v8::FunctionTemplate::New(
                          isolate, ForceSetPropertyCallback));
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
