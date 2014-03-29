// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_BROWSER_MURPHY_MAINLOOP_H_
#define XWALK_TIZEN_BROWSER_MURPHY_MAINLOOP_H_

#include <murphy/common/mainloop.h>

#include "base/logging.h"
#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/waitable_event.h"

namespace tizen {

class MurphyMainloop {
 public:
  MurphyMainloop(const char* log = NULL, const char* debug = NULL);
  virtual ~MurphyMainloop();

  mrp_mainloop_t* getMainloop() {
    return mainloop_;
  }

 private:
  mrp_mainloop_t* mainloop_;

  // set up Murphy logging to bridge to the native logging infra
  static void setupLogger(const char* log, const char* debug);

  // create and set up Murphy mainloop
  bool setupMainloop();

  // crosswalk mainloop abstraction operations
  static void* AddIo(void* glue_data, int fd, mrp_io_event_t events,
                     void (*cb)(void* glue_data, void* id, int fd,
                                mrp_io_event_t events, void* user_data),
                     void* user_data);
  static void  DelIo(void* glue_data, void* id);

  static void* AddTimer(void* glue_data, unsigned int msecs,
                        void (*cb)(void* glue_data, void* id, void* user_data),
                        void* user_data);
  static void  DelTimer(void* glue_data, void* id);
  static void  ModTimer(void* glue_data, void* id, unsigned int msecs);

  static void* AddDefer(void* glue_data,
                        void (*cb)(void* glue_data, void* id, void* user_data),
                        void* user_data);
  static void  DelDefer(void* glue_data, void* id);
  static void  ModDefer(void* glue_data, void* id, int enabled);

  static void Unregister(void* data);
};

}  // namespace tizen

#endif  // XWALK_TIZEN_BROWSER_MURPHY_MAINLOOP_H_
