// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_DIALOG_DIALOG_EXTENSION_H_
#define XWALK_EXPERIMENTAL_DIALOG_DIALOG_EXTENSION_H_

#include <string>
#include <vector>
#include "base/files/file_path.h"
#include "base/values.h"
#include "ui/shell_dialogs/select_file_dialog.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"

using ui::SelectFileDialog;

namespace xwalk {
namespace experimental {

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class DialogExtension : public XWalkExtension,
                        public RuntimeRegistryObserver {
 public:
  explicit DialogExtension(RuntimeRegistry* runtime_registry);
  virtual ~DialogExtension();

  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;

  // RuntimeRegistryObserver implementation.
  virtual void OnRuntimeAdded(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(Runtime* runtime) OVERRIDE {}
  virtual void OnRuntimeAppIconChanged(Runtime* runtime) OVERRIDE {}

 private:
  friend class DialogInstance;

  RuntimeRegistry* runtime_registry_;
  gfx::NativeWindow owning_window_;
};


class DialogInstance : public XWalkExtensionInstance,
                      public SelectFileDialog::Listener {
 public:
  explicit DialogInstance(DialogExtension* extension);
  virtual ~DialogInstance();

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  // ui::SelectFileDialog::Listener implementation.
  virtual void FileSelected(const base::FilePath& path,
    int index, void* params) OVERRIDE;
  virtual void MultiFilesSelected(
    const std::vector<base::FilePath>& files, void* params) OVERRIDE;

 private:
  void OnShowOpenDialog(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnShowSaveDialog(scoped_ptr<XWalkExtensionFunctionInfo> info);

  DialogExtension* extension_;
  scoped_refptr<SelectFileDialog> dialog_;

  XWalkExtensionFunctionHandler handler_;
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_DIALOG_DIALOG_EXTENSION_H_
