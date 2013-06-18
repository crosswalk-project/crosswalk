// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_H_
#define CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_H_

#include <stdint.h>
#include <string>

namespace cameo {
namespace extensions {

// Message exchanging interface to be implemented by Cameo
// extensions. Currently the same extension is shared by all the
// RenderViews of the process, distinguished by an id.
class CameoExtension {
 public:
  // Interface used by CameoExtension to post messages, this is
  // implemented by Cameo itself, extensions should not care about
  // this.
  class Poster {
   public:
    virtual void PostMessage(const int32_t render_view_id,
                             const std::string& extension,
                             const std::string& msg) = 0;
   protected:
    virtual ~Poster() {}  // So the client can't delete it.
  };

  // Defines the possible threads can be used to run the handler function.
  enum HandlerThread {
    HANDLER_THREAD_FILE,
    HANDLER_THREAD_UI
  };

  CameoExtension(Poster* poster,
                 const std::string& name,
                 HandlerThread thread);
  virtual ~CameoExtension();

  // Returns the JavaScript API code that will be executed in the
  // renderer process. It allows the extension provide a function or
  // object based interface on top of the message passing.
  virtual const char* GetJavaScriptAPI() = 0;

  // Allow the extension to handle messages sent from JavaScript code
  // running in renderer process, |render_view_id| identifies which
  // RenderView send the message. Extensions should keep track of it
  // to post messages back correctly.
  //
  // Note this function will be called in the thread the extension was
  // registered, see CameoExtensionHost::RegisterExtension().
  virtual void HandleMessage(const int32_t render_view_id,
                             const std::string& msg) = 0;

  // Function to be used by extensions to post messages back to
  // JavaScript in the renderer process. |render_view_id| will be used
  // to route the message to the right RenderView.
  void PostMessage(const int32_t render_view_id, const std::string& msg);

  std::string name() const { return name_; }
  HandlerThread thread() const { return thread_; }

 private:
  Poster* poster_;

  // Name of extension, used for dispatching messages.
  std::string name_;

  // Thread that will be used for handling messages of this extension.
  HandlerThread thread_;
};

}  // namespace extensions
}  // namespace cameo

#endif  // CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_H_
