# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Generates a value for the android:versionCode attribute in AndroidManifest.xml.
"""

import argparse
import sys


# This dictionary associates a certain Android ABI name to an integer that will
# be part of the final version code. Bigger numbers will generate a larger
# version code that will have preference in the Play Store.
ANDROID_ABI_KEYS = {
  'armeabi': 1,
  'armeabi-v7a': 2,
  'arm64-v8a': 3,
  'x86': 4,
  'x86_64': 5,
}


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--abi-name', required=True,
                      choices=sorted(ANDROID_ABI_KEYS.keys()),
                      help='Android ABI name, as defined in '
                      'src/build/common.gypi.')
  parser.add_argument('--version', required=True,
                      help='Crosswalk version number in the '
                      'MAJOR.MINOR.BUILD.PATCH format.')

  args = parser.parse_args()

  shift = ANDROID_ABI_KEYS[args.abi_name]
  major, minor, build, patch = map(int, args.version.split('.'))

  # We derive the version code from Crosswalk's version number plus an
  # additional key value related to the CPU architecture being targeted.
  # Crosswalk's major and minor version numbers are always increasing, so we
  # use their sum in the higher order digits of the version code, followed by
  # the build version number that increases with each canary, the patch number
  # increased with each beta and a final value that depends on the CPU
  # architecture (it is used to differentiate the APKs uploaded to the Play
  # Store based on the target architecture).
  print '%d' % ((major+minor)*100000 + build*1000 + patch*10 + shift)


if __name__ == '__main__':
  sys.exit(main())
