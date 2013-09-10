// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_RESOURCE_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_RESOURCE_H_

#include <string>

#include "base/files/file_path.h"

namespace xwalk {
namespace application {

// Represents a resource inside an application. For example, an image, or a
// JavaScript file. This is more complicated than just a simple FilePath
// because application resources can come from multiple physical file locations
// depending on locale.
class ApplicationResource {
 public:
  // SymlinkPolicy decides whether we'll allow resources to be a symlink to
  // anywhere, or whether they must end up within the application root.
  enum SymlinkPolicy {
    SYMLINKS_MUST_RESOLVE_WITHIN_ROOT,
    FOLLOW_SYMLINKS_ANYWHERE,
  };

  ApplicationResource();

  ApplicationResource(const std::string& application_id,
                    const base::FilePath& application_root,
                    const base::FilePath& relative_path);

  ~ApplicationResource();

  // set_follow_symlinks_anywhere allows the resource to be a symlink to
  // anywhere in the filesystem. By default, resources have to be within
  // |application_root| after resolving symlinks.
  void set_follow_symlinks_anywhere();

  // Returns actual path to the resource (default or locale specific). In the
  // browser process, this will DCHECK if not called on the file thread. To
  // easily load application images on the UI thread, see ImageLoader.
  const base::FilePath& GetFilePath() const;

  // Gets the physical file path for the application resource, taking into
  // account localization. In the browser process, this will DCHECK if not
  // called on the file thread. To easily load application images on the UI
  // thread, see ImageLoader.
  //
  // The relative path must not resolve to a location outside of
  // |application_root|. Iff |file_can_symlink_outside_root| is true, then the
  // file can be a symlink that links outside of |application_root|.
  static base::FilePath GetFilePath(const base::FilePath& application_root,
                                    const base::FilePath& relative_path,
                                    SymlinkPolicy symlink_policy);

  // Getters
  const std::string& application_id() const { return application_id_; }
  const base::FilePath& application_root() const { return application_root_; }
  const base::FilePath& relative_path() const { return relative_path_; }

  bool empty() const { return application_root().empty(); }

  // Unit test helpers.
  base::FilePath::StringType NormalizeSeperators(
      const base::FilePath::StringType& path) const;
  bool ComparePathWithDefault(const base::FilePath& path) const;

 private:
  // The id of the application that this resource is associated with.
  std::string application_id_;

  // Application root.
  base::FilePath application_root_;

  // Relative path to resource.
  base::FilePath relative_path_;

  // If |follow_symlinks_anywhere_| is true then the resource itself must be
  // within |application_root|, but it can be a symlink to a file that is not.
  bool follow_symlinks_anywhere_;

  // Full path to application resource. Starts empty.
  mutable base::FilePath full_resource_path_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_RESOURCE_H_
