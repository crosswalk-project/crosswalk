// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/android/xwalk_render_view_ext.h"

#include <string>

#include "base/bind.h"
#include "base/strings/string_piece.h"
#include "content/public/common/url_constants.h"
#include "content/public/renderer/android_content_detection_prefixes.h"
#include "content/public/renderer/document_state.h"
#include "content/public/renderer/render_view.h"
#include "skia/ext/refptr.h"
#include "third_party/WebKit/public/platform/WebSize.h"
#include "third_party/WebKit/public/platform/WebURL.h"
#include "third_party/WebKit/public/platform/WebVector.h"
#include "third_party/WebKit/public/web/WebDataSource.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebElement.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebHitTestResult.h"
#include "third_party/WebKit/public/web/WebNode.h"
#include "third_party/WebKit/public/web/WebNodeList.h"
#include "third_party/WebKit/public/web/WebSecurityOrigin.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "xwalk/runtime/common/android/xwalk_hit_test_data.h"
#include "xwalk/runtime/common/android/xwalk_render_view_messages.h"

namespace xwalk {

namespace {

GURL GetAbsoluteUrl(const WebKit::WebNode& node, const string16& url_fragment) {
  return GURL(node.document().completeURL(url_fragment));
}

string16 GetHref(const WebKit::WebElement& element) {
  // Get the actual 'href' attribute, which might relative if valid or can
  // possibly contain garbage otherwise, so not using absoluteLinkURL here.
  return element.getAttribute("href");
}

GURL GetAbsoluteSrcUrl(const WebKit::WebElement& element) {
  if (element.isNull())
    return GURL();
  return GetAbsoluteUrl(element, element.getAttribute("src"));
}

WebKit::WebNode GetImgChild(const WebKit::WebNode& node) {
  // This implementation is incomplete (for example if is an area tag) but
  // matches the original WebViewClassic implementation.

  WebKit::WebNodeList list = node.getElementsByTagName("img");
  if (list.length() > 0)
    return list.item(0);
  return WebKit::WebNode();
}

bool RemovePrefixAndAssignIfMatches(const base::StringPiece& prefix,
                                    const GURL& url,
                                    std::string* dest) {
  const base::StringPiece spec(url.possibly_invalid_spec());

  if (spec.starts_with(prefix)) {
    dest->assign(spec.begin() + prefix.length(), spec.end());
    return true;
  }
  return false;
}

void DistinguishAndAssignSrcLinkType(const GURL& url, XWalkHitTestData* data) {
  if (RemovePrefixAndAssignIfMatches(
      content::kAddressPrefix,
      url,
      &data->extra_data_for_type)) {
    data->type = XWalkHitTestData::GEO_TYPE;
  } else if (RemovePrefixAndAssignIfMatches(
      content::kPhoneNumberPrefix,
      url,
      &data->extra_data_for_type)) {
    data->type = XWalkHitTestData::PHONE_TYPE;
  } else if (RemovePrefixAndAssignIfMatches(
      content::kEmailPrefix,
      url,
      &data->extra_data_for_type)) {
    data->type = XWalkHitTestData::EMAIL_TYPE;
  } else {
    data->type = XWalkHitTestData::SRC_LINK_TYPE;
    data->extra_data_for_type = url.possibly_invalid_spec();
  }
}

void PopulateHitTestData(const GURL& absolute_link_url,
                         const GURL& absolute_image_url,
                         bool is_editable,
                         XWalkHitTestData* data) {
  // Note: Using GURL::is_empty instead of GURL:is_valid due to the
  // WebViewClassic allowing any kind of protocol which GURL::is_valid
  // disallows. Similar reasons for using GURL::possibly_invalid_spec instead of
  // GURL::spec.
  if (!absolute_image_url.is_empty())
    data->img_src = absolute_image_url;

  const bool is_javascript_scheme =
      absolute_link_url.SchemeIs(content::kJavaScriptScheme);
  const bool has_link_url = !absolute_link_url.is_empty();
  const bool has_image_url = !absolute_image_url.is_empty();

  if (has_link_url && !has_image_url && !is_javascript_scheme) {
    DistinguishAndAssignSrcLinkType(absolute_link_url, data);
  } else if (has_link_url && has_image_url && !is_javascript_scheme) {
    data->type = XWalkHitTestData::SRC_IMAGE_LINK_TYPE;
    data->extra_data_for_type = data->img_src.possibly_invalid_spec();
  } else if (!has_link_url && has_image_url) {
    data->type = XWalkHitTestData::IMAGE_TYPE;
    data->extra_data_for_type = data->img_src.possibly_invalid_spec();
  } else if (is_editable) {
    data->type = XWalkHitTestData::EDIT_TEXT_TYPE;
    DCHECK_EQ(data->extra_data_for_type.length(), 0u);
  }
}

}  // namespace

XWalkRenderViewExt::XWalkRenderViewExt(content::RenderView* render_view)
    : content::RenderViewObserver(render_view), page_scale_factor_(0.0f) {
  render_view->GetWebView()->setPermissionClient(this);
}

XWalkRenderViewExt::~XWalkRenderViewExt() {
}

// static
void XWalkRenderViewExt::RenderViewCreated(content::RenderView* render_view) {
  new XWalkRenderViewExt(render_view);  // |render_view| takes ownership.
}

bool XWalkRenderViewExt::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkRenderViewExt, message)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_DocumentHasImages,
                        OnDocumentHasImagesRequest)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_DoHitTest, OnDoHitTest)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_SetTextZoomLevel, OnSetTextZoomLevel)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_ResetScrollAndScaleState,
                        OnResetScrollAndScaleState)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_SetInitialPageScale, OnSetInitialPageScale)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkRenderViewExt::OnDocumentHasImagesRequest(int id) {
  bool hasImages = false;
  if (render_view()) {
    WebKit::WebView* webview = render_view()->GetWebView();
    if (webview) {
      WebKit::WebVector<WebKit::WebElement> images;
      webview->mainFrame()->document().images(images);
      hasImages = !images.isEmpty();
    }
  }
  Send(new XWalkViewHostMsg_DocumentHasImagesResponse(routing_id(), id,
                                                   hasImages));
}

bool XWalkRenderViewExt::allowImage(WebKit::WebFrame* frame,
                                 bool enabled_per_settings,
                                 const WebKit::WebURL& image_url) {
  // Implementing setBlockNetworkImages, so allow local scheme images to be
  // loaded.
  if (enabled_per_settings)
    return true;

  // For compatibility, only blacklist network schemes instead of whitelisting.
  const GURL url(image_url);
  return !(url.SchemeIs(content::kHttpScheme) ||
           url.SchemeIs(content::kHttpsScheme) ||
           url.SchemeIs(chrome::kFtpScheme));
}

void XWalkRenderViewExt::DidCommitProvisionalLoad(WebKit::WebFrame* frame,
                                               bool is_new_navigation) {
  content::DocumentState* document_state =
      content::DocumentState::FromDataSource(frame->dataSource());
  if (document_state->can_load_local_resources()) {
    WebKit::WebSecurityOrigin origin = frame->document().securityOrigin();
    origin.grantLoadLocalResources();
  }
}

void XWalkRenderViewExt::DidCommitCompositorFrame() {
  UpdatePageScaleFactor();
}

void XWalkRenderViewExt::UpdatePageScaleFactor() {
  if (page_scale_factor_ != render_view()->GetWebView()->pageScaleFactor()) {
    page_scale_factor_ = render_view()->GetWebView()->pageScaleFactor();
    Send(new XWalkViewHostMsg_PageScaleFactorChanged(routing_id(),
                                                  page_scale_factor_));
  }
}

void XWalkRenderViewExt::FocusedNodeChanged(const WebKit::WebNode& node) {
  if (node.isNull() || !node.isElementNode() || !render_view())
    return;

  // Note: element is not const due to innerText() is not const.
  WebKit::WebElement element = node.toConst<WebKit::WebElement>();
  XWalkHitTestData data;

  data.href = GetHref(element);
  data.anchor_text = element.innerText();

  GURL absolute_link_url;
  if (node.isLink())
    absolute_link_url = GetAbsoluteUrl(node, data.href);

  GURL absolute_image_url;
  const WebKit::WebNode child_img = GetImgChild(node);
  if (!child_img.isNull() && child_img.isElementNode()) {
    absolute_image_url =
        GetAbsoluteSrcUrl(child_img.toConst<WebKit::WebElement>());
  }

  PopulateHitTestData(absolute_link_url,
                      absolute_image_url,
                      render_view()->IsEditableNode(node),
                      &data);
  Send(new XWalkViewHostMsg_UpdateHitTestData(routing_id(), data));
}

void XWalkRenderViewExt::OnDoHitTest(int view_x, int view_y) {
  if (!render_view() || !render_view()->GetWebView())
    return;

  const WebKit::WebHitTestResult result =
      render_view()->GetWebView()->hitTestResultAt(
          WebKit::WebPoint(view_x, view_y));
  XWalkHitTestData data;

  if (!result.urlElement().isNull()) {
    data.anchor_text = result.urlElement().innerText();
    data.href = GetHref(result.urlElement());
  }

  PopulateHitTestData(result.absoluteLinkURL(),
                      result.absoluteImageURL(),
                      result.isContentEditable(),
                      &data);
  Send(new XWalkViewHostMsg_UpdateHitTestData(routing_id(), data));
}

void XWalkRenderViewExt::OnSetTextZoomLevel(double zoom_level) {
  if (!render_view() || !render_view()->GetWebView())
    return;
  // Hide selection and autofill popups.
  render_view()->GetWebView()->hidePopups();
  render_view()->GetWebView()->setZoomLevel(zoom_level);
}

void XWalkRenderViewExt::OnResetScrollAndScaleState() {
  if (!render_view() || !render_view()->GetWebView())
    return;
  render_view()->GetWebView()->resetScrollAndScaleState();
}

void XWalkRenderViewExt::OnSetInitialPageScale(double page_scale_factor) {
  if (!render_view() || !render_view()->GetWebView())
    return;
  render_view()->GetWebView()->setInitialPageScaleOverride(
      page_scale_factor);
}

}  // namespace xwalk
