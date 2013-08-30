// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_service.h"

#include "base/callback.h"
#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/singleton.h"
#include "base/scoped_native_library.h"
#include "base/strings/string_util.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/extensions/browser/xwalk_extension_web_contents_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_external.h"
#include "xwalk/extensions/common/xwalk_external_extension.h"
#include "content/public/browser/render_process_host.h"

namespace xwalk {
namespace extensions {

namespace {

XWalkExtensionService::RegisterExtensionsCallback
g_register_extensions_callback;

}

XWalkExtensionService::XWalkExtensionService(RuntimeRegistry* runtime_registry)
    : runtime_registry_(runtime_registry),
      render_process_host_(NULL) {
  // FIXME(cmarcelo): Once we update Chromium code, replace RuntimeRegistry
  // dependency with callbacks to track WebContents, since we currently don't
  // depend on xwalk::Runtime features.
  runtime_registry_->AddObserver(this);

  if (!g_register_extensions_callback.is_null())
    g_register_extensions_callback.Run(this);
}

XWalkExtensionService::~XWalkExtensionService() {
  runtime_registry_->RemoveObserver(this);
}

namespace {

bool ValidateExtensionName(const std::string& extension_name) {
  bool dot_allowed = false;
  bool digit_or_underscore_allowed = false;
  for (size_t i = 0; i < extension_name.size(); ++i) {
    char c = extension_name[i];
    if (IsAsciiDigit(c)) {
      if (!digit_or_underscore_allowed)
        return false;
    } else if (c == '_') {
      if (!digit_or_underscore_allowed)
        return false;
    } else if (c == '.') {
      if (!dot_allowed)
        return false;
      dot_allowed = false;
      digit_or_underscore_allowed = false;
    } else if (IsAsciiAlpha(c)) {
      dot_allowed = true;
      digit_or_underscore_allowed = true;
    } else {
      return false;
    }
  }

  // If after going through the entire name we finish with dot_allowed, it means
  // the previous character is not a dot, so it's a valid name.
  return dot_allowed;
}

}  // namespace

bool XWalkExtensionService::RegisterExtension(
    scoped_ptr<XWalkExtension> extension) {
  // Note: for now we only support registering new extensions before
  // render process hosts were created.
  CHECK(!render_process_host_);

  if (!ValidateExtensionName(extension->name())) {
    LOG(WARNING) << "Ignoring extension with invalid name: "
                 << extension->name();
    return false;
  }

  base::AutoLock lock(in_process_extensions_lock_);
  return in_process_extensions_.RegisterExtension(extension.Pass());
}

void XWalkExtensionService::RegisterExternalExtensionsForPath(
    const base::FilePath& path) {
  CHECK(file_util::DirectoryExists(path));

  // FIXME(leandro): Use GetNativeLibraryName() to obtain the proper
  // extension for the current platform.
  const base::FilePath::StringType pattern = FILE_PATH_LITERAL("*.so");
  base::FileEnumerator libraries(
      path, false, base::FileEnumerator::FILES, pattern);

  for (base::FilePath extension_path = libraries.Next();
        !extension_path.empty(); extension_path = libraries.Next()) {
    // FIXME(cmarcelo): Once we get rid of the current C API in favor of the new
    // one, move this NativeLibrary manipulation back inside
    // XWalkExternalExtension.
    base::ScopedNativeLibrary library(extension_path);
    if (!library.is_valid()) {
      LOG(WARNING) << "Ignoring " << extension_path.AsUTF8Unsafe()
                   << " as external extension because is not valid library.";
      continue;
    }

    if (library.GetFunctionPointer("XW_Initialize")) {
      scoped_ptr<XWalkExternalExtension> extension(
          new XWalkExternalExtension(extension_path, library.Release()));
      if (extension->is_valid())
        RegisterExtension(extension.PassAs<XWalkExtension>());
    } else if (library.GetFunctionPointer("xwalk_extension_init")) {
      scoped_ptr<old::XWalkExternalExtension> extension(
          new old::XWalkExternalExtension(library.Release()));
      if (extension->is_valid())
        RegisterExtension(extension.PassAs<XWalkExtension>());
    } else {
      LOG(WARNING) << "Ignoring " << extension_path.AsUTF8Unsafe()
                   << " as external extension because"
                   << " doesn't contain valid entry point.";
    }
  }
}

void XWalkExtensionService::OnRenderProcessHostCreated(
    content::RenderProcessHost* host) {
  // FIXME(cmarcelo): For now we support only one render process host.
  if (render_process_host_)
    return;

  render_process_host_ = host;
  RegisterExtensionsForNewHost(render_process_host_);

  // Attach extensions to already existing runtimes. Related the conditional in
  // OnRuntimeAdded.
  const RuntimeList& runtimes = RuntimeRegistry::Get()->runtimes();
  for (size_t i = 0; i < runtimes.size(); i++)
    CreateWebContentsHandler(render_process_host_, runtimes[i]->web_contents());
}

void XWalkExtensionService::CreateRunnersForHandler(
    XWalkExtensionWebContentsHandler* handler, int64_t frame_id) {
  // FIXME(tmpsantos) The main reason why we need this lock here is
  // because this object lives in the UI Thread but this particular
  // method is called from the IO Thread (the MessageFilter calls
  // the WebContentsHandler that ultimately calls this method). This
  // code path should be clarified.
  base::AutoLock lock(in_process_extensions_lock_);
  in_process_extensions_.CreateRunnersForHandler(handler, frame_id);
}

void XWalkExtensionService::OnRuntimeAdded(Runtime* runtime) {
  if (render_process_host_)
    CreateWebContentsHandler(render_process_host_, runtime->web_contents());
}

void XWalkExtensionService::OnRuntimeRemoved(Runtime* runtime) {
  XWalkExtensionWebContentsHandler* handler =
      XWalkExtensionWebContentsHandler::FromWebContents(
          runtime->web_contents());
  handler->ClearMessageFilter();
}

// static
void XWalkExtensionService::SetRegisterExtensionsCallbackForTesting(
    const RegisterExtensionsCallback& callback) {
  g_register_extensions_callback = callback;
}

void XWalkExtensionService::RegisterExtensionsForNewHost(
    content::RenderProcessHost* host) {
  base::AutoLock lock(in_process_extensions_lock_);
  in_process_extensions_.RegisterExtensionsForNewHost(host);
}

void XWalkExtensionService::CreateWebContentsHandler(
    content::RenderProcessHost* host, content::WebContents* web_contents) {
  XWalkExtensionWebContentsHandler::CreateForWebContents(web_contents);
  XWalkExtensionWebContentsHandler* handler =
      XWalkExtensionWebContentsHandler::FromWebContents(web_contents);
  handler->set_extension_service(this);
  handler->set_render_process_host(host);
}

bool ValidateExtensionNameForTesting(const std::string& extension_name) {
  return ValidateExtensionName(extension_name);
}

}  // namespace extensions
}  // namespace xwalk
