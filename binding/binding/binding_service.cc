// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/binding/binding_service.h"

#include "base/file_util.h"
#include "base/path_service.h"
#include "build/build_config.h"
#include "third_party/jsoncpp/source/include/json/reader.h"
#include "third_party/jsoncpp/source/include/json/writer.h"
#include "xwalk/binding/binding/binding_functions.h"
#include "xwalk/binding/binding/binding_provider.h"
#include "xwalk/binding/binding/binding_utils.h"

#if defined(OS_WIN)
#include "base/win/registry.h"
#define MODULE_EXTENSION_NAME  FILE_PATH_LITERAL("dll")
#define MODULE_PATH_DELIMITER  FILE_PATH_LITERAL(";")
#define MODULE_PATH_DEFAULT \
    FILE_PATH_LITERAL("%ProgramFiles%\\Crosswalk\\Bindings")
#else
#define MODULE_EXTENSION_NAME  FILE_PATH_LITERAL("so")
#define MODULE_PATH_DELIMITER  FILE_PATH_LITERAL(":")
#define MODULE_PATH_DEFAULT    FILE_PATH_LITERAL("/usr/lib/xwalk/")
#endif
#define MODULE_PATH_ENVNAME    FILE_PATH_LITERAL("XWALK_BINDING_PATH")
#define MODULE_CACHE_FILENAME  FILE_PATH_LITERAL(".xwalk-binding.json")

namespace xwalk {

BindingService* BindingService::singleton_ = NULL;
BindingService* BindingService::GetService() {
  if (!singleton_) {
    singleton_ = new BindingService();
    singleton_->Initialize();
  }
  return singleton_;
}

BindingService::BindingService() {
}

BindingService::~BindingService() {
  provider_map_.clear();
  feature_map_.clear();
}

void BindingService::Initialize() {
  base::FilePath path = utils::GetHomeDirectory();
  path.Append(MODULE_CACHE_FILENAME);
  CollectProviderDirectories();
  if (!file_util::PathExists(path) || !Import(path)) {
    for (size_t i = 0; i < directories_.size(); i++)
      ScanProviders(directories_[i]);
    Export(path);
  }
}

bool BindingService::AddProviderDirectory(const base::FilePath& dir) {
  if (!file_util::PathExists(dir))
    return false;
  for (size_t i = 0; i < directories_.size(); i++) {
    if (directories_[i] == dir)
      return false;
  }
  directories_.push_back(dir);
  return true;
}

bool BindingService::AddProviderDirectories(
    const base::FilePath::StringType& str) {
  size_t size = directories_.size();
  for (size_t pos = 0; pos < str.length(); ++pos) {
    size_t pos0 = pos;
    pos = str.find(MODULE_PATH_DELIMITER, pos);
    if (pos == std::string::npos)
      pos = str.length();
    if (pos == pos0)  continue;

    base::FilePath dir(str.substr(pos0, pos - pos0));
    AddProviderDirectory(dir);
  }
  return directories_.size() > size;
}

bool BindingService::CollectProviderDirectories() {
  // User specified
  base::FilePath::StringType str;
  str = utils::GetEnvironmentString(MODULE_PATH_ENVNAME);
  AddProviderDirectories(str);

#if defined(OS_WIN)
  // Registry for Windows only
  if (directories_.empty()) {
    const wchar_t* keyname = L"Software\\Intel\\Crosswalk";
    base::win::RegKey key;
    if (key.Open(HKEY_CURRENT_USER, keyname, KEY_READ) == ERROR_SUCCESS) {
      if (key.ReadValue(L"BindingModulePath", &str) == ERROR_SUCCESS)
        AddProviderDirectories(str);
      key.Close();
    }
  }
#endif

  // Bundled with application
  base::FilePath dir;
  if (PathService::Get(base::DIR_MODULE, &dir)) {
    dir = dir.Append(FILE_PATH_LITERAL("bindings"));
    AddProviderDirectory(dir);
  }

  // Fallback to default directory
  if (directories_.empty()) {
    str = utils::ExpandEnvironmentString(MODULE_PATH_DEFAULT);
    AddProviderDirectory(base::FilePath(str));
  }
  return !directories_.empty();
}

void BindingService::ScanProviders(const base::FilePath& dir) {
  // Scan all native library in a directory
  LOG(INFO) << ("Scan providers in directory: " + dir.AsUTF8Unsafe());
  file_util::FileEnumerator iter(dir, false,
      file_util::FileEnumerator::FILES,
      FILE_PATH_LITERAL("*.") MODULE_EXTENSION_NAME);
  for (base::FilePath path = iter.Next(); !path.empty(); path = iter.Next()) {
    BindingProvider* provider = new BindingProviderDSO(path);
    if (!provider->PreInitialize()) {
      delete provider;
      continue;
    }
    AddProvider(provider);
    LOG(INFO) << ("Load " + path.AsUTF8Unsafe() + " successfully");
  }
}

bool BindingService::Export(const base::FilePath& path) {
  LOG(INFO) << ("Export to " + path.AsUTF8Unsafe());

  Json::UInt i;
  Json::Value root;

  // serializ providers and featires
  Json::Value providers;
  ScopedVector<BindingProvider>::const_iterator it;
  for (i = 0, it = providers_.begin(); it != providers_.end(); ++it, ++i) {
    if ((*it)->path().empty())
      continue;
    providers[i]["path"] = (*it)->path().AsUTF8Unsafe();
    Json::Value features;
    std::vector<std::string> f = (*it)->GetFeatures();
    for (Json::UInt j = 0; j < f.size(); j++) {
      features[j]["uri"] = f[j];
      features[j]["binding"] = (*it)->GetBinding(f[j]);
    }
    providers[i]["features"] = features;
  }
  root["providers"] = providers;

  // serializ directories
  Json::Value directories;
  for (i = 0; i < directories_.size(); ++i)
    directories[i] = directories_[i].AsUTF8Unsafe();
  root["directories"] = directories;

  Json::StyledWriter writer;
  std::string data = writer.write(root);
  return file_util::WriteFile(path, data.c_str(), data.size()) < 0;
}

bool BindingService::Import(const base::FilePath& path) {
  // Read and parse file
  std::string data;
  Json::Reader reader;
  Json::Value root;
  if (!file_util::ReadFileToString(path, &data)) {
    LOG(WARNING) << ("Can't read list file: " + path.AsUTF8Unsafe());
    return false;
  }
  if (!reader.parse(data, root, false)) {
    LOG(WARNING) << "Can't parse list file: "
                 << reader.getFormattedErrorMessages();
    return false;
  }

  // Check validation
  Json::Value directories = root["directories"];
  if (!directories.isArray())  return false;
  base::Time time0 = utils::GetLastModifiedTime(path);
  std::vector<base::FilePath> dirs;
  for (Json::UInt i = 0; i < directories.size(); i++) {
    if (!directories[i].isString())  continue;

    // Check timestamp of directory
    base::FilePath dir(
        base::FilePath::FromUTF8Unsafe(directories[i].asString()));
    base::Time time = utils::GetLastModifiedTime(dir);
    if (time >= time0 || time.is_null()) {
      LOG(INFO) << "Directory " << directories[i].asString() << " is changed.";
      return false;
    }

    // Check timestamp of files
    file_util::FileEnumerator iter(dir, false,
        file_util::FileEnumerator::FILES,
        FILE_PATH_LITERAL("*.") MODULE_EXTENSION_NAME);
    for (base::FilePath dso = iter.Next(); !dso.empty(); dso = iter.Next()) {
      time = utils::GetLastModifiedTime(dso);
      if (time >= time0 || time.is_null()) {
        LOG(INFO) << "File " << dso.AsUTF8Unsafe() << " is changed.";
        return false;
      }
    }
    dirs.push_back(dir);
  }
  if (dirs != directories_) {
    LOG(INFO) << "Directories are changed.";
    return false;
  }

  // Build provider information
  Json::Value providers = root["providers"];
  if (!providers.isArray())  return false;
  LOG(INFO) << ("Import from " + path.AsUTF8Unsafe());
  for (Json::UInt i = 0; i < providers.size(); i++) {
    // check path
    if (!providers[i]["path"].isString())
      continue;
    base::FilePath file(base::FilePath::FromUTF8Unsafe(
        providers[i]["path"].asString()));
    if (!file_util::PathExists(file))
      continue;

    // check provider
    ScopedVector<BindingProvider>::const_iterator it;
    for (it = providers_.begin(); it != providers_.end(); ++it)
      if ((*it)->path() == file)
        break;
    if (it != providers_.end())
      continue;

    // features
    Json::Value features = providers[i]["features"];
    if (!features.isArray())
      continue;
    BindingProvider* provider = new BindingProviderDSO(file);
    for (Json::UInt j = 0; j < features.size(); j++) {
      // check uri and binding
      std::string uri, binding;
      if (features[j]["uri"].isString())
        uri = features[j]["uri"].asString();
      if (uri.empty() || !features[j]["binding"].isString())
        continue;
      binding = features[j]["binding"].asString();

      // add this feature
      provider->AddFeature(uri, binding);
      provider_map_.insert(ProviderMap::value_type(uri, provider));
      if (!binding.empty())
        feature_map_.insert(FeatureMap::value_type(binding, uri));
    }
    providers_.push_back(provider);
  }
  return true;
}

void BindingService::AddProvider(BindingProvider* provider) {
  for (size_t i = 0; i < providers_.size(); i++) {
    if (providers_[i] == provider)
      return;
  }

  std::vector<std::string> features = provider->GetFeatures();
  for (size_t i = 0; i < features.size(); i++) {
    if (GetProvider(features[i])) {
      LOG(WARNING) << ("Feature " + features[i] +
                       " has been registered, skip it.");
      continue;
    }
    provider_map_.insert(ProviderMap::value_type(features[i], provider));
    std::string binding = provider->GetBinding(features[i]);
    if (!binding.empty())
      feature_map_.insert(FeatureMap::value_type(binding, features[i]));
  }
  providers_.push_back(provider);
}

BindingProvider* BindingService::GetProvider(const std::string& uri) const {
  // Trim the query part
  size_t pos = uri.find('?');
  ProviderMap::const_iterator it = provider_map_.find(uri.substr(0, pos));
  if (it != provider_map_.end())
    return it->second;
  else
    return NULL;
}

std::string BindingService::GetFeature(const std::string& binding) const {
  FeatureMap::const_iterator it = feature_map_.find(binding);
  return (it != feature_map_.end())? it->second: NULL;
}

bool BindingService::BindFeature(NPObject* root,
                                 const std::string& uri) const {
  const XWalkBindingFunctions* host_funcs = GetBindingFunctions();

  // Create API Object
  BindingProvider* provider = GetProvider(uri);
  if (!provider) {
    LOG(WARNING) << ("Can't bind unknown feature: " + uri);
    return false;
  }
  NPObject* api = provider->CreateObject(uri, root);
  if (!api)  return false;

  std::string binding = provider->GetBinding(uri);
  if (binding.empty()) {
    LOG(INFO) << ("No binding name for feature: " + uri);
    host_funcs->retainobject(api);
    return true;
  }

  size_t pos0 = 0, pos;
  std::string prop;
  NPIdentifier id;
  NPVariant var;
  NPObject* object = root;
  while ((pos = binding.find('.', pos0)) != std::string::npos) {
    prop = binding.substr(pos0, pos - pos0);
    id = host_funcs->getstringidentifier(prop.c_str());
    if (host_funcs->hasproperty(NULL, object, id)) {
      host_funcs->getproperty(NULL, object, id, &var);
      object = NPVARIANT_TO_OBJECT(var);
      pos0 = pos + 1;
    } else {   // deal with dependence
      std::string dep_binding = binding.substr(0, pos);
      std::string dep_uri = GetFeature(dep_binding);
      if (!dep_uri.size()) {
        LOG(WARNING) << "Dependence not found: " << dep_binding;
        return false;
      }
      if (!BindFeature(root, dep_uri))
        return false;
    }
  }

  prop = binding.substr(pos0, binding.size() - pos0);
  id = host_funcs->getstringidentifier(prop.c_str());
  OBJECT_TO_NPVARIANT(api, var);
  host_funcs->setproperty(NULL, object, id, &var);
  LOG(INFO) << "Bind " << uri << " to " << binding;
  return true;
}

void BindingService::UnbindFeature(NPObject* root,
                                   const std::string& uri) const {
  BindingProvider* provider = GetProvider(uri);
  if (!provider)  return;

  // FIXME(zliang7) Remove API object

  // Release provider
  provider->ReleaseObject(uri);
}

}  // namespace xwalk
