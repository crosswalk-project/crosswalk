// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_protocols.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/string_util.h"
#include "base/stringprintf.h"
#include "base/threading/thread_restrictions.h"
#include "base/threading/worker_pool.h"
#include "base/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/extensions/extension_info_map.h"
#include "chrome/browser/extensions/image_loader.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/extensions/background_info.h"
#include "chrome/common/extensions/csp_handler.h"
#include "chrome/common/extensions/extension.h"
#include "chrome/common/extensions/extension_file_util.h"
#include "chrome/common/extensions/incognito_handler.h"
#include "chrome/common/extensions/manifest_handlers/icons_handler.h"
#include "chrome/common/extensions/manifest_handlers/shared_module_info.h"
#include "chrome/common/extensions/manifest_url_handler.h"
#include "chrome/common/extensions/web_accessible_resources_handler.h"
#include "chrome/common/url_constants.h"
#include "content/public/browser/resource_request_info.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension_resource.h"
#include "googleurl/src/url_util.h"
#include "grit/component_extension_resources_map.h"
#include "net/base/mime_util.h"
#include "net/base/net_errors.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_response_info.h"
#include "net/url_request/url_request_error_job.h"
#include "net/url_request/url_request_file_job.h"
#include "net/url_request/url_request_simple_job.h"
#include "ui/base/resource/resource_bundle.h"

using content::ResourceRequestInfo;
using extensions::Extension;
using extensions::SharedModuleInfo;

namespace {

net::HttpResponseHeaders* BuildHttpHeaders(
    const std::string& content_security_policy, bool send_cors_header) {
  std::string raw_headers;
  raw_headers.append("HTTP/1.1 200 OK");
  if (!content_security_policy.empty()) {
    raw_headers.append(1, '\0');
    raw_headers.append("Content-Security-Policy: ");
    raw_headers.append(content_security_policy);
  }

  if (send_cors_header) {
    raw_headers.append(1, '\0');
    raw_headers.append("Access-Control-Allow-Origin: *");
  }
  raw_headers.append(2, '\0');
  return new net::HttpResponseHeaders(raw_headers);
}

void ReadMimeTypeFromFile(const base::FilePath& filename,
                          std::string* mime_type,
                          bool* result) {
  *result = net::GetMimeTypeFromFile(filename, mime_type);
}

class URLRequestResourceBundleJob : public net::URLRequestSimpleJob {
 public:
  URLRequestResourceBundleJob(net::URLRequest* request,
                              net::NetworkDelegate* network_delegate,
                              const base::FilePath& filename,
                              int resource_id,
                              const std::string& content_security_policy,
                              bool send_cors_header)
      : net::URLRequestSimpleJob(request, network_delegate),
        filename_(filename),
        resource_id_(resource_id),
        weak_factory_(this) {
    response_info_.headers = BuildHttpHeaders(content_security_policy,
                                              send_cors_header);
  }

  // Overridden from URLRequestSimpleJob:
  virtual int GetData(std::string* mime_type,
                      std::string* charset,
                      std::string* data,
                      const net::CompletionCallback& callback) const OVERRIDE {
    const ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    *data = rb.GetRawDataResource(resource_id_).as_string();

    std::string* read_mime_type = new std::string;
    bool* read_result = new bool;
    bool posted = base::WorkerPool::PostTaskAndReply(
        FROM_HERE,
        base::Bind(&ReadMimeTypeFromFile, filename_,
                   base::Unretained(read_mime_type),
                   base::Unretained(read_result)),
        base::Bind(&URLRequestResourceBundleJob::OnMimeTypeRead,
                   weak_factory_.GetWeakPtr(),
                   mime_type, charset, data,
                   base::Owned(read_mime_type),
                   base::Owned(read_result),
                   callback),
        true /* task is slow */);
    DCHECK(posted);

    return net::ERR_IO_PENDING;
  }

  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE {
    *info = response_info_;
  }

 private:
  virtual ~URLRequestResourceBundleJob() { }

  void OnMimeTypeRead(std::string* out_mime_type,
                      std::string* charset,
                      std::string* data,
                      std::string* read_mime_type,
                      bool* read_result,
                      const net::CompletionCallback& callback) {
    *out_mime_type = *read_mime_type;
    if (StartsWithASCII(*read_mime_type, "text/", false)) {
      // All of our HTML files should be UTF-8 and for other resource types
      // (like images), charset doesn't matter.
      DCHECK(IsStringUTF8(*data));
      *charset = "utf-8";
    }
    int result = *read_result? net::OK: net::ERR_INVALID_URL;
    callback.Run(result);
  }

  // We need the filename of the resource to determine the mime type.
  base::FilePath filename_;

  // The resource bundle id to load.
  int resource_id_;

  net::HttpResponseInfo response_info_;

  mutable base::WeakPtrFactory<URLRequestResourceBundleJob> weak_factory_;
};

class GeneratedBackgroundPageJob : public net::URLRequestSimpleJob {
 public:
  GeneratedBackgroundPageJob(net::URLRequest* request,
                             net::NetworkDelegate* network_delegate,
                             const scoped_refptr<const Extension> extension,
                             const std::string& content_security_policy)
      : net::URLRequestSimpleJob(request, network_delegate),
        extension_(extension) {
    const bool send_cors_headers = false;
    response_info_.headers = BuildHttpHeaders(content_security_policy,
                                              send_cors_headers);
  }

  // Overridden from URLRequestSimpleJob:
  virtual int GetData(std::string* mime_type,
                      std::string* charset,
                      std::string* data,
                      const net::CompletionCallback& callback) const OVERRIDE {
    *mime_type = "text/html";
    *charset = "utf-8";

    *data = "<!DOCTYPE html>\n<body>\n";
    const std::vector<std::string>& background_scripts =
        extensions::BackgroundInfo::GetBackgroundScripts(extension_);
    for (size_t i = 0; i < background_scripts.size(); ++i) {
      *data += "<script src=\"";
      *data += background_scripts[i];
      *data += "\"></script>\n";
    }

    return net::OK;
  }

  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE {
    *info = response_info_;
  }

 private:
  virtual ~GeneratedBackgroundPageJob() {}

  scoped_refptr<const Extension> extension_;
  net::HttpResponseInfo response_info_;
};

void ReadResourceFilePath(const extensions::ExtensionResource& resource,
                          base::FilePath* file_path) {
  *file_path = resource.GetFilePath();
}

class URLRequestExtensionJob : public net::URLRequestFileJob {
 public:
  URLRequestExtensionJob(net::URLRequest* request,
                         net::NetworkDelegate* network_delegate,
                         const std::string& extension_id,
                         const base::FilePath& directory_path,
                         const base::FilePath& relative_path,
                         const std::string& content_security_policy,
                         bool send_cors_header)
    : net::URLRequestFileJob(request, network_delegate, base::FilePath()),
      // TODO(tc): Move all of these files into resources.pak so we don't break
      // when updating on Linux.
      resource_(extension_id, directory_path, relative_path),
      weak_factory_(this) {
      response_info_.headers = BuildHttpHeaders(content_security_policy,
                                                send_cors_header);
  }

  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE {
    *info = response_info_;
  }

  virtual void Start() OVERRIDE {
    base::FilePath* read_file_path = new base::FilePath;
    bool posted = base::WorkerPool::PostTaskAndReply(
        FROM_HERE,
        base::Bind(&ReadResourceFilePath, resource_,
                   base::Unretained(read_file_path)),
        base::Bind(&URLRequestExtensionJob::OnFilePathRead,
                   weak_factory_.GetWeakPtr(),
                   base::Owned(read_file_path)),
        true /* task is slow */);
    DCHECK(posted);
  }

 private:
  virtual ~URLRequestExtensionJob() {}

  void OnFilePathRead(base::FilePath* read_file_path) {
    file_path_ = *read_file_path;
    URLRequestFileJob::Start();
  }

  net::HttpResponseInfo response_info_;
  extensions::ExtensionResource resource_;
  base::WeakPtrFactory<URLRequestExtensionJob> weak_factory_;
};

bool ExtensionCanLoadInIncognito(const ResourceRequestInfo* info,
                                 const std::string& extension_id,
                                 ExtensionInfoMap* extension_info_map) {
  if (!extension_info_map->IsIncognitoEnabled(extension_id))
    return false;

  // Only allow incognito toplevel navigations to extension resources in
  // split mode. In spanning mode, the extension must run in a single process,
  // and an incognito tab prevents that.
  if (info->GetResourceType() == ResourceType::MAIN_FRAME) {
    const Extension* extension =
        extension_info_map->extensions().GetByID(extension_id);
    return extension && extensions::IncognitoInfo::IsSplitMode(extension);
  }

  return true;
}

// Returns true if an chrome-extension:// resource should be allowed to load.
// TODO(aa): This should be moved into ExtensionResourceRequestPolicy, but we
// first need to find a way to get CanLoadInIncognito state into the renderers.
bool AllowExtensionResourceLoad(net::URLRequest* request,
                                bool is_incognito,
                                const Extension* extension,
                                ExtensionInfoMap* extension_info_map) {
  const ResourceRequestInfo* info = ResourceRequestInfo::ForRequest(request);

  // We have seen crashes where info is NULL: crbug.com/52374.
  if (!info) {
    LOG(ERROR) << "Allowing load of " << request->url().spec()
               << "from unknown origin. Could not find user data for "
               << "request.";
    return true;
  }

  if (is_incognito && !ExtensionCanLoadInIncognito(info, request->url().host(),
                                                   extension_info_map)) {
    return false;
  }

  // The following checks are meant to replicate similar set of checks in the
  // renderer process, performed by ResourceRequestPolicy::CanRequestResource.
  // These are not exactly equivalent, because we don't have the same bits of
  // information. The two checks need to be kept in sync as much as possible, as
  // an exploited renderer can bypass the checks in ResourceRequestPolicy.

  // Check if the extension for which this request is made is indeed loaded in
  // the process sending the request. If not, we need to explicitly check if
  // the resource is explicitly accessible or fits in a set of exception cases.
  // Note: This allows a case where two extensions execute in the same renderer
  // process to request each other's resources. We can't do a more precise
  // check, since the renderer can lie about which extension has made the
  // request.
  if (extension_info_map->process_map().Contains(
      request->url().host(), info->GetChildID())) {
    return true;
  }

  if (!content::PageTransitionIsWebTriggerable(info->GetPageTransition()))
    return false;

  // The following checks require that we have an actual extension object. If we
  // don't have it, allow the request handling to continue with the rest of the
  // checks.
  if (!extension)
    return true;

  // Disallow loading of packaged resources for hosted apps. We don't allow
  // hybrid hosted/packaged apps. The one exception is access to icons, since
  // some extensions want to be able to do things like create their own
  // launchers.
  std::string resource_root_relative_path =
      request->url().path().empty() ? std::string()
                                    : request->url().path().substr(1);
  if (extension->is_hosted_app() &&
      !extensions::IconsInfo::GetIcons(extension)
          .ContainsPath(resource_root_relative_path)) {
    LOG(ERROR) << "Denying load of " << request->url().spec() << " from "
               << "hosted app.";
    return false;
  }

  // Extensions with web_accessible_resources: allow loading by regular
  // renderers. Since not all subresources are required to be listed in a v2
  // manifest, we must allow all loads if there are any web accessible
  // resources. See http://crbug.com/179127.
  if (extension->manifest_version() < 2 ||
      extensions::WebAccessibleResourcesInfo::HasWebAccessibleResources(
      extension)) {
    return true;
  }

  // If there aren't any explicitly marked web accessible resources, the
  // load should be allowed only if it is by DevTools. A close approximation is
  // checking if the extension contains a DevTools page.
  if (extensions::ManifestURL::GetDevToolsPage(extension).is_empty())
    return false;

  return true;
}

// Returns true if the given URL references an icon in the given extension.
bool URLIsForExtensionIcon(const GURL& url, const Extension* extension) {
  DCHECK(url.SchemeIs(extensions::kExtensionScheme));

  if (!extension)
    return false;

  std::string path = url.path();
  DCHECK_EQ(url.host(), extension->id());
  DCHECK(path.length() > 0 && path[0] == '/');
  path = path.substr(1);
  return extensions::IconsInfo::GetIcons(extension).ContainsPath(path);
}

class ExtensionProtocolHandler
    : public net::URLRequestJobFactory::ProtocolHandler {
 public:
  ExtensionProtocolHandler(bool is_incognito,
                           ExtensionInfoMap* extension_info_map)
      : is_incognito_(is_incognito),
        extension_info_map_(extension_info_map) {}

  virtual ~ExtensionProtocolHandler() {}

  virtual net::URLRequestJob* MaybeCreateJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const OVERRIDE;

 private:
  const bool is_incognito_;
  ExtensionInfoMap* const extension_info_map_;
  DISALLOW_COPY_AND_ASSIGN(ExtensionProtocolHandler);
};

// Creates URLRequestJobs for extension:// URLs.
net::URLRequestJob*
ExtensionProtocolHandler::MaybeCreateJob(
    net::URLRequest* request, net::NetworkDelegate* network_delegate) const {
  // chrome-extension://extension-id/resource/path.js
  std::string extension_id = request->url().host();
  const Extension* extension =
      extension_info_map_->extensions().GetByID(extension_id);

  // TODO(mpcomplete): better error code.
  if (!AllowExtensionResourceLoad(
           request, is_incognito_, extension, extension_info_map_)) {
    return new net::URLRequestErrorJob(
        request, network_delegate, net::ERR_ADDRESS_UNREACHABLE);
  }

  base::FilePath directory_path;
  if (extension)
    directory_path = extension->path();
  if (directory_path.value().empty()) {
    const Extension* disabled_extension =
        extension_info_map_->disabled_extensions().GetByID(extension_id);
    if (URLIsForExtensionIcon(request->url(), disabled_extension))
      directory_path = disabled_extension->path();
    if (directory_path.value().empty()) {
      LOG(WARNING) << "Failed to GetPathForExtension: " << extension_id;
      return NULL;
    }
  }

  std::string content_security_policy;
  bool send_cors_header = false;
  if (extension) {
    std::string resource_path = request->url().path();
    content_security_policy =
        extensions::CSPInfo::GetResourceContentSecurityPolicy(extension,
                                                              resource_path);
    if ((extension->manifest_version() >= 2 ||
         extensions::WebAccessibleResourcesInfo::HasWebAccessibleResources(
             extension)) &&
        extensions::WebAccessibleResourcesInfo::IsResourceWebAccessible(
            extension, resource_path))
      send_cors_header = true;
  }

  std::string path = request->url().path();
  if (path.size() > 1 &&
      path.substr(1) == extension_filenames::kGeneratedBackgroundPageFilename) {
    return new GeneratedBackgroundPageJob(
        request, network_delegate, extension, content_security_policy);
  }

  base::FilePath resources_path;
  base::FilePath relative_path;
  // Try to load extension resources from chrome resource file if
  // directory_path is a descendant of resources_path. resources_path
  // corresponds to src/chrome/browser/resources in source tree.
  if (PathService::Get(chrome::DIR_RESOURCES, &resources_path) &&
      // Since component extension resources are included in
      // component_extension_resources.pak file in resources_path, calculate
      // extension relative path against resources_path.
      resources_path.AppendRelativePath(directory_path, &relative_path)) {
    base::FilePath request_path =
        extension_file_util::ExtensionURLToRelativeFilePath(request->url());
    int resource_id;
    if (extensions::ImageLoader::IsComponentExtensionResource(
        directory_path, request_path, &resource_id)) {
      relative_path = relative_path.Append(request_path);
      relative_path = relative_path.NormalizePathSeparators();
      return new URLRequestResourceBundleJob(
          request,
          network_delegate,
          relative_path,
          resource_id,
          content_security_policy,
          send_cors_header);
    }
  }

  relative_path =
      extension_file_util::ExtensionURLToRelativeFilePath(request->url());

  if (SharedModuleInfo::IsImportedPath(path)) {
    std::string new_extension_id;
    std::string new_relative_path;
    SharedModuleInfo::ParseImportedPath(path, &new_extension_id,
                                        &new_relative_path);
    const Extension* new_extension =
        extension_info_map_->extensions().GetByID(new_extension_id);

    bool first_party_in_import = false;
    // NB: This first_party_for_cookies call is not for security, it is only
    // used so an exported extension can limit the visible surface to the
    // extension that imports it, more or less constituting its API.
    const std::string& first_party_path =
        request->first_party_for_cookies().path();
    if (SharedModuleInfo::IsImportedPath(first_party_path)) {
      std::string first_party_id;
      std::string dummy;
      SharedModuleInfo::ParseImportedPath(first_party_path, &first_party_id,
                                          &dummy);
      if (first_party_id == new_extension_id) {
        first_party_in_import = true;
      }
    }

    if (SharedModuleInfo::ImportsExtensionById(extension, new_extension_id) &&
        new_extension &&
        (first_party_in_import ||
         SharedModuleInfo::IsExportAllowed(new_extension, new_relative_path))) {
      directory_path = new_extension->path();
      extension_id = new_extension_id;
#if defined(OS_POSIX)
      relative_path = base::FilePath(new_relative_path);
#elif defined(OS_WIN)
      relative_path = base::FilePath(UTF8ToWide(new_relative_path));
#endif
    } else {
      return NULL;
    }
  }

  return new URLRequestExtensionJob(request,
                                    network_delegate,
                                    extension_id,
                                    directory_path,
                                    relative_path,
                                    content_security_policy,
                                    send_cors_header);
}

}  // namespace

net::URLRequestJobFactory::ProtocolHandler* CreateExtensionProtocolHandler(
    bool is_incognito,
    ExtensionInfoMap* extension_info_map) {
  return new ExtensionProtocolHandler(is_incognito, extension_info_map);
}
