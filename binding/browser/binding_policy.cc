// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/browser/binding_policy.h"

#include <set>

#include "base/command_line.h"
#include "xwalk/binding/browser/binding_policy_test.h"
#include "xwalk/binding/common/binding_switches.h"

namespace xwalk {

BindingPolicy* BindingPolicy::singleton_ = NULL;
BindingPolicy* BindingPolicy::GetService() {
  if (!singleton_)
    singleton_ = new BindingPolicy();
  return singleton_;
}

BindingPolicy::BindingPolicy() {
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kBindingTest))
    AddEntry(new BindingPolicyTest());
}

void BindingPolicy::AddEntry(BindingPolicy::Entry* entry) {
  for (size_t i = 0; i < entries_.size(); i++) {
    if (entries_[i] == entry)
      return;
  }
  entries_.push_back(entry);
}

void BindingPolicy::RemoveEntry(BindingPolicy::Entry* entry) {
  ScopedVector<BindingPolicy::Entry>::iterator it;
  for (it = entries_.begin(); it != entries_.end(); ++it) {
    if (*it == entry) {
      entries_.erase(it);
      return;
    }
  }
}

std::vector<std::string> BindingPolicy::GetFeatures(const GURL& url) {
  std::set<std::string> features;
  std::set<std::string>::iterator it;
  std::vector<std::string> f;

  // Grant
  for (size_t i = 0; i < entries_.size(); i++) {
    f = entries_[i]->Grant(url);
    for (size_t j = 0; j < f.size(); j++)
      features.insert(f[j]);
  }

  // Deny
  for (size_t i = 0; i < entries_.size(); i++) {
    f = entries_[i]->Deny(url);
    for (size_t j = 0; j < f.size(); j++) {
      it = features.find(f[j]);
      if (it != features.end())
        features.erase(f[j]);
    }
  }

  f.clear();
  for (it = features.begin(); it != features.end(); ++it)
    f.push_back(*it);
  return f;
}

}  // namespace xwalk
