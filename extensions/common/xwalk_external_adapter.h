// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTERNAL_ADAPTER_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTERNAL_ADAPTER_H_

#include <map>
#include "base/memory/singleton.h"
#include "xwalk/extensions/public/XW_Extension.h"
#include "xwalk/extensions/public/XW_Extension_SyncMessage.h"
#include "xwalk/extensions/common/xwalk_external_context.h"
#include "xwalk/extensions/common/xwalk_external_extension.h"

// NOTE: Those macros define functions that are used in the structs by
// GetInterface(). They dispatch the function to the appropriate
// extension or instance.

#define DEFINE_FUNCTION_1(TYPE, INTERFACE, NAME, ARG1)          \
  static void INTERFACE ## NAME(XW_ ## TYPE xw, ARG1 arg1) {    \
    Get ## TYPE(xw)->INTERFACE ## NAME(arg1);                   \
  }

#define DEFINE_FUNCTION_2(TYPE, INTERFACE, NAME, ARG1, ARG2)             \
  static void INTERFACE ## NAME(XW_ ## TYPE xw, ARG1 arg1, ARG2 arg2) {  \
    Get ## TYPE(xw)->INTERFACE ## NAME(arg1, arg2);                      \
  }

#define DEFINE_RET_FUNCTION_0(TYPE, INTERFACE, NAME, RET_ARG)   \
  static RET_ARG INTERFACE ## NAME(XW_ ## TYPE xw) {            \
    return Get ## TYPE(xw)->INTERFACE ## NAME();                \
  }

template <typename T> struct DefaultSingletonTraits;

namespace xwalk {
namespace extensions {

class XWalkExternalExtension;
class XWalkExternalContext;

// Provides the "C Interfaces" defined in XW_Extension.h and maps the
// functions from external extension to their implementations in
// XWalkExternalExtension and XWalkExternalContext. We have only one
// adapter per process.
class XWalkExternalAdapter {
 public:
  static XWalkExternalAdapter* GetInstance();

  XW_Extension GetNextXWExtension();
  XW_Instance GetNextXWInstance();

  // This adds the extension to the adapter's mapping, so C calls to
  // its corresponding XW_Extension are correctly dispatched.
  void RegisterExtension(XWalkExternalExtension* extension);
  void UnregisterExtension(XWalkExternalExtension* extension);

  // This adds the context to the adapter's mapping, so C calls to
  // its corresponding XW_Instance are correctly dispatched.
  void RegisterInstance(XWalkExternalContext* context);
  void UnregisterInstance(XWalkExternalContext* context);

  // Returns the correct struct according to interface asked. This is
  // passed to external extensions in XW_Initialize() call.
  static const void* GetInterface(const char* name);

 private:
  friend struct DefaultSingletonTraits<XWalkExternalAdapter>;

  XWalkExternalAdapter();
  ~XWalkExternalAdapter();

  bool IsValidXWExtension(XW_Extension xw_extension);
  bool IsValidXWInstance(XW_Instance xw_instance);

  static XWalkExternalExtension* GetExtension(XW_Extension xw_extension);
  static XWalkExternalContext* GetInstance(XW_Instance xw_instance);

  // XW_CoreInterface_1 from XW_Extension.h.
  DEFINE_FUNCTION_1(Extension, Core, SetExtensionName, const char*);
  DEFINE_FUNCTION_1(Extension, Core, SetJavaScriptAPI, const char*);
  DEFINE_FUNCTION_2(Extension, Core, RegisterInstanceCallbacks,
                    XW_CreatedInstanceCallback, XW_DestroyedInstanceCallback);
  DEFINE_FUNCTION_1(Extension, Core, RegisterShutdownCallback,
                    XW_ShutdownCallback);
  DEFINE_FUNCTION_1(Instance, Core, SetInstanceData, void*);
  DEFINE_RET_FUNCTION_0(Instance, Core, GetInstanceData, void*);

  // XW_MessagingInterface_1 from XW_Extension.h.
  DEFINE_FUNCTION_1(Extension, Messaging, Register, XW_HandleMessageCallback);
  DEFINE_FUNCTION_1(Instance, Messaging, PostMessage, const char*);

  // XW_Internal_SyncMessaging_1 from XW_Extension_SyncMessage.h.
  DEFINE_FUNCTION_1(Extension, SyncMessaging, Register,
                    XW_HandleSyncMessageCallback);
  DEFINE_FUNCTION_1(Instance, SyncMessaging, SetSyncReply, const char*);

  typedef std::map<XW_Extension, XWalkExternalExtension*> ExtensionMap;
  ExtensionMap extension_map_;

  typedef std::map<XW_Instance, XWalkExternalContext*> InstanceMap;
  InstanceMap instance_map_;

  XW_Extension next_xw_extension_;
  XW_Instance next_xw_instance_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExternalAdapter);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTERNAL_ADAPTER_H_
