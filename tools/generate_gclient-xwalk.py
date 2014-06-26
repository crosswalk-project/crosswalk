#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This script is responsible for generating .gclient-xwalk in the top-level
source directory from DEPS.xwalk.
"""

import optparse
import os
import pprint


CROSSWALK_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GCLIENT_ROOT = os.path.dirname(os.path.dirname(CROSSWALK_ROOT))


def GenerateGClientXWalk(options):
  with open(os.path.join(CROSSWALK_ROOT, 'DEPS.xwalk')) as deps_file:
    deps_contents = deps_file.read()

  if 'XWALK_OS_ANDROID' in os.environ:
    deps_contents += 'target_os = [\'android\']\n'
  if options.cache_dir:
    deps_contents += 'cache_dir = %s\n' % pprint.pformat(options.cache_dir)

  with open(os.path.join(GCLIENT_ROOT, '.gclient-xwalk'), 'w') as gclient_file:
    gclient_file.write(deps_contents)


def main():
  option_parser = optparse.OptionParser()
  option_parser.add_option('--cache-dir',
                           help='Set "cache_dir" in the .gclient file to this '
                                'directory, so that all git repositories are '
                                'cached there and shared across multiple '
                                'clones.')
  options, _ = option_parser.parse_args()
  GenerateGClientXWalk(options)


if __name__ == '__main__':
  main()
