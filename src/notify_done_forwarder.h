// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_NOTIFY_DONE_FORWARDER_H_
#define CONTENT_SHELL_NOTIFY_DONE_FORWARDER_H_

#include "base/basictypes.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace content {

class NotifyDoneForwarder : public WebContentsObserver,
                            public WebContentsUserData<NotifyDoneForwarder> {
 public:
  virtual ~NotifyDoneForwarder();

  // WebContentsObserver implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  friend class WebContentsUserData<NotifyDoneForwarder>;

  explicit NotifyDoneForwarder(WebContents* web_contents);

  void OnTestFinishedInSecondaryWindow();

  DISALLOW_COPY_AND_ASSIGN(NotifyDoneForwarder);
};

}  // namespace content

#endif  // CONTENT_SHELL_NOTIFY_DONE_FORWARDER_H_
