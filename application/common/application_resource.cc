// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_resource.h"

#include <vector>

#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/threading/thread_restrictions.h"

namespace xwalk {
namespace application {
namespace {
const base::FilePath::StringType WGT_LOCALE_DIRECTORY =
    FILE_PATH_LITERAL("locales");
}  // namespace

ApplicationResource::ApplicationResource() : follow_symlinks_anywhere_(false) {
}

ApplicationResource::ApplicationResource(const std::string& application_id,
                                     const base::FilePath& application_root,
                                     const base::FilePath& relative_path)
    : application_id_(application_id),
      application_root_(application_root),
      relative_path_(relative_path),
      follow_symlinks_anywhere_(false) {
}

ApplicationResource::~ApplicationResource() {}

void ApplicationResource::set_follow_symlinks_anywhere() {
  follow_symlinks_anywhere_ = true;
}

const base::FilePath& ApplicationResource::GetFilePath() const {
  if (application_root_.empty() || relative_path_.empty()) {
    DCHECK(full_resource_path_.empty());
    return full_resource_path_;
  }

  // We've already checked, just return last value.
  if (!full_resource_path_.empty())
    return full_resource_path_;

  for (std::list<std::string>::const_iterator it = locales_.begin();
       it != locales_.end(); ++it) {
    full_resource_path_ = GetFilePath(
        application_root_,
        base::FilePath(WGT_LOCALE_DIRECTORY)
        .AppendASCII(*it).Append(relative_path_),
        follow_symlinks_anywhere_ ?
        FOLLOW_SYMLINKS_ANYWHERE : SYMLINKS_MUST_RESOLVE_WITHIN_ROOT);
    if (!full_resource_path_.empty())
      return full_resource_path_;
  }
  full_resource_path_ = GetFilePath(
      application_root_, relative_path_,
      follow_symlinks_anywhere_ ?
          FOLLOW_SYMLINKS_ANYWHERE : SYMLINKS_MUST_RESOLVE_WITHIN_ROOT);
  return full_resource_path_;
}

// static
base::FilePath ApplicationResource::GetFilePath(
    const base::FilePath& application_root,
    const base::FilePath& relative_path,
    SymlinkPolicy symlink_policy) {
  // We need to resolve the parent references in the application_root
  // path on its own because IsParent doesn't like parent references.
  base::FilePath clean_application_root(
      base::MakeAbsoluteFilePath(application_root));
  if (clean_application_root.empty())
    return base::FilePath();

  base::FilePath full_path = clean_application_root.Append(relative_path);

  // If we are allowing the file to be a symlink outside of the root, then the
  // path before resolving the symlink must still be within it.
  if (symlink_policy == FOLLOW_SYMLINKS_ANYWHERE) {
    std::vector<base::FilePath::StringType> components;
    relative_path.GetComponents(&components);
    int depth = 0;

    for (std::vector<base::FilePath::StringType>::const_iterator
         i = components.begin(); i != components.end(); i++) {
      if (*i == base::FilePath::kParentDirectory) {
        depth--;
      } else if (*i != base::FilePath::kCurrentDirectory) {
        depth++;
      }
      if (depth < 0) {
        return base::FilePath();
      }
    }
  }

  // We must resolve the absolute path of the combined path when
  // the relative path contains references to a parent folder (i.e., '..').
  // We also check if the path exists because the posix version of
  // MakeAbsoluteFilePath will fail if the path doesn't exist, and we want the
  // same behavior on Windows... So until the posix and Windows version of
  // MakeAbsoluteFilePath are unified, we need an extra call to PathExists,
  // unfortunately.
  // TODO(mad): Fix this once MakeAbsoluteFilePath is unified.
  full_path = base::MakeAbsoluteFilePath(full_path);
  if (base::PathExists(full_path) &&
      (symlink_policy == FOLLOW_SYMLINKS_ANYWHERE ||
       clean_application_root.IsParent(full_path))) {
    return full_path;
  }

  return base::FilePath();
}

// Unit-testing helpers.
base::FilePath::StringType ApplicationResource::NormalizeSeperators(
    const base::FilePath::StringType& path) const {
#if defined(FILE_PATH_USES_WIN_SEPARATORS)
  base::FilePath::StringType win_path = path;
  for (size_t i = 0; i < win_path.length(); i++) {
    if (base::FilePath::IsSeparator(win_path[i]))
      win_path[i] = base::FilePath::kSeparators[0];
  }
  return win_path;
#else
  return path;
#endif  // FILE_PATH_USES_WIN_SEPARATORS
}

bool ApplicationResource::ComparePathWithDefault(
    const base::FilePath& path) const {
  // Make sure we have a cached value to test against...
  if (full_resource_path_.empty())
    GetFilePath();
  if (NormalizeSeperators(path.value()) ==
    NormalizeSeperators(full_resource_path_.value())) {
    return true;
  } else {
    return false;
  }
}

}  // namespace application
}  // namespace xwalk
