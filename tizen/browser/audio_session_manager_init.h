// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Contains code that should be used for initializing the
// audio session manager library as a whole.

#ifndef XWALK_TIZEN_BROWSER_AUDIO_SESSION_MANAGER_INIT_H_
#define XWALK_TIZEN_BROWSER_AUDIO_SESSION_MANAGER_INIT_H_

#include "content/common/content_export.h"

namespace tizen {

// Attempts to initialize the audio session manager library.
// Returns true if everything was successfully initialized, false otherwise.
CONTENT_EXPORT bool InitializeAudioSessionManager();

}  // namespace tizen

#endif  // XWALK_TIZEN_BROWSER_AUDIO_SESSION_MANAGER_INIT_H_
