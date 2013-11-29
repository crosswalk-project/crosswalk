// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/browser/audio_session_manager_init.h"

#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "xwalk/tizen/browser/audio_session_manager_stubs.h"

using xwalk_tizen_browser::kModuleAudio_session_manager;
using xwalk_tizen_browser::InitializeStubs;
using xwalk_tizen_browser::StubPathMap;

namespace tizen {

static const base::FilePath::CharType kAsmModuleLib[] =
    FILE_PATH_LITERAL("libaudio-session-mgr.so.0");

// Audio session manager must only be initialized once, so use a
// LazyInstance to ensure this.
class AudioSessionManagerInitializer {
 public:
  bool Initialize() {
    if (!tried_initialize_) {
      tried_initialize_ = true;
      StubPathMap paths;

      paths[kModuleAudio_session_manager].push_back(kAsmModuleLib);
      initialized_ = InitializeStubs(paths);
    }
    return initialized_;
  }

 private:
  friend struct base::DefaultLazyInstanceTraits<AudioSessionManagerInitializer>;

  AudioSessionManagerInitializer()
      : initialized_(false),
        tried_initialize_(false) {
  }

  bool initialized_;
  bool tried_initialize_;

  DISALLOW_COPY_AND_ASSIGN(AudioSessionManagerInitializer);
};

static base::LazyInstance<AudioSessionManagerInitializer>::Leaky g_asm_library =
    LAZY_INSTANCE_INITIALIZER;

bool InitializeAudioSessionManager() {
  return g_asm_library.Get().Initialize();
}

}  // namespace tizen
