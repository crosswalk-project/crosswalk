#!/usr/bin/env python
#
# Copyright (c) 2015 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import optparse
import os
import shutil
import sys
import subprocess

GYP_ANDROID_DIR = os.path.join(os.path.dirname(__file__),
                               os.pardir, os.pardir, os.pardir,
                               'build',
                               'android',
                               'gyp')
sys.path.append(GYP_ANDROID_DIR)

from util import build_utils


def DoCompress(dest_path, sources):
  build_utils.DeleteDirectory(dest_path)
  build_utils.MakeDirectory(dest_path)

  for source in sources:
    shutil.copy(source, dest_path)
    file_to_compress = os.path.join(dest_path, os.path.basename(source))
    subprocess.check_call(['lzma', '-f', file_to_compress])


def DoShowOutputNames(dest_path, sources):
  for source in sources:
    print('%s.lzma' % os.path.join(dest_path, os.path.basename(source)))


def main():
  parser = optparse.OptionParser()
  parser.add_option('--dest-path',
                    help='Destination directory for compressed files')
  parser.add_option('--mode', choices=('compress', 'show-output-names'),
                    help='Whether to compress the files or show their '
                    'compressed names')
  parser.add_option('--sources', help='The list of files to be compressed')

  options, _ = parser.parse_args(sys.argv)
  sources = build_utils.ParseGypList(options.sources)

  if options.mode == 'compress':
    return DoCompress(options.dest_path, sources)
  else:
    return DoShowOutputNames(options.dest_path, sources)


if __name__ == '__main__':
  sys.exit(main())
