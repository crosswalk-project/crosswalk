// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/binding_object_store.h"

#include "base/stl_util.h"
#include "xwalk/sysapps/common/common.h"

using namespace xwalk::jsapi::common; // NOLINT

namespace xwalk {
namespace sysapps {

BindingObjectStore::BindingObjectStore(XWalkExtensionFunctionHandler* handler)
    : objects_deleter_(&objects_) {
  handler->Register("JSObjectCollected",
      base::Bind(&BindingObjectStore::OnJSObjectCollected,
                 base::Unretained(this)));
  handler->Register("postMessageToObject",
      base::Bind(&BindingObjectStore::OnPostMessageToObject,
                 base::Unretained(this)));
}

BindingObjectStore::~BindingObjectStore() {}

void BindingObjectStore::AddBindingObject(const std::string& id,
                                          std::unique_ptr<BindingObject> obj) {
  if (base::ContainsKey(objects_, id)) {
    LOG(WARNING) << "The object with the ID " << id << " already exists.";
    return;
  }

  objects_[id] = obj.release();
}

bool BindingObjectStore::HasObjectForTesting(const std::string& id) const {
  return base::ContainsKey(objects_, id);
}

void BindingObjectStore::OnJSObjectCollected(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<DestroyObject::Params>
      params(DestroyObject::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  BindingObjectMap::iterator it = objects_.find(params->object_id);
  if (it == objects_.end()) {
    LOG(WARNING) << "Attempt to destroy inexistent object with the ID "
        << params->object_id;
    return;
  }

  delete it->second;
  objects_.erase(it);
}

void BindingObjectStore::OnPostMessageToObject(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<PostMessageToObject::Params>
      params(PostMessageToObject::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  BindingObjectMap::iterator it = objects_.find(params->object_id);
  if (it == objects_.end())
    return;

  if (!params->arguments->IsType(base::Value::TYPE_LIST)) {
    LOG(WARNING) << "Malformed message sent to the object with the ID "
        << params->object_id << ".";
    return;
  }

  std::unique_ptr<base::ListValue> new_args(
      static_cast<base::ListValue*>(params->arguments.release()));

  std::unique_ptr<XWalkExtensionFunctionInfo> new_info(
      new XWalkExtensionFunctionInfo(
          params->name,
          std::move(new_args),
          info->post_result_cb()));

  if (!it->second->HandleFunction(std::move(new_info))) {
    LOG(WARNING) << "The object with the ID " << params->object_id << " has no "
        "handler for the function " << params->name << ".";
    return;
  }
}

}  // namespace sysapps
}  // namespace xwalk
