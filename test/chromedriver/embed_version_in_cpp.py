#!/usr/bin/env python
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Embeds Chrome user data files in C++ code."""

import optparse
import os
import sys

import chrome_paths
import cpp_source

sys.path.insert(0, os.path.join(chrome_paths.GetSrc(), 'build', 'util'))
import lastchange


def main():
  parser = optparse.OptionParser()
  parser.add_option('', '--version-file')
  parser.add_option(
      '', '--directory', type='string', default='.',
      help='Path to directory where the cc/h  file should be created')
  options, args = parser.parse_args()

  version = open(options.version_file, 'r').read().strip()
  revision = lastchange.FetchVersionInfo(None).revision
  if revision:
    version += '.' + revision.strip()

  global_string_map = {
      'kChromeDriverVersion': version
  }
  cpp_source.WriteSource('version',
                         'chrome/test/chromedriver',
                         options.directory, global_string_map)


if __name__ == '__main__':
  sys.exit(main())

