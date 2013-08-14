// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_web_contents_handler.h"

#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(
    xwalk::extensions::XWalkExtensionWebContentsHandler);

namespace xwalk {
namespace extensions {

// Keeps track of runners for a WebContentsHandler and their associated
// information (name and frame id), providing queries useful for handler
// operation. One WebContentsHandler handles runners for all its frames.
class RunnerStore {
 public:
  RunnerStore() {}
  ~RunnerStore() { DeleteAllFrames(); }

  void AddFrame(int64_t frame_id);
  void AddRunner(int64_t frame_id, XWalkExtensionRunner* runner);

  XWalkExtensionRunner* GetRunnerByFrameAndName(int64_t frame_id,
                                                const std::string& name);
  int64_t GetFrameForRunner(const XWalkExtensionRunner* runner) {
    return frame_for_runner_[runner];
  }

  void DeleteFrame(int64_t frame_id);
  void DeleteAllFrames();

 private:
  typedef std::map<std::string, XWalkExtensionRunner*> RunnerMap;
  typedef std::map<int64_t, RunnerMap*> RunnersForFrameMap;
  RunnersForFrameMap runners_for_frame_;

  void DeleteRunnerMap(RunnerMap* runners);

  typedef std::map<const XWalkExtensionRunner*, int64_t> FrameForRunnerMap;
  FrameForRunnerMap frame_for_runner_;
};

void RunnerStore::AddFrame(int64_t frame_id) {
  CHECK(runners_for_frame_.find(frame_id) == runners_for_frame_.end());
  runners_for_frame_[frame_id] = new RunnerMap;
}

void RunnerStore::AddRunner(int64_t frame_id, XWalkExtensionRunner* runner) {
  CHECK(runners_for_frame_.find(frame_id) != runners_for_frame_.end());
  RunnerMap* runners = runners_for_frame_[frame_id];
  (*runners)[runner->extension_name()] = runner;
  frame_for_runner_[runner] = frame_id;
}

XWalkExtensionRunner* RunnerStore::GetRunnerByFrameAndName(
    int64_t frame_id, const std::string& name) {
  RunnerMap* runners = runners_for_frame_[frame_id];
  RunnerMap::iterator it = runners->find(name);
  if (it == runners->end())
    return NULL;
  return it->second;
}

void RunnerStore::DeleteFrame(int64_t frame_id) {
  RunnersForFrameMap::iterator it = runners_for_frame_.find(frame_id);
  if (it == runners_for_frame_.end())
    return;
  DeleteRunnerMap(it->second);
  delete it->second;
  runners_for_frame_.erase(it);
}

void RunnerStore::DeleteAllFrames() {
  RunnersForFrameMap::iterator it = runners_for_frame_.begin();
  for (; it != runners_for_frame_.end(); ++it) {
    DeleteRunnerMap(it->second);
    delete it->second;
  }
  runners_for_frame_.clear();
}

void RunnerStore::DeleteRunnerMap(RunnerMap* runners) {
  RunnerMap::iterator it = runners->begin();
  for (; it != runners->end(); ++it) {
    frame_for_runner_.erase(it->second);
    delete it->second;
  }
  runners->clear();
}


XWalkExtensionWebContentsHandler::XWalkExtensionWebContentsHandler(
    content::WebContents* contents)
    : WebContentsObserver(contents),
      runners_(new RunnerStore) {}

XWalkExtensionWebContentsHandler::~XWalkExtensionWebContentsHandler() {}

void XWalkExtensionWebContentsHandler::AttachExtensionRunner(
    int64_t frame_id, XWalkExtensionRunner* runner) {
  runners_->AddRunner(frame_id, runner);
}

void XWalkExtensionWebContentsHandler::HandleMessageFromContext(
    const XWalkExtensionRunner* runner, scoped_ptr<base::Value> msg) {
  base::ListValue list;
  list.Append(msg.release());

  int64_t frame_id = runners_->GetFrameForRunner(runner);
  Send(new XWalkViewMsg_PostMessage(web_contents()->GetRoutingID(), frame_id,
                                    runner->extension_name(), list));
}

bool XWalkExtensionWebContentsHandler::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionWebContentsHandler, message)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_PostMessage, OnPostMessage)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_SendSyncMessage, OnSendSyncMessage)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_DidCreateScriptContext,
                        DidCreateScriptContext)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_WillReleaseScriptContext,
                        WillReleaseScriptContext)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionWebContentsHandler::OnPostMessage(
    int64_t frame_id, const std::string& extension_name,
    const base::ListValue& msg) {
  XWalkExtensionRunner* runner =
      runners_->GetRunnerByFrameAndName(frame_id, extension_name);
  if (!runner) {
    LOG(WARNING) << "Couldn't handle message for unregistered extension '"
                 << extension_name << "'";
    return;
  }

  // The const_cast is needed to remove the only Value contained by the
  // ListValue (which is solely used as wrapper, since Value doesn't
  // have param traits for serialization) and we pass the ownership to to
  // HandleMessage. It is safe to do this because the |msg| won't be used
  // anywhere else when this function returns. Saves a DeepCopy(), which
  // can be costly depending on the size of Value.
  base::Value* value;
  const_cast<base::ListValue*>(&msg)->Remove(0, &value);
  runner->PostMessageToContext(scoped_ptr<base::Value>(value));
}

void XWalkExtensionWebContentsHandler::OnSendSyncMessage(
    int64_t frame_id, const std::string& extension_name,
    const base::ListValue& msg, base::ListValue* result) {
  XWalkExtensionRunner* runner =
      runners_->GetRunnerByFrameAndName(frame_id, extension_name);
  if (!runner) {
    LOG(WARNING) << "Couldn't handle message for unregistered extension '"
                 << extension_name << "'";
    return;
  }

  base::Value* value;
  const_cast<base::ListValue*>(&msg)->Remove(0, &value);

  scoped_ptr<base::Value> resultValue =
      runner->SendSyncMessageToContext(scoped_ptr<base::Value>(value));

  result->Append(resultValue.release());
}

namespace {

const GURL kAboutBlankURL = GURL("about:blank");

}

void XWalkExtensionWebContentsHandler::DidCreateScriptContext(
    int64_t frame_id) {
  // TODO(cmarcelo): We will create runners on demand, this will allow us get
  // rid of this check.
  if (web_contents()->GetURL() != kAboutBlankURL) {
    runners_->AddFrame(frame_id);
    extension_service_->CreateRunnersForHandler(this, frame_id);
  }
}

void XWalkExtensionWebContentsHandler::WillReleaseScriptContext(
    int64_t frame_id) {
  runners_->DeleteFrame(frame_id);
}

}  // namespace extensions
}  // namespace xwalk
