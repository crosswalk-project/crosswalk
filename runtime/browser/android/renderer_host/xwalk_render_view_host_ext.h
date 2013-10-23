// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_RENDERER_HOST_XWALK_RENDER_VIEW_HOST_EXT_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_RENDERER_HOST_XWALK_RENDER_VIEW_HOST_EXT_H_

#include <map>

#include "base/callback_forward.h"
#include "base/threading/non_thread_safe.h"
#include "content/public/browser/web_contents_observer.h"
#include "xwalk/runtime/common/android/xwalk_hit_test_data.h"

class GURL;

namespace content {
struct FrameNavigateParams;
struct LoadCommittedDetails;
}  // namespace content

namespace xwalk {

// Provides RenderViewHost wrapper functionality for sending WebView-specific
// IPC messages to the renderer and from there to WebKit.
class XWalkRenderViewHostExt : public content::WebContentsObserver,
                               public base::NonThreadSafe {
 public:
  // To send receive messages to a RenderView we take the WebContents instance,
  // as it internally handles RenderViewHost instances changing underneath us.
  explicit XWalkRenderViewHostExt(content::WebContents* contents);
  virtual ~XWalkRenderViewHostExt();

  // |result| will be invoked with the outcome of the request.
  typedef base::Callback<void(bool)> DocumentHasImagesResult; // NOLINT *
  void DocumentHasImages(DocumentHasImagesResult result);

  // Clear all WebCore memory cache (not only for this view).
  void ClearCache();

  // Do a hit test at the view port coordinates and asynchronously update
  // |last_hit_test_data_|. |view_x| and |view_y| are in density independent
  // pixels used by WebKit::WebView.
  void RequestNewHitTestDataAt(int view_x, int view_y);

  // Optimization to avoid unnecessary Java object creation on hit test.
  bool HasNewHitTestData() const;
  void MarkHitTestDataRead();

  // Return |last_hit_test_data_|. Note that this is unavoidably racy;
  // the corresponding public WebView API is as well.
  const XWalkHitTestData& GetLastHitTestData() const;

  // Sets the zoom level for text only. Used in layout modes other than
  // Text Autosizing.
  void SetTextZoomLevel(double level);

  void ResetScrollAndScaleState();

  // Sets the initial page scale. This overrides initial scale set by
  // the meta viewport tag.
  void SetInitialPageScale(double page_scale_factor);
  void SetJsOnlineProperty(bool network_up);

 private:
  // content::WebContentsObserver implementation.
  virtual void RenderViewGone(base::TerminationStatus status) OVERRIDE;
  virtual void DidNavigateAnyFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  void OnDocumentHasImagesResponse(int msg_id, bool has_images);
  void OnUpdateHitTestData(const XWalkHitTestData& hit_test_data);
  void OnPictureUpdated();

  bool IsRenderViewReady() const;

  std::map<int, DocumentHasImagesResult> pending_document_has_images_requests_;

  // Master copy of hit test data on the browser side. This is updated
  // as a result of DoHitTest called explicitly or when the FocusedNodeChanged
  // is called in XWalkRenderViewExt.
  XWalkHitTestData last_hit_test_data_;

  bool has_new_hit_test_data_;

  DISALLOW_COPY_AND_ASSIGN(XWalkRenderViewHostExt);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_RENDERER_HOST_XWALK_RENDER_VIEW_HOST_EXT_H_
