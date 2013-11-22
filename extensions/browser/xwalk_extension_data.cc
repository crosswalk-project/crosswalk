// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_data.h"

#include "content/public/browser/browser_thread.h"
#include "xwalk/extensions/browser/xwalk_extension_process_host.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"

using content::BrowserThread;

namespace xwalk {
namespace extensions {

XWalkExtensionData::XWalkExtensionData()
    : in_process_message_filter_(NULL),
      extension_thread_(NULL),
      render_process_host_(NULL) {}

XWalkExtensionData::~XWalkExtensionData() {
  DCHECK(in_process_extension_thread_server_);
  DCHECK(in_process_ui_thread_server_);
  DCHECK(extension_thread_);

  in_process_extension_thread_server_->Invalidate();
  in_process_ui_thread_server_->Invalidate();

  extension_thread_->message_loop()->DeleteSoon(
      FROM_HERE, in_process_extension_thread_server_.release());

  if (extension_process_host_) {
    BrowserThread::DeleteSoon(
        BrowserThread::IO, FROM_HERE, extension_process_host_.release());
  }
}

}  // namespace extensions
}  // namespace xwalk
