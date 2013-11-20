// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handler.h"

#include <set>

#include "base/stl_util.h"
#include "xwalk/application/common/manifest_handlers/permissions_handler.h"

namespace xwalk {
namespace application {

ManifestHandler::~ManifestHandler() {
}

bool ManifestHandler::Validate(scoped_refptr<const Application> application,
                               std::string* error,
                               std::vector<InstallWarning>* warnings) const {
  return true;
}

bool ManifestHandler::AlwaysParseForType(Manifest::Type type) const {
  return false;
}

bool ManifestHandler::AlwaysValidateForType(Manifest::Type type) const {
  return false;
}

std::vector<std::string> ManifestHandler::PrerequisiteKeys() const {
  return std::vector<std::string>();
}

ManifestHandlerRegistry* ManifestHandlerRegistry::registry_ = NULL;

ManifestHandlerRegistry::ManifestHandlerRegistry(
    const std::vector<ManifestHandler*>& handlers) {
  for (std::vector<ManifestHandler*>::const_iterator it = handlers.begin();
       it != handlers.end(); ++it) {
    Register(*it);
  }

  ReorderHandlersGivenDependencies();
}

ManifestHandlerRegistry::~ManifestHandlerRegistry() {
}

// static
ManifestHandlerRegistry* ManifestHandlerRegistry::GetInstance() {
  if (!registry_) {
    std::vector<ManifestHandler*> handlers;
    // FIXME: Add manifest handlers here like this:
    // handlers.push_back(new xxxHandler);
    handlers.push_back(new PermissionsHandler);

    registry_ = new ManifestHandlerRegistry(handlers);
  }
  return registry_;
}

void ManifestHandlerRegistry::Register(ManifestHandler* handler) {
  const std::vector<std::string>& keys = handler->Keys();
  for (size_t i = 0; i < keys.size(); ++i) {
    handlers_[keys[i]] = handler;
  }
}

bool ManifestHandlerRegistry::ParseAppManifest(
    scoped_refptr<Application> application, string16* error) {
  std::map<int, ManifestHandler*> handlers_by_order;
  for (ManifestHandlerMap::iterator iter = handlers_.begin();
       iter != handlers_.end(); ++iter) {
    ManifestHandler* handler = iter->second;
    if (application->GetManifest()->HasPath(iter->first) ||
        handler->AlwaysParseForType(application->GetType())) {
      handlers_by_order[order_map_[handler]] = handler;
    }
  }
  for (std::map<int, ManifestHandler*>::iterator iter =
           handlers_by_order.begin();
       iter != handlers_by_order.end(); ++iter) {
    if (!(iter->second)->Parse(application, error))
      return false;
  }
  return true;
}

bool ManifestHandlerRegistry::ValidateAppManifest(
    scoped_refptr<const Application> application,
    std::string* error,
    std::vector<InstallWarning>* warnings) {
  for (ManifestHandlerMap::iterator iter = handlers_.begin();
       iter != handlers_.end(); ++iter) {
    ManifestHandler* handler = iter->second;
    if ((application->GetManifest()->HasPath(iter->first) ||
         handler->AlwaysValidateForType(application->GetType())) &&
        !handler->Validate(application, error, warnings))
      return false;
  }
  return true;
}

// static
void ManifestHandlerRegistry::SetInstanceForTesting(
    ManifestHandlerRegistry* registry) {
    registry_ = registry;
}

void ManifestHandlerRegistry::ReorderHandlersGivenDependencies() {
  std::set<ManifestHandler*> unsorted_handlers;
  for (ManifestHandlerMap::const_iterator iter = handlers_.begin();
       iter != handlers_.end(); ++iter) {
    unsorted_handlers.insert(iter->second);
  }

  int order = 0;
  while (true) {
    std::set<ManifestHandler*> next_unsorted_handlers;
    for (std::set<ManifestHandler*>::const_iterator iter =
             unsorted_handlers.begin();
         iter != unsorted_handlers.end(); ++iter) {
      ManifestHandler* handler = *iter;
      const std::vector<std::string>& prerequisites =
          handler->PrerequisiteKeys();
      int unsatisfied = prerequisites.size();
      for (size_t i = 0; i < prerequisites.size(); ++i) {
        ManifestHandlerMap::const_iterator prereq_iter =
            handlers_.find(prerequisites[i]);
        CHECK(prereq_iter != handlers_.end())
            << "Application manifest handler depends on unrecognized key "
            << prerequisites[i];
        // Prerequisite is in our map.
        if (ContainsKey(order_map_, prereq_iter->second))
          unsatisfied--;
      }
      if (unsatisfied == 0) {
        order_map_[handler] = order;
        order++;
      } else {
        // Put in the list for next time.
        next_unsorted_handlers.insert(handler);
      }
    }
    if (next_unsorted_handlers.size() == unsorted_handlers.size())
      break;
    unsorted_handlers.swap(next_unsorted_handlers);
  }

  // If there are any leftover unsorted handlers, they must have had
  // circular dependencies.
  CHECK_EQ(unsorted_handlers.size(), 0) << "Application manifest handlers have "
                                        << "circular dependencies!";
}

}  // namespace application
}  // namespace xwalk
