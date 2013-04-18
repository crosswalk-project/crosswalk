// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_BROWSER_SHELL_MESSAGE_FILTER_H_
#define CAMEO_SRC_BROWSER_SHELL_MESSAGE_FILTER_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "content/public/browser/browser_message_filter.h"

namespace quota {
class QuotaManager;
}

namespace webkit_database {
class DatabaseTracker;
}

namespace cameo {

class ShellMessageFilter : public content::BrowserMessageFilter {
 public:
  ShellMessageFilter(int render_process_id,
                     webkit_database::DatabaseTracker* database_tracker,
                     quota::QuotaManager* quota_manager);

 private:
  virtual ~ShellMessageFilter();

  // BrowserMessageFilter implementation.
  virtual bool OnMessageReceived(const IPC::Message& message,
                                 bool* message_was_ok) OVERRIDE;

  int render_process_id_;

  webkit_database::DatabaseTracker* database_tracker_;
  quota::QuotaManager* quota_manager_;

  DISALLOW_COPY_AND_ASSIGN(ShellMessageFilter);
};

}  // namespace cameo

#endif // CAMEO_SRC_BROWSER_SHELL_MESSAGE_FILTER_H_