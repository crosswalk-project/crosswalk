// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_H_
#define CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_H_

#include <stdint.h>
#include <string>
#include "base/callback_forward.h"

#include "base/callback.h"

namespace cameo {
namespace extensions {

class CameoExtensionWrapper;

// Message exchanging interface to be implemented by Cameo extensions. This
// is essentially a factory for Context objects that will handle messages. In
// Cameo we create a Context per WebContents. Context objects allow us to
// keep separated state for each execution.
class CameoExtension {
 public:
  explicit CameoExtension(const std::string& name);
  virtual ~CameoExtension();

  // Returns the JavaScript API code that will be executed in the
  // renderer process. It allows the extension provide a function or
  // object based interface on top of the message passing.
  virtual const char* GetJavaScriptAPI() = 0;

  // Callback type used by Contexts to send messages. Callbacks of this type
  // will be created by the extension system and handled to Context.
  typedef base::Callback<void(const std::string& msg)> PostMessageCallback;

  class Context {
   public:
    virtual ~Context() {}
    // Allow to handle messages sent from JavaScript code running in renderer
    // process.
    virtual void HandleMessage(const std::string& msg) = 0;

   protected:
    explicit Context(const PostMessageCallback& post_message)
        : post_message_(post_message) {}

    // Function to be used by extensions contexts to post messages back to
    // JavaScript in the renderer process.
    void PostMessage(const std::string& msg) {
      post_message_.Run(msg);
    }

   private:
    PostMessageCallback post_message_;

    DISALLOW_COPY_AND_ASSIGN(Context);
  };

  // Create a Context with the given |post_message| callback.
  virtual Context* CreateContext(const PostMessageCallback& post_message) = 0;

  std::string name() const { return name_; }

 private:
  friend class CameoExtensionWrapper;
  friend class Context;

  // Name of extension, used for dispatching messages.
  std::string name_;

  DISALLOW_COPY_AND_ASSIGN(CameoExtension);
};

}  // namespace extensions
}  // namespace cameo

#endif  // CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_H_
