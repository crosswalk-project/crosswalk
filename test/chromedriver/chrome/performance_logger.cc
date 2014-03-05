// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/chrome/performance_logger.h"

#include "base/json/json_writer.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "xwalk/test/chromedriver/chrome/devtools_client.h"
#include "xwalk/test/chromedriver/chrome/log.h"
#include "xwalk/test/chromedriver/chrome/status.h"

namespace {

// DevTools event domain prefixes to intercept.
const char* const kDomains[] = {"Network.", "Page.", "Timeline."};

const char* const kDomainEnableCommands[] = {
    "Network.enable", "Page.enable", "Timeline.start"
};

// Returns whether the event belongs to one of kDomains.
bool ShouldLogEvent(const std::string& method) {
  for (size_t i_domain = 0; i_domain < arraysize(kDomains); ++i_domain) {
    if (StartsWithASCII(method, kDomains[i_domain], true /* case_sensitive */))
      return true;
  }
  return false;
}

}  // namespace

PerformanceLogger::PerformanceLogger(Log* log)
    : log_(log) {}

Status PerformanceLogger::OnConnected(DevToolsClient* client) {
  base::DictionaryValue params;  // All our enable commands have empty params.
  for (size_t i_cmd = 0; i_cmd < arraysize(kDomainEnableCommands); ++i_cmd) {
    Status status = client->SendCommand(kDomainEnableCommands[i_cmd], params);
    if (status.IsError())
      return status;
  }
  return Status(kOk);
}

Status PerformanceLogger::OnEvent(
    DevToolsClient* client,
    const std::string& method,
    const base::DictionaryValue& params) {
  if (!ShouldLogEvent(method))
    return Status(kOk);

  base::DictionaryValue log_message_dict;
  log_message_dict.SetString("webview", client->GetId());
  log_message_dict.SetString("message.method", method);
  log_message_dict.Set("message.params", params.DeepCopy());
  std::string log_message_json;
  // TODO(klm): extract timestamp from params?
  // Look at where it is for Page, Network, Timeline events.
  base::JSONWriter::Write(&log_message_dict, &log_message_json);

  log_->AddEntry(Log::kInfo, log_message_json);
  return Status(kOk);
}
