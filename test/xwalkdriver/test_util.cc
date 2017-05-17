// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/test_util.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

RestoreKeyboardLayoutOnDestruct::RestoreKeyboardLayoutOnDestruct() {
#if defined(OS_WIN)
  layout_ = GetKeyboardLayout(NULL);
#elif defined(OS_MACOSX)
  layout_.reset(TISCopyCurrentKeyboardInputSource());
#elif defined(OS_LINUX)
  NOTIMPLEMENTED();
#endif
}

RestoreKeyboardLayoutOnDestruct::~RestoreKeyboardLayoutOnDestruct() {
#if defined(OS_WIN)
  ActivateKeyboardLayout(layout_, 0);
#elif defined(OS_MACOSX)
  TISSelectInputSource(layout_);
#elif defined(OS_LINUX)
  NOTIMPLEMENTED();
#endif
}

#if defined(OS_WIN)
bool SwitchKeyboardLayout(const std::string& input_locale_identifier) {
  HKL layout = LoadKeyboardLayout(
      UTF8ToWide(input_locale_identifier).c_str(), 0);
  if (!layout)
    return false;
  return !!ActivateKeyboardLayout(layout, 0);
}
#endif  // defined(OS_WIN)

#if defined(OS_MACOSX)
bool SwitchKeyboardLayout(const std::string& input_source_id) {
  base::ScopedCFTypeRef<CFMutableDictionaryRef> filter_dict(
      CFDictionaryCreateMutable(kCFAllocatorDefault,
                                1,
                                &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks));
  base::ScopedCFTypeRef<CFStringRef> id_ref(CFStringCreateWithCString(
      kCFAllocatorDefault, input_source_id.c_str(), kCFStringEncodingUTF8));
  CFDictionaryAddValue(filter_dict, kTISPropertyInputSourceID, id_ref);
  base::ScopedCFTypeRef<CFArrayRef> sources(
      TISCreateInputSourceList(filter_dict, true));
  if (CFArrayGetCount(sources) != 1)
    return false;
  TISInputSourceRef source = (TISInputSourceRef)CFArrayGetValueAtIndex(
      sources, 0);
  return TISSelectInputSource(source) == noErr;
}
#endif  // defined(OS_MACOSX)
