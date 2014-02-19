// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/main_document_handler.h"

#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

MainDocumentInfo::MainDocumentInfo() {
}

MainDocumentInfo::~MainDocumentInfo() {
}

MainDocumentHandler::MainDocumentHandler() {
}

MainDocumentHandler::~MainDocumentHandler() {
}

bool MainDocumentHandler::Parse(scoped_refptr<ApplicationData> application,
                                base::string16* error) {
  const Manifest* manifest = application->GetManifest();
  const base::DictionaryValue* dict = NULL;
  if (!manifest->GetDictionary(keys::kAppMainKey, &dict)) {
    *error = base::ASCIIToUTF16("Invalid value of app.main.");
    return false;
  }

  scoped_ptr<MainDocumentInfo> main_doc_info(new MainDocumentInfo);
  if (!ParseMainScripts(main_doc_info.get(), application, error) ||
      !ParseMainSource(main_doc_info.get(), application, error))
    return false;
  if (!main_doc_info->GetMainURL().is_valid()) {
    *error = base::ASCIIToUTF16("app.main doesn't contain a valid main document.");
    return false;
  }

  application->SetManifestData(keys::kAppMainKey, main_doc_info.release());
  return true;
}

std::vector<std::string> MainDocumentHandler::Keys() const {
  return std::vector<std::string>(1, keys::kAppMainKey);
}

bool MainDocumentHandler::ParseMainSource(MainDocumentInfo* info,
                                          const ApplicationData* application,
                                          base::string16* error) {
  const Manifest* manifest = application->GetManifest();
  std::string main_source;
  if (manifest->HasPath(keys::kAppMainSourceKey) &&
      !manifest->GetString(keys::kAppMainSourceKey, &main_source)) {
    *error = base::ASCIIToUTF16("Invalid value of app.main.source");
    return false;
  }

  if (!main_source.empty()) {
    if (!info->GetMainURL().is_empty())
      LOG(WARNING) << "Manifest contains more than one main document.";
    info->SetMainURL(application->GetResourceURL(main_source));
  }
  return true;
}

bool MainDocumentHandler::ParseMainScripts(MainDocumentInfo* info,
                                           const ApplicationData* application,
                                           base::string16* error) {
  const Manifest* manifest = application->GetManifest();
  const base::ListValue* main_scripts = NULL;
  if (manifest->HasPath(keys::kAppMainScriptsKey) &&
      !manifest->GetList(keys::kAppMainScriptsKey, &main_scripts)) {
    *error = base::ASCIIToUTF16("Invalid value of app.main.scripts");
    return false;
  }

  if (main_scripts && main_scripts->GetSize()) {
    std::vector<std::string> scripts;
    for (size_t i = 0; i < main_scripts->GetSize(); ++i) {
      std::string script;
      if (!main_scripts->GetString(i, &script)) {
        *error = base::ASCIIToUTF16("Invalid value of app.main.scripts list");
        return false;
      }
      scripts.push_back(script);
    }
    // We implicitly creat a main document to include these scripts.
    info->SetMainURL(
        application->GetResourceURL(kGeneratedMainDocumentFilename));
    info->SetMainScripts(scripts);
  }
  return true;
}

}  // namespace application
}  // namespace xwalk
