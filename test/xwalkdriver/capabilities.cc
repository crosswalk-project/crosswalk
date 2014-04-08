// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/capabilities.h"

#include <map>

#include "base/bind.h"
#include "base/callback.h"
#include "base/json/string_escape.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/logging.h"
#include "net/base/net_util.h"

namespace {

typedef base::Callback<Status(const base::Value&, Capabilities*)> Parser;

Status ParseBoolean(
    bool* to_set,
    const base::Value& option,
    Capabilities* capabilities) {
  if (!option.GetAsBoolean(to_set))
    return Status(kUnknownError, "must be a boolean");
  return Status(kOk);
}

Status ParseString(std::string* to_set,
                   const base::Value& option,
                   Capabilities* capabilities) {
  std::string str;
  if (!option.GetAsString(&str))
    return Status(kUnknownError, "must be a string");
  if (str.empty())
    return Status(kUnknownError, "cannot be empty");
  *to_set = str;
  return Status(kOk);
}

Status ParseFilePath(base::FilePath* to_set,
                     const base::Value& option,
                     Capabilities* capabilities) {
  base::FilePath::StringType str;
  if (!option.GetAsString(&str))
    return Status(kUnknownError, "must be a string");
  *to_set = base::FilePath(str);
  return Status(kOk);
}

Status ParseDict(scoped_ptr<base::DictionaryValue>* to_set,
                 const base::Value& option,
                 Capabilities* capabilities) {
  const base::DictionaryValue* dict = NULL;
  if (!option.GetAsDictionary(&dict))
    return Status(kUnknownError, "must be a dictionary");
  to_set->reset(dict->DeepCopy());
  return Status(kOk);
}

Status IgnoreDeprecatedOption(
    const char* option_name,
    const base::Value& option,
    Capabilities* capabilities) {
  LOG(WARNING) << "Deprecated xwalk option is ignored: " << option_name;
  return Status(kOk);
}

Status IgnoreCapability(const base::Value& option, Capabilities* capabilities) {
  return Status(kOk);
}

Status ParseLogPath(const base::Value& option, Capabilities* capabilities) {
  if (!option.GetAsString(&capabilities->log_path))
    return Status(kUnknownError, "must be a string");
  return Status(kOk);
}

Status ParseSwitches(const base::Value& option,
                     Capabilities* capabilities) {
  const base::ListValue* switches_list = NULL;
  if (!option.GetAsList(&switches_list))
    return Status(kUnknownError, "must be a list");
  for (size_t i = 0; i < switches_list->GetSize(); ++i) {
    std::string arg_string;
    if (!switches_list->GetString(i, &arg_string))
      return Status(kUnknownError, "each argument must be a string");
    capabilities->switches.SetUnparsedSwitch(arg_string);
  }
  return Status(kOk);
}

Status ParseExtensions(const base::Value& option, Capabilities* capabilities) {
  const base::ListValue* extensions = NULL;
  if (!option.GetAsList(&extensions))
    return Status(kUnknownError, "must be a list");
  for (size_t i = 0; i < extensions->GetSize(); ++i) {
    std::string extension;
    if (!extensions->GetString(i, &extension)) {
      return Status(kUnknownError,
                    "each extension must be a base64 encoded string");
    }
    capabilities->extensions.push_back(extension);
  }
  return Status(kOk);
}

Status ParseProxy(const base::Value& option, Capabilities* capabilities) {
  const base::DictionaryValue* proxy_dict;
  if (!option.GetAsDictionary(&proxy_dict))
    return Status(kUnknownError, "must be a dictionary");
  std::string proxy_type;
  if (!proxy_dict->GetString("proxyType", &proxy_type))
    return Status(kUnknownError, "'proxyType' must be a string");
  proxy_type = StringToLowerASCII(proxy_type);
  if (proxy_type == "direct") {
    capabilities->switches.SetSwitch("no-proxy-server");
  } else if (proxy_type == "system") {
    // Xwalk default.
  } else if (proxy_type == "pac") {
    CommandLine::StringType proxy_pac_url;
    if (!proxy_dict->GetString("proxyAutoconfigUrl", &proxy_pac_url))
      return Status(kUnknownError, "'proxyAutoconfigUrl' must be a string");
    capabilities->switches.SetSwitch("proxy-pac-url", proxy_pac_url);
  } else if (proxy_type == "autodetect") {
    capabilities->switches.SetSwitch("proxy-auto-detect");
  } else if (proxy_type == "manual") {
    const char* proxy_servers_options[][2] = {
        {"ftpProxy", "ftp"}, {"httpProxy", "http"}, {"sslProxy", "https"}};
    const base::Value* option_value = NULL;
    std::string proxy_servers;
    for (size_t i = 0; i < arraysize(proxy_servers_options); ++i) {
      if (!proxy_dict->Get(proxy_servers_options[i][0], &option_value) ||
          option_value->IsType(base::Value::TYPE_NULL)) {
        continue;
      }
      std::string value;
      if (!option_value->GetAsString(&value)) {
        return Status(
            kUnknownError,
            base::StringPrintf("'%s' must be a string",
                               proxy_servers_options[i][0]));
      }
      // Converts into Xwalk proxy scheme.
      // Example: "http=localhost:9000;ftp=localhost:8000".
      if (!proxy_servers.empty())
        proxy_servers += ";";
      proxy_servers += base::StringPrintf(
          "%s=%s", proxy_servers_options[i][1], value.c_str());
    }

    std::string proxy_bypass_list;
    if (proxy_dict->Get("noProxy", &option_value) &&
        !option_value->IsType(base::Value::TYPE_NULL)) {
      if (!option_value->GetAsString(&proxy_bypass_list))
        return Status(kUnknownError, "'noProxy' must be a string");
    }

    if (proxy_servers.empty() && proxy_bypass_list.empty()) {
      return Status(kUnknownError,
                    "proxyType is 'manual' but no manual "
                    "proxy capabilities were found");
    }
    if (!proxy_servers.empty())
      capabilities->switches.SetSwitch("proxy-server", proxy_servers);
    if (!proxy_bypass_list.empty()) {
      capabilities->switches.SetSwitch("proxy-bypass-list",
                                       proxy_bypass_list);
    }
  } else {
    return Status(kUnknownError, "unrecognized proxy type:" + proxy_type);
  }
  return Status(kOk);
}

Status ParseExcludeSwitches(const base::Value& option,
                            Capabilities* capabilities) {
  const base::ListValue* switches = NULL;
  if (!option.GetAsList(&switches))
    return Status(kUnknownError, "must be a list");
  for (size_t i = 0; i < switches->GetSize(); ++i) {
    std::string switch_name;
    if (!switches->GetString(i, &switch_name)) {
      return Status(kUnknownError,
                    "each switch to be removed must be a string");
    }
    capabilities->exclude_switches.insert(switch_name);
  }
  return Status(kOk);
}

Status ParseUseExistingBrowser(const base::Value& option,
                               Capabilities* capabilities) {
  std::string server_addr;
  if (!option.GetAsString(&server_addr))
    return Status(kUnknownError, "must be 'host:port'");

  std::vector<std::string> values;
  base::SplitString(server_addr, ':', &values);
  if (values.size() != 2)
    return Status(kUnknownError, "must be 'host:port'");

  int port = 0;
  base::StringToInt(values[1], &port);
  if (port <= 0)
    return Status(kUnknownError, "port must be > 0");

  capabilities->debugger_address = NetAddress(values[0], port);
  return Status(kOk);
}

Status ParseLoggingPrefs(const base::Value& option,
                         Capabilities* capabilities) {
  const base::DictionaryValue* logging_prefs = NULL;
  if (!option.GetAsDictionary(&logging_prefs))
    return Status(kUnknownError, "must be a dictionary");

  for (base::DictionaryValue::Iterator pref(*logging_prefs);
       !pref.IsAtEnd(); pref.Advance()) {
    std::string type = pref.key();
    Log::Level level;
    std::string level_name;
    if (!pref.value().GetAsString(&level_name) ||
        !WebDriverLog::NameToLevel(level_name, &level)) {
      return Status(kUnknownError, "invalid log level for '" + type + "' log");
    }
    capabilities->logging_prefs.insert(std::make_pair(type, level));
  }
  return Status(kOk);
}

Status ParseXwalkOptions(
    const base::Value& capability,
    Capabilities* capabilities) {
  const base::DictionaryValue* xwalk_options = NULL;
  if (!capability.GetAsDictionary(&xwalk_options))
    return Status(kUnknownError, "must be a dictionary");

  bool is_android = xwalk_options->HasKey("androidPackage");
  bool is_existing = xwalk_options->HasKey("debuggerAddress");

  std::map<std::string, Parser> parser_map;
  // Ignore 'args', 'binary' and 'extensions' capabilities by default, since the
  // Java client always passes them.
  parser_map["args"] = base::Bind(&IgnoreCapability);
  parser_map["binary"] = base::Bind(&IgnoreCapability);
  parser_map["extensions"] = base::Bind(&IgnoreCapability);
  if (is_android) {
    parser_map["androidActivity"] =
        base::Bind(&ParseString, &capabilities->android_activity);
    parser_map["androidDeviceSerial"] =
        base::Bind(&ParseString, &capabilities->android_device_serial);
    parser_map["androidPackage"] =
        base::Bind(&ParseString, &capabilities->android_package);
    parser_map["androidProcess"] =
        base::Bind(&ParseString, &capabilities->android_process);
    parser_map["args"] = base::Bind(&ParseSwitches);
  } else if (is_existing) {
    parser_map["debuggerAddress"] = base::Bind(&ParseUseExistingBrowser);
  } else {
    parser_map["args"] = base::Bind(&ParseSwitches);
    parser_map["binary"] = base::Bind(&ParseFilePath, &capabilities->binary);
    parser_map["detach"] = base::Bind(&ParseBoolean, &capabilities->detach);
    parser_map["excludeSwitches"] = base::Bind(&ParseExcludeSwitches);
    parser_map["extensions"] = base::Bind(&ParseExtensions);
    parser_map["forceDevToolsScreenshot"] = base::Bind(
        &ParseBoolean, &capabilities->force_devtools_screenshot);
    parser_map["loadAsync"] = base::Bind(&IgnoreDeprecatedOption, "loadAsync");
    parser_map["localState"] =
        base::Bind(&ParseDict, &capabilities->local_state);
    parser_map["logPath"] = base::Bind(&ParseLogPath);
    parser_map["minidumpPath"] =
        base::Bind(&ParseString, &capabilities->minidump_path);
    parser_map["prefs"] = base::Bind(&ParseDict, &capabilities->prefs);
  }

  for (base::DictionaryValue::Iterator it(*xwalk_options); !it.IsAtEnd();
       it.Advance()) {
    if (parser_map.find(it.key()) == parser_map.end()) {
      return Status(kUnknownError,
                    "unrecognized xwalk option: " + it.key());
    }
    Status status = parser_map[it.key()].Run(it.value(), capabilities);
    if (status.IsError())
      return Status(kUnknownError, "cannot parse " + it.key(), status);
  }
  return Status(kOk);
}

}  // namespace

Switches::Switches() {}

Switches::~Switches() {}

void Switches::SetSwitch(const std::string& name) {
  SetSwitch(name, NativeString());
}

void Switches::SetSwitch(const std::string& name, const std::string& value) {
#if defined(OS_WIN)
  SetSwitch(name, base::UTF8ToUTF16(value));
#else
  switch_map_[name] = value;
#endif
}

void Switches::SetSwitch(const std::string& name, const base::string16& value) {
#if defined(OS_WIN)
  switch_map_[name] = value;
#else
  SetSwitch(name, UTF16ToUTF8(value));
#endif
}

void Switches::SetSwitch(const std::string& name, const base::FilePath& value) {
  SetSwitch(name, value.value());
}

void Switches::SetFromSwitches(const Switches& switches) {
  for (SwitchMap::const_iterator iter = switches.switch_map_.begin();
       iter != switches.switch_map_.end();
       ++iter) {
    switch_map_[iter->first] = iter->second;
  }
}

void Switches::SetUnparsedSwitch(const std::string& unparsed_switch) {
  std::string value;
  size_t equals_index = unparsed_switch.find('=');
  if (equals_index != std::string::npos)
    value = unparsed_switch.substr(equals_index + 1);

  std::string name;
  size_t start_index = 0;
  if (unparsed_switch.substr(0, 2) == "--")
    start_index = 2;
  name = unparsed_switch.substr(start_index, equals_index - start_index);

  SetSwitch(name, value);
}

void Switches::RemoveSwitch(const std::string& name) {
  switch_map_.erase(name);
}

bool Switches::HasSwitch(const std::string& name) const {
  return switch_map_.count(name) > 0;
}

std::string Switches::GetSwitchValue(const std::string& name) const {
  NativeString value = GetSwitchValueNative(name);
#if defined(OS_WIN)
  return base::UTF16ToUTF8(value);
#else
  return value;
#endif
}

Switches::NativeString Switches::GetSwitchValueNative(
    const std::string& name) const {
  SwitchMap::const_iterator iter = switch_map_.find(name);
  if (iter == switch_map_.end())
    return NativeString();
  return iter->second;
}

size_t Switches::GetSize() const {
  return switch_map_.size();
}

void Switches::AppendToCommandLine(CommandLine* command) const {
  for (SwitchMap::const_iterator iter = switch_map_.begin();
       iter != switch_map_.end();
       ++iter) {
    command->AppendSwitchNative(iter->first, iter->second);
  }
}

std::string Switches::ToString() const {
  std::string str;
  SwitchMap::const_iterator iter = switch_map_.begin();
  while (iter != switch_map_.end()) {
    str += "--" + iter->first;
    std::string value = GetSwitchValue(iter->first);
    if (value.length()) {
      if (value.find(' ') != std::string::npos)
        value = base::GetQuotedJSONString(value);
      str += "=" + value;
    }
    ++iter;
    if (iter == switch_map_.end())
      break;
    str += " ";
  }
  return str;
}

Capabilities::Capabilities()
    : detach(false),
      force_devtools_screenshot(false) {}

Capabilities::~Capabilities() {}

bool Capabilities::IsAndroid() const {
  return !android_package.empty();
}

bool Capabilities::IsExistingBrowser() const {
  return debugger_address.IsValid();
}

Status Capabilities::Parse(const base::DictionaryValue& desired_caps) {
  std::map<std::string, Parser> parser_map;
  parser_map["xwalkOptions"] = base::Bind(&ParseXwalkOptions);
  parser_map["loggingPrefs"] = base::Bind(&ParseLoggingPrefs);
  parser_map["proxy"] = base::Bind(&ParseProxy);
  for (std::map<std::string, Parser>::iterator it = parser_map.begin();
       it != parser_map.end(); ++it) {
    const base::Value* capability = NULL;
    if (desired_caps.Get(it->first, &capability)) {
      Status status = it->second.Run(*capability, this);
      if (status.IsError()) {
        return Status(
            kUnknownError, "cannot parse capability: " + it->first, status);
      }
    }
  }
  return Status(kOk);
}
