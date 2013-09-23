// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "base/memory/linked_ptr.h"
#include "base/strings/string16.h"
#include "xwalk/application/common/application.h"

namespace xwalk {
namespace application {

class ManifestHandler {
 public:
  virtual ~ManifestHandler();

  // Returns false in case of failure and sets writes error message
  // in |error| if present.
  virtual bool Parse(scoped_refptr<Application> application,
                     string16* error) = 0;

  // Returns false in case of failure and sets writes error message
  // in |error| if present.
  virtual bool Validate(scoped_refptr<const Application> application,
                        std::string* error,
                        std::vector<InstallWarning>* warnings) const;

  // If false (the default), only parse the manifest if a registered
  // key is present in the manifest. If true, always attempt to parse
  // the manifest for this application type, even if no registered keys
  // are present. This allows specifying a default parsed value for
  // application that don't declare our key in the manifest.
  virtual bool AlwaysParseForType(Manifest::Type type) const;

  // Same as AlwaysParseForType, but for Validate instead of Parse.
  virtual bool AlwaysValidateForType(Manifest::Type type) const;

  // The list of keys that, if present, should be parsed before calling our
  // Parse (typically, because our Parse needs to read those keys).
  // Defaults to empty.
  virtual std::vector<std::string> PrerequisiteKeys() const;

  // The keys to register handler for (in Register).
  virtual std::vector<std::string> Keys() const = 0;
};

class ManifestHandlerRegistry {
 public:
  ~ManifestHandlerRegistry();

  static ManifestHandlerRegistry* GetInstance();

  bool ParseAppManifest(
       scoped_refptr<Application> application, string16* error);
  bool ValidateAppManifest(scoped_refptr<const Application> application,
                           std::string* error,
                           std::vector<InstallWarning>* warnings);

 private:
  friend class ScopedTestingManifestHandlerRegistry;
  explicit ManifestHandlerRegistry(
       const std::vector<ManifestHandler*>& handlers);

  // Register a manifest handler for keys, which are provided by Keys() method
  // in ManifestHandler implementer.
  void Register(ManifestHandler* handler);

  void ReorderHandlersGivenDependencies();

  // Sets a new global registry, for testing purposes.
  static void SetInstanceForTesting(ManifestHandlerRegistry* registry);

  typedef std::map<std::string, ManifestHandler*> ManifestHandlerMap;
  typedef std::map<ManifestHandler*, int> ManifestHandlerOrderMap;

  ManifestHandlerMap handlers_;

  // Handlers are executed in order; lowest order first.
  ManifestHandlerOrderMap order_map_;

  static ManifestHandlerRegistry* registry_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLER_H_
