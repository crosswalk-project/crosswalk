// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_EXTERNAL_H_
#define CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_EXTERNAL_H_

#include <string>
#include "cameo/src/extensions/browser/cameo_extension.h"

namespace base {
class FilePath;
class ScopedNativeLibrary;
};

namespace cameo {
namespace extensions {

typedef struct CCameoExtension_ CCameoExtension;
typedef struct CCameoExtensionContext_ CCameoExtensionContext;
typedef struct CCameoExtensionContextAPI_ CCameoExtensionContextAPI;

// CameoExternalExtension bridges an extension contained in a shared
// library. See extensions/public/cameo_extension_public.h for details
// of how this library should be implemented.
// Note that POD are used in the borders to keep compatibility with the C
// ABI.
class CameoExternalExtension : public CameoExtension {
 public:
  explicit CameoExternalExtension(const base::FilePath& library_path);
  virtual ~CameoExternalExtension();

  virtual const char* GetJavaScriptAPI();

  virtual CameoExtension::Context* CreateContext(
      const CameoExtension::PostMessageCallback& post_message);

  bool is_valid();

 private:
  class ExternalContext : public CameoExtension::Context {
   private:
     CCameoExtensionContext* context_;

     static const CCameoExtensionContextAPI* GetAPIWrappers();
     static void PostMessageWrapper(CCameoExtensionContext* context,
                                    const char* message);
   public:
     ExternalContext(
        CameoExternalExtension* external,
        const CameoExtension::PostMessageCallback& post_message,
        CCameoExtensionContext* context);
    ~ExternalContext();

    virtual void HandleMessage(const std::string& msg) OVERRIDE;
  };

  base::ScopedNativeLibrary* library_;
  CCameoExtension* wrapped_;

  DISALLOW_COPY_AND_ASSIGN(CameoExternalExtension);
};

}  // namespace extensions
}  // namespace cameo

#endif  // CAMEO_SRC_EXTENSIONS_BROWSER_CAMEO_EXTENSION_EXTERNAL_H_
