#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This script is used to remove src/third_party/speex if it is checked out from
its own repository.

Starting with Chromium M39, it has become part of the Chromium source tree
itself, but the change causes "gclient sync" to consider src/third_party/speex
a directory that can be removed and as we pass --delete_unversioned_trees and
--reset to "gclient sync" in fetch_deps.py, we have to do this kind of analysis
before running gclient:

* If src/third_party/speex has its own .git/, it means it is checked out from
  its own git repository and we have to remove it before syncing so that the
  directory is created as part of chromium-crosswalk.
* Otherwise, we just do not do anything and assume this has already been done.
"""

import os
import shutil


SRC_PATH = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(
    __file__))))


if __name__ == '__main__':
  speex_root = os.path.join(SRC_PATH, 'third_party', 'speex')

  if os.path.isdir(os.path.join(speex_root, '.git')):
    print 'Removing separate "%s" git checkout.' % speex_root
    shutil.rmtree(speex_root)
