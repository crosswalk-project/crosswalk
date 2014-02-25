// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The Linux implementation is compatible with Mac in this case, but not with
// Android. We only need a separated file for Mac because of the rules in
// build/filename_rules.gypi not handling "Linux && Mac && !Android".
#include "xwalk/sysapps/device_capabilities/cpu_info_provider_linux.cc"
