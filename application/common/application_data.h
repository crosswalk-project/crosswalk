// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_DATA_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_DATA_H_

#include <algorithm>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_util.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread_checker.h"
#include "url/gurl.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/permission_types.h"
#include "xwalk/application/common/package/package.h"

namespace base {
class DictionaryValue;
class ListValue;
class Version;
}

namespace xwalk {
namespace application {

class ApplicationData : public base::RefCountedThreadSafe<ApplicationData> {
 public:
  // Where an application was loaded from.
  enum SourceType {
    INTERNAL,         // From internal application registry.
    LOCAL_DIRECTORY,  // From a persistently stored unpacked application
    TEMP_DIRECTORY,   // From a temporary folder
    EXTERNAL_URL      // From an arbitrary URL
  };

  struct ManifestData;

  struct ApplicationIdCompare {
    bool operator()(const std::string& s1, const std::string& s2) const {
      return base::strcasecmp(s1.c_str(), s2.c_str()) < 0;
    }
  };

  typedef std::map<const std::string, linked_ptr<ManifestData> >
      ManifestDataMap;
  typedef std::map<std::string,
      scoped_refptr<ApplicationData>, ApplicationIdCompare>
        ApplicationDataMap;
  typedef ApplicationDataMap::iterator ApplicationDataMapIterator;

  // A base class for parsed manifest data that APIs want to store on
  // the application. Related to base::SupportsUserData, but with an immutable
  // thread-safe interface to match Application.
  struct ManifestData {
    virtual ~ManifestData() {}
  };

  static scoped_refptr<ApplicationData> Create(const base::FilePath& app_path,
      const std::string& explicit_id, SourceType source_type,
          scoped_ptr<Manifest> manifest, std::string* error_message);

  // Returns an absolute url to a resource inside of an application. The
  // |application_url| argument should be the url() from an Application object.
  // The |relative_path| can be untrusted user input. The returned URL will
  // either be invalid() or a child of |application_url|.
  // NOTE: Static so that it can be used from multiple threads.
  static GURL GetResourceURL(const GURL& application_url,
                             const std::string& relative_path);
  GURL GetResourceURL(const std::string& relative_path) const;

  // Returns the base application url for a given |application_id|.
  static GURL GetBaseURLFromApplicationId(const std::string& application_id);

  // Get the manifest data associated with the key, or NULL if there is none.
  // Can only be called after InitValue is finished.
  ManifestData* GetManifestData(const std::string& key) const;

  // Sets |data| to be associated with the key. Takes ownership of |data|.
  // Can only be called before InitValue is finished. Not thread-safe;
  // all SetManifestData calls should be on only one thread.
  void SetManifestData(const std::string& key, ManifestData* data);

  // Accessors:
  const base::FilePath& path() const { return path_; }
#if defined(OS_TIZEN)  // FIXME : This method should be removed.
  void set_path(const base::FilePath& path) { path_ = path; }
#endif
  const GURL& URL() const { return application_url_; }
  SourceType source_type() const { return source_type_; }
  Manifest::Type manifest_type() const { return manifest_->type(); }
  const std::string& ID() const { return application_id_; }
#if defined(OS_TIZEN)
  std::string GetPackageID() const;
#endif
  const base::Version* Version() const { return version_.get(); }
  const std::string VersionString() const;
  const std::string& Name() const { return name_; }
  const std::string& NonLocalizedName() const { return non_localized_name_; }
  const std::string& Description() const { return description_; }

  const Manifest* GetManifest() const {
    return manifest_.get();
  }

  // App-related.
  bool IsHostedApp() const;

  // Permission related.
  StoredPermission GetPermission(
      const std::string& permission_name) const;
  bool SetPermission(const std::string& permission_name,
                     StoredPermission perm);
  void ClearPermissions();
  PermissionSet GetManifestPermissions() const;

  bool HasCSPDefined() const;

  bool SetApplicationLocale(const std::string& locale, base::string16* error);

 private:
  friend class base::RefCountedThreadSafe<ApplicationData>;
  friend class ApplicationStorageImpl;

  ApplicationData(const base::FilePath& path,
      SourceType source_type, scoped_ptr<Manifest> manifest);
  virtual ~ApplicationData();

  // Initialize the application from a parsed manifest.
  bool Init(const std::string& explicit_id, base::string16* error);

  // Chooses the application ID for an application based on a variety of
  // criteria. The chosen ID will be set in |manifest|.
  bool LoadID(const std::string& explicit_id, base::string16* error);
  // The following are helpers for InitFromValue to load various features of the
  // application from the manifest.
  bool LoadName(base::string16* error);
  bool LoadVersion(base::string16* error);
  bool LoadDescription(base::string16* error);

  // The application's human-readable name. Name is used for display purpose. It
  // might be wrapped with unicode bidi control characters so that it is
  // displayed correctly in RTL context.
  // NOTE: Name is UTF-8 and may contain non-ascii characters.
  std::string name_;

  // A non-localized version of the application's name. This is useful for
  // debug output.
  std::string non_localized_name_;

  // The version of this application's manifest. We increase the manifest
  // version when making breaking changes to the application system.
  // Version 1 was the first manifest version (implied by a lack of a
  // manifest_version attribute in the application's manifest). We initialize
  // this member variable to 0 to distinguish the "uninitialized" case from
  // the case when we know the manifest version actually is 1.
  int manifest_version_;

  // The absolute path to the directory the application is stored in.
  base::FilePath path_;

  // A persistent, globally unique ID. An application's ID is used in things
  // like directory structures and URLs, and is expected to not change across
  // versions.
  std::string application_id_;

  // The base application url for the application.
  GURL application_url_;

  // The application's version.
  scoped_ptr<base::Version> version_;

  // An optional longer description of the application.
  std::string description_;

  // The manifest from which this application was created.
  scoped_ptr<Manifest> manifest_;

  // Stored parsed manifest data.
  ManifestDataMap manifest_data_;

  // Set to true at the end of InitValue when initialization is finished.
  bool finished_parsing_manifest_;

  // Ensures that any call to GetManifestData() prior to finishing
  // initialization happens from the same thread (this can happen when certain
  // parts of the initialization process need information from previous parts).
  base::ThreadChecker thread_checker_;

  // Application's persistent permissions.
  StoredPermissionMap permission_map_;

  // The source the application was loaded from.
  SourceType source_type_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationData);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_DATA_H_
