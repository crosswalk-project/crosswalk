// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_web_contents_handler.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "xwalk/extensions/browser/xwalk_extension_message_filter.h"
#include "xwalk/extensions/browser/xwalk_extension_runner_store.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(
    xwalk::extensions::XWalkExtensionWebContentsHandler);

using content::BrowserThread;

namespace xwalk {
namespace extensions {

XWalkExtensionWebContentsHandler::XWalkExtensionWebContentsHandler(
    content::WebContents* contents)
    : web_contents_(contents),
      render_process_host_(NULL),
      message_filter_(NULL),
      runners_(new XWalkExtensionRunnerStore) {}

XWalkExtensionWebContentsHandler::~XWalkExtensionWebContentsHandler() {
  // The RunnerStore lives in the IO Thread and it can
  // only be deleted from there.
  BrowserThread::DeleteSoon(BrowserThread::IO, FROM_HERE, runners_.release());
}

void XWalkExtensionWebContentsHandler::AttachExtensionRunner(
    int64_t frame_id, XWalkExtensionRunner* runner) {
  runners_->AddRunner(frame_id, runner);
}

void XWalkExtensionWebContentsHandler::set_render_process_host(
    content::RenderProcessHost* host) {
  DCHECK(!render_process_host_);
  render_process_host_ = host;

  // The filter is owned by the IPC channel, but a reference is kept for
  // explicitly removing it from the channel in case the runtime gets
  // removed but the channel is still used by other runtimes sharing
  // the same renderer.
  message_filter_ = new XWalkExtensionMessageFilter(this);
  render_process_host_->GetChannel()->AddFilter(message_filter_);
}

void XWalkExtensionWebContentsHandler::ClearMessageFilter(void) {
  DCHECK(render_process_host_ && message_filter_);

  // RemoveFilter() deletes the filter internally.
  if (render_process_host_->GetChannel())
    render_process_host_->GetChannel()->RemoveFilter(message_filter_);

  message_filter_ = NULL;
}

void XWalkExtensionWebContentsHandler::HandleMessageFromContext(
    const XWalkExtensionRunner* runner, scoped_ptr<base::Value> msg) {
  if (message_filter_)
    message_filter_->PostMessage(runner, msg.Pass());
}

namespace {

const GURL kAboutBlankURL = GURL("about:blank");

}

void XWalkExtensionWebContentsHandler::DidCreateScriptContext(
    int64_t frame_id) {
  // TODO(cmarcelo): We will create runners on demand, this will allow us get
  // rid of this check.
  if (web_contents_->GetURL() != kAboutBlankURL)
    extension_service_->CreateRunnersForHandler(this, frame_id);
}

}  // namespace extensions
}  // namespace xwalk
