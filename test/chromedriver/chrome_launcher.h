// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_LAUNCHER_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_LAUNCHER_H_

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "xwalk/test/chromedriver/capabilities.h"
#include "xwalk/test/chromedriver/net/sync_websocket_factory.h"

class CommandLine;
class DevToolsEventListener;

namespace base {
class DictionaryValue;
class FilePath;
}

class Chrome;
class DeviceManager;
class PortManager;
class PortServer;
class Status;
class URLRequestContextGetter;

Status LaunchChrome(
    URLRequestContextGetter* context_getter,
    const SyncWebSocketFactory& socket_factory,
    DeviceManager* device_manager,
    PortServer* port_server,
    PortManager* port_manager,
    const Capabilities& capabilities,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    scoped_ptr<Chrome>* chrome);

namespace internal {
Status ProcessExtensions(const std::vector<std::string>& extensions,
                         const base::FilePath& temp_dir,
                         bool include_automation_extension,
                         Switches* switches,
                         std::vector<std::string>* bg_pages);
Status PrepareUserDataDir(
    const base::FilePath& user_data_dir,
    const base::DictionaryValue* custom_prefs,
    const base::DictionaryValue* custom_local_state);
}  // namespace internal

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_LAUNCHER_H_
