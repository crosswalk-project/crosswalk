// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/wifidirect/wifidirect_extension.h"

#include <algorithm>
#include <map>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/child_process_security_policy.h"
#include "grit/xwalk_resources.h"
#include "ui/base/resource/resource_bundle.h"

namespace xwalk {
namespace experimental {

WifiDirectExtension::WifiDirectExtension(
    content::RenderProcessHost* host) {
  host_ = host;
  set_name("xwalk.experimental.wifidirect");
  set_javascript_api(
      ResourceBundle::GetSharedInstance().GetRawDataResource(
          IDR_XWALK_WIFIDIRECT_API).as_string());
}

WifiDirectExtension::~WifiDirectExtension() {}

XWalkExtensionInstance* WifiDirectExtension::CreateInstance() {
  return new WifiDirectInstance(host_);
}

WifiDirectInstance::WifiDirectInstance(
    content::RenderProcessHost* host)
    : handler_(this),
      host_(host) {
}

void WifiDirectInstance::HandleMessage(scoped_ptr<base::Value> msg) {
    base::DictionaryValue* jsonInput = NULL;
    std::string message;
    std::string cmd;
    msg->GetAsString(&message);
	scoped_ptr<base::Value> parsed_message = base::JSONReader::Read(message);
    if (!parsed_message ||
        !parsed_message->GetAsDictionary(&jsonInput) ||
        !jsonInput->GetString("cmd", &cmd)) {
        LOG(ERROR) << "Invalid message format:" << message;
        return;
    }

    int asyncCallId;
    if (!jsonInput->GetInteger("asyncCallId", &asyncCallId)) {
        LOG(ERROR) << "Invalid async call id.";
        return;
    }

	scoped_refptr<WifiDirectTaskRunner> runner(
        new WifiDirectTaskRunner(host_->GetID(),
            parsed_message.Pass(),
            this));

	runner->DoTask();
}

void WifiDirectInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
}

WifiDirectTaskRunner::WifiDirectTaskRunner(
    int process_id,
    scoped_ptr<base::Value> msg,
    XWalkExtensionInstance* instance)
    : process_id_(process_id),
    msg_(msg.Pass()),
    instance_(instance) {
}

void WifiDirectTaskRunner::DoTask() {
    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&WifiDirectTaskRunner::WifiDirectTaskAndSendResponse, this));
}

void WifiDirectTaskRunner::WifiDirectTaskAndSendResponse() {
    CHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    base::DictionaryValue* jsonInput = NULL;
    std::string cmd;
    if (!msg_->GetAsDictionary(&jsonInput) ||
        !jsonInput->GetString("cmd", &cmd)) {
        sendErrorResponse("Invalid message format:");
        return;
    }

    if (cmd == "init") {
        init();
    }
}

void WifiDirectTaskRunner::sendErrorResponse(const std::string& message) {
    scoped_ptr<base::DictionaryValue> out(new base::DictionaryValue());
    scoped_ptr<base::DictionaryValue> data(new base::DictionaryValue());
    scoped_ptr<base::DictionaryValue> error(new base::DictionaryValue());
    base::DictionaryValue* jsonInput = NULL;
    int asyncCallId = -1;
    if (!msg_->GetAsDictionary(&jsonInput) ||
        !jsonInput->GetInteger("cmd", &asyncCallId)) {
        LOG(ERROR) << message;
        return;
    }
    out->Set("data", data.Pass());
    error->SetString("mesage", message);
    error->SetInteger("errorCode", 0);
    data->Set("error", error.Pass());

    out->SetInteger("asyncCallId", asyncCallId);
    std::string msg_string;
    base::JSONWriter::Write(*out, &msg_string);
    instance_->PostMessageToJS(
        scoped_ptr<base::Value>(
            new base::StringValue(msg_string)));
}


#if !defined(OS_WIN)
void WifiDirectTaskRunner::init() {}
#endif

}  // namespace experimental
}  // namespace xwalk
