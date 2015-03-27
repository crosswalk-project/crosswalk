#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This script is responsible for generating .gclient-xwalk in the top-level
source directory from DEPS.xwalk.

User-configurable values such as |cache_dir| are fetched from .gclient instead.
"""

import logging
import optparse
import os
import pprint


CROSSWALK_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GCLIENT_ROOT = os.path.dirname(os.path.dirname(CROSSWALK_ROOT))


def ParseGClientConfig():
  """
  Parses the top-level .gclient file (NOT .gclient-xwalk) and returns the
  values set there as a dictionary.
  """
  with open(os.path.join(GCLIENT_ROOT, '.gclient')) as dot_gclient:
    config = {}
    exec(dot_gclient, config)
  return config


def GenerateGClientXWalk(options):
  with open(os.path.join(CROSSWALK_ROOT, 'DEPS.xwalk')) as deps_file:
    deps_contents = deps_file.read()

  if 'XWALK_OS_ANDROID' in os.environ:
    deps_contents += 'target_os = [\'android\']\n'

  gclient_config = ParseGClientConfig()
  if options.cache_dir:
    logging.warning('--cache_dir is deprecated and will be removed in '
                    'Crosswalk 8. You should set cache_dir in .gclient '
                    'instead.')
    cache_dir = options.cache_dir
  else:
    cache_dir = gclient_config.get('cache_dir')
  deps_contents += 'cache_dir = %s\n' % pprint.pformat(cache_dir)

  with open(os.path.join(GCLIENT_ROOT, '.gclient-xwalk'), 'w') as gclient_file:
    gclient_file.write(deps_contents)


def main():
  option_parser = optparse.OptionParser()
  # TODO(rakuco): Remove in Crosswalk 8.
  option_parser.add_option('--cache-dir',
                           help='DEPRECATED Set "cache_dir" in .gclient-xwalk '
                                'to this directory, so that all git '
                                'repositories are cached there.')
  options, _ = option_parser.parse_args()
  GenerateGClientXWalk(options)


if __name__ == '__main__':
  main()
