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
#include "xwalk/extensions/browser/xwalk_extension_internal.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"

using ui::SelectFileDialog;

namespace xwalk {
namespace experimental {

using extensions::XWalkExtension;
using extensions::XWalkInternalExtension;

class DialogExtension : public XWalkInternalExtension,
                        public RuntimeRegistryObserver {
 public:
  explicit DialogExtension(RuntimeRegistry* runtime_registry);
  virtual ~DialogExtension();

  // XWalkExtension implementation.
  virtual const char* GetJavaScriptAPI() OVERRIDE;
  virtual Context* CreateContext(
    const PostMessageCallback& post_message) OVERRIDE;

  // RuntimeRegistryObserver implementation.
  virtual void OnRuntimeAdded(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(Runtime* runtime) OVERRIDE {}
  virtual void OnRuntimeAppIconChanged(Runtime* runtime) OVERRIDE {}

 private:
  friend class DialogContext;

  RuntimeRegistry* runtime_registry_;
  gfx::NativeWindow owning_window_;
};


class DialogContext : public XWalkInternalExtension::InternalContext,
                      public SelectFileDialog::Listener {
 public:
  DialogContext(DialogExtension* extension,
    const XWalkExtension::PostMessageCallback& post_message);

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  // ui::SelectFileDialog::Listener implementation.
  virtual void FileSelected(const base::FilePath& path,
    int index, void* params) OVERRIDE;
  virtual void MultiFilesSelected(
    const std::vector<base::FilePath>& files, void* params) OVERRIDE;

 private:
  void OnShowOpenDialog(const std::string& function_name,
                        const std::string& callback_id, base::ListValue* args);
  void OnShowSaveDialog(const std::string& function_name,
                        const std::string& callback_id, base::ListValue* args);

  DialogExtension* extension_;
  scoped_refptr<SelectFileDialog> dialog_;
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_DIALOG_DIALOG_EXTENSION_H_
