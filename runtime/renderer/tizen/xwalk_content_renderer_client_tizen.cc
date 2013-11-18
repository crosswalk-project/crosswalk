// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/tizen/xwalk_content_renderer_client_tizen.h"

#include <string>

#include "base/files/file_path.h"
#include "net/base/net_util.h"
#include "url/gurl.h"

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

};  // namespace

bool XWalkContentRendererClientTizen::WillSendRequest(
    WebKit::WebFrame*, content::PageTransition, const GURL& url,
    const GURL& first_party_for_cookies, GURL* new_url) {
  DCHECK(new_url);

  if (!first_party_for_cookies.SchemeIsFile())
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

}  // namespace xwalk
