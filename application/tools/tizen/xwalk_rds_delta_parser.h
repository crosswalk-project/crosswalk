// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_TIZEN_XWALK_RDS_DELTA_PARSER_H_
#define XWALK_APPLICATION_TOOLS_TIZEN_XWALK_RDS_DELTA_PARSER_H_

#include <map>
#include <string>

#include "base/files/file_path.h"

class RDSDeltaParser {
 public:
  RDSDeltaParser(const base::FilePath& app_path, const std::string& pkgid);
  bool Parse();
  bool ApplyParsedData();

 private:
  base::FilePath app_dir_;
  base::FilePath rds_dir_;
  std::multimap<std::string, std::string> parsed_data_;

  bool AddFile(const std::string& file_name);
  bool DeleteFile(const std::string& file_name);
  bool ModifyFile(const std::string& file_name);
};

#endif  // XWALK_APPLICATION_TOOLS_TIZEN_XWALK_RDS_DELTA_PARSER_H_
