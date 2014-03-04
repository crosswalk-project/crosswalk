// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/chrome_launcher.h"

#include <algorithm>
#include <vector>

#include "base/base64.h"
#include "base/basictypes.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/format_macros.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/process/kill.h"
#include "base/process/launch.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/values.h"
#include "chrome/test/chromedriver/chrome/chrome_android_impl.h"
#include "chrome/test/chromedriver/chrome/chrome_desktop_impl.h"
#include "chrome/test/chromedriver/chrome/chrome_existing_impl.h"
#include "chrome/test/chromedriver/chrome/chrome_finder.h"
#include "chrome/test/chromedriver/chrome/device_manager.h"
#include "chrome/test/chromedriver/chrome/devtools_http_client.h"
#include "chrome/test/chromedriver/chrome/embedded_automation_extension.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/chrome/user_data_dir.h"
#include "chrome/test/chromedriver/chrome/version.h"
#include "chrome/test/chromedriver/chrome/web_view.h"
#include "chrome/test/chromedriver/chrome/zip.h"
#include "chrome/test/chromedriver/net/port_server.h"
#include "chrome/test/chromedriver/net/url_request_context_getter.h"
#include "crypto/sha2.h"

#if defined(OS_POSIX)
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace {

const char* kCommonSwitches[] = {
    "ignore-certificate-errors", "metrics-recording-only"};

#if defined(OS_LINUX)
const char* kEnableCrashReport = "enable-crash-reporter-for-testing";
#endif

Status UnpackAutomationExtension(const base::FilePath& temp_dir,
                                 base::FilePath* automation_extension) {
  std::string decoded_extension;
  if (!base::Base64Decode(kAutomationExtension, &decoded_extension))
    return Status(kUnknownError, "failed to base64decode automation extension");

  base::FilePath extension_zip = temp_dir.AppendASCII("internal.zip");
  int size = static_cast<int>(decoded_extension.length());
  if (file_util::WriteFile(extension_zip, decoded_extension.c_str(), size)
      != size) {
    return Status(kUnknownError, "failed to write automation extension zip");
  }

  base::FilePath extension_dir = temp_dir.AppendASCII("internal");
  if (!zip::Unzip(extension_zip, extension_dir))
    return Status(kUnknownError, "failed to unzip automation extension");

  *automation_extension = extension_dir;
  return Status(kOk);
}

Status PrepareCommandLine(int port,
                          const Capabilities& capabilities,
                          CommandLine* prepared_command,
                          base::ScopedTempDir* user_data_dir,
                          base::ScopedTempDir* extension_dir,
                          std::vector<std::string>* extension_bg_pages) {
  base::FilePath program = capabilities.binary;
  if (program.empty()) {
    if (!FindChrome(&program))
      return Status(kUnknownError, "cannot find Chrome binary");
  } else if (!base::PathExists(program)) {
    return Status(kUnknownError,
                  base::StringPrintf("no chrome binary at %" PRFilePath,
                                     program.value().c_str()));
  }
  CommandLine command(program);
  Switches switches;

  // TODO(chrisgao): Add "disable-sync" when chrome 30- is not supported.
  // For chrome 30-, it leads to crash when opening chrome://settings.
  for (size_t i = 0; i < arraysize(kCommonSwitches); ++i)
    switches.SetSwitch(kCommonSwitches[i]);
  switches.SetSwitch("disable-hang-monitor");
  switches.SetSwitch("disable-prompt-on-repost");
  switches.SetSwitch("full-memory-crash-report");
  switches.SetSwitch("no-first-run");
  switches.SetSwitch("disable-background-networking");
  switches.SetSwitch("disable-web-resources");
  switches.SetSwitch("safebrowsing-disable-auto-update");
  switches.SetSwitch("safebrowsing-disable-download-protection");
  switches.SetSwitch("disable-client-side-phishing-detection");
  switches.SetSwitch("disable-component-update");
  switches.SetSwitch("disable-default-apps");
  switches.SetSwitch("enable-logging");
  switches.SetSwitch("logging-level", "1");
  switches.SetSwitch("password-store", "basic");
  switches.SetSwitch("use-mock-keychain");
  switches.SetSwitch("remote-debugging-port", base::IntToString(port));

  for (std::set<std::string>::const_iterator iter =
           capabilities.exclude_switches.begin();
       iter != capabilities.exclude_switches.end();
       ++iter) {
    switches.RemoveSwitch(*iter);
  }
  switches.SetFromSwitches(capabilities.switches);

  if (!switches.HasSwitch("user-data-dir")) {
    command.AppendArg("data:,");
    if (!user_data_dir->CreateUniqueTempDir())
      return Status(kUnknownError, "cannot create temp dir for user data dir");
    switches.SetSwitch("user-data-dir", user_data_dir->path().value());
    Status status = internal::PrepareUserDataDir(
        user_data_dir->path(), capabilities.prefs.get(),
        capabilities.local_state.get());
    if (status.IsError())
      return status;
  }

  if (!extension_dir->CreateUniqueTempDir()) {
    return Status(kUnknownError,
                  "cannot create temp dir for unpacking extensions");
  }
  Status status = internal::ProcessExtensions(capabilities.extensions,
                                              extension_dir->path(),
                                              true,
                                              &switches,
                                              extension_bg_pages);
  if (status.IsError())
    return status;
  switches.AppendToCommandLine(&command);
  *prepared_command = command;
  return Status(kOk);
}

Status WaitForDevToolsAndCheckVersion(
    const NetAddress& address,
    URLRequestContextGetter* context_getter,
    const SyncWebSocketFactory& socket_factory,
    scoped_ptr<DevToolsHttpClient>* user_client) {
  scoped_ptr<DevToolsHttpClient> client(new DevToolsHttpClient(
      address, context_getter, socket_factory));
  base::TimeTicks deadline =
      base::TimeTicks::Now() + base::TimeDelta::FromSeconds(20);
  Status status = client->Init(deadline - base::TimeTicks::Now());
  if (status.IsError())
    return status;
  if (client->build_no() < kMinimumSupportedChromeBuildNo) {
    return Status(kUnknownError, "Chrome version must be >= " +
        GetMinimumSupportedChromeVersion());
  }

  while (base::TimeTicks::Now() < deadline) {
    WebViewsInfo views_info;
    client->GetWebViewsInfo(&views_info);
    for (size_t i = 0; i < views_info.GetSize(); ++i) {
      if (views_info.Get(i).type == WebViewInfo::kPage) {
        *user_client = client.Pass();
        return Status(kOk);
      }
    }
    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(50));
  }
  return Status(kUnknownError, "unable to discover open pages");
}

Status LaunchExistingChromeSession(
    URLRequestContextGetter* context_getter,
    const SyncWebSocketFactory& socket_factory,
    const Capabilities& capabilities,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    scoped_ptr<Chrome>* chrome) {
  Status status(kOk);
  scoped_ptr<DevToolsHttpClient> devtools_client;
  status = WaitForDevToolsAndCheckVersion(
      capabilities.debugger_address, context_getter, socket_factory,
      &devtools_client);
  if (status.IsError()) {
    return Status(kUnknownError, "cannot connect to chrome at " +
                      capabilities.debugger_address.ToString(),
                  status);
  }
  chrome->reset(new ChromeExistingImpl(devtools_client.Pass(),
                                       devtools_event_listeners));
  return Status(kOk);
}

Status LaunchDesktopChrome(
    URLRequestContextGetter* context_getter,
    int port,
    scoped_ptr<PortReservation> port_reservation,
    const SyncWebSocketFactory& socket_factory,
    const Capabilities& capabilities,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    scoped_ptr<Chrome>* chrome) {
  CommandLine command(CommandLine::NO_PROGRAM);
  base::ScopedTempDir user_data_dir;
  base::ScopedTempDir extension_dir;
  std::vector<std::string> extension_bg_pages;
  Status status = PrepareCommandLine(port,
                                     capabilities,
                                     &command,
                                     &user_data_dir,
                                     &extension_dir,
                                     &extension_bg_pages);
  if (status.IsError())
    return status;

  base::LaunchOptions options;

#if defined(OS_LINUX)
  // If minidump path is set in the capability, enable minidump for crashes.
  if (!capabilities.minidump_path.empty()) {
    VLOG(0) << "Minidump generation specified. Will save dumps to: "
            << capabilities.minidump_path;

    options.environ["CHROME_HEADLESS"] = 1;
    options.environ["BREAKPAD_DUMP_LOCATION"] = capabilities.minidump_path;

    if (!command.HasSwitch(kEnableCrashReport))
      command.AppendSwitch(kEnableCrashReport);
  }
#endif

#if !defined(OS_WIN)
  if (!capabilities.log_path.empty())
    options.environ["CHROME_LOG_FILE"] = capabilities.log_path;
  if (capabilities.detach)
    options.new_process_group = true;
#endif

#if defined(OS_POSIX)
  base::FileHandleMappingVector no_stderr;
  int devnull = -1;
  file_util::ScopedFD scoped_devnull(&devnull);
  if (!CommandLine::ForCurrentProcess()->HasSwitch("verbose")) {
    // Redirect stderr to /dev/null, so that Chrome log spew doesn't confuse
    // users.
    devnull = open("/dev/null", O_WRONLY);
    if (devnull == -1)
      return Status(kUnknownError, "couldn't open /dev/null");
    no_stderr.push_back(std::make_pair(devnull, STDERR_FILENO));
    options.fds_to_remap = &no_stderr;
  }
#endif

#if defined(OS_WIN)
  std::string command_string = base::WideToUTF8(command.GetCommandLineString());
#else
  std::string command_string = command.GetCommandLineString();
#endif
  VLOG(0) << "Launching chrome: " << command_string;
  base::ProcessHandle process;
  if (!base::LaunchProcess(command, options, &process))
    return Status(kUnknownError, "chrome failed to start");

  scoped_ptr<DevToolsHttpClient> devtools_client;
  status = WaitForDevToolsAndCheckVersion(
      NetAddress(port), context_getter, socket_factory, &devtools_client);

  if (status.IsError()) {
    int exit_code;
    base::TerminationStatus chrome_status =
        base::GetTerminationStatus(process, &exit_code);
    if (chrome_status != base::TERMINATION_STATUS_STILL_RUNNING) {
      std::string termination_reason;
      switch (chrome_status) {
        case base::TERMINATION_STATUS_NORMAL_TERMINATION:
          termination_reason = "exited normally";
          break;
        case base::TERMINATION_STATUS_ABNORMAL_TERMINATION:
          termination_reason = "exited abnormally";
          break;
        case base::TERMINATION_STATUS_PROCESS_WAS_KILLED:
          termination_reason = "was killed";
          break;
        case base::TERMINATION_STATUS_PROCESS_CRASHED:
          termination_reason = "crashed";
          break;
        default:
          termination_reason = "unknown";
          break;
      }
      return Status(kUnknownError,
                    "Chrome failed to start: " + termination_reason);
    }
    if (!base::KillProcess(process, 0, true)) {
      int exit_code;
      if (base::GetTerminationStatus(process, &exit_code) ==
          base::TERMINATION_STATUS_STILL_RUNNING)
        return Status(kUnknownError, "cannot kill Chrome", status);
    }
    return status;
  }
  scoped_ptr<ChromeDesktopImpl> chrome_desktop(
      new ChromeDesktopImpl(devtools_client.Pass(),
                            devtools_event_listeners,
                            port_reservation.Pass(),
                            process,
                            command,
                            &user_data_dir,
                            &extension_dir));
  for (size_t i = 0; i < extension_bg_pages.size(); ++i) {
    VLOG(0) << "Waiting for extension bg page load: " << extension_bg_pages[i];
    scoped_ptr<WebView> web_view;
    Status status = chrome_desktop->WaitForPageToLoad(
        extension_bg_pages[i], base::TimeDelta::FromSeconds(10), &web_view);
    if (status.IsError()) {
      return Status(kUnknownError,
                    "failed to wait for extension background page to load: " +
                        extension_bg_pages[i],
                    status);
    }
  }
  *chrome = chrome_desktop.Pass();
  return Status(kOk);
}

Status LaunchAndroidChrome(
    URLRequestContextGetter* context_getter,
    int port,
    scoped_ptr<PortReservation> port_reservation,
    const SyncWebSocketFactory& socket_factory,
    const Capabilities& capabilities,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    DeviceManager* device_manager,
    scoped_ptr<Chrome>* chrome) {
  Status status(kOk);
  scoped_ptr<Device> device;
  if (capabilities.android_device_serial.empty()) {
    status = device_manager->AcquireDevice(&device);
  } else {
    status = device_manager->AcquireSpecificDevice(
        capabilities.android_device_serial, &device);
  }
  if (!status.IsOk())
    return status;

  Switches switches(capabilities.switches);
  for (size_t i = 0; i < arraysize(kCommonSwitches); ++i)
    switches.SetSwitch(kCommonSwitches[i]);
  switches.SetSwitch("disable-fre");
  switches.SetSwitch("enable-remote-debugging");
  status = device->StartApp(capabilities.android_package,
                            capabilities.android_activity,
                            capabilities.android_process,
                            switches.ToString(), port);
  if (!status.IsOk()) {
    device->StopApp();
    return status;
  }

  scoped_ptr<DevToolsHttpClient> devtools_client;
  status = WaitForDevToolsAndCheckVersion(NetAddress(port),
                                          context_getter,
                                          socket_factory,
                                          &devtools_client);
  if (status.IsError())
    return status;

  chrome->reset(new ChromeAndroidImpl(devtools_client.Pass(),
                                      devtools_event_listeners,
                                      port_reservation.Pass(),
                                      device.Pass()));
  return Status(kOk);
}

}  // namespace

Status LaunchChrome(
    URLRequestContextGetter* context_getter,
    const SyncWebSocketFactory& socket_factory,
    DeviceManager* device_manager,
    PortServer* port_server,
    PortManager* port_manager,
    const Capabilities& capabilities,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    scoped_ptr<Chrome>* chrome) {
  if (capabilities.IsExistingBrowser()) {
    return LaunchExistingChromeSession(
        context_getter, socket_factory,
        capabilities, devtools_event_listeners, chrome);
  }

  int port = 0;
  scoped_ptr<PortReservation> port_reservation;
  Status port_status(kOk);
  if (port_server)
    port_status = port_server->ReservePort(&port, &port_reservation);
  else
    port_status = port_manager->ReservePort(&port, &port_reservation);
  if (port_status.IsError())
    return Status(kUnknownError, "cannot reserve port for Chrome", port_status);

  if (capabilities.IsAndroid()) {
    return LaunchAndroidChrome(context_getter,
                               port,
                               port_reservation.Pass(),
                               socket_factory,
                               capabilities,
                               devtools_event_listeners,
                               device_manager,
                               chrome);
  } else {
    return LaunchDesktopChrome(context_getter,
                               port,
                               port_reservation.Pass(),
                               socket_factory,
                               capabilities,
                               devtools_event_listeners,
                               chrome);
  }
}

namespace internal {

void ConvertHexadecimalToIDAlphabet(std::string* id) {
  for (size_t i = 0; i < id->size(); ++i) {
    int val;
    if (base::HexStringToInt(base::StringPiece(id->begin() + i,
                                               id->begin() + i + 1),
                             &val)) {
      (*id)[i] = val + 'a';
    } else {
      (*id)[i] = 'a';
    }
  }
}

std::string GenerateExtensionId(const std::string& input) {
  uint8 hash[16];
  crypto::SHA256HashString(input, hash, sizeof(hash));
  std::string output = StringToLowerASCII(base::HexEncode(hash, sizeof(hash)));
  ConvertHexadecimalToIDAlphabet(&output);
  return output;
}

Status GetExtensionBackgroundPage(const base::DictionaryValue* manifest,
                                  const std::string& id,
                                  std::string* bg_page) {
  std::string bg_page_name;
  bool persistent = true;
  manifest->GetBoolean("background.persistent", &persistent);
  const base::Value* unused_value;
  if (manifest->Get("background.scripts", &unused_value))
    bg_page_name = "_generated_background_page.html";
  manifest->GetString("background.page", &bg_page_name);
  manifest->GetString("background_page", &bg_page_name);
  if (bg_page_name.empty() || !persistent)
    return Status(kOk);
  *bg_page = "chrome-extension://" + id + "/" + bg_page_name;
  return Status(kOk);
}

Status ProcessExtension(const std::string& extension,
                        const base::FilePath& temp_dir,
                        base::FilePath* path,
                        std::string* bg_page) {
  // Decodes extension string.
  // Some WebDriver client base64 encoders follow RFC 1521, which require that
  // 'encoded lines be no more than 76 characters long'. Just remove any
  // newlines.
  std::string extension_base64;
  RemoveChars(extension, "\n", &extension_base64);
  std::string decoded_extension;
  if (!base::Base64Decode(extension_base64, &decoded_extension))
    return Status(kUnknownError, "cannot base64 decode");

  // Get extension's ID from public key in crx file.
  // Assumes crx v2. See http://developer.chrome.com/extensions/crx.html.
  std::string key_len_str = decoded_extension.substr(8, 4);
  if (key_len_str.size() != 4)
    return Status(kUnknownError, "cannot extract public key length");
  uint32 key_len = *reinterpret_cast<const uint32*>(key_len_str.c_str());
  std::string public_key = decoded_extension.substr(16, key_len);
  if (key_len != public_key.size())
    return Status(kUnknownError, "invalid public key length");
  std::string public_key_base64;
  if (!base::Base64Encode(public_key, &public_key_base64))
    return Status(kUnknownError, "cannot base64 encode public key");
  std::string id = GenerateExtensionId(public_key);

  // Unzip the crx file.
  base::ScopedTempDir temp_crx_dir;
  if (!temp_crx_dir.CreateUniqueTempDir())
    return Status(kUnknownError, "cannot create temp dir");
  base::FilePath extension_crx = temp_crx_dir.path().AppendASCII("temp.crx");
  int size = static_cast<int>(decoded_extension.length());
  if (file_util::WriteFile(extension_crx, decoded_extension.c_str(), size) !=
      size) {
    return Status(kUnknownError, "cannot write file");
  }
  base::FilePath extension_dir = temp_dir.AppendASCII("extension_" + id);
  if (!zip::Unzip(extension_crx, extension_dir))
    return Status(kUnknownError, "cannot unzip");

  // Parse the manifest and set the 'key' if not already present.
  base::FilePath manifest_path(extension_dir.AppendASCII("manifest.json"));
  std::string manifest_data;
  if (!base::ReadFileToString(manifest_path, &manifest_data))
    return Status(kUnknownError, "cannot read manifest");
  scoped_ptr<base::Value> manifest_value(base::JSONReader::Read(manifest_data));
  base::DictionaryValue* manifest;
  if (!manifest_value || !manifest_value->GetAsDictionary(&manifest))
    return Status(kUnknownError, "invalid manifest");
  if (!manifest->HasKey("key")) {
    manifest->SetString("key", public_key_base64);
    base::JSONWriter::Write(manifest, &manifest_data);
    if (file_util::WriteFile(
            manifest_path, manifest_data.c_str(), manifest_data.size()) !=
        static_cast<int>(manifest_data.size())) {
      return Status(kUnknownError, "cannot add 'key' to manifest");
    }
  }

  // Get extension's background page URL, if there is one.
  std::string bg_page_tmp;
  Status status = GetExtensionBackgroundPage(manifest, id, &bg_page_tmp);
  if (status.IsError())
    return status;

  *path = extension_dir;
  if (bg_page_tmp.size())
    *bg_page = bg_page_tmp;
  return Status(kOk);
}

void UpdateExtensionSwitch(Switches* switches,
                           const char name[],
                           const base::FilePath::StringType& extension) {
  base::FilePath::StringType value = switches->GetSwitchValueNative(name);
  if (value.length())
    value += FILE_PATH_LITERAL(",");
  value += extension;
  switches->SetSwitch(name, value);
}

Status ProcessExtensions(const std::vector<std::string>& extensions,
                         const base::FilePath& temp_dir,
                         bool include_automation_extension,
                         Switches* switches,
                         std::vector<std::string>* bg_pages) {
  std::vector<std::string> bg_pages_tmp;
  std::vector<base::FilePath::StringType> extension_paths;
  for (size_t i = 0; i < extensions.size(); ++i) {
    base::FilePath path;
    std::string bg_page;
    Status status = ProcessExtension(extensions[i], temp_dir, &path, &bg_page);
    if (status.IsError()) {
      return Status(
          kUnknownError,
          base::StringPrintf("cannot process extension #%" PRIuS, i + 1),
          status);
    }
    extension_paths.push_back(path.value());
    if (bg_page.length())
      bg_pages_tmp.push_back(bg_page);
  }

  if (include_automation_extension) {
    base::FilePath automation_extension;
    Status status = UnpackAutomationExtension(temp_dir, &automation_extension);
    if (status.IsError())
      return status;
    if (switches->HasSwitch("disable-extensions")) {
      UpdateExtensionSwitch(switches, "load-component-extension",
                            automation_extension.value());
    } else {
      extension_paths.push_back(automation_extension.value());
    }
  }

  if (extension_paths.size()) {
    base::FilePath::StringType extension_paths_value = JoinString(
        extension_paths, FILE_PATH_LITERAL(','));
    UpdateExtensionSwitch(switches, "load-extension", extension_paths_value);
  }
  bg_pages->swap(bg_pages_tmp);
  return Status(kOk);
}

Status WritePrefsFile(
    const std::string& template_string,
    const base::DictionaryValue* custom_prefs,
    const base::FilePath& path) {
  int code;
  std::string error_msg;
  scoped_ptr<base::Value> template_value(base::JSONReader::ReadAndReturnError(
          template_string, 0, &code, &error_msg));
  base::DictionaryValue* prefs;
  if (!template_value || !template_value->GetAsDictionary(&prefs)) {
    return Status(kUnknownError,
                  "cannot parse internal JSON template: " + error_msg);
  }

  if (custom_prefs) {
    for (base::DictionaryValue::Iterator it(*custom_prefs); !it.IsAtEnd();
         it.Advance()) {
      prefs->Set(it.key(), it.value().DeepCopy());
    }
  }

  std::string prefs_str;
  base::JSONWriter::Write(prefs, &prefs_str);
  VLOG(0) << "Populating " << path.BaseName().value()
          << " file: " << PrettyPrintValue(*prefs);
  if (static_cast<int>(prefs_str.length()) != file_util::WriteFile(
          path, prefs_str.c_str(), prefs_str.length())) {
    return Status(kUnknownError, "failed to write prefs file");
  }
  return Status(kOk);
}

Status PrepareUserDataDir(
    const base::FilePath& user_data_dir,
    const base::DictionaryValue* custom_prefs,
    const base::DictionaryValue* custom_local_state) {
  base::FilePath default_dir = user_data_dir.AppendASCII("Default");
  if (!file_util::CreateDirectory(default_dir))
    return Status(kUnknownError, "cannot create default profile directory");

  Status status = WritePrefsFile(
      kPreferences,
      custom_prefs,
      default_dir.AppendASCII("Preferences"));
  if (status.IsError())
    return status;

  status = WritePrefsFile(
      kLocalState,
      custom_local_state,
      user_data_dir.AppendASCII("Local State"));
  if (status.IsError())
    return status;

  // Write empty "First Run" file, otherwise Chrome will wipe the default
  // profile that was written.
  if (file_util::WriteFile(
          user_data_dir.AppendASCII("First Run"), "", 0) != 0) {
    return Status(kUnknownError, "failed to write first run file");
  }
  return Status(kOk);
}

}  // namespace internal
