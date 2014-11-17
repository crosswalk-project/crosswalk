// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/package/unpacked_wgt_package.h"

#include <string>

#include "base/file_util.h"
#include "base/files/scoped_file.h"
#include "third_party/libxml/chromium/libxml_utils.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/id_util.h"

#if defined(OS_TIZEN)
#include "xwalk/application/common/tizen/signature_validator.h"
#endif

namespace xwalk {
namespace application {
namespace {

#if defined(OS_TIZEN)
const char kIdNodeName[] = "application";
#else
const char kIdNodeName[] = "widget";
#endif

}

UnpackedWGTPackage::UnpackedWGTPackage(const base::FilePath& path)
    : Package(path) {
  if (!base::PathExists(path))
    return;
  manifest_type_ = Manifest::TYPE_WIDGET;
  base::FilePath extracted_path = path;

  XmlReader xml;
  if (!xml.LoadFile(extracted_path.Append(FILE_PATH_LITERAL("config.xml"))
                    .MaybeAsASCII())) {
    LOG(ERROR) << "Unable to load WGT package config.xml file.";
    return;
  }

  while (!xml.SkipToElement()) {
    if (!xml.Read()) {
      LOG(ERROR) << "Unable to read WGT package config.xml file.";
      return;
    }
  }

  std::string value;
  while (xml.Read()) {
    std::string node_name = xml.NodeName();
    if (node_name == kIdNodeName) {
      xml.NodeAttribute("id", &value);
      break;
    }
  }

  if (!value.empty()) {
#if defined(OS_TIZEN)
    id_ = value;
    is_valid_ =
      SignatureValidator::Check(extracted_path) != SignatureValidator::INVALID;
#else
    id_ = GenerateId(value);
    is_valid_ = true;
#endif
  }
  scoped_ptr<base::ScopedFILE> file(
      new base::ScopedFILE(base::OpenFile(path, "rb")));

  file_ = file.Pass();
}

UnpackedWGTPackage::~UnpackedWGTPackage() {
}

bool UnpackedWGTPackage::ExtractToTemporaryDir(base::FilePath* result_path) {
  if (is_extracted_) {
    *result_path = temp_dir_.path();
    return true;
  }

  if (!CreateTempDirectory()) {
    LOG(ERROR) << "Can't create a temporary"
                  "directory for extracting the package content.";
    return false;
  }

  if (!CopyDirectoryContents(source_path_, temp_dir_.path())) {
    LOG(ERROR) << "An error occurred during copying files.";
    return false;
  }

  is_extracted_ = true;

  *result_path = temp_dir_.path();

  return true;
}

bool UnpackedWGTPackage::ExtractTo(const base::FilePath& target_path) {
  if (!DirectoryExists(target_path)) {
    LOG(ERROR) << "The directory " << target_path.MaybeAsASCII()
               << "does not exist.";
    return false;
  }

  if (!IsDirectoryEmpty(target_path)) {
    LOG(ERROR) << "The directory " << target_path.MaybeAsASCII()
               << "is not empty.";
    return false;
  }

  if (!CopyDirectoryContents(source_path_, target_path)) {
    LOG(ERROR) << "An error occurred during copying files.";
    return false;
  }

  return true;
}

}  // namespace application
}  // namespace xwalk
