// Copyright (c) 2016 Medic Mobile, Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_SPECIAL_STORAGE_POLICY_H_
#define XWALK_RUNTIME_BROWSER_XWALK_SPECIAL_STORAGE_POLICY_H_

#include "storage/browser/quota/special_storage_policy.h"

namespace xwalk {

class XWalkSpecialStoragePolicy : public storage::SpecialStoragePolicy {
 public:
  XWalkSpecialStoragePolicy();

  // storage::SpecialStoragePolicy methods used by storage subsystems and the
  // browsing data remover. These methods are safe to call on any thread.
  bool IsStorageProtected(const GURL& origin) override;
  bool IsStorageUnlimited(const GURL& origin) override;
  bool IsStorageSessionOnly(const GURL& origin) override;
  bool CanQueryDiskSize(const GURL& origin) override;
  bool HasIsolatedStorage(const GURL& origin) override;
  bool HasSessionOnlyOrigins() override;
  bool IsStorageDurable(const GURL& origin) override;

 protected:
  ~XWalkSpecialStoragePolicy() override;
};

}  // namespace xwalk

#endif // XWALK_RUNTIME_BROWSER_XWALK_SPECIAL_STORAGE_POLICY_H_
