// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_MAIN_DOCUMENT_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_MAIN_DOCUMENT_HANDLER_H_

#include <string>
#include <vector>

#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class MainDocumentInfo : public ApplicationData::ManifestData {
 public:
  MainDocumentInfo();
  virtual ~MainDocumentInfo();

  GURL GetMainURL() const { return main_url_; }
  void SetMainURL(const GURL& url) { main_url_ = url; }

  const std::vector<std::string>& GetMainScripts() const {
    return main_scripts_;
  }
  void SetMainScripts(const std::vector<std::string>& scripts) {
    main_scripts_ = scripts;
  }

 private:
  GURL main_url_;
  std::vector<std::string> main_scripts_;

  DISALLOW_COPY_AND_ASSIGN(MainDocumentInfo);
};

inline MainDocumentInfo* ToMainDocumentInfo(
    ApplicationData::ManifestData* data) {
  return static_cast<MainDocumentInfo*>(data);
}

class MainDocumentHandler : public ManifestHandler {
 public:
  MainDocumentHandler();
  virtual ~MainDocumentHandler();

  virtual bool Parse(scoped_refptr<ApplicationData> application,
                     string16* error) OVERRIDE;
  virtual std::vector<std::string> Keys() const OVERRIDE;

 private:
  bool ParseMainSource(MainDocumentInfo* info,
                       const ApplicationData* manifest,
                       string16* error);

  bool ParseMainScripts(MainDocumentInfo* info,
                        const ApplicationData* manifest,
                        string16* error);

  DISALLOW_COPY_AND_ASSIGN(MainDocumentHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_MAIN_DOCUMENT_HANDLER_H_
