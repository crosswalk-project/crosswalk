// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_NATIVE_FILE_SYSTEM_NATIVE_FILE_SYSTEM_EXTENSION_H_
#define XWALK_EXPERIMENTAL_NATIVE_FILE_SYSTEM_NATIVE_FILE_SYSTEM_EXTENSION_H_

#include <string>

#include "base/values.h"
#include "content/public/browser/render_process_host.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace experimental {

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class NativeFileSystemExtension : public XWalkExtension {
 public:
  explicit NativeFileSystemExtension(content::RenderProcessHost* host);
  virtual ~NativeFileSystemExtension();

  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;

 private:
  content::RenderProcessHost* host_;
};

class NativeFileSystemInstance : public XWalkExtensionInstance {
 public:
  explicit NativeFileSystemInstance(content::RenderProcessHost* host);

  // XWalkExtensionInstance implementation.
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

 private:
  XWalkExtensionFunctionHandler handler_;
  content::RenderProcessHost* host_;
};

class FileSystemChecker
    : public base::RefCountedThreadSafe<FileSystemChecker> {
 public:
  FileSystemChecker(
      int process_id,
      const std::string& path,
      const std::string& root_name,
      const std::string& promise_id,
      XWalkExtensionInstance* instance);
  void DoTask();

 private:
  friend class base::RefCountedThreadSafe<FileSystemChecker>;
  virtual ~FileSystemChecker() {}
  void RegisterFileSystemsAndSendResponse();

  int process_id_;
  std::string path_;
  std::string root_name_;
  std::string promise_id_;
  XWalkExtensionInstance* instance_;

  DISALLOW_COPY_AND_ASSIGN(FileSystemChecker);
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_NATIVE_FILE_SYSTEM_NATIVE_FILE_SYSTEM_EXTENSION_H_
