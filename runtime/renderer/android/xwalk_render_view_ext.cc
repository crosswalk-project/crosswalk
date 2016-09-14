// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/android/xwalk_render_view_ext.h"

#include <string>

#include "base/bind.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/url_constants.h"
#include "content/public/renderer/android_content_detection_prefixes.h"
#include "content/public/renderer/document_state.h"
#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/platform/WebSecurityOrigin.h"
#include "third_party/WebKit/public/platform/WebSize.h"
#include "third_party/WebKit/public/platform/WebURL.h"
#include "third_party/WebKit/public/platform/WebVector.h"
#include "third_party/WebKit/public/web/WebDataSource.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebElement.h"
#include "third_party/WebKit/public/web/WebElementCollection.h"
#include "third_party/WebKit/public/web/WebFrameWidget.h"
#include "third_party/WebKit/public/web/WebHitTestResult.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebNode.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "url/url_canon.h"
#include "url/url_constants.h"
#include "url/url_util.h"
#include "xwalk/runtime/common/android/xwalk_hit_test_data.h"
#include "xwalk/runtime/common/android/xwalk_render_view_messages.h"

namespace xwalk {

namespace {

GURL GetAbsoluteUrl(const blink::WebNode& node,
                    const base::string16& url_fragment) {
  return GURL(node.document().completeURL(url_fragment));
}

base::string16 GetHref(const blink::WebElement& element) {
  // Get the actual 'href' attribute, which might relative if valid or can
  // possibly contain garbage otherwise, so not using absoluteLinkURL here.
  return element.getAttribute("href");
}

GURL GetAbsoluteSrcUrl(const blink::WebElement& element) {
  if (element.isNull())
    return GURL();
  return GetAbsoluteUrl(element, element.getAttribute("src"));
}

blink::WebElement GetImgChild(const blink::WebNode& node) {
  // This implementation is incomplete (for example if is an area tag) but
  // matches the original WebViewClassic implementation.

  blink::WebElementCollection collection = node.getElementsByHTMLTagName("img");
  DCHECK(!collection.isNull());
  return collection.firstItem();
}

bool RemovePrefixAndAssignIfMatches(const base::StringPiece& prefix,
                                    const GURL& url,
                                    std::string* dest) {
  const base::StringPiece spec(url.possibly_invalid_spec());

  if (spec.starts_with(prefix)) {
    url::RawCanonOutputW<1024> output;
    url::DecodeURLEscapeSequences(spec.data() + prefix.length(),
                                  spec.length() - prefix.length(), &output);
    *dest =
        base::UTF16ToUTF8(base::StringPiece16(output.data(), output.length()));
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
      absolute_link_url.SchemeIs(url::kJavaScriptScheme);
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
    : content::RenderViewObserver(render_view) {
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
    IPC_MESSAGE_HANDLER(XWalkViewMsg_SetBackgroundColor, OnSetBackgroundColor)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_SetTextZoomFactor, OnSetTextZoomFactor)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkRenderViewExt::OnDocumentHasImagesRequest(int id) {
  bool hasImages = false;
  if (render_view()) {
    blink::WebView* webview = render_view()->GetWebView();
    if (webview) {
      blink::WebDocument document = webview->mainFrame()->document();
      const blink::WebElement child_img = GetImgChild(document);
      hasImages = !child_img.isNull();
    }
  }
  Send(new XWalkViewHostMsg_DocumentHasImagesResponse(routing_id(), id,
                                                   hasImages));
}

void XWalkRenderViewExt::DidCommitProvisionalLoad(blink::WebLocalFrame* frame,
                                                  bool is_new_navigation) {
  content::DocumentState* document_state =
      content::DocumentState::FromDataSource(frame->dataSource());
  if (document_state->can_load_local_resources()) {
    blink::WebSecurityOrigin origin = frame->document().getSecurityOrigin();
    origin.grantLoadLocalResources();
  }
}

void XWalkRenderViewExt::FocusedNodeChanged(const blink::WebNode& node) {
  if (node.isNull() || !node.isElementNode() || !render_view())
    return;

  const blink::WebElement element = node.toConst<blink::WebElement>();
  XWalkHitTestData data;

  data.href = GetHref(element);
  data.anchor_text = element.textContent();

  GURL absolute_link_url;
  if (node.isLink())
    absolute_link_url = GetAbsoluteUrl(node, data.href);

  GURL absolute_image_url;
  const blink::WebElement child_img = GetImgChild(element);
  if (!child_img.isNull()) {
    absolute_image_url =
        GetAbsoluteSrcUrl(child_img);
  }

  PopulateHitTestData(absolute_link_url,
                      absolute_image_url,
                      element.isEditable(),
                      &data);
  Send(new XWalkViewHostMsg_UpdateHitTestData(routing_id(), data));
}

void XWalkRenderViewExt::OnDestruct() {
  delete this;
}

void XWalkRenderViewExt::OnDoHitTest(const gfx::PointF& touch_center,
                                     const gfx::SizeF& touch_area) {
  if (!render_view() || !render_view()->GetWebView())
    return;

  const blink::WebHitTestResult result =
      render_view()->GetWebView()->hitTestResultForTap(
          blink::WebPoint(touch_center.x(), touch_center.y()),
          blink::WebSize(touch_area.width(), touch_area.height()));
  XWalkHitTestData data;

  if (!result.urlElement().isNull()) {
    data.anchor_text = result.urlElement().textContent();
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

void XWalkRenderViewExt::OnSetBackgroundColor(SkColor c) {
  if (!render_view() || !render_view()->GetWebFrameWidget())
    return;
  blink::WebFrameWidget* web_frame_widget = render_view()->GetWebFrameWidget();
  web_frame_widget->setBaseBackgroundColor(c);
}

void XWalkRenderViewExt::OnSetTextZoomFactor(float zoom_factor) {
  if (!render_view() || !render_view()->GetWebView())
    return;
  // Hide selection and autofill popups.
  render_view()->GetWebView()->hidePopups();
  render_view()->GetWebView()->setTextZoomFactor(zoom_factor);
}

}  // namespace xwalk
