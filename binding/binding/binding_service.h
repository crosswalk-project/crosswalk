// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_BINDING_BINDING_SERVICE_H_
#define XWALK_BINDING_BINDING_BINDING_SERVICE_H_

#include <map>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"

struct NPObject;

namespace xwalk {

class BindingProvider;

class BindingService {
 public:
  ~BindingService();
  static BindingService* GetService(void);

  void AddProvider(BindingProvider* provider);
  BindingProvider* GetProvider(const std::string& uri) const;

  bool BindFeature(NPObject* root, const std::string& uri) const;
  void UnbindFeature(NPObject* root, const std::string& uri) const;

 private:
  BindingService();
  void Initialize(void);
  std::string GetFeature(const std::string& binding) const;

  base::FilePath GetDefaultExportedFile(void) const;
  bool Export(const base::FilePath& path);
  bool Import(const base::FilePath& path);
  void ScanProviders(const base::FilePath& dir);

  bool CollectProviderDirectories(void);
  bool AddProviderDirectory(const base::FilePath& dir);
  bool AddProviderDirectories(const base::FilePath::StringType& str);
  std::vector<base::FilePath> directories_;

  typedef std::map<std::string, BindingProvider*> ProviderMap;
  typedef std::map<std::string, std::string> FeatureMap;
  ProviderMap provider_map_;
  FeatureMap feature_map_;
  ScopedVector<BindingProvider> providers_;

  static BindingService* singleton_;
  DISALLOW_COPY_AND_ASSIGN(BindingService);
};

}  // namespace xwalk

#endif  // XWALK_BINDING_BINDING_BINDING_SERVICE_H_
