#!/usr/bin/env python

# Copyright 2011 The Chromium Authors. All rights reserved.
# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import fnmatch
import optparse
import os
import sys


def FindInDirectory(directory, filename_filter):
  files = []
  for root, _dirnames, filenames in os.walk(directory):
    if os.path.abspath(directory) == os.path.abspath(root):
      continue
    matched_files = fnmatch.filter(filenames, filename_filter)
    files.extend((os.path.join(root, f) for f in matched_files))
  return files


def main():
  parser = optparse.OptionParser()
  info = ('product dir to clean up icu data files in')
  parser.add_option('--product-dir', help=info)
  options, _ = parser.parse_args()

  icu_datas = FindInDirectory(options.product_dir, 'icudtl.dat')
  for icu_data in icu_datas:
    os.remove(icu_data)


if __name__ == '__main__':
  sys.exit(main())
