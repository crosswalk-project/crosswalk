// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_v8_context.h"

#include "base/logging.h"

namespace xwalk {
namespace extensions {

void V8ContextSet::Add(V8Context* v8_context) {
  ContextSet::iterator it = v8_context_set_.find(v8_context);
  DCHECK(it == v8_context_set_.end());
  v8_context_set_.insert(v8_context);
}

void V8ContextSet::Remove(V8Context* v8_context) {
  v8_context_set_.erase(v8_context);
}

V8Context* V8ContextSet::Get(v8::Handle<v8::Context> context) const {
  ContextSet::iterator it = v8_context_set_.begin();
  for (; it != v8_context_set_.end(); it++) {
    if ((*it)->context() == context)
      return *it;
  }

  return NULL;
}

}  // namespace extensions
}  // namespace xwalk
