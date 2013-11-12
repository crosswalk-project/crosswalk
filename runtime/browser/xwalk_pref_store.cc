// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_pref_store.h"

#include "base/memory/scoped_ptr.h"
#include "base/values.h"

XWalkPrefStore::XWalkPrefStore() {}

XWalkPrefStore::~XWalkPrefStore() {}

bool XWalkPrefStore::GetValue(const std::string& key,
                                const Value** value) const {
  return prefs_.GetValue(key, value);
}

bool XWalkPrefStore::GetMutableValue(const std::string& key,
                                       Value** value) {
  return prefs_.GetValue(key, value);
}

void XWalkPrefStore::AddObserver(PrefStore::Observer* observer) {
  observers_.AddObserver(observer);
}

void XWalkPrefStore::RemoveObserver(PrefStore::Observer* observer) {
  observers_.RemoveObserver(observer);
}

bool XWalkPrefStore::HasObservers() const {
  return observers_.might_have_observers();
}

bool XWalkPrefStore::IsInitializationComplete() const {
  return true;
}

void XWalkPrefStore::SetValue(const std::string& key, Value* value) {
  DCHECK(value);
  if (prefs_.SetValue(key, value))
      ReportValueChanged(key);
}

void XWalkPrefStore::SetValueSilently(const std::string& key, Value* value) {
  prefs_.SetValue(key, value);
}

void XWalkPrefStore::RemoveValue(const std::string& key) {
  if (prefs_.RemoveValue(key))
    ReportValueChanged(key);
}

void XWalkPrefStore::MarkNeedsEmptyValue(const std::string& key) {
}

bool XWalkPrefStore::ReadOnly() const {
  return false;
}

PersistentPrefStore::PrefReadError XWalkPrefStore::GetReadError() const {
  return PersistentPrefStore::PREF_READ_ERROR_NONE;
}

PersistentPrefStore::PrefReadError XWalkPrefStore::ReadPrefs() {
  return PersistentPrefStore::PREF_READ_ERROR_NONE;
}

void XWalkPrefStore::ReadPrefsAsync(ReadErrorDelegate* error_delegate_raw) {
}

void XWalkPrefStore::ReportValueChanged(const std::string& key) {
  FOR_EACH_OBSERVER(Observer, observers_, OnPrefValueChanged(key));
}
