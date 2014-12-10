// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_LOCALE_LISTENER_H_
#define XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_LOCALE_LISTENER_H_

#include <glib.h>
#include <string>

#include "base/threading/simple_thread.h"

namespace xwalk {

// FIXME: Do we need to watch system locale change
// now (we do not keep the cache of the installed
// apps anymore)? This class should probably be
// removed.
class TizenLocaleListener : public base::SimpleThread {
 public:
  TizenLocaleListener();
  virtual ~TizenLocaleListener();

  void Run() override;

  // Get the latest application locale from system.
  // locale is a langtag defined in [BCP47]
  std::string GetLocale() const;
  // Set the locale and apply this locale to all applications.
  // Locale is a langtag defined in [BCP47].
  // This function will called by TizenLocaleListener when locale is changed.
  void SetLocale(const std::string& locale);

 private:
  GMainLoop*  main_loop_;
  // The locale is a langtag defined in [BCP47]
  std::string locale_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_LOCALE_LISTENER_H_
