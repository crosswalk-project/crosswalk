// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/package/wgt_package.h"

#include "base/files/file_util.h"
#include "base/files/scoped_file.h"
#include "third_party/libxml/chromium/libxml_utils.h"
#include "xwalk/application/common/id_util.h"

namespace xwalk {
namespace application {

namespace {

const char kIdNodeName[] = "widget";

}  // namespace

WGTPackage::~WGTPackage() {
}

WGTPackage::WGTPackage(const base::FilePath& path)
    : Package(path, Manifest::TYPE_WIDGET) {
  if (!base::PathExists(path))
    return;
  base::FilePath extracted_path;
  // FIXME : we should not call 'extract' here!
  if (!ExtractToTemporaryDir(&extracted_path))
    return;

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
    id_ = GenerateId(value);
    is_valid_ = true;
  }
  std::unique_ptr<base::ScopedFILE> file(
      new base::ScopedFILE(base::OpenFile(path, "rb")));

  file_ = std::move(file);
}

// static
const std::vector<std::string>& WGTPackage::GetDefaultWidgetEntryPages() {
  static std::vector<std::string> entry_pages;
  if (entry_pages.empty()) {
    entry_pages.push_back("index.htm");
    entry_pages.push_back("index.html");
    entry_pages.push_back("index.svg");
    entry_pages.push_back("index.xhtml");
    entry_pages.push_back("index.xht");
  }

  return entry_pages;
}

}  // namespace application
}  // namespace xwalk
