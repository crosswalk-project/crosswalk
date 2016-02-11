#!/usr/bin/env python
#
# Copyright (c) 2016 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This program generates a ZIP file containing Crosswalk and supporting files and
directories with everything required to run Crosswalk on Windows.
"""

import argparse
import os
import sys
import zipfile


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--build-dir', required=True,
                      help='Top-level build directory.')
  parser.add_argument('--dest', required=True,
                      help='Name of the ZIP file that will be generated.')
  parser.add_argument('--dirs', nargs='*',
                      help='Directories to package.')
  parser.add_argument('--files', nargs='*',
                      help='Files to package.')
  args = parser.parse_args()

  with zipfile.ZipFile(args.dest, 'w', zipfile.ZIP_DEFLATED) as zip_file:
    for filename in args.files:
      zip_file.write(filename, os.path.relpath(filename, args.build_dir))
    for dirname in args.dirs:
      for root, _, files in os.walk(dirname):
        for filename in files:
          filepath = os.path.join(root, filename)
          zip_path = os.path.relpath(filepath, args.build_dir)
          zip_file.write(filepath, zip_path)


if __name__ == '__main__':
  sys.exit(main())
