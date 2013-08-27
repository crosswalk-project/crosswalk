// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_INTERNAL_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_INTERNAL_H_

#include <map>
#include <string>
#include "base/memory/scoped_ptr.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace extensions {

// Base class for writing internal extensions. Internal extensions are the
// extensions shipped by default with Crosswalk and thus, have access to
// internal Chromium data types. This allow the extensions to use message
// serializers generated from IDLs and/or JSON Schema.
class XWalkInternalExtension : public XWalkExtension {
 public:
  XWalkInternalExtension() {}

  virtual XWalkExtensionInstance* CreateInstance(
      const XWalkExtension::PostMessageCallback& post_message) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkInternalExtension);
};

// An internal extension should use a context derived from
// XWalkInternalExtensionInstance.
// The default message handler will dispatch the messages coming from the
// renderer to the handlers registered beforehand. The best practice is to
// register the handlers on the constructor of the context.
class XWalkInternalExtensionInstance : public XWalkExtensionInstance,
                                       public XWalkExtensionFunctionHandler {
 public:
  explicit XWalkInternalExtensionInstance(
      const XWalkExtension::PostMessageCallback& post_message);
  virtual ~XWalkInternalExtensionInstance();

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  // Send the result back and invokes a callback on the renderer. The
  // |callback_id| must be the same as the one got on the function handler.
  // The |result| should be created using the output from Results::Create(),
  // function generated from the IDL describing the JavaScript API. It is a
  // valid optimization not post a result in case the |callback_id| is an
  // empty string, because it won't have any practical effect other than noise
  // at the IPC channel. This can be the case when the user of the JavaScript
  // API omits the callback. If the JavaScript function doesn't take a
  // callback at all, you won't need to call this method.
  void PostResult(const std::string& callback_id,
                  scoped_ptr<base::ListValue> result);

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkInternalExtensionInstance);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_INTERNAL_H_
