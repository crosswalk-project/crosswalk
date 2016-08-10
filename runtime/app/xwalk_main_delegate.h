// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_APP_XWALK_MAIN_DELEGATE_H_
#define XWALK_RUNTIME_APP_XWALK_MAIN_DELEGATE_H_

#include <memory>
#include <string>

#include "base/compiler_specific.h"
#include "content/public/app/content_main_delegate.h"
#include "xwalk/runtime/common/xwalk_content_client.h"

namespace xwalk {

class XWalkRunner;
class XWalkResourceDelegate;

class XWalkMainDelegate : public content::ContentMainDelegate {
 public:
  XWalkMainDelegate();
  ~XWalkMainDelegate() override;

  // ContentMainDelegate implementation:
  bool BasicStartupComplete(int* exit_code) override;
  void PreSandboxStartup() override;
  void SandboxInitialized(const std::string& process_type) override;
  int RunProcess(const std::string& process_type,
      const content::MainFunctionParams& main_function_params) override;
  void ProcessExiting(const std::string& process_type) override;
#if defined(OS_POSIX) && !defined(OS_ANDROID) && !defined(OS_MACOSX)
  void ZygoteStarting(
      ScopedVector<content::ZygoteForkDelegate>* delegates) override;
#endif
  content::ContentBrowserClient* CreateContentBrowserClient() override;
  content::ContentRendererClient* CreateContentRendererClient() override;

 private:
  void InitializeResourceBundle();

  std::unique_ptr<XWalkRunner> xwalk_runner_;
  std::unique_ptr<content::ContentRendererClient> renderer_client_;
  std::unique_ptr<content::ContentClient> content_client_;
  std::unique_ptr<XWalkResourceDelegate> resource_delegate_;

  DISALLOW_COPY_AND_ASSIGN(XWalkMainDelegate);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_APP_XWALK_MAIN_DELEGATE_H_
