// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_SESSION_COMMANDS_H_
#define CHROME_TEST_CHROMEDRIVER_SESSION_COMMANDS_H_

#include <string>

#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/test/chromedriver/command.h"
#include "xwalk/test/chromedriver/net/sync_websocket_factory.h"

namespace base {
class DictionaryValue;
class Value;
}

class DeviceManager;
class PortManager;
class PortServer;
struct Session;
class Status;
class URLRequestContextGetter;

struct InitSessionParams {
  InitSessionParams(scoped_refptr<URLRequestContextGetter> context_getter,
                    const SyncWebSocketFactory& socket_factory,
                    DeviceManager* device_manager,
                    PortServer* port_server,
                    PortManager* port_manager);
  ~InitSessionParams();

  scoped_refptr<URLRequestContextGetter> context_getter;
  SyncWebSocketFactory socket_factory;
  DeviceManager* device_manager;
  PortServer* port_server;
  PortManager* port_manager;
};

// Initializes a session.
Status ExecuteInitSession(
    const InitSessionParams& bound_params,
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

// Quits a session.
Status ExecuteQuit(
    bool allow_detach,
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

// Gets the capabilities of a particular session.
Status ExecuteGetSessionCapabilities(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

// Retrieve the handle of the target window.
Status ExecuteGetCurrentWindowHandle(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

// Close the target window.
Status ExecuteClose(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

// Retrieve the list of all window handles available to the session.
Status ExecuteGetWindowHandles(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

// Change target window to another. The window to target at may be specified by
// its server assigned window handle, or by the value of its name attribute.
Status ExecuteSwitchToWindow(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

// Configure the amount of time that a particular type of operation can execute
// for before they are aborted and a timeout error is returned to the client.
Status ExecuteSetTimeout(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

// Set the timeout for asynchronous scripts.
Status ExecuteSetScriptTimeout(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

// Set the amount of time the driver should wait when searching for elements.
Status ExecuteImplicitlyWait(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

Status ExecuteIsLoading(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

Status ExecuteGetLocation(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

Status ExecuteGetWindowPosition(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

Status ExecuteSetWindowPosition(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

Status ExecuteGetWindowSize(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

Status ExecuteSetWindowSize(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

Status ExecuteMaximizeWindow(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

Status ExecuteGetAvailableLogTypes(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

Status ExecuteGetLog(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

Status ExecuteUploadFile(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value);

#endif  // CHROME_TEST_CHROMEDRIVER_SESSION_COMMANDS_H_
