// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_COMMON_BINDING_OBJECT_STORE_H_
#define XWALK_SYSAPPS_COMMON_BINDING_OBJECT_STORE_H_

#include <map>
#include <string>
#include "base/memory/scoped_ptr.h"
#include "base/stl_util.h"
#include "xwalk/extensions/browser/xwalk_extension_internal.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/sysapps/common/binding_object.h"

namespace xwalk {
namespace sysapps {

using extensions::XWalkExtension;
using extensions::XWalkInternalExtensionInstance;

// This class acts likes a container of objects that have a counterpart in
// the JavaScript context. It handles the dispatching of messages to the
// destination object based on a unique identifier associated to every
// BindingObject. This class owns the BindingObjects it is managing.
class BindingObjectStore : public XWalkInternalExtensionInstance {
 public:
  BindingObjectStore(const XWalkExtension::PostMessageCallback& post_message);
  virtual ~BindingObjectStore();

 protected:
  void AddBindingObject(const std::string& id, scoped_ptr<BindingObject> obj);

 private:
  // This method is evoked every time a JavaScript Binding object is collected
  // by the garbage collector, so we can also destroy the native counterpart.
  void OnDestroyObject(const FunctionInfo& info);
  void OnPostMessageToObject(const FunctionInfo& info);

  typedef std::map<std::string, BindingObject*> BindingObjectMap;
  BindingObjectMap objects_;
  STLValueDeleter<BindingObjectMap> objects_deleter_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_COMMON_BINDING_OBJECT_STORE_H_
