// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTERNAL_EXTENSION_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTERNAL_EXTENSION_H_

#include <string>
#include "base/scoped_native_library.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/public/XW_Extension.h"
#include "xwalk/extensions/public/XW_Extension_SyncMessage.h"

namespace base {
class FilePath;
};

namespace xwalk {
namespace extensions {

class XWalkExternalAdapter;
class XWalkExternalInstance;

// XWalkExternalExtension implements an XWalkExtension backed by a shared
// library implemented using our C ABI (see XW_Extension.h).
//
// It works together with XWalkExternalAdapter to handle calls from shared
// library, and store the callbacks to call it back later. The associated
// XW_Extension is used to identify this extension when calling the shared
// library.
class XWalkExternalExtension : public XWalkExtension {
 public:
  explicit XWalkExternalExtension(const base::FilePath& path);

  virtual ~XWalkExternalExtension();

  bool is_valid();

 private:
  friend class XWalkExternalAdapter;
  friend class XWalkExternalInstance;

  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;

  // XW_CoreInterface_1 (from XW_Extension.h) implementation.
  void CoreSetExtensionName(const char* name);
  void CoreSetJavaScriptAPI(const char* js_api);
  void CoreRegisterInstanceCallbacks(
      XW_CreatedInstanceCallback created_callback,
      XW_DestroyedInstanceCallback destroyed_callback);
  void CoreRegisterShutdownCallback(XW_ShutdownCallback callback);
  void EntryPointsSetExtraJSEntryPoints(const char** entry_points);

  bool PermissionsCheckAPIAccessControl(const char* app_id,
                                        const char* api_name);

  // XW_MessagingInterface_1 (from XW_Extension.h) implementation.
  void MessagingRegister(XW_HandleMessageCallback callback);

  // XW_Internal_SyncMessagingInterface_1 (from XW_Extension.h) implementation.
  void SyncMessagingRegister(XW_HandleSyncMessageCallback callback);

  base::ScopedNativeLibrary library_;
  XW_Extension xw_extension_;

  XW_CreatedInstanceCallback created_instance_callback_;
  XW_DestroyedInstanceCallback destroyed_instance_callback_;
  XW_ShutdownCallback shutdown_callback_;
  XW_HandleMessageCallback handle_msg_callback_;
  XW_HandleSyncMessageCallback handle_sync_msg_callback_;

  bool initialized_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExternalExtension);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTERNAL_EXTENSION_H_
