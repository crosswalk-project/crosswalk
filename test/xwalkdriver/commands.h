// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_COMMANDS_H_
#define XWALK_TEST_XWALKDRIVER_COMMANDS_H_

#include <string>

#include "base/callback_forward.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/test/xwalkdriver/command.h"
#include "xwalk/test/xwalkdriver/session_thread_map.h"

namespace base {
class DictionaryValue;
class Value;
}

struct Session;
class Status;

// Gets status/info about XwalkDriver.
void ExecuteGetStatus(
    const base::DictionaryValue& params,
    const std::string& session_id,
    const CommandCallback& callback);

// Creates a new session.
void ExecuteCreateSession(
    SessionThreadMap* session_thread_map,
    const Command& init_session_cmd,
    const base::DictionaryValue& params,
    const std::string& session_id,
    const CommandCallback& callback);

// Quits all sessions.
void ExecuteQuitAll(
    const Command& quit_command,
    SessionThreadMap* session_thread_map,
    const base::DictionaryValue& params,
    const std::string& session_id,
    const CommandCallback& callback);

typedef base::Callback<Status(
    Session* session,
    const base::DictionaryValue&,
    scoped_ptr<base::Value>*)> SessionCommand;

// Executes a given session command, after acquiring access to the appropriate
// session.
void ExecuteSessionCommand(
    SessionThreadMap* session_thread_map,
    const char* command_name,
    const SessionCommand& command,
    bool return_ok_without_session,
    const base::DictionaryValue& params,
    const std::string& session_id,
    const CommandCallback& callback);

namespace internal {
void CreateSessionOnSessionThreadForTesting(const std::string& id);
}  // namespace internal

#endif  // XWALK_TEST_XWALKDRIVER_COMMANDS_H_
