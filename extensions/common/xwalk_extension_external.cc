// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_external.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace extensions {

// TODO(cmarcelo): Remove this entire namespace and its contents when
// we move to new C API.
namespace old {

#define INTERNAL_IMPLEMENTATION
#include "xwalk/extensions/public/xwalk_extension_public.h"
#undef INTERNAL_IMPLEMENTATION

static const int32_t kImplementedAPIVersion = 1;

XWalkExternalExtension::XWalkExternalExtension(
      const base::FilePath& library_path)
      : library_(library_path)
      , wrapped_(0) {
  Initialize();
}

XWalkExternalExtension::XWalkExternalExtension(base::NativeLibrary library)
    : library_(library),
      wrapped_(NULL) {
  Initialize();
}

XWalkExternalExtension::~XWalkExternalExtension() {
  if (wrapped_ && wrapped_->shutdown)
    wrapped_->shutdown(wrapped_);
}

bool XWalkExternalExtension::is_valid() {
  if (!library_.is_valid())
    return false;

  if (!wrapped_)
    return false;
  if (wrapped_->api_version != kImplementedAPIVersion)
    return false;
  if (!wrapped_->get_javascript)
    return false;
  if (!wrapped_->context_create)
    return false;

  return true;
}

const char* XWalkExternalExtension::GetJavaScriptAPI() {
  return wrapped_->get_javascript(wrapped_);
}

XWalkExtensionInstance* XWalkExternalExtension::CreateInstance(
      const XWalkExtension::PostMessageCallback& post_message) {
  CXWalkExtensionContext* context = wrapped_->context_create(wrapped_);
  if (!context)
    return NULL;

  return new XWalkExternalExtensionInstance(this, post_message, context);
}

void XWalkExternalExtension::Initialize() {
  if (!library_.is_valid())
    return;

  typedef CXWalkExtension* (*EntryPoint)(int32_t api_version);
  EntryPoint initialize = reinterpret_cast<EntryPoint>(
        library_.GetFunctionPointer("xwalk_extension_init"));
  if (!initialize)
    return;

  wrapped_ = initialize(kImplementedAPIVersion);
  if (wrapped_)
    set_name(wrapped_->name);
}

XWalkExternalExtensionInstance::XWalkExternalExtensionInstance(
      XWalkExternalExtension* extension,
      const XWalkExtension::PostMessageCallback& post_message,
      CXWalkExtensionContext* context)
      : context_(context) {
  SetPostMessageCallback(post_message);
  context->internal_data = this;
  context->api = GetAPIWrappers();
}

const CXWalkExtensionContextAPI*
      XWalkExternalExtensionInstance::GetAPIWrappers() {
  static const CXWalkExtensionContextAPI api = {
    &PostMessageWrapper,
    &SetSyncReplyWrapper
  };

  return &api;
}

void XWalkExternalExtensionInstance::PostMessageWrapper(
      CXWalkExtensionContext* context, const char* msg) {
  XWalkExternalExtensionInstance* self =
        reinterpret_cast<XWalkExternalExtensionInstance*>(
              context->internal_data);
  self->PostMessage(scoped_ptr<base::Value>(new base::StringValue(msg)));
}

void XWalkExternalExtensionInstance::SetSyncReplyWrapper(
      CXWalkExtensionContext* context, const char* reply) {
  XWalkExternalExtensionInstance* self =
      reinterpret_cast<XWalkExternalExtensionInstance*>(
          context->internal_data);
  self->SetSyncReply(reply);
}

void XWalkExternalExtensionInstance::HandleMessage(
      scoped_ptr<base::Value> msg) {
  std::string string_msg;
  msg->GetAsString(&string_msg);

  if (context_->handle_message)
    context_->handle_message(context_, string_msg.c_str());
}

scoped_ptr<base::Value>
XWalkExternalExtensionInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  if (!context_->handle_sync_message) {
    LOG(WARNING) << "Ignoring sync message sent for external extension "
                 << "which doesn't support it.";
    return scoped_ptr<base::Value>(base::Value::CreateNullValue());
  }
  CHECK(sync_reply_.empty());

  std::string string_msg;
  msg->GetAsString(&string_msg);

  context_->handle_sync_message(context_, string_msg.c_str());
  scoped_ptr<base::Value> reply(new base::StringValue(sync_reply_));
  sync_reply_.clear();

  return reply.Pass();
}

void XWalkExternalExtensionInstance::SetSyncReply(const char* reply) {
  sync_reply_ = reply;
}

XWalkExternalExtensionInstance::~XWalkExternalExtensionInstance() {
  if (context_->destroy)
    context_->destroy(context_);
}

}  // namespace old

}  // namespace extensions
}  // namespace xwalk
