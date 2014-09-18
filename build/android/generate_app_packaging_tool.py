#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import os
import shutil
import sys
from common_function import RemoveUnusedFilesInReleaseMode

def Clean(dir_to_clean):
  if os.path.isdir(dir_to_clean):
    shutil.rmtree(dir_to_clean)


def PrepareFromXwalk(src_dir, target_dir):
  '''Prepare different files for app packaging tools. All resources are used by
  make_apk.py.
  '''
  # The version of yui compressor.
  yui_compressor_version = '2.4.8'

  # Get the dir of source code from src_dir: ../../.
  source_code_dir = os.path.dirname(os.path.dirname(src_dir))

  # The directory to copy libraries and code from.
  jar_src_dir = os.path.join(src_dir, 'lib.java')
  xwalk_core_library_dir = os.path.join(src_dir, 'xwalk_core_library')

  # The directory to copy libraries, code and resources to.
  app_target_dir = os.path.join(target_dir, 'template')
  jar_target_dir = os.path.join(app_target_dir, 'libs')

  # The directory for source packaging tools.
  tools_src_dir = os.path.join(source_code_dir, 'xwalk/app/tools/android')

  # The source file/directory list to be copied and the target directory list.
  source_target_list = [
    (os.path.join(source_code_dir, 'xwalk/VERSION'), target_dir),

    # This jar is needed for minifying and obfuscating the javascript and css.
    (os.path.join(tools_src_dir,
                  'libs/yuicompressor-%s.jar' % yui_compressor_version),
     target_dir),

    # The app wrapper code. It's the template Java code.
    (os.path.join(source_code_dir, 'xwalk/app/android/app_template'),
     app_target_dir),

    (os.path.join(jar_src_dir, 'xwalk_app_runtime_java.jar'), jar_target_dir),

    # XWalk Core Library
    (xwalk_core_library_dir, os.path.join(target_dir, 'xwalk_core_library')),

    # Build and python tools.
    (os.path.join(tools_src_dir, 'ant', 'xwalk-debug.keystore'), target_dir),
    (os.path.join(tools_src_dir, 'app_info.py'), target_dir),
    (os.path.join(tools_src_dir, 'compress_js_and_css.py'), target_dir),
    (os.path.join(tools_src_dir, 'customize.py'), target_dir),
    (os.path.join(tools_src_dir, 'customize_launch_screen.py'), target_dir),
    (os.path.join(tools_src_dir, 'extension_manager.py'), target_dir),
    (os.path.join(tools_src_dir, 'handle_permissions.py'), target_dir),
    (os.path.join(tools_src_dir, 'handle_xml.py'), target_dir),
    (os.path.join(tools_src_dir, 'make_apk.py'), target_dir),
    (os.path.join(tools_src_dir, 'manifest_json_parser.py'), target_dir),
    (os.path.join(tools_src_dir, 'parse_xpk.py'), target_dir),
    (os.path.join(tools_src_dir, 'util.py'), target_dir)
  ]

  for index in range(len(source_target_list)):
    source_path, target_path = source_target_list[index]

    # Process source.
    if not os.path.exists(source_path):
      print ('The source path "%s" does not exist.' % source_path)
      continue

    source_is_file = os.path.isfile(source_path)

    # Process target.
    if source_is_file and not os.path.exists(target_path):
      os.makedirs(target_path)
    if not source_is_file and os.path.isdir(target_path):
      shutil.rmtree(target_path)

    # Do copy.
    if source_is_file:
      shutil.copy(source_path, target_path)
    else:
      shutil.copytree(source_path, target_path)

  # Remove unused files.
  mode = os.path.basename(os.path.dirname(target_dir))
  RemoveUnusedFilesInReleaseMode(mode, os.path.join(target_dir, 'native_libs'))


def main(args):
  if len(args) != 1:
    print 'You must provide only one argument: folder to update'
    return 1
  target_dir = args[0]
  src_dir = os.path.dirname(target_dir)
  Clean(target_dir)
  PrepareFromXwalk(src_dir, target_dir)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
