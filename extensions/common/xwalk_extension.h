// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_H_

#include <stdint.h>
#include <string>
#include "base/callback_forward.h"
#include "base/callback.h"
#include "base/values.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionWrapper;

// Message exchanging interface to be implemented by Crosswalk extensions. This
// is essentially a factory for Context objects that will handle messages. In
// Crosswalk we create a Context per WebContents. Context objects allow us to
// keep separated state for each execution.
class XWalkExtension {
 public:
  XWalkExtension();
  virtual ~XWalkExtension();

  // Returns the JavaScript API code that will be executed in the
  // renderer process. It allows the extension provide a function or
  // object based interface on top of the message passing.
  virtual const char* GetJavaScriptAPI() = 0;

  // Callback type used by Contexts to send messages. Callbacks of this type
  // will be created by the extension system and handled to Context. The
  // callback will take the ownership of the message.
  typedef base::Callback<void(scoped_ptr<base::Value> msg)> PostMessageCallback;

  class Context {
   public:
    virtual ~Context();
    // Allow to handle messages sent from JavaScript code running in renderer
    // process.
    virtual void HandleMessage(scoped_ptr<base::Value> msg) = 0;

    // Allow to handle synchronous messages sent from JavaScript code. Renderer
    // will block until this function returns.
    virtual scoped_ptr<base::Value> HandleSyncMessage(
        scoped_ptr<base::Value> msg);

   protected:
    explicit Context(const PostMessageCallback& post_message);

    // Function to be used by extensions contexts to post messages back to
    // JavaScript in the renderer process. This function will take the ownership
    // of the message.
    void PostMessage(scoped_ptr<base::Value> msg) {
      post_message_.Run(msg.Pass());
    }

   private:
    PostMessageCallback post_message_;

    DISALLOW_COPY_AND_ASSIGN(Context);
  };

  // Create a Context with the given |post_message| callback.
  virtual Context* CreateContext(const PostMessageCallback& post_message) = 0;

  std::string name() const { return name_; }

 protected:
  void set_name(const std::string& name) { name_ = name; }

 private:
  friend class XWalkExtensionWrapper;
  friend class Context;

  // Name of extension, used for dispatching messages.
  std::string name_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtension);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_H_
