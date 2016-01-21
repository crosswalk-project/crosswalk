#!/usr/bin/env python

# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Given a list of JAR files passed via --jars, produced one single JAR file with
all their contents merged. JAR files outside --build-dir are ignored.
"""

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
  parser.add_option('--build-dir',
                    help='Base build directory, such as out/Release. JARs '
                    'outside this directory will be skipped.')
  parser.add_option('--jars', help='The jars to merge.')
  parser.add_option('--output-jar', help='Name of the merged JAR file.')

  options, _ = parser.parse_args()
  build_dir = os.path.abspath(options.build_dir)

  with build_utils.TempDir() as temp_dir:
    for jar_file in build_utils.ParseGypList(options.jars):
      if not os.path.abspath(jar_file).startswith(build_dir):
        continue
      build_utils.ExtractAll(jar_file, path=temp_dir, pattern='*.class')
    jar.JarDirectory(temp_dir, options.output_jar)


if __name__ == '__main__':
  sys.exit(main())
