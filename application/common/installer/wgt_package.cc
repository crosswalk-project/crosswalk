// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/installer/wgt_package.h"

#include "base/file_util.h"
#include "base/files/scoped_file.h"
#include "third_party/libxml/chromium/libxml_utils.h"
#include "xwalk/application/common/id_util.h"

namespace xwalk {
namespace application {
namespace {
#if defined(OS_TIZEN)
const char kIdNodeName[] = "application";
#else
const char kIdNodeName[] = "widget";
#endif
}

WGTPackage::~WGTPackage() {
}

WGTPackage::WGTPackage(const base::FilePath& path)
  : Package(path) {
  if (!base::PathExists(path))
    return;
  type_ = WGT;
  base::FilePath extracted_path;
  if (!Extract(&extracted_path))
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

  if (!value.empty())
    id_ = GenerateId(value);

  is_valid_ = true;

  scoped_ptr<base::ScopedFILE> file(
        new base::ScopedFILE(base::OpenFile(path, "rb")));

  file_ = file.Pass();
}

}  // namespace application
}  // namespace xwalk
