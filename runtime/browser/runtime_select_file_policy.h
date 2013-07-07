// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_SELECT_FILE_POLICY_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_SELECT_FILE_POLICY_H_

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "ui/shell_dialogs/select_file_policy.h"

namespace content {
class WebContents;
}

// A chrome specific SelectFilePolicy that checks local_state(), and will
// display an infobar on the weakly owned |source_contents|.
class RuntimeSelectFilePolicy : public ui::SelectFilePolicy {
 public:
  RuntimeSelectFilePolicy();
  virtual ~RuntimeSelectFilePolicy();

  // Overridden from ui::SelectFilePolicy:
  virtual bool CanOpenSelectFileDialog() OVERRIDE;
  virtual void SelectFileDenied() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(RuntimeSelectFilePolicy);
};

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_SELECT_FILE_POLICY_H_
