#!/usr/bin/env python
# Copyright (c) 2009 The Chromium Authors. All rights reserved.
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
VersionCode is needed for AndroidManifest.xml,

This script will generate version code based on
VERSION file for xwalk's runtime library apk.
"""

import optparse
import sys

def fetch_values_from_file(values_dict, file_name):
  """
  Fetches KEYWORD=VALUE settings from the specified file.

  Everything to the left of the first '=' is the keyword,
  everything to the right is the value.  No stripping of
  white space, so beware.

  The file must exist, otherwise you get the Python exception from open().
  """
  for line in open(file_name, 'r').readlines():
    key, val = line.rstrip('\r\n').split('=', 1)
    if key in ['MAJOR', 'MINOR', 'BUILD', 'PATCH']:
      try:
        values_dict[key] = int(val)
      except ValueError:
        return False
  return True


def calculate_version_code(values_dict, shift):
  """
  Version Code is calculated based on the four version integers.

  Major is for crosswalk's large update, and minor is for based chromium.
  Major and minor will always be increasing, so use the sum of them is
  enough.
  For each major and minor refresh, build will reset to 0. After that,
  the build will be increasing for 6 weeks (12 weeks if we skip one upstream
  beta rebasing), so 100 numbers for build are enough.
  After we branch it from trunk, the patch will be increasing for the rest of
  this branch's life, 100 numbers are also enough since it will last for most
  24 weeks, but the version increasing will be much less frequent after branch
  point.
  Shift is the last bit for different configurations we want to upload to
  PlayStore.
  """
  try:
    major = values_dict['MAJOR']
    minor = values_dict['MINOR']
    build = values_dict['BUILD']
    patch = values_dict['PATCH']
    return (major + minor) * 100000 +\
           build * 1000 +\
           patch * 10 +\
           shift
  except KeyError:
    return 0


def main():
  option_parser = optparse.OptionParser()

  option_parser.add_option('--file', '-f', default='VERSION',
      help='Path to the version file.')
  option_parser.add_option('--shift', '-s', default=0, type='int',
      help='Shift for different configurations, PlayStore '
           'requires different version code for multiple apks')
  options, _ = option_parser.parse_args()

  versions = {}

  version_code = 0
  if fetch_values_from_file(versions, options.file):
    version_code = calculate_version_code(versions, options.shift)
  print '%d' % version_code
  if version_code == 0:
    return 1
  else:
    return 0    


if __name__ == '__main__':
  sys.exit(main())
