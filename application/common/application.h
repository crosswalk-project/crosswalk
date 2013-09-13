// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_H_

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
#include "base/synchronization/lock.h"
#include "base/threading/thread_checker.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/install_warning.h"
#include "url/gurl.h"

namespace base {
class DictionaryValue;
class ListValue;
class Version;
}

#if defined(OS_TIZEN_MOBILE)
namespace tizen {
class AppcoreContext;
}
#endif

namespace xwalk {
namespace application {

// Represents a xwalk application.
// Once created, an Application object is immutable, with the exception of its
// RuntimeData. This makes it safe to use on any thread, since access to the
// RuntimeData is protected by a lock.
class Application : public base::RefCountedThreadSafe<Application> {
 public:
  struct ManifestData;

  typedef std::map<const std::string, linked_ptr<ManifestData> >
      ManifestDataMap;

  // A base class for parsed manifest data that APIs want to store on
  // the application. Related to base::SupportsUserData, but with an immutable
  // thread-safe interface to match Application.
  struct ManifestData {
    virtual ~ManifestData() {}
  };

  static scoped_refptr<Application> Create(const base::FilePath& path,
      Manifest::SourceType source_type,
      const base::DictionaryValue& manifest_data,
      const std::string& explicit_id,
      std::string* error_message);

  // Checks to see if the application has a valid ID.
  static bool IsIDValid(const std::string& id);

  Manifest::Type GetType() const;

  // Returns an absolute url to a resource inside of an application. The
  // |application_url| argument should be the url() from an Application object.
  // The |relative_path| can be untrusted user input. The returned URL will
  // either be invalid() or a child of |application_url|.
  // NOTE: Static so that it can be used from multiple threads.
  static GURL GetResourceURL(const GURL& application_url,
                             const std::string& relative_path);
  GURL GetResourceURL(const std::string& relative_path) const {
    return GetResourceURL(URL(), relative_path);
  }

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

  const base::FilePath& Path() const { return path_; }
  const GURL& URL() const { return application_url_; }
  Manifest::SourceType GetSourceType() const;
  const std::string& ID() const;
  const base::Version* Version() const { return version_.get(); }
  const std::string VersionString() const;
  const std::string& Name() const { return name_; }
  const std::string& NonLocalizedName() const { return non_localized_name_; }
  const std::string& Description() const { return description_; }
  int ManifestVersion() const { return manifest_version_; }

  const Manifest* GetManifest() const {
    return manifest_.get();
  }

  // App-related.
  bool IsPlatformApp() const;
  bool IsHostedApp() const;

 private:
  friend class base::RefCountedThreadSafe<Application>;

  // Chooses the application ID for an application based on a variety of
  // criteria. The chosen ID will be set in |manifest|.
  static bool InitApplicationID(Manifest* manifest,
                              const base::FilePath& path,
                              const std::string& explicit_id,
                              string16* error);

  Application(const base::FilePath& path,
            scoped_ptr<Manifest> manifest);
  virtual ~Application();

  // Initialize the application from a parsed manifest.
  bool Init(string16* error);

  // The following are helpers for InitFromValue to load various features of the
  // application from the manifest.
  bool LoadName(string16* error);
  bool LoadVersion(string16* error);
  bool LoadDescription(string16* error);
  bool LoadManifestVersion(string16* error);

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

  // Any warnings that occurred when trying to create/parse the application.
  std::vector<InstallWarning> install_warnings_;

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

#if defined(OS_TIZEN_MOBILE)
  scoped_ptr<tizen::AppcoreContext> appcore_context_;
#endif

  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_H_
