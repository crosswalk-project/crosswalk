# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

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
  parser.add_argument('--android-template', required=True,
                      help='Path to the main template directory.')
  parser.add_argument('--core-library-dir', required=True,
                      help='Path to xwalk_core_library/.')
  parser.add_argument('--extra-files',
                      help='Extra files to add to the template.')
  parser.add_argument('--output-dir', required=True,
                      help='Directory where the app template files will be '
                      'stored.')
  parser.add_argument('--shared-library-dir', required=True,
                      help='Path to xwalk_shared_library/.')
  parser.add_argument('--stamp', required=True,
                      help='Path to touch on success.')
  parser.add_argument('--xwalk-runtime-jar', required=True,
                      help='Path to the Crosswalk runtime JAR.')

  args = parser.parse_args(build_utils.ExpandFileArgs(sys.argv[1:]))

  build_utils.DeleteDirectory(args.output_dir)
  build_utils.MakeDirectory(args.output_dir)

  shutil.copytree(args.android_template,
                  os.path.join(args.output_dir, 'template'),
                  # Make sure app/android/app_template's BUILD.gn is
                  # not included.
                  ignore=shutil.ignore_patterns('*.gn'))
  shutil.copytree(args.core_library_dir,
                  os.path.join(args.output_dir, 'xwalk_core_library'))
  shutil.copytree(args.shared_library_dir,
                  os.path.join(args.output_dir, 'xwalk_shared_library'))

  # For now at least, we have to continue calling the runtime JAR
  # xwalk_app_runtime_java.jar to avoid breaking app-tools.
  template_libs_dir = os.path.join(args.output_dir, 'template', 'libs')
  build_utils.MakeDirectory(template_libs_dir)
  shutil.copy2(args.xwalk_runtime_jar,
               os.path.join(template_libs_dir, 'xwalk_app_runtime_java.jar'))

  for extra_file in build_utils.ParseGypList(args.extra_files):
    # For now we just assume all files are supposed to go to the root of the
    # output directory.
    shutil.copy2(extra_file, args.output_dir)

  build_utils.Touch(args.stamp)


if __name__ == '__main__':
  sys.exit(main())
