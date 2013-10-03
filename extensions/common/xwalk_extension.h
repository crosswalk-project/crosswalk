// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_H_

#include <string>
#include "base/callback.h"
#include "base/values.h"

namespace xwalk {
namespace extensions {

// Crosswalk Extensions are used to provide native code functionality associated
// with a JS API. The native side and JS side communicate by exchanging
// messages. The message semantics are up to the extension implementer. The
// architecture allows us to run the native side of the extension in a separated
// process.
//
// In Crosswalk we provide a C API for external extensions implemented separated
// out of Crosswalk binary. Those are loaded when application starts. See
// XWalkExternalExtension for it's implementation and XW_Extension.h for the C
// API.

class XWalkExtensionInstance;

// XWalkExtension is a factory class to be implemented by each extension, and
// used to create extension instance objects. It also holds information valid
// for all the instances, like the JavaScript API. See also
// XWalkExtensionInstance.
class XWalkExtension {
 public:
  virtual ~XWalkExtension();

  // Returns the JavaScript API code that will be executed in the render
  // process. It allows the extension provide a function or object based
  // interface on top of the message passing.
  virtual const char* GetJavaScriptAPI() = 0;

  virtual XWalkExtensionInstance* CreateInstance() = 0;

  std::string name() const { return name_; }

 protected:
  XWalkExtension();
  void set_name(const std::string& name) { name_ = name; }

 private:
  // Name of extension, used for dispatching messages.
  std::string name_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtension);
};

// XWalkExtensionInstance represents an instance of a certain extension, which
// is created per ScriptContext created by Crosswalk (which happens for every
// frame loaded).
//
// XWalkExtensionInstance objects allow us to keep separated state for each
// execution.
class XWalkExtensionInstance {
 public:
  virtual ~XWalkExtensionInstance();

  // Allow to handle messages sent from JavaScript code running in renderer
  // process.
  virtual void HandleMessage(scoped_ptr<base::Value> msg) = 0;

  // Allow to handle synchronous messages sent from JavaScript code. Renderer
  // will block until SendSyncReplyToJS() is called with the reply. The reply
  // can be sent after HandleSyncMessage() function returns.
  virtual void HandleSyncMessage(scoped_ptr<base::Value> msg);

  // Callbacks used by extension instance to communicate back to JS. These are
  // set by the extension system. Callbacks will take the ownership of the
  // message.
  typedef base::Callback<void(scoped_ptr<base::Value> msg)> PostMessageCallback;
  typedef base::Callback<void(scoped_ptr<base::Value> msg)>
      SendSyncReplyCallback;

  void SetPostMessageCallback(const PostMessageCallback& callback);
  void SetSendSyncReplyCallback(const SendSyncReplyCallback& callback);

  // Function to be used by extensions Instances to post messages back to
  // JavaScript in the renderer process. This function will take the ownership
  // of the message.
  void PostMessageToJS(scoped_ptr<base::Value> msg) {
    post_message_.Run(msg.Pass());
  }

 protected:
  XWalkExtensionInstance();

  // Unblocks the renderer waiting on a SyncMessage.
  void SendSyncReplyToJS(scoped_ptr<base::Value> reply) {
    send_sync_reply_.Run(reply.Pass());
  }

 private:
  PostMessageCallback post_message_;
  SendSyncReplyCallback send_sync_reply_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionInstance);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_H_
