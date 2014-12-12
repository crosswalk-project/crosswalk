// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/tizen/xwalk_rds_delta_parser.h"

#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/string_split.h"

namespace {

const char kRDSDeltaFile[] = ".rds_delta";
const std::string kRDSWidgetPath = "res/wgt/";
const std::string kKeyAdd = "#add";
const std::string kKeyDelete = "#delete";
const std::string kKeyModify = "#modify";
const base::FilePath kRDSTempDir = base::FilePath("/opt/usr/apps/tmp/");
const std::list<std::string> key_list = {kKeyDelete, kKeyAdd, kKeyModify};

}  // namespace

RDSDeltaParser::RDSDeltaParser(const base::FilePath& app_path,
    const std::string& pkgid) : app_dir_(app_path) {
  rds_dir_ = kRDSTempDir.Append(pkgid);
}

bool RDSDeltaParser::Parse() {
  base::FilePath rds_file = rds_dir_.AppendASCII(kRDSDeltaFile);

  if (!base::PathExists(rds_file)) {
    LOG(ERROR) << "RDS delta file " << rds_file.value()
               << " does not exists!";
    return false;
  }

  std::string file_buffer;
  if (!base::ReadFileToString(rds_file, &file_buffer)) {
    LOG(ERROR) << "RDS delta file " << rds_file.value()
               << " cannot be read!";
    return false;
  }

  std::vector<std::string> lines;
  base::SplitString(file_buffer, '\n', &lines);

  if (lines.empty()) {
    LOG(ERROR) << "RDS delta file " << rds_file.value() << " is empty!";
    return false;
  }

  std::string key;
  parsed_data_.clear();
  for (const auto& line : lines) {
    for (const auto& it : key_list) {
      if (line == it) {
        key = line;
        break;
      }
    }
    if (key == line || line.empty() || line == "\n") {
      continue;
    }
    parsed_data_.insert(std::pair<std::string, std::string>(key,
        line.substr(kRDSWidgetPath.length())));
  }
  return true;
}

bool RDSDeltaParser::ApplyParsedData() {
  for (const auto& it : parsed_data_) {
    if (it.first == kKeyDelete)
      return DeleteFile(it.second);
    if (it.first == kKeyAdd)
      return AddFile(it.second);
    if (it.first == kKeyModify)
      return ModifyFile(it.second);
  }
  return true;
}

bool RDSDeltaParser::AddFile(const std::string& file_name) {
  base::FilePath src_path = rds_dir_.Append(kRDSWidgetPath).Append(file_name);

  if (!base::PathExists(src_path)) {
    LOG(ERROR) << "File " << src_path.value() << " does not exists!";
    return false;
  }
  base::FilePath dst_file = app_dir_.Append(file_name);
  if (!base::DirectoryExists(dst_file.DirName()) &&
      !base::CreateDirectory(dst_file.DirName())) {
    LOG(ERROR) << "Can't create directory " << dst_file.DirName().value();
    return false;
  }
  if (!base::CopyFile(src_path, dst_file)) {
    LOG(ERROR) << "Error when adding a file " << src_path.BaseName().value()
               << " to " << dst_file.DirName().value();
    return false;
  }
  return true;
}

bool RDSDeltaParser::DeleteFile(const std::string& file_name) {
  base::FilePath dst_file = app_dir_.Append(file_name);

  if (!base::DeleteFile(dst_file, true)) {
    LOG(ERROR) << "Error when deleting a file " << dst_file.value();
    return false;
  }
  return true;
}

bool RDSDeltaParser::ModifyFile(const std::string& file_name) {
  base::FilePath src_file = rds_dir_.Append(kRDSWidgetPath).Append(file_name);

  if (!base::PathExists(src_file)) {
    LOG(ERROR) << "File " << src_file.value() << " does not exists!";
    return false;
  }

  base::FilePath dst_file = app_dir_.Append(file_name);
  if (!base::CopyFile(src_file, dst_file)) {
    LOG(ERROR) << "Error when copying a file " << src_file.value()
               << " to " << dst_file.DirName().value();
    return false;
  }
  return true;
}
