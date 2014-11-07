// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_UNITTEST_UTIL_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_UNITTEST_UTIL_H_

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "xwalk/application/common/application_data.h"

namespace xwalk {
namespace application {

scoped_ptr<base::DictionaryValue> CreateDefaultW3CManifest();

#if defined(OS_TIZEN)

scoped_ptr<base::DictionaryValue> CreateDefaultWGTManifest();

#endif  // defined(OS_TIZEN)

scoped_refptr<ApplicationData> CreateApplication(
    const base::DictionaryValue& manifest);

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_UNITTEST_UTIL_H_
