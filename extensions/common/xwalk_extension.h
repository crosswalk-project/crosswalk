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
class XWalkExtensionInstance;

// Message exchanging interface to be implemented by Crosswalk extensions. This
// is essentially a factory for XWalkExtensionInstance objects that will handle
// messages.
class XWalkExtension {
 public:
  XWalkExtension();
  virtual ~XWalkExtension();

  // Returns the JavaScript API code that will be executed in the
  // renderer process. It allows the extension provide a function or
  // object based interface on top of the message passing.
  virtual const char* GetJavaScriptAPI() = 0;

  // Callback type used by Instances to send messages. Callbacks of this type
  // will be created by the extension system and handled to
  // XWalkExtensionInstance. Callback will take the ownership of the message.
  typedef base::Callback<void(scoped_ptr<base::Value> msg)> PostMessageCallback;

  typedef base::Callback<void(scoped_ptr<base::Value> msg)>
      SendSyncReplyCallback;

  // Create an XWalkExtensionInstance with the given |post_message| callback.
  virtual XWalkExtensionInstance* CreateInstance() = 0;

  std::string name() const { return name_; }

 protected:
  void set_name(const std::string& name) { name_ = name; }

 private:
  friend class XWalkExtensionWrapper;
  friend class XWalkExtensionInstance;

  // Name of extension, used for dispatching messages.
  std::string name_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtension);
};

// XWalkExtensionInstance represents a C++ instance of a certain extension,
// which is created per Frame/ScriptContext per WebContents.
// XWalkExtensionInstance objects allow us to keep separated state for each
// execution.
class XWalkExtensionInstance {
 public:
  virtual ~XWalkExtensionInstance();
  // Allow to handle messages sent from JavaScript code running in renderer
  // process.
  virtual void HandleMessage(scoped_ptr<base::Value> msg) = 0;

  // Allow to handle synchronous messages sent from JavaScript code. Renderer
  // will block until SendSyncReplyToJS is called with the reply. Note that
  // it may be called from "outside" of the HandleSyncMessage call.
  virtual void HandleSyncMessage(scoped_ptr<base::Value> msg);

  void SetPostMessageCallback(
      const XWalkExtension::PostMessageCallback& post_message);

  void SetSendSyncReplyCallback(
      const XWalkExtension::SendSyncReplyCallback& callback);

 protected:
  explicit XWalkExtensionInstance();

  // Function to be used by extensions Instances to post messages back to
  // JavaScript in the renderer process. This function will take the ownership
  // of the message.
  void PostMessageToJS(scoped_ptr<base::Value> msg) {
    post_message_.Run(msg.Pass());
  }

  // Unblocks the renderer waiting on a SyncMessage.
  void SendSyncReplyToJS(scoped_ptr<base::Value> reply) {
    send_sync_reply_.Run(reply.Pass());
  }

 private:
  XWalkExtension::PostMessageCallback post_message_;
  XWalkExtension::SendSyncReplyCallback send_sync_reply_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionInstance);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_H_
