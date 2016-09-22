# Copyright (c) 2016 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import argparse
import os
import sys

GYP_ANDROID_DIR = os.path.join(os.path.dirname(__file__), os.pardir,
                               os.pardir, os.pardir, os.pardir, os.pardir,
                               'build',
                               'android',
                               'gyp',
                               'util')
sys.path.append(GYP_ANDROID_DIR)

import build_utils


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--classpath', required=True, action='append',
                      help='What to use as javadoc\'s classpath. Can be '
                      'specified multiple times.')
  parser.add_argument('--input-srcjar',
                      help='Source JAR containing more files to process.')
  parser.add_argument('--java-files', required=True,
                      help='Java files to process.')
  parser.add_argument('--output-dir', required=True,
                      help='Directory to store the documentation.')
  parser.add_argument('--srcjar-files',
                      help='When specified, only process the given files '
                      'inside the srcjar set via --input-srcjar.')
  parser.add_argument('--stamp', required=True,
                      help='File to touch on success.')

  options = parser.parse_args(build_utils.ExpandFileArgs(sys.argv[1:]))
  options.java_files = build_utils.ParseGypList(options.java_files)

  if options.srcjar_files and options.input_srcjar is None:
    print '--srcjar-files specified without --input-srcjar. Ignoring files.'

  with build_utils.TempDir() as temp_dir:
    java_files = options.java_files
    if options.input_srcjar:
      if options.srcjar_files:
        pattern = lambda f: f in options.srcjar_files
        java_files += [os.path.join(temp_dir, f) for f in
                       build_utils.ParseGypList(options.srcjar_files)]
      else:
        pattern = None
        java_files += [os.path.join(temp_dir, '*.java')]
      build_utils.ExtractAll(options.input_srcjar, path=temp_dir)

    classpath = ':'.join(options.classpath)
    javadoc_cmd = ['javadoc', '-d', options.output_dir,
                   '-classpath', classpath]
    javadoc_cmd.extend(java_files)

    build_utils.CheckOutput(javadoc_cmd, print_stderr=True)

  build_utils.Touch(options.stamp)


if __name__ == '__main__':
  sys.exit(main())
