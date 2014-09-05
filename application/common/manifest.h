// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_H_

#include <list>
#include <map>
#include <string>
#include <set>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/strings/string16.h"
#include "base/values.h"
#include "xwalk/application/common/installer/package.h"

namespace xwalk {
namespace application {

// Wraps the DictionaryValue form of application's manifest. Enforces access to
// properties of the manifest using ManifestFeatureProvider.
class Manifest {
 public:
  // Where an application was loaded from.
  enum SourceType {
    INVALID_TYPE,
    INTERNAL,           // Load from internal application registry.
    COMMAND_LINE,       // Load from an unpacked application from command line.
    NUM_TYPES
  };

  enum Type {
    TYPE_UNKNOWN = 0,
    TYPE_HOSTED_APP,
    TYPE_PACKAGED_APP
  };

  Manifest(SourceType source_type, scoped_ptr<base::DictionaryValue> value);
  ~Manifest();

  const std::string& GetApplicationID() const { return application_id_; }
  void SetApplicationID(const std::string& id) { application_id_ = id; }

  SourceType GetSourceType() const { return source_type_; }

  // Returns false and |error| will be non-empty if the manifest is malformed.
  // |warnings| will be populated if there are keys in the manifest that cannot
  // be specified by the application type.
  bool ValidateManifest(std::string* error) const;

  // Returns the manifest type.
  Type GetType() const { return type_; }

  bool IsPackaged() const { return type_ == TYPE_PACKAGED_APP; }
  bool IsHosted() const { return type_ == TYPE_HOSTED_APP; }

  // These access the wrapped manifest value, returning false when the property
  // does not exist or if the manifest type can't access it.
  bool HasKey(const std::string& key) const;
  bool HasPath(const std::string& path) const;
  bool Get(const std::string& path, const base::Value** out_value) const;
  bool Get(const std::string& path, base::Value** out_value) const;
  bool GetBoolean(const std::string& path, bool* out_value) const;
  bool GetInteger(const std::string& path, int* out_value) const;

  // If the path is supported by i18n, we can get a locale string from
  // this two GetString function. The following is locale priority:
  // Application locale (locale get from system).                 | high
  // Default locale (defaultlocale attribute of widget element)
  // Unlocalized (the element without xml:lang attribute)
  // Auto ("en-us"(tizen is "en-gb") will be considered as a default)
  // First (the worst case we get the first element)              | low
  bool GetString(const std::string& path, std::string* out_value) const;
  bool GetString(const std::string& path, base::string16* out_value) const;

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
  const base::DictionaryValue* value() const { return data_.get(); }

  const std::string& default_locale() const {
    return default_locale_;
  }

  // Update user agent locale when system locale is changed.
  void SetSystemLocale(const std::string& locale);

 private:
  void ParseWGTI18n();
  void ParseWGTI18nEachPath(const std::string& path);
  bool ParseWGTI18nEachElement(base::Value* value,
                               const std::string& path,
                               const std::string& locale = "");

  // Returns true if the application can specify the given |path|.
  bool CanAccessPath(const std::string& path) const;
  bool CanAccessKey(const std::string& key) const;

  // A persistent, globally unique ID. An application's ID is used in things
  // like directory structures and URLs, and is expected to not change across
  // versions. It is generated as a SHA-256 hash of the application's public
  // key, or as a hash of the path in the case of unpacked applications.
  std::string application_id_;

#if defined(OS_TIZEN)
  // Unique package id for tizen platform
  std::string package_id_;
#endif

  // The source the application was loaded from.
  SourceType source_type_;

  // The underlying dictionary representation of the manifest.
  scoped_ptr<base::DictionaryValue> data_;
  scoped_ptr<base::DictionaryValue> i18n_data_;

  std::string default_locale_;
  scoped_ptr<std::list<std::string> > user_agent_locales_;

  Type type_;

  DISALLOW_COPY_AND_ASSIGN(Manifest);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_H_
