// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_H_

#include <map>
#include <string>
#include <set>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/string16.h"
#include "base/values.h"
#include "xwalk/application/common/install_warning.h"

namespace xwalk_application {
struct InstallWarning;

// Wraps the DictionaryValue form of application's manifest. Enforces access to
// properties of the manifest using ManifestFeatureProvider.
class Manifest {
 public:
  // What an application was loaded from.
  // NOTE: These values are stored as integers in the preferences and used
  // in histograms so don't remove or reorder existing items.  Just append
  // to the end.
  enum Location {
    INVALID_LOCATION,
    UNPACKED,           // From loading an unpacked application from the
                        // applications settings page.
    NUM_LOCATIONS
  };

  enum Type {
    TYPE_UNKNOWN = 0,
    TYPE_HOSTED_APP,
    TYPE_PLATFORM_APP
  };

  Manifest(Location location, scoped_ptr<DictionaryValue> value);
  virtual ~Manifest();

  const std::string& application_id() const { return application_id_; }
  void set_application_id(const std::string& id) { application_id_ = id; }

  Location location() const { return location_; }

  // Returns false and |error| will be non-empty if the manifest is malformed.
  // |warnings| will be populated if there are keys in the manifest that cannot
  // be specified by the application type.
  bool ValidateManifest(std::string* error,
                        std::vector<InstallWarning>* warnings) const;

  // The version of this application's manifest. We increase the manifest
  // version when making breaking changes to the application system. If the
  // manifest contains no explicit manifest version, this returns the current
  // system default.
  int GetManifestVersion() const;

  // Returns the manifest type.
  Type type() const { return type_; }

  bool is_platform_app() const { return type_ == TYPE_PLATFORM_APP; }
  bool is_hosted_app() const { return type_ == TYPE_HOSTED_APP; }

  // These access the wrapped manifest value, returning false when the property
  // does not exist or if the manifest type can't access it.
  bool HasKey(const std::string& key) const;
  bool HasPath(const std::string& path) const;
  bool Get(const std::string& path, const base::Value** out_value) const;
  bool GetBoolean(const std::string& path, bool* out_value) const;
  bool GetInteger(const std::string& path, int* out_value) const;
  bool GetString(const std::string& path, std::string* out_value) const;
  bool GetString(const std::string& path, string16* out_value) const;
  bool GetDictionary(const std::string& path,
                     const base::DictionaryValue** out_value) const;
  bool GetList(const std::string& path,
               const base::ListValue** out_value) const;

  // Returns a new Manifest equal to this one, passing ownership to
  // the caller.
  Manifest* DeepCopy() const;

  // Returns true if this equals the |other| manifest.
  bool Equals(const Manifest* other) const;

  // Gets the underlying DictionaryValue representing the manifest.
  // Note: only use this when you KNOW you don't need the validation.
  const base::DictionaryValue* value() const { return value_.get(); }

 private:
  // Returns true if the application can specify the given |path|.
  bool CanAccessPath(const std::string& path) const;
  bool CanAccessKey(const std::string& key) const;

  // A persistent, globally unique ID. An application's ID is used in things
  // like directory structures and URLs, and is expected to not change across
  // versions. It is generated as a SHA-256 hash of the application's public
  // key, or as a hash of the path in the case of unpacked applications.
  std::string application_id_;

  // The location the application was loaded from.
  Location location_;

  // The underlying dictionary representation of the manifest.
  scoped_ptr<base::DictionaryValue> value_;

  Type type_;

  DISALLOW_COPY_AND_ASSIGN(Manifest);
};

}  // namespace xwalk_application

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_H_
