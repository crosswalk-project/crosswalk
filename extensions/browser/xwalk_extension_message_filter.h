// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_MESSAGE_FILTER_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_MESSAGE_FILTER_H_

#include <stdint.h>
#include <string>
#include "content/public/browser/browser_message_filter.h"
#include "base/basictypes.h"
#include "base/values.h"

using content::BrowserThread;

namespace xwalk {
namespace extensions {

class XWalkExtensionRunner;
class XWalkExtensionRunnerStore;
class XWalkExtensionWebContentsHandler;

// The ExtensionMessageFilter will handle all messages directed to a particular
// WebContent in the IO Thread leaving the UI Thread undisturbed. The filter is
// owned by the Channel it is filtering, but it can be explicitly removed from
// the channel by the WebContentsHandler that created the filter.
class XWalkExtensionMessageFilter : public content::BrowserMessageFilter {
 public:
  explicit XWalkExtensionMessageFilter(
      XWalkExtensionWebContentsHandler* handler);
  virtual ~XWalkExtensionMessageFilter();

  void PostMessage(const XWalkExtensionRunner* runner,
                   scoped_ptr<base::Value> msg);

  // content::BrowserMessageFilter implementation.
  virtual bool OnMessageReceived(const IPC::Message& message,
                                 bool* message_was_ok) OVERRIDE;

 private:
  // IPC message handlers.
  void OnPostMessage(int64_t frame_id, const std::string& extension_name,
                     const base::ListValue& msg);
  void OnSendSyncMessage(int64_t frame_id, const std::string& extension_name,
                         const base::ListValue& msg, base::ListValue* result);
  void DidCreateScriptContext(int64_t frame_id);
  void WillReleaseScriptContext(int64_t frame_id);

  XWalkExtensionWebContentsHandler* handler_;
  XWalkExtensionRunnerStore* runners_;
  int routing_id_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionMessageFilter);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_MESSAGE_FILTER_H_
