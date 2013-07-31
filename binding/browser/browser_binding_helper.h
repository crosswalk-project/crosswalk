// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_BROWSER_BROWSER_BINDING_HELPER_H_
#define XWALK_BINDING_BROWSER_BROWSER_BINDING_HELPER_H_

#include <string>
#include <vector>

#include "content/public/browser/web_contents_observer.h"

namespace IPC {
struct ChannelHandle;
}

namespace xwalk {

class BrowserBindingHelper : public content::WebContentsObserver {
 public:
  explicit BrowserBindingHelper(content::WebContents* web_contents);
  ~BrowserBindingHelper();

  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  // FIXME(zliang7): how to do when web_contents is destroyed

 private:
  void OnOpenBindingChannel(const GURL& url, IPC::Message* reply_msg);
  void CreateBindingChannel(const GURL& url,
                            const std::vector<std::string> features);
  void DidCreateBindingChannel(const IPC::ChannelHandle& channel_handle);

  IPC::Message* delay_reply_msg_;

  DISALLOW_COPY_AND_ASSIGN(BrowserBindingHelper);
};

}  // namespace xwalk

#endif  // XWALK_BINDING_BROWSER_BROWSER_BINDING_HELPER_H_
