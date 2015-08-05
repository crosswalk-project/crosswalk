#!/usr/bin/env python

# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import sys

GYP_ANDROID_DIR = os.path.join(os.path.dirname(__file__),
                               os.pardir, os.pardir, os.pardir,
                               'build',
                               'android',
                               'gyp')
sys.path.append(GYP_ANDROID_DIR)

import jar
from util import build_utils


def main():
  parser = optparse.OptionParser()
  parser.add_option('--jars', help='The jars to merge.')
  parser.add_option('--jar-path', help='The output merged jar file.')

  options, _ = parser.parse_args()

  with build_utils.TempDir() as temp_dir:
    for jar_file in build_utils.ParseGypList(options.jars):
      build_utils.ExtractAll(jar_file, path=temp_dir, pattern='*.class')
    jar.JarDirectory(temp_dir, [], options.jar_path)


if __name__ == '__main__':
  sys.exit(main())
