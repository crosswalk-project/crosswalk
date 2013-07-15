// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_external.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace extensions {

#define INTERNAL_IMPLEMENTATION
#include "xwalk/extensions/public/xwalk_extension_public.h"
#undef INTERNAL_IMPLEMENTATION

static const int32_t kImplementedAPIVersion = 1;

XWalkExternalExtension::XWalkExternalExtension(
      const base::FilePath& library_path)
      : library_(library_path)
      , wrapped_(0) {
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

XWalkExtension::Context* XWalkExternalExtension::CreateContext(
      const XWalkExtension::PostMessageCallback& post_message) {
  CXWalkExtensionContext* context = wrapped_->context_create(wrapped_);
  if (!context)
    return NULL;

  return new ExternalContext(this, post_message, context);
}

XWalkExternalExtension::ExternalContext::ExternalContext(
      XWalkExternalExtension* extension,
      const XWalkExtension::PostMessageCallback& post_message,
      CXWalkExtensionContext* context)
      : XWalkExtension::Context(post_message)
      , context_(context) {
  context->internal_data = this;
  context->api = GetAPIWrappers();
}

const CXWalkExtensionContextAPI*
      XWalkExternalExtension::ExternalContext::GetAPIWrappers() {
  static const CXWalkExtensionContextAPI api = {
    &PostMessageWrapper
  };

  return &api;
}

void XWalkExternalExtension::ExternalContext::PostMessageWrapper(
      CXWalkExtensionContext* context, const char* msg) {
  XWalkExternalExtension::ExternalContext* self =
        reinterpret_cast<XWalkExternalExtension::ExternalContext*>(
              context->internal_data);
  self->PostMessage(scoped_ptr<base::Value>(new base::StringValue(msg)));
}

void XWalkExternalExtension::ExternalContext::HandleMessage(
      scoped_ptr<base::Value> msg) {
  std::string string_msg;
  msg->GetAsString(&string_msg);

  if (context_->handle_message)
    context_->handle_message(context_, string_msg.c_str());
}

XWalkExternalExtension::ExternalContext::~ExternalContext() {
  if (context_->destroy)
    context_->destroy(context_);
}

}  // namespace extensions
}  // namespace xwalk
