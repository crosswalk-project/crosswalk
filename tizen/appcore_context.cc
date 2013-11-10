// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/appcore_context.h"

#include <appcore-common.h>
#include <aul.h>
#include <malloc.h>
#include <tizen.h>
#include "base/logging.h"
#include "base/message_loop/message_loop.h"

// AppcoreContextImpl uses private Tizen appcore API to reuse Tizen 2.0
// implementation.
// This private implementation of appcore is a bit huge. We will decide later
// whether to maintain the code similar to appcore private implementation
// in Crosswalk or not after investigating appcore in Tizen 3.0.
extern "C" {
// define in app.h
int app_get_id(char** id);

// define in app_private.h
int app_get_package_app_name(const char* package, char** name);
void app_finalizer_execute(void);

// define in status.c
int aul_status_update(int status);

// define in appcore-internal.h
enum app_event {
  AE_UNKNOWN,
  AE_CREATE,
  AE_TERMINATE,
  AE_PAUSE,
  AE_RESUME,
  AE_RESET,
  AE_LOWMEM_POST,
  AE_MEM_FLUSH,
  AE_MAX
};

struct ui_ops {
  void* data;
  void (*cb_app)(enum app_event evnt, void* data, bundle*);
};
}

namespace tizen {

// FIXME: this implementation is not compatible with shared process mode,
// because Tizen task switcher cannot recognize multiple tasks per process.
// Shared process mode requires one Crosswalk process to include multiple tasks
// (= web applications). It is not supported by current task switcher.
class AppcoreContextImpl
    : public AppcoreContext {
 public:
  AppcoreContextImpl();
  virtual ~AppcoreContextImpl();

  bool Initialize();

 private:
  static void HandleAppcoreEvents(enum app_event, void*, bundle*);
  void HandleAppcoreEventsInternal(enum app_event, bundle*);

  char* package_;
  char* application_name_;
  bool initialized_;
  struct ui_ops appcore_operations_;

  DISALLOW_COPY_AND_ASSIGN(AppcoreContextImpl);
};

scoped_ptr<AppcoreContext> AppcoreContext::Create() {
  scoped_ptr<AppcoreContextImpl> context(new AppcoreContextImpl());
  if (context->Initialize())
    return context.PassAs<AppcoreContext>();
  return scoped_ptr<AppcoreContext>();
}

AppcoreContextImpl::AppcoreContextImpl()
    : package_(NULL),
      application_name_(NULL),
      initialized_(false) {
  appcore_operations_.data = this;
  appcore_operations_.cb_app = HandleAppcoreEvents;
}

AppcoreContextImpl::~AppcoreContextImpl() {
  if (initialized_) {
    app_finalizer_execute();
    aul_status_update(STATUS_DYING);
    appcore_exit();
    DCHECK(package_ && application_name_);
  }

  // app_get_id() and app_get_package_app_name() allocated them using malloc.
  free(package_);
  free(application_name_);
}

bool AppcoreContextImpl::Initialize() {
  if (app_get_id(&package_) != TIZEN_ERROR_NONE) {
    LOG(ERROR) << "Failed to get the package: " << package_;
    return false;
  }

  if (app_get_package_app_name(package_, &application_name_) !=
      TIZEN_ERROR_NONE) {
    LOG(ERROR) << "Failed to get the package's application name: "
               << application_name_;
    return false;
  }

  DCHECK(application_name_ && application_name_[0] != '\0');
  char* argv[2] = {'\0', };
  int r = appcore_init(application_name_, &appcore_operations_, 1, argv);
  if (r == -1) {
    LOG(ERROR) << "Failed to initialize appcore. application name: "
               << application_name_;
    return false;
  }

  initialized_ = true;
  return true;
}


void AppcoreContextImpl::HandleAppcoreEvents(enum app_event event,
                                                  void* data,
                                                  bundle* b) {
  static_cast<AppcoreContextImpl*>(data)->
      HandleAppcoreEventsInternal(event, b);
}

void AppcoreContextImpl::HandleAppcoreEventsInternal(enum app_event event,
                                                          bundle* b) {
  DCHECK(initialized_);
  if (event >= AE_MAX)
    return;

  switch (event) {
    case AE_TERMINATE:
      LOG(INFO) << "[XWalk " << getpid() <<"] TERMINATE";
      base::MessageLoop::current()->QuitNow();
      break;
    default:
      break;
  }
}

}  // namespace tizen

