// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/binding_object.h"

namespace xwalk {
namespace sysapps {

BindingObject::BindingObject() : instance_(NULL) {}

BindingObject::~BindingObject() {}

void BindingObject::PostResult(const std::string& callback_id,
                               scoped_ptr<base::ListValue> result) {
  DCHECK(instance_);
  instance_->PostResult(callback_id, result.Pass());
}

}  // namespace sysapps
}  // namespace xwalk
