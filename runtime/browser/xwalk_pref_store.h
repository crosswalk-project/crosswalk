// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PREF_STORE_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PREF_STORE_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/observer_list.h"
#include "base/prefs/persistent_pref_store.h"
#include "base/prefs/pref_value_map.h"

// A light-weight prefstore implementation that keeps preferences
// in a memory backed store. This is not a persistent prefstore -- we
// subclass the PersistentPrefStore here since it is needed by the
// PrefService, which in turn is needed by the Autofill component.
class XWalkPrefStore : public PersistentPrefStore {
 public:
  XWalkPrefStore();

  // Overriden from PrefStore.
  virtual bool GetValue(const std::string& key,
                        const base::Value** result) const OVERRIDE;
  virtual void AddObserver(PrefStore::Observer* observer) OVERRIDE;
  virtual void RemoveObserver(PrefStore::Observer* observer) OVERRIDE;
  virtual bool HasObservers() const OVERRIDE;
  virtual bool IsInitializationComplete() const OVERRIDE;

  // PersistentPrefStore overrides:
  virtual bool GetMutableValue(const std::string& key,
                               base::Value** result) OVERRIDE;
  virtual void ReportValueChanged(const std::string& key) OVERRIDE;
  virtual void SetValue(const std::string& key, base::Value* value) OVERRIDE;
  virtual void SetValueSilently(const std::string& key,
                                base::Value* value) OVERRIDE;
  virtual void RemoveValue(const std::string& key) OVERRIDE;
  virtual void MarkNeedsEmptyValue(const std::string& key) OVERRIDE;
  virtual bool ReadOnly() const OVERRIDE;
  virtual PrefReadError GetReadError() const OVERRIDE;
  virtual PersistentPrefStore::PrefReadError ReadPrefs() OVERRIDE;
  virtual void ReadPrefsAsync(ReadErrorDelegate* error_delegate) OVERRIDE;
  virtual void CommitPendingWrite() OVERRIDE {}

 protected:
  virtual ~XWalkPrefStore();

 private:
  // Stores the preference values.
  PrefValueMap prefs_;

  ObserverList<PrefStore::Observer, true> observers_;

  DISALLOW_COPY_AND_ASSIGN(XWalkPrefStore);
};

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PREF_STORE_H_
