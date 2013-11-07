// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef XWALK_RUNTIME_BROWSER_ANDROID_STATE_SERIALIZER_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_STATE_SERIALIZER_H_

#include "base/compiler_specific.h"

class Pickle;
class PickleIterator;

namespace content {

class NavigationEntry;
class WebContents;

}  // namespace content

namespace xwalk {

// Write and restore a WebContents to and from a pickle. Return true on
// success.

// Note that |pickle| may be changed even if function returns false.
bool WriteToPickle(const content::WebContents& web_contents,
                   Pickle* pickle) WARN_UNUSED_RESULT;

// |web_contents| will not be modified if function returns false.
bool RestoreFromPickle(PickleIterator* iterator,
                       content::WebContents* web_contents) WARN_UNUSED_RESULT;


namespace internal {
// Functions below are individual helper functiosn called by functions above.
// They are broken up for unit testing, and should not be called out side of
// tests.
bool WriteHeaderToPickle(Pickle* pickle) WARN_UNUSED_RESULT;
bool RestoreHeaderFromPickle(PickleIterator* iterator) WARN_UNUSED_RESULT;
bool WriteNavigationEntryToPickle(const content::NavigationEntry& entry,
                                  Pickle* pickle) WARN_UNUSED_RESULT;
bool RestoreNavigationEntryFromPickle(
    PickleIterator* iterator,
    content::NavigationEntry* entry) WARN_UNUSED_RESULT;

}  // namespace internal

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_STATE_SERIALIZER_H_
