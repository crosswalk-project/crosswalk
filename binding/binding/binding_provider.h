// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_BINDING_BINDING_PROVIDER_H_
#define XWALK_BINDING_BINDING_BINDING_PROVIDER_H_

#include <map>
#include <string>
#include <vector>

#include "base/atomicops.h"
#include "base/files/file_path.h"
#include "base/native_library.h"
#include "xwalk/binding/public/xwalk_binding.h"

namespace xwalk {

class BindingService;

class BindingProvider {
 public:
  BindingProvider(XWalkBinding_InitializeFunc initialize,
                  XWalkBinding_GetFactoryFunc getfactory,
                  XWalkBinding_ShutdownFunc shutdown);
  virtual ~BindingProvider();

  virtual bool PreInitialize(void);
  virtual std::string GetDescription(const std::string& uri,
                                     const std::string& locale) const;
  bool IsInitialized() const { return !factories_.empty(); }
  virtual base::FilePath path() const { return base::FilePath(); }

  std::vector<std::string> GetFeatures(void) const;
  std::string GetBinding(const std::string& uri) const;
  NPObject* CreateObject(const std::string &uri, NPObject* root);
  void ReleaseObject(const std::string &uri);

 protected:
  BindingProvider();
  virtual bool Initialize();
  virtual void Shutdown();
  void AddFeature(const std::string& uri, const std::string& binding) {
    if (bindings_.find(uri) == bindings_.end())
      bindings_.insert(BindingMap::value_type(uri, binding));
  }

  XWalkBinding_InitializeFunc initialize_;
  XWalkBinding_GetFactoryFunc getfactory_;
  XWalkBinding_ShutdownFunc shutdown_;
  friend class BindingService;

 private:
  base::subtle::Atomic32 refcnt_;
  typedef std::map<std::string, std::string> BindingMap;
  BindingMap bindings_;
  typedef std::map<std::string, NPObject* (*)(NPObject* root)> FactoryMap;
  FactoryMap factories_;

  DISALLOW_COPY_AND_ASSIGN(BindingProvider);
};


class BindingProviderDSO : public BindingProvider {
 public:
  explicit BindingProviderDSO(const base::FilePath& path)
      : path_(path), handle_(NULL) {}
  ~BindingProviderDSO() {}

  std::string GetDescription(const std::string& uri,
                             const std::string& locale) const OVERRIDE;

  bool PreInitialize(void) OVERRIDE;
  bool Initialize(void) OVERRIDE;
  void Shutdown(void) OVERRIDE;
  base::FilePath path() const OVERRIDE { return path_; }

  bool Load(void);
  void Unload(void);

 private:
  base::FilePath path_;
  base::NativeLibrary handle_;

  DISALLOW_COPY_AND_ASSIGN(BindingProviderDSO);
};

}  // namespace xwalk

#endif  // XWALK_BINDING_BINDING_BINDING_PROVIDER_H_
