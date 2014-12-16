// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_platform_util.h"

#include "base/bind.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/process/kill.h"
#include "base/process/launch.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_proxy.h"
#include "url/gurl.h"

namespace platform_util {
namespace {
// The system default web browser path.
// In some Tizen releases, there exists a system browser called 'MiniBrowser',
// which we can use to open an external link from a web app.
const char kWebBrowserPath[] = "/usr/bin/MiniBrowser";

typedef base::Callback<void (const GURL&, dbus::MessageWriter*)> ParamsWriter;

struct ProtocolDBusServiceInfo {
  const char* scheme;
  const char* service;
  const char* interface;
  const char* method;
  const char* object_path;
  ParamsWriter params_writer;
};

void ParseAndWriteTelParams(const GURL& url,
                            dbus::MessageWriter* dbus_writer) {
  // Phone number
  dbus_writer->AppendString(url.GetContent());
  // Always don't auto dial
  dbus_writer->AppendBool(false);
}

void ParseAndWriteMailParams(const GURL& uri,
                             dbus::MessageWriter* dbus_writer) {
}

void ParseAndWriteSmsParams(const GURL& url,
                            dbus::MessageWriter* dbus_writer) {
  // Support the case sms:12345678?body=Hello
  std::string query = url.query();
  std::string content = url.GetContent();
  // Extract the phone number
  const std::string number = content.substr(0, content.find(query) - 1);
  // Remove "body="
  const std::string body = query.erase(0, 5);

  dbus_writer->AppendString(number);
  dbus_writer->AppendString(body);
  // Always don't auto dial
  dbus_writer->AppendBool(false);
}

const ProtocolDBusServiceInfo protocol_dbus_services[] = {
  {"tel", "org.tizen.dialer", "org.tizen.dialer.Control", "Dial", "/",
      base::Bind(ParseAndWriteTelParams)},
  {"mailto", "org.tizen.email_service", "org.tizen.email_service", "Launch",
      "/org/tizen/email_service",
      base::Bind(ParseAndWriteMailParams)},
  {"sms", "org.tizen.dialer", "org.tizen.dialer.Control", "Send", "/",
      base::Bind(ParseAndWriteSmsParams)},
};

bool CallDbusService(const ProtocolDBusServiceInfo& info,
                     const GURL& url) {
  const dbus::ObjectPath dbus_path(info.object_path);

  dbus::Bus::Options options;
  scoped_refptr<dbus::Bus> bus(new dbus::Bus(options));
  dbus::ObjectProxy* app_proxy =
      bus->GetObjectProxy(info.service, dbus_path);
  if (!app_proxy) {
    VLOG(1) << "app_proxy failed.";
    return false;
  }

  dbus::MethodCall method_call(info.interface, info.method);
  dbus::MessageWriter writer(&method_call);
  info.params_writer.Run(url, &writer);

  app_proxy->CallMethod(&method_call,
                        dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
                        dbus::ObjectProxy::EmptyResponseCallback());

  return true;
}
}  // namespace

bool HandleExternalProtocol(const GURL& url) {
  for (size_t i = 0; i < arraysize(protocol_dbus_services); ++i) {
    const ProtocolDBusServiceInfo& info = protocol_dbus_services[i];
    if (url.SchemeIs(info.scheme)) {
      return CallDbusService(info, url);
    }
  }
  return false;
}

void OpenExternal(const GURL& url) {
  if (url.SchemeIsHTTPOrHTTPS()) {
    LOG(INFO) << "Open in WebBrowser.";
    std::vector<std::string> argv;
    if (base::PathExists(base::FilePath(kWebBrowserPath)))
      argv.push_back(kWebBrowserPath);
    else
      argv.push_back("xwalk-launcher");
    argv.push_back(url.spec());
    base::ProcessHandle handle;

    if (base::LaunchProcess(argv, base::LaunchOptions(), &handle))
      base::EnsureProcessGetsReaped(handle);
  } else if (!HandleExternalProtocol(url)) {
    LOG(ERROR) << "Can not handle url: " << url.spec();
  }
}

}  // namespace platform_util
