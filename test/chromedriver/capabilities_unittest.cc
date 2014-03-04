// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/capabilities.h"

#include "base/values.h"
#include "chrome/test/chromedriver/chrome/log.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(Switches, Empty) {
  Switches switches;
  CommandLine cmd(CommandLine::NO_PROGRAM);
  switches.AppendToCommandLine(&cmd);
  ASSERT_EQ(0u, cmd.GetSwitches().size());
  ASSERT_EQ("", switches.ToString());
}

TEST(Switches, NoValue) {
  Switches switches;
  switches.SetSwitch("hello");

  ASSERT_TRUE(switches.HasSwitch("hello"));
  ASSERT_EQ("", switches.GetSwitchValue("hello"));

  CommandLine cmd(CommandLine::NO_PROGRAM);
  switches.AppendToCommandLine(&cmd);
  ASSERT_TRUE(cmd.HasSwitch("hello"));
  ASSERT_EQ(FILE_PATH_LITERAL(""), cmd.GetSwitchValueNative("hello"));
  ASSERT_EQ("--hello", switches.ToString());
}

TEST(Switches, Value) {
  Switches switches;
  switches.SetSwitch("hello", "there");

  ASSERT_TRUE(switches.HasSwitch("hello"));
  ASSERT_EQ("there", switches.GetSwitchValue("hello"));

  CommandLine cmd(CommandLine::NO_PROGRAM);
  switches.AppendToCommandLine(&cmd);
  ASSERT_TRUE(cmd.HasSwitch("hello"));
  ASSERT_EQ(FILE_PATH_LITERAL("there"), cmd.GetSwitchValueNative("hello"));
  ASSERT_EQ("--hello=there", switches.ToString());
}

TEST(Switches, FromOther) {
  Switches switches;
  switches.SetSwitch("a", "1");
  switches.SetSwitch("b", "1");

  Switches switches2;
  switches2.SetSwitch("b", "2");
  switches2.SetSwitch("c", "2");

  switches.SetFromSwitches(switches2);
  ASSERT_EQ("--a=1 --b=2 --c=2", switches.ToString());
}

TEST(Switches, Remove) {
  Switches switches;
  switches.SetSwitch("a", "1");
  switches.RemoveSwitch("a");
  ASSERT_FALSE(switches.HasSwitch("a"));
}

TEST(Switches, Quoting) {
  Switches switches;
  switches.SetSwitch("hello", "a  b");
  switches.SetSwitch("hello2", "  '\"  ");

  ASSERT_EQ("--hello=\"a  b\" --hello2=\"  '\\\"  \"", switches.ToString());
}

TEST(Switches, Multiple) {
  Switches switches;
  switches.SetSwitch("switch");
  switches.SetSwitch("hello", "there");

  CommandLine cmd(CommandLine::NO_PROGRAM);
  switches.AppendToCommandLine(&cmd);
  ASSERT_TRUE(cmd.HasSwitch("switch"));
  ASSERT_TRUE(cmd.HasSwitch("hello"));
  ASSERT_EQ(FILE_PATH_LITERAL("there"), cmd.GetSwitchValueNative("hello"));
  ASSERT_EQ("--hello=there --switch", switches.ToString());
}

TEST(Switches, Unparsed) {
  Switches switches;
  switches.SetUnparsedSwitch("a");
  switches.SetUnparsedSwitch("--b");
  switches.SetUnparsedSwitch("--c=1");
  switches.SetUnparsedSwitch("d=1");
  switches.SetUnparsedSwitch("-e=--1=1");

  ASSERT_EQ("---e=--1=1 --a --b --c=1 --d=1", switches.ToString());
}

TEST(ParseCapabilities, WithAndroidPackage) {
  Capabilities capabilities;
  base::DictionaryValue caps;
  caps.SetString("chromeOptions.androidPackage", "abc");
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_TRUE(capabilities.IsAndroid());
  ASSERT_EQ("abc", capabilities.android_package);
}

TEST(ParseCapabilities, EmptyAndroidPackage) {
  Capabilities capabilities;
  base::DictionaryValue caps;
  caps.SetString("chromeOptions.androidPackage", std::string());
  Status status = capabilities.Parse(caps);
  ASSERT_FALSE(status.IsOk());
}

TEST(ParseCapabilities, IllegalAndroidPackage) {
  Capabilities capabilities;
  base::DictionaryValue caps;
  caps.SetInteger("chromeOptions.androidPackage", 123);
  Status status = capabilities.Parse(caps);
  ASSERT_FALSE(status.IsOk());
}

TEST(ParseCapabilities, LogPath) {
  Capabilities capabilities;
  base::DictionaryValue caps;
  caps.SetString("chromeOptions.logPath", "path/to/logfile");
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_STREQ("path/to/logfile", capabilities.log_path.c_str());
}

TEST(ParseCapabilities, Args) {
  Capabilities capabilities;
  base::ListValue args;
  args.AppendString("arg1");
  args.AppendString("arg2=val");
  base::DictionaryValue caps;
  caps.Set("chromeOptions.args", args.DeepCopy());

  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());

  ASSERT_EQ(2u, capabilities.switches.GetSize());
  ASSERT_TRUE(capabilities.switches.HasSwitch("arg1"));
  ASSERT_TRUE(capabilities.switches.HasSwitch("arg2"));
  ASSERT_EQ("", capabilities.switches.GetSwitchValue("arg1"));
  ASSERT_EQ("val", capabilities.switches.GetSwitchValue("arg2"));
}

TEST(ParseCapabilities, Prefs) {
  Capabilities capabilities;
  base::DictionaryValue prefs;
  prefs.SetString("key1", "value1");
  prefs.SetString("key2.k", "value2");
  base::DictionaryValue caps;
  caps.Set("chromeOptions.prefs", prefs.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_TRUE(capabilities.prefs->Equals(&prefs));
}

TEST(ParseCapabilities, LocalState) {
  Capabilities capabilities;
  base::DictionaryValue local_state;
  local_state.SetString("s1", "v1");
  local_state.SetString("s2.s", "v2");
  base::DictionaryValue caps;
  caps.Set("chromeOptions.localState", local_state.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_TRUE(capabilities.local_state->Equals(&local_state));
}

TEST(ParseCapabilities, Extensions) {
  Capabilities capabilities;
  base::ListValue extensions;
  extensions.AppendString("ext1");
  extensions.AppendString("ext2");
  base::DictionaryValue caps;
  caps.Set("chromeOptions.extensions", extensions.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(2u, capabilities.extensions.size());
  ASSERT_EQ("ext1", capabilities.extensions[0]);
  ASSERT_EQ("ext2", capabilities.extensions[1]);
}

TEST(ParseCapabilities, UnrecognizedProxyType) {
  Capabilities capabilities;
  base::DictionaryValue proxy;
  proxy.SetString("proxyType", "unknown proxy type");
  base::DictionaryValue caps;
  caps.Set("proxy", proxy.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_FALSE(status.IsOk());
}

TEST(ParseCapabilities, IllegalProxyType) {
  Capabilities capabilities;
  base::DictionaryValue proxy;
  proxy.SetInteger("proxyType", 123);
  base::DictionaryValue caps;
  caps.Set("proxy", proxy.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_FALSE(status.IsOk());
}

TEST(ParseCapabilities, DirectProxy) {
  Capabilities capabilities;
  base::DictionaryValue proxy;
  proxy.SetString("proxyType", "DIRECT");
  base::DictionaryValue caps;
  caps.Set("proxy", proxy.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(1u, capabilities.switches.GetSize());
  ASSERT_TRUE(capabilities.switches.HasSwitch("no-proxy-server"));
}

TEST(ParseCapabilities, SystemProxy) {
  Capabilities capabilities;
  base::DictionaryValue proxy;
  proxy.SetString("proxyType", "system");
  base::DictionaryValue caps;
  caps.Set("proxy", proxy.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(0u, capabilities.switches.GetSize());
}

TEST(ParseCapabilities, PacProxy) {
  Capabilities capabilities;
  base::DictionaryValue proxy;
  proxy.SetString("proxyType", "PAC");
  proxy.SetString("proxyAutoconfigUrl", "test.wpad");
  base::DictionaryValue caps;
  caps.Set("proxy", proxy.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(1u, capabilities.switches.GetSize());
  ASSERT_EQ("test.wpad", capabilities.switches.GetSwitchValue("proxy-pac-url"));
}

TEST(ParseCapabilities, MissingProxyAutoconfigUrl) {
  Capabilities capabilities;
  base::DictionaryValue proxy;
  proxy.SetString("proxyType", "PAC");
  proxy.SetString("httpProxy", "http://localhost:8001");
  base::DictionaryValue caps;
  caps.Set("proxy", proxy.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_FALSE(status.IsOk());
}

TEST(ParseCapabilities, AutodetectProxy) {
  Capabilities capabilities;
  base::DictionaryValue proxy;
  proxy.SetString("proxyType", "autodetect");
  base::DictionaryValue caps;
  caps.Set("proxy", proxy.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(1u, capabilities.switches.GetSize());
  ASSERT_TRUE(capabilities.switches.HasSwitch("proxy-auto-detect"));
}

TEST(ParseCapabilities, ManualProxy) {
  Capabilities capabilities;
  base::DictionaryValue proxy;
  proxy.SetString("proxyType", "manual");
  proxy.SetString("ftpProxy", "localhost:9001");
  proxy.SetString("httpProxy", "localhost:8001");
  proxy.SetString("sslProxy", "localhost:10001");
  proxy.SetString("noProxy", "google.com, youtube.com");
  base::DictionaryValue caps;
  caps.Set("proxy", proxy.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(2u, capabilities.switches.GetSize());
  ASSERT_EQ(
      "ftp=localhost:9001;http=localhost:8001;https=localhost:10001",
      capabilities.switches.GetSwitchValue("proxy-server"));
  ASSERT_EQ(
      "google.com, youtube.com",
      capabilities.switches.GetSwitchValue("proxy-bypass-list"));
}

TEST(ParseCapabilities, MissingSettingForManualProxy) {
  Capabilities capabilities;
  base::DictionaryValue proxy;
  proxy.SetString("proxyType", "manual");
  base::DictionaryValue caps;
  caps.Set("proxy", proxy.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_FALSE(status.IsOk());
}

TEST(ParseCapabilities, IgnoreNullValueForManualProxy) {
  Capabilities capabilities;
  base::DictionaryValue proxy;
  proxy.SetString("proxyType", "manual");
  proxy.SetString("ftpProxy", "localhost:9001");
  proxy.Set("sslProxy", base::Value::CreateNullValue());
  proxy.Set("noProxy", base::Value::CreateNullValue());
  base::DictionaryValue caps;
  caps.Set("proxy", proxy.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(1u, capabilities.switches.GetSize());
  ASSERT_TRUE(capabilities.switches.HasSwitch("proxy-server"));
  ASSERT_EQ(
      "ftp=localhost:9001",
      capabilities.switches.GetSwitchValue("proxy-server"));
}

TEST(ParseCapabilities, LoggingPrefsOk) {
  Capabilities capabilities;
  base::DictionaryValue logging_prefs;
  logging_prefs.SetString("Network", "INFO");
  base::DictionaryValue caps;
  caps.Set("loggingPrefs", logging_prefs.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(1u, capabilities.logging_prefs.size());
  ASSERT_EQ(Log::kInfo, capabilities.logging_prefs["Network"]);
}

TEST(ParseCapabilities, LoggingPrefsNotDict) {
  Capabilities capabilities;
  base::DictionaryValue caps;
  caps.SetString("loggingPrefs", "INFO");
  Status status = capabilities.Parse(caps);
  ASSERT_FALSE(status.IsOk());
}

TEST(ParseCapabilities, ExcludeSwitches) {
  Capabilities capabilities;
  base::ListValue exclude_switches;
  exclude_switches.AppendString("switch1");
  exclude_switches.AppendString("switch2");
  base::DictionaryValue caps;
  caps.Set("chromeOptions.excludeSwitches", exclude_switches.DeepCopy());
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(2u, capabilities.exclude_switches.size());
  const std::set<std::string>& switches = capabilities.exclude_switches;
  ASSERT_TRUE(switches.find("switch1") != switches.end());
  ASSERT_TRUE(switches.find("switch2") != switches.end());
}

TEST(ParseCapabilities, UseExistingBrowser) {
  Capabilities capabilities;
  base::DictionaryValue caps;
  caps.SetString("chromeOptions.debuggerAddress", "abc:123");
  Status status = capabilities.Parse(caps);
  ASSERT_TRUE(status.IsOk());
  ASSERT_TRUE(capabilities.IsExistingBrowser());
  ASSERT_EQ("abc", capabilities.debugger_address.host());
  ASSERT_EQ(123, capabilities.debugger_address.port());
}
