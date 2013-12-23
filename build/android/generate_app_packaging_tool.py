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


def PrepareFromChromium(target_dir):
  gyp_dir =  os.path.join(target_dir, 'scripts', 'gyp')
  if not os.path.exists(gyp_dir):
    os.makedirs(gyp_dir)
  shutil.copytree('../build/android/gyp/util', os.path.join(gyp_dir, 'util'))
  shutil.copy('../build/android/gyp/ant.py', gyp_dir)


def PrepareFromXwalk(src_dir, target_dir):
  '''Prepare different files for app packaging tools. All resources are used by
  make_apk.py.
  '''
  # Get the dir of source code from src_dir: ../../.
  source_code_dir = os.path.dirname(os.path.dirname(src_dir))

  # The directories for source and target .jar files.
  jar_src_dir = os.path.join(src_dir, 'lib.java')
  jar_target_dir = os.path.join(target_dir, 'libs')

  # The directories for generated resources.
  gen_res_src_dir = os.path.join(src_dir, 'gen')
  gen_res_target_dir = os.path.join(target_dir, 'gen')

  # The directory for source packaging tools.
  tools_src_dir = os.path.join(source_code_dir, 'xwalk/app/tools/android')

  # The directories for source and target gyp.
  gyp_src_dir =  os.path.join(tools_src_dir, 'gyp')
  gyp_target_dir = os.path.join(target_dir, 'scripts/gyp')

  # The source file/directory list to be copied and the target directory list.
  source_target_list = [
    (os.path.join(source_code_dir, 'xwalk/VERSION'), target_dir),

    # This jar is needed for 'javac' compile.
    (os.path.join(jar_src_dir, 'xwalk_app_runtime_java.jar'), jar_target_dir),
    (os.path.join(jar_src_dir, 'xwalk_core_embedded.dex.jar'), jar_target_dir),

    # Native library, like libxwalkcore.so.
    (os.path.join(src_dir, 'xwalk_runtime_lib_apk/libs/x86'),
     os.path.join(target_dir, 'native_libs/x86/libs/x86')),
    (os.path.join(src_dir, 'xwalk_runtime_lib_apk/libs/armeabi-v7a'),
     os.path.join(target_dir, 'native_libs/armeabi-v7a/libs/armeabi-v7a')),

    # Native source package(xwalk.pak) and related js files for extension.
    (os.path.join(src_dir, 'xwalk_runtime_lib/assets'),
     os.path.join(target_dir, 'native_libs_res')),

    # Various Java resources.
    (os.path.join(source_code_dir, 'content/public/android/java/res'),
     os.path.join(target_dir, 'libs_res/content')),
    (os.path.join(source_code_dir, 'ui/android/java/res'),
     os.path.join(target_dir, 'libs_res/ui')),
    (os.path.join(source_code_dir, 'xwalk/runtime/android/java/res'),
     os.path.join(target_dir, 'libs_res/runtime')),

    (os.path.join(gen_res_src_dir, 'ui_java/java_R'),
     os.path.join(gen_res_target_dir, 'ui_java/java_R')),
    (os.path.join(gen_res_src_dir, 'ui_java/res_crunched'),
     os.path.join(gen_res_target_dir, 'ui_java/res_crunched')),
    (os.path.join(gen_res_src_dir, 'ui_java/res_grit'),
     os.path.join(gen_res_target_dir, 'ui_java/res_grit')),
    (os.path.join(gen_res_src_dir, 'ui_java/res_v14_compatibility'),
     os.path.join(gen_res_target_dir, 'ui_java/res_v14_compatibility')),

    (os.path.join(gen_res_src_dir, 'content_java/java_R'),
     os.path.join(gen_res_target_dir, 'content_java/java_R')),
    (os.path.join(gen_res_src_dir, 'content_java/res_crunched'),
     os.path.join(gen_res_target_dir, 'content_java/res_crunched')),
    (os.path.join(gen_res_src_dir, 'content_java/res_grit'),
     os.path.join(gen_res_target_dir, 'content_java/res_grit')),
    (os.path.join(gen_res_src_dir, 'content_java/res_v14_compatibility'),
     os.path.join(gen_res_target_dir, 'content_java/res_v14_compatibility')),

    (os.path.join(gen_res_src_dir, 'xwalk_core_java/java_R'),
     os.path.join(gen_res_target_dir, 'xwalk_core_java/java_R')),
    (os.path.join(gen_res_src_dir, 'xwalk_core_java/res_crunched'),
     os.path.join(gen_res_target_dir, 'xwalk_core_java/res_crunched')),
    (os.path.join(gen_res_src_dir, 'xwalk_core_java/res_grit'),
     os.path.join(gen_res_target_dir, 'xwalk_core_java/res_grit')),
    (os.path.join(gen_res_src_dir, 'xwalk_core_java/res_v14_compatibility'),
     os.path.join(gen_res_target_dir, 'xwalk_core_java/res_v14_compatibility')),

    # The app wrapper code. It's the template Java code.
    (os.path.join(source_code_dir, 'xwalk/app/android/app_template'),
     os.path.join(target_dir, 'app_src')),

    # Copy below 5 files to overwrite the existing ones from Chromium.
    (os.path.join(gyp_src_dir, 'util/build_utils.py'),
     os.path.join(gyp_target_dir, 'util')),
    (os.path.join(gyp_src_dir, 'dex.py'), gyp_target_dir),
    (os.path.join(gyp_src_dir, 'finalize_apk.py'), gyp_target_dir),
    (os.path.join(gyp_src_dir, 'jar.py'), gyp_target_dir),
    (os.path.join(gyp_src_dir, 'javac.py'), gyp_target_dir),

    # Build and python tools.
    (os.path.join(tools_src_dir, 'ant'),
     os.path.join(target_dir, 'scripts/ant')),
    (os.path.join(tools_src_dir, 'customize.py'), target_dir),
    (os.path.join(tools_src_dir, 'make_apk.py'), target_dir),
    (os.path.join(tools_src_dir, 'manifest_json_parser.py'), target_dir),
    (os.path.join(tools_src_dir, 'parse_xpk.py'), target_dir)
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
  PrepareFromChromium(target_dir)
  PrepareFromXwalk(src_dir, target_dir)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
