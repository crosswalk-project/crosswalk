# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import errno
import os
import shutil
import sys


def GenerateAppTemplate(build_dir, build_mode, output_dir, source_dir):
  """
  Prepares xwalk_app_template/, which contains files used for packaging
  Crosswalk apps. Its primary consumer is app-tools.
  """
  dirs = (
    (os.path.join(source_dir, 'app/android/app_template'),
     os.path.join(output_dir, 'template')),
    (os.path.join(build_dir, 'xwalk_core_library'),
     os.path.join(output_dir, 'xwalk_core_library')),
    (os.path.join(build_dir, 'xwalk_shared_library'),
     os.path.join(output_dir, 'xwalk_shared_library')),
  )
  files = (
    (os.path.join(source_dir, 'API_VERSION'),
     os.path.join(output_dir, 'API_VERSION')),
    (os.path.join(source_dir, 'VERSION'),
     os.path.join(output_dir, 'VERSION')),
    (os.path.join(build_dir, 'lib.java', 'xwalk_app_runtime_java.jar'),
     os.path.join(output_dir, 'template', 'libs',
                  'xwalk_app_runtime_java.jar')),
  )

  for src, dest in dirs:
    # Make sure app/android/app_template's BUILD.gn is not included.
    shutil.copytree(src, dest, ignore=shutil.ignore_patterns('*.gn'))
  for src, dest in files:
    try:
      os.makedirs(os.path.dirname(dest))
    except OSError, e:
      if e.errno == errno.EEXIST:
        pass
    shutil.copy2(src, dest)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--build-dir', required=True,
                      help='Build directory location.')
  parser.add_argument('--build-mode', required=True,
                      help='Build mode (Release, Debug etc).')
  parser.add_argument('--output-dir', required=True,
                      help='Directory where the app template files will be '
                      'stored.')
  parser.add_argument('--source-dir', required=True,
                      help='Top-level source directory location.')
  args = parser.parse_args()

  shutil.rmtree(args.output_dir, ignore_errors=True)
  os.makedirs(args.output_dir)
  GenerateAppTemplate(args.build_dir, args.build_mode, args.output_dir,
                      args.source_dir)


if __name__ == '__main__':
  sys.exit(main())
