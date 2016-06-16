// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PREF_STORE_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PREF_STORE_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/observer_list.h"
#include "components/prefs/persistent_pref_store.h"
#include "components/prefs/pref_value_map.h"

// A light-weight prefstore implementation that keeps preferences
// in a memory backed store. This is not a persistent prefstore -- we
// subclass the PersistentPrefStore here since it is needed by the
// PrefService, which in turn is needed by the Autofill component.
class XWalkPrefStore : public PersistentPrefStore {
 public:
  XWalkPrefStore();

  // Overriden from PrefStore.
  bool GetValue(const std::string& key,
                const base::Value** result) const override;
  void AddObserver(PrefStore::Observer* observer) override;
  void RemoveObserver(PrefStore::Observer* observer) override;
  bool HasObservers() const override;
  bool IsInitializationComplete() const override;

  // PersistentPrefStore overrides:
  bool GetMutableValue(const std::string& key, base::Value** result) override;
  void ReportValueChanged(const std::string& key, uint32_t flags) override;
  void SetValue(const std::string& key,
                std::unique_ptr<base::Value> value,
                uint32_t flags) override;
  void SetValueSilently(const std::string& key,
                        std::unique_ptr<base::Value> value,
                        uint32_t flags) override;
  void RemoveValue(const std::string& key, uint32_t flags) override;
  bool ReadOnly() const override;
  PrefReadError GetReadError() const override;
  PersistentPrefStore::PrefReadError ReadPrefs() override;
  void ReadPrefsAsync(ReadErrorDelegate* error_delegate) override;
  void CommitPendingWrite() override {}
  void SchedulePendingLossyWrites() override {}
  void ClearMutableValues() override {}

 protected:
  ~XWalkPrefStore() override;

 private:
  // Stores the preference values.
  PrefValueMap prefs_;

  base::ObserverList<PrefStore::Observer, true> observers_;

  DISALLOW_COPY_AND_ASSIGN(XWalkPrefStore);
};

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PREF_STORE_H_
