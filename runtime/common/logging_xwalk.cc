// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"

// Need to include this before most other files because it defines
// IPC_MESSAGE_LOG_ENABLED. We need to use it to define
// IPC_MESSAGE_MACROS_LOG_ENABLED so render_messages.h will generate the
// ViewMsgLog et al. functions.
#include "ipc/ipc_message.h"

// On Windows, the about:ipc dialog shows IPCs; on POSIX, we hook up a
// logger in this file.  (We implement about:ipc on Mac but implement
// the loggers here anyway).  We need to do this real early to be sure
// IPC_MESSAGE_MACROS_LOG_ENABLED doesn't get undefined.
#if defined(OS_POSIX) && defined(IPC_MESSAGE_LOG_ENABLED)
#define IPC_MESSAGE_MACROS_LOG_ENABLED
#include "content/public/common/content_ipc_logging.h"
#define IPC_LOG_TABLE_ADD_ENTRY(msg_id, logger) \
    content::RegisterIPCLogger(msg_id, logger)
#endif

#if defined(OS_WIN)
#include <windows.h>
#endif

#include "xwalk/runtime/common/logging_xwalk.h"

#include <fstream>  // NOLINT
#include <string>  // NOLINT

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/debug/debugger.h"
#include "base/debug/dump_without_crashing.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/common/content_switches.h"
#include "ipc/ipc_logging.h"
#include "xwalk/runtime/common/xwalk_paths.h"

#if defined(OS_WIN)
#include <initguid.h>
#include "base/logging_win.h"
#endif

namespace {

// This should be true for exactly the period between the end of
// InitXwalkLogging() and the beginning of CleanupXwalkLogging().
bool xwalk_logging_initialized_ = false;

// Set if we called InitXwalkLogging() but failed to initialize.
bool xwalk_logging_failed_ = false;

// This should be true for exactly the period between the end of
// InitXwalkLogging() and the beginning of CleanupXwalkLogging().
bool xwalk_logging_redirected_ = false;

#if defined(OS_WIN)
// {7FE69228-633E-4f06-80C1-527FEA23E3A7}
const GUID kXwalkTraceProviderName = {
    0x7fe69228, 0x633e, 0x4f06,
        { 0x80, 0xc1, 0x52, 0x7f, 0xea, 0x23, 0xe3, 0xa7 } };
#endif

}  // anonymous namespace

namespace logging {

LoggingDestination DetermineLogMode(const base::CommandLine& command_line) {
  // only use OutputDebugString in debug mode
#ifdef NDEBUG
  bool enable_logging = false;
  const char *kInvertLoggingSwitch = switches::kEnableLogging;
  const logging::LoggingDestination kDefaultLoggingMode = logging::LOG_TO_FILE;
#else
  bool enable_logging = true;
  const char *kInvertLoggingSwitch = switches::kDisableLogging;
  const logging::LoggingDestination kDefaultLoggingMode = logging::LOG_TO_ALL;
#endif

  if (command_line.HasSwitch(kInvertLoggingSwitch))
    enable_logging = !enable_logging;

  logging::LoggingDestination log_mode;
  if (enable_logging) {
    // Let --enable-logging=stderr force only stderr, particularly useful for
    // non-debug builds where otherwise you can't get logs to stderr at all.
    if (command_line.GetSwitchValueASCII(switches::kEnableLogging) == "stderr")
      log_mode = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    else
      log_mode = kDefaultLoggingMode;
  } else {
    log_mode = logging::LOG_NONE;
  }
  return log_mode;
}

void InitXwalkLogging(const base::CommandLine& command_line,
                      OldFileDeletionState delete_old_log_file) {
  DCHECK(!xwalk_logging_initialized_) <<
      "Attempted to initialize logging when it was already initialized.";

  LoggingDestination logging_dest = DetermineLogMode(command_line);
  LogLockingState log_locking_state = LOCK_LOG_FILE;
  base::FilePath log_path;

  // Don't resolve the log path unless we need to. Otherwise we leave an open
  // ALPC handle after sandbox lockdown on Windows.
  if ((logging_dest & LOG_TO_FILE) != 0) {
    log_path = GetLogFileName();
  } else {
    log_locking_state = DONT_LOCK_LOG_FILE;
  }

  logging::LoggingSettings settings;
  settings.logging_dest = logging_dest;
  settings.log_file = log_path.value().c_str();
  settings.lock_log = log_locking_state;
  settings.delete_old = delete_old_log_file;
  bool success = logging::InitLogging(settings);

  if (!success) {
    DPLOG(ERROR) << "Unable to initialize logging to " << log_path.value();
    xwalk_logging_failed_ = true;
    return;
  }

  // Default to showing error dialogs.
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kNoErrorDialogs))
    logging::SetShowErrorDialogs(true);

  // we want process and thread IDs because we have a lot of things running
  logging::SetLogItems(true,  // enable_process_id
                       true,  // enable_thread_id
                       true,  // enable_timestamp
                       false);  // enable_tickcount

  // Use a minimum log level if the command line asks for one. Ignore this
  // switch if there's vlog level switch present too (as both of these switches
  // refer to the same underlying log level, and the vlog level switch has
  // already been processed inside logging::InitLogging). If there is neither
  // log level nor vlog level specified, then just leave the default level
  // (INFO).
  if (command_line.HasSwitch(switches::kLoggingLevel) &&
      logging::GetMinLogLevel() >= 0) {
    std::string log_level =
        command_line.GetSwitchValueASCII(switches::kLoggingLevel);
    int level = 0;
    if (base::StringToInt(log_level, &level) && level >= 0 &&
        level < LOG_NUM_SEVERITIES) {
      logging::SetMinLogLevel(level);
    } else {
      DLOG(WARNING) << "Bad log level: " << log_level;
    }
  }

#if defined(OS_WIN)
  // Enable trace control and transport through event tracing for Windows.
  logging::LogEventProvider::Initialize(kXwalkTraceProviderName);
#endif

  xwalk_logging_initialized_ = true;
}

// This is a no-op, but we'll keep it around in case
// we need to do more cleanup in the future.
void CleanupXwalkLogging() {
  if (xwalk_logging_failed_)
    return;  // We failed to initiailize logging, no cleanup.

  DCHECK(xwalk_logging_initialized_) <<
      "Attempted to clean up logging when it wasn't initialized.";

  CloseLogFile();

  xwalk_logging_initialized_ = false;
  xwalk_logging_redirected_ = false;
}

base::FilePath GetLogFileName() {
  const base::FilePath log_filename(FILE_PATH_LITERAL("xwalk_debug.log"));
  base::FilePath log_path;

  if (PathService::Get(xwalk::DIR_LOGS, &log_path)) {
    log_path = log_path.Append(log_filename);
    return log_path;
  } else {
    // error with path service, just use some default file somewhere
    return log_filename;
  }
}

}  // namespace logging
