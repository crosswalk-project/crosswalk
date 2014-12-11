// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_UNITTEST_UTIL_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_UNITTEST_UTIL_H_

#include <string>
#include <vector>
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest.h"

namespace xwalk {
namespace application {

extern const char kDefaultManifestName[];
extern const char kDefaultManifestVersion[];
extern const char kDefaultWidgetName[];
extern const char kDefaultWidgetVersion[];

#if defined(OS_TIZEN)

extern const char kDefaultWidgetPackageId[];
extern const char kDefaultWidgetApplicationName[];
extern const char kDefaultWidgetRequiredVersion[];

#endif  // defined(OS_TIZEN)

// Creates a minimal valid manifest configuration.
scoped_ptr<base::DictionaryValue> CreateDefaultManifestConfig();

// Creates a minimal valid widget configuration.
scoped_ptr<base::DictionaryValue> CreateDefaultWidgetConfig();

// Creates an ApplicationData for specified configuration data.
scoped_refptr<ApplicationData> CreateApplication(Manifest::Type type,
    const base::DictionaryValue& manifest);

// Creates a path to element under parent element.
// For example, calling MakePath("a.b", "c") produces "a.b.c".
std::string MakeElementPath(const std::string& parent,
    const std::string& element);

// In a parent dictionary adds a child dictionary under a specified key.
// If it is a first dictionary under the key, it is added as dictionary
// directly, otherwise it is added as another dictionary in a list of
// dictionaries. If parent is null, it does nothing.
bool AddDictionary(const std::string& key,
    scoped_ptr<base::DictionaryValue> child, base::DictionaryValue* parent);

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_UNITTEST_UTIL_H_
