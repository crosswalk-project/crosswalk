// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_H_

#include <map>
#include <set>
#include <string>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/runtime/browser/runtime.h"

class GURL;

namespace xwalk {
class RuntimeContext;
}

namespace xwalk {
namespace application {

class ApplicationHost;
class Manifest;

class Application : public Runtime::Observer {
 public:
  Application(scoped_refptr<const ApplicationData> data,
              xwalk::RuntimeContext* context);
  virtual ~Application();

  bool Launch();

  Runtime* GetMainDocumentRuntime() const { return main_runtime_; }

  const ApplicationData* data() const { return application_data_; }

 protected:
  // Runtime::Observer implementation.
  virtual void OnRuntimeAdded(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(Runtime* runtime) OVERRIDE;

 private:
  bool RunMainDocument();
  bool RunFromLocalPath();
  void CloseMainDocument();

  xwalk::RuntimeContext* runtime_context_;
  scoped_refptr<const ApplicationData> application_data_;
  xwalk::Runtime* main_runtime_;
  base::WeakPtrFactory<Application> weak_ptr_factory_;
  std::set<Runtime*> runtimes_;

  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_H_
