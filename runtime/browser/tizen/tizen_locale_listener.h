// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_LOCALE_LISTENER_H_
#define XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_LOCALE_LISTENER_H_

#include <glib.h>
#include <string>

#include "base/threading/simple_thread.h"

namespace xwalk {

class TizenLocaleListener : public base::SimpleThread {
  public:
    TizenLocaleListener();
    virtual ~TizenLocaleListener();

    virtual void Run() OVERRIDE;

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
