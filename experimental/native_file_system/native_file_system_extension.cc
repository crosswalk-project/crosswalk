// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/native_file_system/native_file_system_extension.h"

#include <algorithm>
#include <map>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/child_process_security_policy.h"
#include "grit/xwalk_resources.h"
#include "storage/browser/fileapi/file_system_url.h"
#include "storage/browser/fileapi/isolated_context.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/experimental/native_file_system/virtual_root_provider.h"

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
}

void NativeFileSystemInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  base::DictionaryValue* msg_value = NULL;
  if (!msg->GetAsDictionary(&msg_value) || NULL == msg_value) {
    LOG(ERROR) << "Message object should be a dictionary.";
    return;
  }
  std::string promise_id_string;
  if (!msg_value->GetString("_promise_id", &promise_id_string)) {
    LOG(ERROR) << "Invalid promise id.";
    return;
  }
  std::string cmd_string;
  if (!msg_value->GetString("cmd", &cmd_string) ||
      "requestNativeFileSystem" != cmd_string) {
    LOG(ERROR) << "Invalid cmd: " << cmd_string;
    return;
  }
  std::string virtual_root_string;
  if (!msg_value->GetString("data.virtual_root", &virtual_root_string)) {
    LOG(ERROR) << "Invalid virtual root: " << virtual_root_string;
    return;
  }

  std::string upper_virtual_root_string = virtual_root_string;
  std::transform(upper_virtual_root_string.begin(),
      upper_virtual_root_string.end(),
      upper_virtual_root_string.begin(),
      ::toupper);
  std::string real_path =
      VirtualRootProvider::GetInstance()->GetRealPath(
          upper_virtual_root_string);
  if (real_path.empty()) {
    const scoped_ptr<base::DictionaryValue> res(new base::DictionaryValue());
    res->SetString("_promise_id", promise_id_string);
    res->SetString("cmd", "requestNativeFileSystem_ret");
    res->SetBoolean("data.error", true);
    res->SetString("data.errorMessage", "Invalid name of virtual root.");
    std::string msg_string;
    base::JSONWriter::Write(*res, &msg_string);
    PostMessageToJS(scoped_ptr<base::Value>(new base::StringValue(msg_string)));
    return;
  }

  scoped_refptr<FileSystemChecker> checker(
      new FileSystemChecker(host_->GetID(),
                            real_path,
                            virtual_root_string,
                            promise_id_string,
                            this));
  checker->DoTask();
}

void NativeFileSystemInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  base::DictionaryValue* dict;
  std::string command;

  if (!msg->GetAsDictionary(&dict) || !dict->GetString("cmd", &command)) {
    LOG(ERROR) << "Fail to handle command sync message.";
    SendSyncReplyToJS(scoped_ptr<base::Value>(new base::StringValue("")));
    return;
  }

  scoped_ptr<base::Value> result(new base::StringValue(""));
  std::string virtual_root_string = "";
  if ("getRealPath" ==  command &&
      dict->GetString("path", &virtual_root_string)) {
    std::transform(virtual_root_string.begin(),
                   virtual_root_string.end(),
                   virtual_root_string.begin(),
                   ::toupper);
    std::string real_path =
        VirtualRootProvider::GetInstance()->GetRealPath(
            virtual_root_string);
    result.reset(new base::StringValue(real_path));
  } else {
    LOG(ERROR) << command << " ASSERT NOT REACHED.";
  }

  SendSyncReplyToJS(result.Pass());
}

FileSystemChecker::FileSystemChecker(
    int process_id,
    const std::string& path,
    const std::string& root_name,
    const std::string& promise_id,
    XWalkExtensionInstance* instance)
    : process_id_(process_id),
      path_(path),
      root_name_(root_name),
      promise_id_(promise_id),
      instance_(instance) {
}

void FileSystemChecker::DoTask() {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&FileSystemChecker::RegisterFileSystemsAndSendResponse, this));
}

void FileSystemChecker::RegisterFileSystemsAndSendResponse() {
  CHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  storage::IsolatedContext* isolated_context =
      storage::IsolatedContext::GetInstance();
  CHECK(isolated_context);

  std::string filesystem_id = isolated_context->RegisterFileSystemForPath(
      storage::kFileSystemTypeNativeForPlatformApp,
      std::string(),
      base::FilePath::FromUTF8Unsafe(path_),
      &root_name_);

  content::ChildProcessSecurityPolicy* policy =
      content::ChildProcessSecurityPolicy::GetInstance();
  policy->GrantCreateReadWriteFileSystem(process_id_, filesystem_id);

  const scoped_ptr<base::DictionaryValue> res(new base::DictionaryValue());
  res->SetString("_promise_id", promise_id_);
  res->SetString("cmd", "requestNativeFileSystem_ret");
  res->SetBoolean("data.error", false);
  res->SetString("data.file_system_id", filesystem_id);
  std::string msg_string;
  base::JSONWriter::Write(*res, &msg_string);
  instance_->PostMessageToJS(
      scoped_ptr<base::Value>(
          new base::StringValue(msg_string)));
}

}  // namespace experimental
}  // namespace xwalk
