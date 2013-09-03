// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/binding_object_store.h"

#include "xwalk/jsapi/sysapps_common.h"

using namespace xwalk::jsapi::sysapps_common; // NOLINT

namespace xwalk {
namespace sysapps {

BindingObjectStore::BindingObjectStore(
    const XWalkExtension::PostMessageCallback& post_message)
    : XWalkInternalExtensionInstance(post_message),
      objects_deleter_(&objects_) {
  RegisterFunction("destroyObject",
                   &BindingObjectStore::OnDestroyObject);
  RegisterFunction("postMessageToObject",
                   &BindingObjectStore::OnPostMessageToObject);
}

BindingObjectStore::~BindingObjectStore() {}

void BindingObjectStore::AddBindingObject(const std::string& id,
                                          scoped_ptr<BindingObject> obj) {
  if(objects_.find(id) != objects_.end()) {
    LOG(WARNING) << "The object with the ID " << id << " already exists.";
    return;
  }

  objects_[id] = obj.release();
}

void BindingObjectStore::OnDestroyObject(const FunctionInfo& info) {
  scoped_ptr<DestroyObject::Params>
      params(DestroyObject::Params::Create(*info.arguments));

  BindingObjectMap::iterator it = objects_.find(params->object_id);
  if (it == objects_.end()) {
    LOG(WARNING) << "Attempt to destroy inexistent object with the ID "
        << params->object_id;
    return;
  }

  delete it->second;
  objects_.erase(it);
}

void BindingObjectStore::OnPostMessageToObject(const FunctionInfo& info) {
  scoped_ptr<PostMessageToObject::Params>
      params(PostMessageToObject::Params::Create(*info.arguments));

  BindingObjectMap::iterator it = objects_.find(params->object_id);
  if (it == objects_.end())
    return;

  FunctionInfo new_info;
  params->arguments->GetAsList(&new_info.arguments);

  if (!new_info.arguments) {
    LOG(WARNING) << "Malformed message sent to the object with the ID "
        << params->object_id << ".";
    return;
  }

  new_info.name = params->name;
  new_info.callback_id = info.callback_id;

  if (!it->second->HandleFunction(new_info)) {
    LOG(WARNING) << "The object with the ID " << params->object_id << " has no "
        "handler for the function " << params->name << ".";
    return;
  }
}

}  // namespace sysapps
}  // namespace xwalk
