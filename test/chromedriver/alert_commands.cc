// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/alert_commands.h"

#include "base/callback.h"
#include "base/values.h"
#include "xwalk/test/chromedriver/chrome/chrome.h"
#include "xwalk/test/chromedriver/chrome/devtools_client.h"
#include "xwalk/test/chromedriver/chrome/javascript_dialog_manager.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "xwalk/test/chromedriver/chrome/web_view.h"
#include "xwalk/test/chromedriver/session.h"

Status ExecuteAlertCommand(
     const AlertCommand& alert_command,
     Session* session,
     const base::DictionaryValue& params,
     scoped_ptr<base::Value>* value) {
  WebView* web_view = NULL;
  Status status = session->GetTargetWindow(&web_view);
  if (status.IsError())
    return status;

  status = web_view->ConnectIfNecessary();
  if (status.IsError())
    return status;

  status = web_view->HandleReceivedEvents();
  if (status.IsError())
    return status;

  status = web_view->WaitForPendingNavigations(
      session->GetCurrentFrameId(), session->page_load_timeout, true);
  if (status.IsError() && status.code() != kUnexpectedAlertOpen)
    return status;

  return alert_command.Run(session, web_view, params, value);
}

Status ExecuteGetAlert(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  value->reset(base::Value::CreateBooleanValue(
      web_view->GetJavaScriptDialogManager()->IsDialogOpen()));
  return Status(kOk);
}

Status ExecuteGetAlertText(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string message;
  Status status =
      web_view->GetJavaScriptDialogManager()->GetDialogMessage(&message);
  if (status.IsError())
    return status;
  value->reset(base::Value::CreateStringValue(message));
  return Status(kOk);
}

Status ExecuteSetAlertValue(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string text;
  if (!params.GetString("text", &text))
    return Status(kUnknownError, "missing or invalid 'text'");

  if (!web_view->GetJavaScriptDialogManager()->IsDialogOpen())
    return Status(kNoAlertOpen);

  session->prompt_text.reset(new std::string(text));
  return Status(kOk);
}

Status ExecuteAcceptAlert(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  Status status = web_view->GetJavaScriptDialogManager()
      ->HandleDialog(true, session->prompt_text.get());
  session->prompt_text.reset();
  return status;
}

Status ExecuteDismissAlert(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  Status status = web_view->GetJavaScriptDialogManager()
      ->HandleDialog(false, session->prompt_text.get());
  session->prompt_text.reset();
  return status;
}
