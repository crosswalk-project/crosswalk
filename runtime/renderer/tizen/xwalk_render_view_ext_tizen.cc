// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/tizen/xwalk_render_view_ext_tizen.h"

#include <string>

#include "base/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebScriptSource.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "ui/events/keycodes/keyboard_codes_posix.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"

namespace {
std::string GenerateEventJs(const std::string& name) {
  std::string result;
  std::string defineJs =
    "var event = new Event('tizenhwkey');"
    "function defineProps(val) {"
    "  Object.defineProperty(event, 'keyName', {"
    "    enumerable: false,"
    "    configurable: false,"
    "    writable: false,"
    "    value: val"
    "  });"
    "}";
  std::string nameJs = "defineProps('" + name + "');";
  std::string dispatchJs = "document.dispatchEvent(event);";
  result += defineJs;
  result += nameJs;
  result += dispatchJs;

  return result;
}
}  // namespace

namespace xwalk {

XWalkRenderViewExtTizen::XWalkRenderViewExtTizen(
    content::RenderView* render_view)
    : content::RenderViewObserver(render_view) {
  render_view_ = render_view;
  DCHECK(render_view_);
}

XWalkRenderViewExtTizen::~XWalkRenderViewExtTizen() {
}

// static
void XWalkRenderViewExtTizen::RenderViewCreated(
    content::RenderView* render_view) {
  new XWalkRenderViewExtTizen(render_view);  // |render_view| takes ownership.
}

bool XWalkRenderViewExtTizen::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkRenderViewExtTizen, message)
    IPC_MESSAGE_HANDLER(ViewMsg_HWKeyPressed,
                        OnHWKeyPressed)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkRenderViewExtTizen::OnHWKeyPressed(int keycode) {
  std::string event_name;
  if (keycode == ui::VKEY_BACK) {
    event_name = "back";
  } else if (keycode == ui::VKEY_HOME) {
    event_name = "menu";
  } else {
    LOG(INFO) << "Unknown input key code, only support 'back' & 'menu'"
                  "at present.";
    return;
  }

  content::RenderFrame* render_frame = render_view_->GetMainRenderFrame();
  blink::WebFrame* web_frame = render_frame->GetWebFrame();
  blink::WebScriptSource source =
      blink::WebScriptSource(base::ASCIIToUTF16(GenerateEventJs(event_name)));
  web_frame->executeScript(source);
}

}  // namespace xwalk
