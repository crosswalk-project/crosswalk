// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_EXTERNAL_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_EXTERNAL_H_

#include <string>
#include "base/scoped_native_library.h"

#include "xwalk/extensions/common/xwalk_extension.h"

namespace base {
class FilePath;
};

namespace xwalk {
namespace extensions {

typedef struct CXWalkExtension_ CXWalkExtension;
typedef struct CXWalkExtensionContext_ CXWalkExtensionContext;
typedef struct CXWalkExtensionContextAPI_ CXWalkExtensionContextAPI;

// XWalkExternalExtension bridges an extension contained in a shared
// library. See extensions/public/xwalk_extension_public.h for details
// of how this library should be implemented.
// Note that POD are used in the borders to keep compatibility with the C
// ABI.
class XWalkExternalExtension : public XWalkExtension {
 public:
  explicit XWalkExternalExtension(const base::FilePath& library_path);
  virtual ~XWalkExternalExtension();

  virtual const char* GetJavaScriptAPI() OVERRIDE;

  virtual XWalkExtension::Context* CreateContext(
      const XWalkExtension::PostMessageCallback& post_message) OVERRIDE;

  bool is_valid();

 private:
  class ExternalContext : public XWalkExtension::Context {
   private:
     CXWalkExtensionContext* context_;

     static const CXWalkExtensionContextAPI* GetAPIWrappers();
     static void PostMessageWrapper(CXWalkExtensionContext* context,
                                    const char* message);
   public:
     ExternalContext(
        XWalkExternalExtension* external,
        const XWalkExtension::PostMessageCallback& post_message,
        CXWalkExtensionContext* context);
    virtual ~ExternalContext();

    virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;
  };

  base::ScopedNativeLibrary library_;
  CXWalkExtension* wrapped_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExternalExtension);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_EXTERNAL_H_
