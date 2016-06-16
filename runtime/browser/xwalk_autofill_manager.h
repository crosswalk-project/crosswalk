// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_RUNTIME_BROWSER_XWALK_AUTOFILL_MANAGER_H_
#define XWALK_RUNTIME_BROWSER_XWALK_AUTOFILL_MANAGER_H_

#include <memory>

#include "components/prefs/pref_change_registrar.h"

namespace content {
class WebContents;
}

namespace xwalk {

class XWalkContent;

// This class is responsible for creating all the necessary infrastructure
// for the autofill and saving the forms data. On desktop it is owned by
// the Runtime class and on Android by XWalkContent.
class XWalkAutofillManager {
 public:
  explicit XWalkAutofillManager(content::WebContents* web_contents);
  ~XWalkAutofillManager();
  void InitAutofillIfNecessary(bool enabled);
  void CreateUserPrefServiceIfNecessary();

 protected:
  // The WebContents owned by this runtime.
  content::WebContents* web_contents_;

  PrefChangeRegistrar pref_change_registrar_;

 private:
  void UpdateRendererPreferences();

  DISALLOW_COPY_AND_ASSIGN(XWalkAutofillManager);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_AUTOFILL_MANAGER_H_
