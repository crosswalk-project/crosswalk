// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_V8_CONTEXT_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_V8_CONTEXT_H_

#include <set>
#include "base/basictypes.h"
#include "v8/include/v8.h"

namespace WebKit {
class WebFrame;
}

namespace xwalk {
namespace extensions {

// Wrapper for v8 context and frame
class V8Context {
 public:
  V8Context(v8::Handle<v8::Context> context, WebKit::WebFrame* frame)
    : context_(context),
      frame_(frame) {}

  v8::Handle<v8::Context> context() const {
    return context_;
  }

  WebKit::WebFrame* frame() const {
    return frame_;
  }

 private:
  v8::Handle<v8::Context> context_;
  WebKit::WebFrame* frame_;

  DISALLOW_COPY_AND_ASSIGN(V8Context);
};

// Container of V8Context
class V8ContextSet {
 public:
  V8ContextSet() {}
  void Add(V8Context* v8_context);
  void Remove(V8Context* v8_context);
  V8Context* Get(v8::Handle<v8::Context> context) const;

 private:
  typedef std::set<V8Context*> ContextSet;
  ContextSet v8_context_set_;

  DISALLOW_COPY_AND_ASSIGN(V8ContextSet);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_V8_CONTEXT_H_
