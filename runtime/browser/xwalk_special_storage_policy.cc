// Copyright (c) 2016 Medic Mobile, Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_special_storage_policy.h"

using namespace xwalk;

XWalkSpecialStoragePolicy::XWalkSpecialStoragePolicy(void) {}

XWalkSpecialStoragePolicy::~XWalkSpecialStoragePolicy() {}

bool XWalkSpecialStoragePolicy::IsStorageProtected(const GURL& origin) {
  // Prevent data being removed by the browsing data remover.
  return true;
}

bool XWalkSpecialStoragePolicy::IsStorageUnlimited(const GURL& origin) {
  // Prevent data being affected by quota or storage pressure eviction.
  return true;
}

bool XWalkSpecialStoragePolicy::IsStorageDurable(const GURL& origin) {
  // Prevent storage pressure eviction.
  return true;
}

bool XWalkSpecialStoragePolicy::CanQueryDiskSize(const GURL& origin) {
  // Let anything query remaining disk size.  This could potentially allow for
  // fingerprinting of a user, but seems less of a risk in a crosswalk app than
  // it would in a more general browser.
  return true;
}

bool XWalkSpecialStoragePolicy::HasIsolatedStorage(const GURL& origin) {
  // I don't actually know if Crosswalk can guarantee if there is isolated
  // storage or not.  Chrome seems to implement this through a special plugin,
  // so it seems safe to say that we can't guarantee that storage is isolated.
  return false;
}

bool XWalkSpecialStoragePolicy::IsStorageSessionOnly(const GURL& origin) {
  // As per HasSessionOnlyOrigins(), no origins are session-only.
  return false;
}

bool XWalkSpecialStoragePolicy::HasSessionOnlyOrigins() {
  // Do not allow any origins to have session-only storage.
  return false;
}
