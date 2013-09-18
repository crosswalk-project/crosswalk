// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTERNAL_INSTANCE_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTERNAL_INSTANCE_H_

#include <string>
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/public/XW_Extension.h"
#include "xwalk/extensions/public/XW_Extension_SyncMessage.h"

namespace xwalk {
namespace extensions {

class XWalkExternalAdapter;
class XWalkExternalExtension;

// XWalkExternalInstance implements the concrete context of execution of an
// external extension.
//
// It works together with XWalkExternalAdapter to handle calls from shared
// library, and with XWalkExternalExtension to get the appropriate
// callbacks. The associated XW_Instance is used to identify this context when
// calling the shared library.
class XWalkExternalInstance : public XWalkExtensionInstance {
 public:
  XWalkExternalInstance(XWalkExternalExtension* extension,
                        XW_Instance xw_instance);
  virtual ~XWalkExternalInstance();

 private:
  friend class XWalkExternalAdapter;

  // XWalkExtensionInstance implementation.
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;
  virtual void HandleSyncMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  // XW_CoreInterface_1 (from XW_Extension.h) implementation.
  void CoreSetInstanceData(void* data);
  void* CoreGetInstanceData();

  // XW_MessagingInterface_1 (from XW_Extension.h) implementation.
  void MessagingPostMessage(const char* msg);

  // XW_Internal_SyncMessagingInterface_1 (from XW_Extension_SyncMessage.h)
  // implementation.
  void SyncMessagingSetSyncReply(const char* reply);

  XW_Instance xw_instance_;
  std::string sync_reply_;
  XWalkExternalExtension* extension_;
  void* instance_data_;
  bool is_handling_sync_msg_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExternalInstance);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTERNAL_INSTANCE_H_
