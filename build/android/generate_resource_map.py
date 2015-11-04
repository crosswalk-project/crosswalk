#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
import shutil
import sys

GYP_ANDROID_DIR = os.path.join(os.path.dirname(__file__),
                               os.pardir, os.pardir, os.pardir,
                               'build',
                               'android',
                               'gyp')
sys.path.append(GYP_ANDROID_DIR)

from util import build_utils


def CreateResourceMap(r_java, res_map):
  package_regex = re.compile('^package ([a-zA-Z0-9_\.]*);$')
  package = ''
  output_content = []
  for line in open(r_java, 'r').readlines():
    if not package:
      package_match = package_regex.match(line)
      if package_match:
        package = package_match.group(1)
    output_content.append(re.sub(r'\s*=\s*0x[0-9a-f]{8};', ';', line))
  output_path = os.path.join(res_map, os.path.sep.join(package.split('.')))
  if not os.path.isdir(output_path):
    os.makedirs(output_path)
  with open(os.path.join(output_path, 'R.java'), 'w') as output:
    output.write(''.join(output_content))


def main():
  parser = optparse.OptionParser()
  parser.add_option('--gen-dir',
                    help='The folder contains generated R.java.',
                    type='string')
  parser.add_option('--resource-map-dir',
                    help='The folder to place resource map.',
                    type='string')
  parser.add_option('--stamp',
                    help='The file to be stamped on success.',
                    type='string')
  options, _ = parser.parse_args()

  if not os.path.isdir(options.gen_dir):
    return 1
  if os.path.exists(options.resource_map_dir):
    shutil.rmtree(options.resource_map_dir)
  os.makedirs(options.resource_map_dir)
  for root, _, files in os.walk(options.gen_dir):
    if 'R.java' in files:
      if os.path.basename(root) == 'core':
        continue
      r_java = os.path.join(root, 'R.java')
      CreateResourceMap(r_java, options.resource_map_dir)

  build_utils.Touch(options.stamp)


if __name__ == '__main__':
  sys.exit(main())
