// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/chrome/console_logger.h"

#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "chrome/test/chromedriver/chrome/devtools_client.h"
#include "chrome/test/chromedriver/chrome/log.h"
#include "chrome/test/chromedriver/chrome/status.h"

namespace {

// Translates Console.messageAdded.message.level into Log::Level.
bool ConsoleLevelToLogLevel(const std::string& name, Log::Level *out_level) {
  const char* const kConsoleLevelNames[] = {
    "debug", "log", "warning", "error"
  };

  for (size_t i = 0; i < arraysize(kConsoleLevelNames); ++i) {
    if (name == kConsoleLevelNames[i]) {
      CHECK_LE(Log::kDebug + i, static_cast<size_t>(Log::kError));
      *out_level = static_cast<Log::Level>(Log::kDebug + i);
      return true;
    }
  }
  return false;
}

}  // namespace

ConsoleLogger::ConsoleLogger(Log* log)
    : log_(log) {}

Status ConsoleLogger::OnConnected(DevToolsClient* client) {
  base::DictionaryValue params;
  return client->SendCommand("Console.enable", params);
}

Status ConsoleLogger::OnEvent(
    DevToolsClient* client,
    const std::string& method,
    const base::DictionaryValue& params) {
  if (method != "Console.messageAdded")
    return Status(kOk);

  // If the event has proper structure and fields, log formatted.
  // Else it's a weird message that we don't know how to format, log full JSON.
  const base::DictionaryValue *message_dict = NULL;
  if (params.GetDictionary("message", &message_dict)) {
    std::string text;
    std::string level_name;
    Log::Level level = Log::kInfo;
    if (message_dict->GetString("text", &text) && !text.empty() &&
        message_dict->GetString("level", &level_name) &&
        ConsoleLevelToLogLevel(level_name, &level)) {

      const char* origin_cstr = "unknown";
      std::string origin;
      if ((message_dict->GetString("url", &origin) && !origin.empty()) ||
          (message_dict->GetString("source", &origin) && !origin.empty())) {
        origin_cstr = origin.c_str();
      }

      std::string line_column;
      int line = -1;
      if (message_dict->GetInteger("line", &line)) {
        int column = -1;
        if (message_dict->GetInteger("column", &column)) {
          base::SStringPrintf(&line_column, "%d:%d", line, column);
        } else {
          base::SStringPrintf(&line_column, "%d", line);
        }
      } else {
        // No line number, but print anyway, just to maintain the number of
        // fields in the formatted message in case someone wants to parse it.
        line_column = "-";
      }

      std::string source;
      message_dict->GetString("source", &source);
      log_->AddEntry(level, source, base::StringPrintf("%s %s %s",
                                                       origin_cstr,
                                                       line_column.c_str(),
                                                       text.c_str()));

      return Status(kOk);
    }
  }

  // Don't know how to format, log full JSON.
  std::string message_json;
  base::JSONWriter::Write(&params, &message_json);
  log_->AddEntry(Log::kWarning, message_json);
  return Status(kOk);
}
