// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_HOST_H_
#define CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_HOST_H_

#include <map>
#include <string>
#include "cameo/src/extensions/browser/cameo_extension.h"
#include "content/public/browser/browser_thread.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_sender.h"

namespace cameo {
namespace extensions {

// Manages the set of extensions used by the browser process. Mediates
// the message exchange between extensions and the renderer process.
class CameoExtensionHost : public IPC::ChannelProxy::MessageFilter,
                           public IPC::Sender,
                           public CameoExtension::Poster {
 public:
  CameoExtensionHost();
  virtual ~CameoExtensionHost();

  // Takes |extension| ownership and register it with the renderer
  // process so can be used by JavaScript code. Returns false if
  // extension couldn't be registered because another one with the
  // same name exists, otherwise returns true.
  bool RegisterExtension(CameoExtension* extension);

  // MessageFilter implementation.
  virtual void OnFilterAdded(IPC::Channel* channel) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  // Sender implementation.
  virtual bool Send(IPC::Message* message) OVERRIDE;

  // CameoExtension::Poster implementation.
  virtual void PostMessage(const int32_t render_view_id,
                           const std::string& extension,
                           const std::string& msg);

 private:
  // Called when a message was received from renderer process, handles
  // the dispatching to the right extension.
  void OnPostMessage(const IPC::Message& message);

  struct ExtensionEntry {
    CameoExtension* extension;
    content::BrowserThread::ID thread;
  };
  typedef std::map<std::string, ExtensionEntry> ExtensionMap;
  ExtensionMap extensions_;

  IPC::Channel* channel_;

  // FIXME(cmarcelo): Add peer_handle_ so we can kill process if we
  // get a wrong message.
};

}  // namespace extensions
}  // namespace cameo

#endif  // CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_HOST_H_
