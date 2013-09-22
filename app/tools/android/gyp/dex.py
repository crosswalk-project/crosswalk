#!/usr/bin/env python
#
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import optparse
import os
import sys

from util import build_utils
from util import md5_check

def Find(name, path):
  for root, _, files in os.walk(path):
    if name in files:
      return os.path.join(root, name)


def AddExeExtensions(name):
  exts = filter(None, os.environ.get('PATHEXT', '').lower().split(os.pathsep))
  result = []
  result.append(name)
  for e in exts:
    result.append(name + e)
  return result


def DoDex(options, paths):
  dx_binary = ''
  for dx_str in AddExeExtensions('dx'):
    dx_binary = Find(dx_str, options.android_sdk_root)
    if dx_binary:
      break
  dex_cmd = [dx_binary, '--dex', '--output', options.dex_path] + paths

  record_path = '%s.md5.stamp' % options.dex_path
  md5_check.CallAndRecordIfStale(
      lambda: build_utils.CheckCallDie(dex_cmd, suppress_output=True),
      record_path=record_path,
      input_paths=paths,
      input_strings=dex_cmd)

  build_utils.Touch(options.dex_path)


def main():
  parser = optparse.OptionParser()
  parser.add_option('--android-sdk-root', help='Android sdk root directory.')
  parser.add_option('--dex-path', help='Dex output path.')
  parser.add_option('--stamp', help='Path to touch on success.')

  # TODO(newt): remove this once http://crbug.com/177552 is fixed in ninja.
  parser.add_option('--ignore', help='Ignored.')

  options, paths = parser.parse_args()

  DoDex(options, paths)

  if options.stamp:
    build_utils.Touch(options.stamp)


if __name__ == '__main__':
  sys.exit(main())
