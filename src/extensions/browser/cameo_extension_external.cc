// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/extensions/browser/cameo_extension_external.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/scoped_native_library.h"
#include "cameo/src/extensions/browser/cameo_extension.h"

namespace cameo {
namespace extensions {

#include "cameo/src/extensions/public/cameo_extension_public.h"

static const int32_t kImplementedAPIVersion = 1;

CameoExternalExtension::CameoExternalExtension(
      const base::FilePath& library_path)
      : CameoExtension::CameoExtension()
      , library_(new base::ScopedNativeLibrary(library_path))
      , wrapped_(0) {
  if (!library_->is_valid())
    return;

  typedef CCameoExtension* (*EntryPoint)(int32_t api_version);
  EntryPoint initialize = reinterpret_cast<EntryPoint>(
        library_->GetFunctionPointer("cameo_extension_init"));
  if (!initialize)
    return;

  wrapped_ = initialize(kImplementedAPIVersion);
  if (wrapped_)
    set_name(wrapped_->name);
}

CameoExternalExtension::~CameoExternalExtension() {
  if (wrapped_->shutdown)
    wrapped_->shutdown(wrapped_);
  delete library_;
}

bool CameoExternalExtension::is_valid() {
  if (!library_)
    return false;
  if (!library_->is_valid())
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

const char* CameoExternalExtension::GetJavaScriptAPI() {
  return wrapped_->get_javascript(wrapped_);
}

CameoExtension::Context* CameoExternalExtension::CreateContext(
      const CameoExtension::PostMessageCallback& post_message) {
  CCameoExtensionContext* context = wrapped_->context_create(wrapped_);
  if (!context)
    return NULL;

  return new ExternalContext(this, post_message, context);
}

CameoExternalExtension::ExternalContext::ExternalContext(
      CameoExternalExtension* extension,
      const CameoExtension::PostMessageCallback& post_message,
      CCameoExtensionContext* context)
      : CameoExtension::Context(post_message)
      , context_(context) {
  context->internal_data = this;
  context->api = GetAPIWrappers();
}

const CCameoExtensionContextAPI*
      CameoExternalExtension::ExternalContext::GetAPIWrappers() {
  static const CCameoExtensionContextAPI api = {
    .post_message = &PostMessageWrapper
  };

  return &api;
}

void CameoExternalExtension::ExternalContext::PostMessageWrapper(
      CCameoExtensionContext* context, const char* msg) {
  CameoExternalExtension::ExternalContext* self =
        reinterpret_cast<CameoExternalExtension::ExternalContext*>(
              context->internal_data);
  self->PostMessage(msg);
}

void CameoExternalExtension::ExternalContext::HandleMessage(
      const std::string& msg) {
  if (context_->handle_message)
    context_->handle_message(context_, msg.c_str());
}

CameoExternalExtension::ExternalContext::~ExternalContext() {
  if (context_->destroy)
    context_->destroy(context_);
}

}  // namespace extensions
}  // namespace cameo
