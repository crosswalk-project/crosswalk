// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_TEST_UTIL_H_
#define CHROME_TEST_CHROMEDRIVER_TEST_UTIL_H_

#include <string>

#include "base/basictypes.h"

#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_MACOSX)
#include <Carbon/Carbon.h>
#include "base/mac/scoped_cftyperef.h"
#endif

// Restores the keyboard layout that was active at this object's creation
// when this object goes out of scope.
class RestoreKeyboardLayoutOnDestruct {
 public:
  RestoreKeyboardLayoutOnDestruct();
  ~RestoreKeyboardLayoutOnDestruct();

 private:
#if defined(OS_WIN)
  HKL layout_;
#elif defined(OS_MACOSX)
  base::ScopedCFTypeRef<TISInputSourceRef> layout_;
#endif

  DISALLOW_COPY_AND_ASSIGN(RestoreKeyboardLayoutOnDestruct);
};

#if defined(OS_WIN)
// Loads and activates the given keyboard layout. |input_locale_identifier|
// is composed of a device and language ID. Returns true on success.
// See http://msdn.microsoft.com/en-us/library/dd318693(v=vs.85).aspx
// Example: "00000409" is the default en-us keyboard layout.
bool SwitchKeyboardLayout(const std::string& input_locale_identifier);
#endif  // defined(OS_WIN)

#if defined(OS_MACOSX)
// Selects the input source for the given input source ID. Returns true on
// success.
// Example: "com.apple.keyboardlayout.US" is the default en-us keyboard layout.
bool SwitchKeyboardLayout(const std::string& input_source_id);
#endif  // defined(OS_MACOSX)

#endif  // CHROME_TEST_CHROMEDRIVER_TEST_UTIL_H_
