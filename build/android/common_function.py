#!/usr/bin/env python
#
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

def RemoveUnusedFilesInReleaseMode(mode, target_directory):
  # Exclude gdbserver binary in release mode, as it's GPL license.
  if mode == 'Release':
    if os.path.exists(target_directory):
      for root, _, files in os.walk(target_directory):
        if 'gdbserver' in files:
          os.remove(os.path.join(root, 'gdbserver'))
