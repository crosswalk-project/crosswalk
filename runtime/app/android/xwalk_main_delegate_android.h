// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_APP_ANDROID_XWALK_MAIN_DELEGATE_ANDROID_H_
#define XWALK_RUNTIME_APP_ANDROID_XWALK_MAIN_DELEGATE_ANDROID_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "content/public/app/content_main_delegate.h"
#include "xwalk/runtime/app/xwalk_main_delegate.h"

namespace content {
class BrowserMainRunner;
class ContentClient;
}

namespace xwalk {

class XWalkMainDelegateAndroid : public XWalkMainDelegate {
 public:
  XWalkMainDelegateAndroid();
  virtual ~XWalkMainDelegateAndroid();

  // ContentMainDelegate implementation:
  virtual bool BasicStartupComplete(int* exit_code) OVERRIDE;
  virtual void PreSandboxStartup() OVERRIDE;
  virtual int RunProcess(const std::string& process_type,
      const content::MainFunctionParams& main_function_params) OVERRIDE;

  void InitResourceBundle();

 private:
  scoped_ptr<content::BrowserMainRunner> browser_runner_;
  scoped_ptr<content::ContentClient> content_client_;

  DISALLOW_COPY_AND_ASSIGN(XWalkMainDelegateAndroid);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_APP_ANDROID_XWALK_MAIN_DELEGATE_ANDROID_H_
