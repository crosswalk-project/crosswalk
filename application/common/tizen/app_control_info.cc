// Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/tizen/app_control_info.h"

#include <app_service.h>
#include <aul.h>
#include <bundle.h>

#include "base/logging.h"
#include "third_party/re2/re2/re2.h"
#include "url/gurl.h"
#include "url/url_constants.h"

// FIXME: this is temporary hack for API that is private
// this function signature matches the one in capi
extern "C" int service_create_event(bundle* data, struct service_s** service);

namespace {
unsigned kMaximumMimeTypeLength = 256;

bool CheckMimeMatch(const std::string& requested, const std::string& given) {
  if (!given.empty()) {
    std::string regex = re2::RE2::QuoteMeta(given);
    re2::RE2::GlobalReplace(&regex, "\\\\\\*", ".*");
    if (re2::RE2::FullMatch(requested, regex)) {
      return true;
    }
    return false;
  }
  return true;
}

bool CheckUriMatch(const GURL& requested, const GURL& given) {
  // match by wildcard
  std::string regex = re2::RE2::QuoteMeta(given.spec());
  re2::RE2::GlobalReplace(&regex, "\\\\\\*", ".*");
  if (re2::RE2::FullMatch(requested.spec(), regex)) {
    return true;
  }

  // try only scheme
  if (!given.has_host()) {
    if (requested.scheme() == given.scheme()) {
      return true;
    }
  }
  return false;
}

}  // namespace

namespace xwalk {
namespace application {

bool AppControlInfo::Covers(const AppControlInfo& requested) const {
  if (!operation_.empty())
    if (operation_ != requested.operation_)
      return false;

  // FIXME: Fixes/Workarounds that should be done in aul

  // 1. Add file scheme if necessary
  std::string requested_uri = requested.uri();
  if (!requested_uri.empty() && requested_uri[0] == '/')
    requested_uri = std::string(url::kFileScheme) +
        url::kStandardSchemeSeparator + requested_uri;

  GURL uri(requested_uri);
  std::string mime(requested.mime());

  // 2. Check mime type for file:// scheme if no mime type specified
  if (uri.SchemeIsFile() && mime.empty()) {
    char mime_from_aul[kMaximumMimeTypeLength] = {0, };
    if (aul_get_mime_from_file(uri.spec().c_str(), mime_from_aul,
        kMaximumMimeTypeLength) == AUL_R_OK) {
      mime = mime_from_aul;
    }
  }

  // 3. Fix uri if there is only scheme
  GURL given_uri(uri_);
  if (uri_.find(url::kStandardSchemeSeparator) == std::string::npos)
    given_uri = GURL(uri_ + url::kStandardSchemeSeparator);

  if (given_uri.is_valid())
    if (!CheckUriMatch(uri, given_uri))
      return false;

  if (!CheckMimeMatch(mime, mime_))
    return false;

  return true;
}

std::unique_ptr<AppControlInfo> AppControlInfo::CreateFromBundle(
    const std::string& bundle_str) {
  std::string operation;
  std::string uri;
  std::string mime;

  bundle* bundle = bundle_decode(
      reinterpret_cast<const bundle_raw*>(
          bundle_str.data()), bundle_str.size());
  if (!bundle)
    return nullptr;
  service_h service;

  service_create_event(bundle, &service);
  if (!service)
    return nullptr;

  char* operation_str = nullptr;
  if (service_get_operation(service, &operation_str) != SERVICE_ERROR_NONE) {
    service_destroy(service);
    bundle_free(bundle);
    return nullptr;
  }

  if (!operation_str) {
    // if no operation then bundle is invalid
    service_destroy(service);
    bundle_free(bundle);
    return nullptr;
  }
  operation = operation_str;
  free(operation_str);

  char* mime_str = nullptr;
  if (service_get_mime(service, &mime_str) != SERVICE_ERROR_NONE) {
    LOG(ERROR) << "Failed to get mime for appcontrol request";
    service_destroy(service);
    bundle_free(bundle);
    return nullptr;
  }

  if (mime_str)
    mime = mime_str;
  free(mime_str);

  char* uri_str = nullptr;
  if (service_get_uri(service, &uri_str) != SERVICE_ERROR_NONE) {
    LOG(ERROR) << "Failed to get uri for appcontrol request";
    service_destroy(service);
    bundle_free(bundle);
    return nullptr;
  }

  if (uri_str)
    uri = uri_str;
  free(uri_str);

  service_destroy(service);

  bundle_free(bundle);

  return std::unique_ptr<AppControlInfo>(
      new AppControlInfo("", operation, uri, mime));
}


}  // namespace application
}  // namespace xwalk
