// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/native_file_system/native_file_system_extension.h"

#include <memory>
#include <string>

#include "base/callback.h"
#include "base/memory/ptr_util.h"
#include "base/values.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/child_process_security_policy.h"
#include "grit/xwalk_resources.h"
#include "storage/browser/fileapi/file_system_url.h"
#include "storage/browser/fileapi/isolated_context.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/experimental/native_file_system/native_file_system.h"
#include "xwalk/experimental/native_file_system/virtual_root_provider.h"

using namespace xwalk::jsapi::native_file_system;

namespace {

std::unique_ptr<base::StringValue> GetRealPath(std::unique_ptr<base::Value> msg) {
  base::DictionaryValue* dict;
  std::string virtual_root;
  if (!msg->GetAsDictionary(&dict) || !dict->GetString("path", &virtual_root)) {
    LOG(ERROR) << "Malformed getRealPath request.";
    return base::WrapUnique(new base::StringValue(std::string()));
  }

  const std::string real_path =
      VirtualRootProvider::GetInstance()->GetRealPath(virtual_root);
  return base::WrapUnique(new base::StringValue(real_path));
}

void RegisterFileSystemAndSendResponse(
    int process_id,
    const std::string& path,
    const std::string& root_name,
    std::unique_ptr<xwalk::experimental::XWalkExtensionFunctionInfo> info) {
  CHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  storage::IsolatedContext* isolated_context =
      storage::IsolatedContext::GetInstance();
  CHECK(isolated_context);

  const std::string filesystem_id = isolated_context->RegisterFileSystemForPath(
      storage::kFileSystemTypeNativeForPlatformApp, std::string(),
      base::FilePath::FromUTF8Unsafe(path),
      const_cast<std::string*>(&root_name));

  content::ChildProcessSecurityPolicy* policy =
      content::ChildProcessSecurityPolicy::GetInstance();
  policy->GrantCreateReadWriteFileSystem(process_id, filesystem_id);

  info->PostResult(
      RequestNativeFileSystem::Results::Create(filesystem_id, std::string()));
}

}  // namespace

namespace xwalk {
namespace experimental {

NativeFileSystemExtension::NativeFileSystemExtension(
    content::RenderProcessHost* host) {
  host_ = host;
  set_name("xwalk.experimental.native_file_system");
  set_javascript_api(
      ResourceBundle::GetSharedInstance().GetRawDataResource(
          IDR_XWALK_NATIVE_FILE_SYSTEM_API).as_string());
}

NativeFileSystemExtension::~NativeFileSystemExtension() {}

XWalkExtensionInstance* NativeFileSystemExtension::CreateInstance() {
  return new NativeFileSystemInstance(host_);
}

NativeFileSystemInstance::NativeFileSystemInstance(
    content::RenderProcessHost* host)
    : handler_(this),
      host_(host) {
  handler_.Register(
      "requestNativeFileSystem",
      base::Bind(&NativeFileSystemInstance::OnRequestNativeFileSystem,
                 base::Unretained(this)));
}

void NativeFileSystemInstance::HandleMessage(std::unique_ptr<base::Value> msg) {
  handler_.HandleMessage(std::move(msg));
}

void NativeFileSystemInstance::OnRequestNativeFileSystem(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<RequestNativeFileSystem::Params> params(
      RequestNativeFileSystem::Params::Create(*info->arguments()));
  if (!params) {
    LOG(ERROR) << "Malformed parameters passed to " << info->name();
    return;
  }

  const std::string real_path =
      VirtualRootProvider::GetInstance()->GetRealPath(params->path);

  if (real_path.empty()) {
    info->PostResult(RequestNativeFileSystem::Results::Create(
        std::string(), "Invalid virtual root name."));
  } else {
    content::BrowserThread::PostTask(
        content::BrowserThread::UI, FROM_HERE,
        base::Bind(&RegisterFileSystemAndSendResponse, host_->GetID(),
                   real_path, params->path, base::Passed(&info)));
  }
}

void NativeFileSystemInstance::HandleSyncMessage(std::unique_ptr<base::Value> msg) {
  base::DictionaryValue* dict;
  std::string command;

  if (!msg->GetAsDictionary(&dict) || !dict->GetString("command", &command)) {
    LOG(ERROR) << "Fail to handle sync message.";
    SendSyncReplyToJS(std::unique_ptr<base::Value>(new base::StringValue("")));
    return;
  }

  std::unique_ptr<base::Value> result(new base::StringValue(""));
  if (command == "getRealPath") {
    result = GetRealPath(std::move(msg));
  } else {
    LOG(ERROR) << "Unknown command '" << command << "'";
  }
  SendSyncReplyToJS(std::move(result));
}

}  // namespace experimental
}  // namespace xwalk
