// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/binding/binding_provider.h"

#include "base/logging.h"
#include "base/file_util.h"
#include "xwalk/binding/binding/binding_functions.h"
#include "xwalk/binding/binding/binding_utils.h"

#if !defined(USE_JSONCPP)
#include "base/json/json_reader.h"
#include "base/values.h"
#else
#include "third_party/jsoncpp/source/include/json/reader.h"
#endif

namespace xwalk {

BindingProvider::BindingProvider()
    : initialize_(NULL),
      getfactory_(NULL),
      shutdown_(NULL),
      refcnt_(0) {
}

BindingProvider::BindingProvider(XWalkBinding_InitializeFunc initialize,
                                 XWalkBinding_GetFactoryFunc getfactory,
                                 XWalkBinding_ShutdownFunc shutdown)
    : initialize_(initialize),
      getfactory_(getfactory),
      shutdown_(shutdown),
      refcnt_(0) {
  DCHECK(initialize_ && getfactory_ && shutdown_);
}

BindingProvider::~BindingProvider() {
  if (IsInitialized())
    Shutdown();
}

bool BindingProvider::PreInitialize() {
  const XWalkBindingFactory* f = getfactory_();
  if (!f || !f[0].feature)
    return false;
  for (size_t i = 0; f[i].feature; i++)
    AddFeature(f[i].feature, f[i].binding);
  return true;
}

bool BindingProvider::Initialize() {
  DCHECK(!IsInitialized());
  DCHECK(initialize_ && getfactory_ && shutdown_);

  if (!initialize_(GetBindingFunctions()))
    return false;

  const XWalkBindingFactory* f = getfactory_();
  for (size_t i = 0; f[i].feature; i++)
    factories_.insert(FactoryMap::value_type(f[i].feature, f[i].construct));
  return true;
}

void BindingProvider::Shutdown() {
  DCHECK(IsInitialized());
  factories_.clear();
  shutdown_();
}

std::vector<std::string> BindingProvider::GetFeatures() const {
  std::vector<std::string> features;
  BindingMap::const_iterator it;
  for (it = bindings_.begin(); it != bindings_.end(); ++it)
    features.push_back(it->first);
  return features;
}

std::string BindingProvider::GetBinding(const std::string& uri) const {
  // Trim the query part
  size_t pos = uri.find('?');
  BindingMap::const_iterator it = bindings_.find(uri.substr(0, pos));
  return (it != bindings_.end())? it->second: std::string();
}

NPObject* BindingProvider::CreateObject(const std::string &uri,
                                        NPObject* root) {
  if (base::subtle::NoBarrier_AtomicIncrement(&refcnt_, 1) == 1)
    if (!Initialize())
      return NULL;

  NPObject* api = NULL;
  FactoryMap::iterator it = factories_.find(uri);
  if (it != factories_.end())
    api = (it->second)(root);
  else
    LOG(WARNING) << ("Can't create object of WebAPI: " + uri);
  return api;
}

std::string BindingProvider::GetDescription(
    const std::string& uri,
    const std::string& locale) const {
  return std::string();
}

void BindingProvider::ReleaseObject(const std::string &uri) {
  if (base::subtle::NoBarrier_AtomicIncrement(&refcnt_, -1) == 0)
    Shutdown();
}

//////////////////////////////////////////////////////////////////////

bool BindingProviderDSO::PreInitialize() {
  bool rt = Load();
  if (rt) {
    rt = BindingProvider::PreInitialize();
    Unload();
  }
  return rt;
}

bool BindingProviderDSO::Initialize() {
  if (!Load())
    return false;
  return BindingProvider::Initialize();
}

void BindingProviderDSO::Shutdown() {
  BindingProvider::Shutdown();
  Unload();
}

bool BindingProviderDSO::Load() {
  if (handle_) return true;

  std::string error;
  handle_ = base::LoadNativeLibrary(path_, &error);
  if (!handle_) {
    LOG(WARNING) << ("Can't load provider: " + path_.AsUTF8Unsafe()
                     + ". " + error);
    return false;
  }

  initialize_ = (XWalkBinding_InitializeFunc)
      base::GetFunctionPointerFromNativeLibrary(
          handle_, "XWalkBinding_Initialize");
  getfactory_ = (XWalkBinding_GetFactoryFunc)
      base::GetFunctionPointerFromNativeLibrary(
          handle_, "XWalkBinding_GetFactory");
  shutdown_ = (XWalkBinding_ShutdownFunc)
      base::GetFunctionPointerFromNativeLibrary(
          handle_, "XWalkBinding_Shutdown");
  if (!initialize_ || !getfactory_ || !shutdown_) {
    LOG(WARNING) << ("\"" + path_.AsUTF8Unsafe()
                     + "\"is not a valid binding provider.");
    return false;
  }
  return true;
}

void BindingProviderDSO::Unload() {
  if (handle_) {
    initialize_ = NULL;
    getfactory_ = NULL;
    shutdown_ = NULL;
    base::UnloadNativeLibrary(handle_);
    handle_ = NULL;
  }
}

std::string BindingProviderDSO::GetDescription(
    const std::string& uri,
    const std::string& locale) const {
  std::string desc;
  std::string content;
  base::FilePath path = path_.ReplaceExtension(FILE_PATH_LITERAL("json"));
  if (!file_util::PathExists(path) ||
      !file_util::ReadFileToString(path, &content))
    return desc;  // can't read resource file

#if !defined(USE_JSONCPP)
  base::Value* value = base::JSONReader::Read(content);
  if (!value) {
    LOG(WARNING) << ("\"" + path.AsUTF8Unsafe() + "\" is malformed json.");
    return desc;
  }
  if (!value->IsType(base::Value::TYPE_DICTIONARY)) {
    delete value;
    return desc;
  }

  scoped_ptr<base::DictionaryValue> root(
      reinterpret_cast<base::DictionaryValue*>(value));
  if (root->HasKey(uri)) {
    root->GetWithoutPathExpansion(uri, &value);
    if (value->IsType(base::Value::TYPE_DICTIONARY)) {
      base::DictionaryValue* dict =
          reinterpret_cast<base::DictionaryValue*>(value);
      std::string lang = locale;
      if (!dict->HasKey(lang)) {
        lang = utils::GetSystemLocale();  // try system locale
        if (!dict->HasKey(lang))
          lang.assign("en-US");  // fallback to english
        if (!dict->HasKey(lang))
          return uri;
      }
      dict->GetWithoutPathExpansion(lang, &value);
    }
    if (value->IsType(base::Value::TYPE_STRING))
      value->GetAsString(&desc);
  }
  return desc;
#else
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(content, root, false) || !root.isObject())
    return desc;  // invalid resource file

  if (root.isMember(uri)) {
    root = root[uri];
    if (root.isObject()) {
      std::string lang = locale;
      if (!root.isMember(lang)) {
        lang = utils::GetSystemLocale();  // try system locale
        if (!root.isMember(lang))
          lang.assign("en-US");  // fallback to english
        if (!root.isMember(lang))
          return uri;
      }
      root = root[lang];
    }
    if (root.isString())
      desc = root.asString();
  }
  return desc;
#endif
}

}  // namespace xwalk
