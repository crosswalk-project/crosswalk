// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_BROWSER_BINDING_POLICY_H_
#define XWALK_BINDING_BROWSER_BINDING_POLICY_H_

#include <string>
#include <vector>

#include "base/memory/scoped_vector.h"

class GURL;

namespace xwalk {

class BindingPolicy {
 public:
  class Entry {
   public:
    Entry() {}
    virtual ~Entry() {}

    virtual std::vector<std::string> Grant(const GURL& url) {
      return std::vector<std::string>();
    }
    virtual std::vector<std::string> Deny(const GURL& url) {
      return std::vector<std::string>();
    }
  };

  ~BindingPolicy() {}

  static BindingPolicy* GetService();

  void AddEntry(Entry* entry);
  void RemoveEntry(Entry* entry);

  std::vector<std::string> GetFeatures(const GURL& url);

 private:
  BindingPolicy();
  ScopedVector<Entry> entries_;
  static BindingPolicy* singleton_;

  DISALLOW_COPY_AND_ASSIGN(BindingPolicy);
};

}  // namespace xwalk

#endif  // XWALK_BINDING_BROWSER_BINDING_POLICY_H_
