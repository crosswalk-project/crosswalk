// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_UI_APP_MODAL_DIALOGS_JAVASCRIPT_DIALOG_MANAGER_H_
#define CAMEO_SRC_RUNTIME_BROWSER_UI_APP_MODAL_DIALOGS_JAVASCRIPT_DIALOG_MANAGER_H_

namespace content {
class JavaScriptDialogManager;
}

namespace cameo {
class Runtime;

// Returns a JavaScriptDialogManager that creates real dialogs.
// It returns a Singleton instance of JavaScriptDialogManager,
// which should not be deleted.
content::JavaScriptDialogManager* GetJavaScriptDialogManagerInstance();

// Creates and returns a JavaScriptDialogManager owned by |runtime|.
// This is not the Singleton instance, so the caller must delete it.
content::JavaScriptDialogManager* CreateJavaScriptDialogManagerInstance(
    cameo::Runtime* runtime);

}
#endif  // CAMEO_SRC_RUNTIME_BROWSER_UI_APP_MODAL_DIALOGS_JAVASCRIPT_DIALOG_MANAGER_H_
