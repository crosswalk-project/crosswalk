// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/tizen/xwalk_content_renderer_client_tizen.h"

#include <string>

#include "base/files/file_path.h"
#include "base/strings/stringprintf.h"
#include "net/base/filename_util.h"
#include "net/base/net_errors.h"
#include "url/gurl.h"
#include "xwalk/application/common/constants.h"
#include "third_party/WebKit/public/platform/WebURLError.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebDocument.h"

namespace xwalk {

namespace {

const base::FilePath kTizenWebUiFwPath("/usr/share/tizen-web-ui-fw/");
const std::string kTizenWebUiFw = "/tizen-web-ui-fw/";

bool HasKnownExtension(const base::FilePath& path) {
  if (path.MatchesExtension(".js"))
    return true;
  if (path.MatchesExtension(".css"))
    return true;
  if (path.MatchesExtension(".png"))
    return true;
  return false;
}

size_t GetRootPathLength(const GURL& first_party_for_cookies) {
  const std::string& path = first_party_for_cookies.PathForRequest();
  return path.rfind('/');
}

bool URLHasAppOrFileScheme(const GURL& url) {
  if (url.SchemeIs("app"))
    return true;
  if (url.SchemeIsFile())
    return true;
  return false;
}

};  // namespace

bool XWalkContentRendererClientTizen::WillSendRequest(
    blink::WebFrame* frame, content::PageTransition transition_type,
    const GURL& url, const GURL& first_party_for_cookies, GURL* new_url) {
  DCHECK(new_url);

  if (XWalkContentRendererClient::WillSendRequest(
          frame, transition_type, url, first_party_for_cookies, new_url))
    return true;

  if (!URLHasAppOrFileScheme(first_party_for_cookies))
    return false;

  const size_t root_path_length = GetRootPathLength(first_party_for_cookies);
  if (root_path_length == std::string::npos)
    return false;

  const std::string& relative_path = url.path();
  size_t tizen_web_ui_fw_pos = relative_path.find(kTizenWebUiFw,
      root_path_length);
  if (tizen_web_ui_fw_pos == std::string::npos)
    return false;

  tizen_web_ui_fw_pos += kTizenWebUiFw.length();
  const base::FilePath& resource_path = kTizenWebUiFwPath.Append(
      relative_path.substr(tizen_web_ui_fw_pos));

  // FIXME(leandro): base::NormalizeFilePath(resource_path) should be called
  // here to make sure files are really beneath kTizenWebUiFwPath, but the
  // sandbox prevents the Render process from calling realpath().
  if (!kTizenWebUiFwPath.IsParent(resource_path))
    return false;

  if (!HasKnownExtension(resource_path))
    return false;

  GURL replacement_url = net::FilePathToFileURL(resource_path);
  if (!replacement_url.is_valid())
    return false;

  new_url->Swap(&replacement_url);
  return true;
}

bool XWalkContentRendererClientTizen::HasErrorPage(int http_status_code,
                                                   std::string* error_domain) {
  return true;
}

void XWalkContentRendererClientTizen::GetNavigationErrorStrings(
    content::RenderView* render_view,
    blink::WebFrame* frame,
    const blink::WebURLRequest& failed_request,
    const blink::WebURLError& error,
    std::string* error_html,
    base::string16* error_description) {
  if (error_html) {
    *error_html =
        base::StringPrintf("<html><body style=\"text-align: center;\">"
                           "<h1>NET ERROR : %s</h1></body></html>",
                           net::ErrorToString(error.reason));
  }
}

}  // namespace xwalk
