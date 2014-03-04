// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/chrome/log.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/string_util.h"
#include "base/values.h"

void Log::AddEntry(Level level, const std::string& message) {
  AddEntry(level, "", message);
}

void Log::AddEntry(Level level,
                   const std::string& source,
                   const std::string& message) {
  AddEntryTimestamped(base::Time::Now(), level, source, message);
}

namespace {

IsVLogOnFunc g_is_vlog_on_func = NULL;

void TruncateString(std::string* data) {
  const size_t kMaxLength = 200;
  if (data->length() > kMaxLength) {
    data->resize(kMaxLength);
    data->replace(kMaxLength - 3, 3, "...");
  }
}

scoped_ptr<base::Value> SmartDeepCopy(const base::Value* value) {
  const size_t kMaxChildren = 20;
  const base::ListValue* list = NULL;
  const base::DictionaryValue* dict = NULL;
  std::string data;
  if (value->GetAsDictionary(&dict)) {
    scoped_ptr<base::DictionaryValue> dict_copy(new base::DictionaryValue());
    for (base::DictionaryValue::Iterator it(*dict); !it.IsAtEnd();
         it.Advance()) {
      if (dict_copy->size() >= kMaxChildren - 1) {
        dict_copy->SetStringWithoutPathExpansion("~~~", "...");
        break;
      }
      const base::Value* child = NULL;
      dict->GetWithoutPathExpansion(it.key(), &child);
      dict_copy->SetWithoutPathExpansion(it.key(),
                                         SmartDeepCopy(child).release());
    }
    return dict_copy.PassAs<base::Value>();
  } else if (value->GetAsList(&list)) {
    scoped_ptr<base::ListValue> list_copy(new base::ListValue());
    for (size_t i = 0; i < list->GetSize(); ++i) {
      const base::Value* child = NULL;
      if (!list->Get(i, &child))
        continue;
      if (list_copy->GetSize() >= kMaxChildren - 1) {
        list_copy->AppendString("...");
        break;
      }
      list_copy->Append(SmartDeepCopy(child).release());
    }
    return list_copy.PassAs<base::Value>();
  } else if (value->GetAsString(&data)) {
    TruncateString(&data);
    return scoped_ptr<base::Value>(new base::StringValue(data));
  }
  return scoped_ptr<base::Value>(value->DeepCopy());
}

}  // namespace

void InitLogging(IsVLogOnFunc is_vlog_on_func) {
  g_is_vlog_on_func = is_vlog_on_func;
}

bool IsVLogOn(int vlog_level) {
  if (!g_is_vlog_on_func)
    return false;
  return g_is_vlog_on_func(vlog_level);
}

std::string PrettyPrintValue(const base::Value& value) {
  std::string json;
  base::JSONWriter::WriteWithOptions(
      &value, base::JSONWriter::OPTIONS_PRETTY_PRINT, &json);
#if defined(OS_WIN)
  RemoveChars(json, "\r", &json);
#endif
  // Remove the trailing newline.
  if (json.length())
    json.resize(json.length() - 1);
  return json;
}

std::string FormatValueForDisplay(const base::Value& value) {
  scoped_ptr<base::Value> copy(SmartDeepCopy(&value));
  return PrettyPrintValue(*copy);
}

std::string FormatJsonForDisplay(const std::string& json) {
  scoped_ptr<base::Value> value(base::JSONReader::Read(json));
  if (!value)
    value.reset(new base::StringValue(json));
  return FormatValueForDisplay(*value);
}
