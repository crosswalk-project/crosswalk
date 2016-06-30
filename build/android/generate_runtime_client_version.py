#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import sys
import os
import zipfile

def ReplaceVersion(template_file, output_file, xwalk_version):
  output_handle = open(output_file, 'w+')
  # Replace the occupying string with the current xwalk version.
  origin_string = 'TO_BE_REPLACED_BY_PYTHON_SCRIPT'
  for line in open(template_file):
    if origin_string in line:
      line = line.replace(origin_string, xwalk_version)
    output_handle.write(line)
  output_handle.close()
  # Generate srcjar for generated java file.
  # For example: output is ".../version_java/XWalkRuntimeClientVersion.java",
  # then generate srcjar as ".../XWalkRuntimeClientVersion.java.srcjar".
  output_file_dir = os.path.split(os.path.abspath(output_file))[0]
  output_file_name = os.path.split(os.path.abspath(output_file))[1]
  srcjar_file_dir = os.path.split(os.path.abspath(output_file_dir))[0]
  with zipfile.ZipFile(
    os.path.join(srcjar_file_dir, output_file_name + ".srcjar"), 
    'w', zipfile.ZIP_DEFLATED) as srcjar_file:
    for root, _, files in os.walk(output_file_dir):
      for f in files:
        file_path = os.path.join(root, f)
        srcjar_file.write(file_path, f)


def main(argv):
  parser = optparse.OptionParser()
  info = ('The template java file for generating version of runtime client')
  parser.add_option('--template', help=info)
  info = ('The output java file for version')
  parser.add_option('--output', help=info)
  info = ('The current version of xwalk runtime client')
  parser.add_option('--xwalk-version', help=info)
  options, _ = parser.parse_args()

  if len(argv) != 4:
    parser.error("3 arguments are needed. See --help for help info.")

  template_file = options.template
  output_file = options.output
  xwalk_version = options.xwalk_version
  ReplaceVersion(template_file, output_file, xwalk_version)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
