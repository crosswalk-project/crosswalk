#!/usr/bin/env python
#
# Copyright (c) 2015 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import argparse
import os
import shutil
import sys

GYP_ANDROID_DIR = os.path.join(os.path.dirname(__file__),
                               os.pardir, os.pardir, os.pardir,
                               'build',
                               'android',
                               'gyp')
sys.path.append(GYP_ANDROID_DIR)

from util import build_utils


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--dest-path', required=True,
                      help='Destination directory for compressed files.')
  parser.add_argument('--sources', required=True,
                      help='The list of files to be compressed.')

  options = parser.parse_args()
  options.sources = build_utils.ParseGypList(options.sources)

  with build_utils.TempDir() as temp_dir:
    for source in options.sources:
      shutil.copy2(source, temp_dir)
      file_to_compress = os.path.join(temp_dir, os.path.basename(source))
      build_utils.CheckOutput(['lzma', '-f', file_to_compress],
                              print_stderr=True)
    build_utils.DeleteDirectory(options.dest_path)
    shutil.copytree(temp_dir, options.dest_path)


if __name__ == '__main__':
  sys.exit(main())
