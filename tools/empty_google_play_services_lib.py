#!/usr/bin/env python

# Copyright (c) 2015 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Create an empty google-play-services.jar inside src/third_party/android_tools.

https://chromium-review.googlesource.com/#/c/247861 has introduced a check in
android_tools.gyp that makes the gyp configuration process fail if a certain
file that is part of the Google Play Services library is not found.

Since installing that library involves manually accepting an EULA and it is not
used in Crosswalk, we create an empty file with the name android_tools.gyp
checks for so that the build configuration proceeds. If the user chooses to
manually install the library, the empty file is just overwritten and nothing
breaks. Additionally, we also create res/ and src/ so that src/build/java.gypi
can call find(1) in those directories without failing.
"""

import errno
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, os.pardir,
                             'build', 'android'))

from pylib.constants import ANDROID_SDK_ROOT


def CreateDirectory(path):
  """
  Creates |path| and all missing parent directories. Passing a directory that
  already exists does not cause an error.
  """
  try:
    os.makedirs(path)
  except OSError, e:
    if e.errno == errno.EEXIST:
      pass


def CreateFileIfMissing(path):
  """
  Creates an empty file called |path| if it does not already exist.
  """
  if os.path.isfile(path):
    return
  open(path, 'w').close()


if __name__ == '__main__':
  # If ANDROID_SDK_ROOT does not exist, we can assume the android_tools
  # repository has not been checked out. Consequently, this is not an Android
  # build and we do not need to worry about the issue this script works around.
  if not os.path.isdir(ANDROID_SDK_ROOT):
    sys.exit(0)

  google_play_lib_root = os.path.join(
    ANDROID_SDK_ROOT, 'extras', 'google', 'google_play_services', 'libproject',
    'google-play-services_lib')

  CreateDirectory(os.path.join(google_play_lib_root, 'libs'))
  CreateDirectory(os.path.join(google_play_lib_root, 'res'))
  CreateDirectory(os.path.join(google_play_lib_root, 'src'))
  CreateFileIfMissing(os.path.join(google_play_lib_root, 'libs',
                                   'google-play-services.jar'))
