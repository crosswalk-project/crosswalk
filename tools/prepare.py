#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import sys

def Clean(dir_to_clean):
  if os.path.isdir(dir_to_clean):
    shutil.rmtree(dir_to_clean)


def PrepareFromChromium(target_dir):
  gyp_dir =  os.path.join(target_dir, 'scripts', 'gyp')
  if not os.path.exists(gyp_dir):
    os.makedirs(gyp_dir)
  shutil.copytree('../build/android/gyp/util', os.path.join(gyp_dir, 'util'))
  shutil.copy('../build/android/gyp/ant.py', gyp_dir)


def PrepareFromXwalk(target_dir):
  libs_dir = os.path.join(target_dir, 'libs')
  if not os.path.exists(libs_dir):
    os.makedirs(libs_dir)
  src_dir = os.path.join(os.path.dirname(target_dir), 'lib.java')
  jar_file_list = ['xwalk_app_runtime_activity_java.dex.jar',
                   'xwalk_app_runtime_activity_java.jar',
                   'xwalk_app_runtime_client_java.dex.jar',
                   'xwalk_app_runtime_client_java.jar',
                   'xwalk_core_java.dex.jar',
                   'xwalk_core_java.jar']
  for jar_file in jar_file_list:
    shutil.copy(os.path.join(src_dir, jar_file), libs_dir)

  native_libs_target_dir = os.path.join(target_dir, 'native_libs', 'libs')
  native_libs_src_dir = os.path.join(os.path.dirname(target_dir),
                                     'xwalk_runtime_lib_apk', 'libs')
  shutil.copytree(native_libs_src_dir, native_libs_target_dir)

  ant_dir =  os.path.join(target_dir, 'scripts', 'ant')
  if not os.path.exists(ant_dir):
    os.makedirs(ant_dir)
  ant_file_list = ['./app/tools/android/ant/apk-codegen.xml',
                   './app/tools/android/ant/apk-package.xml',
                   './app/tools/android/ant/apk-package-resources.xml',
                   './app/tools/android/ant/xwalk-debug.keystore']
  for ant_file in ant_file_list:
    shutil.copy(ant_file, ant_dir)

  gyp_dir =  os.path.join(target_dir, 'scripts', 'gyp')
  if not os.path.exists(gyp_dir):
    os.makedirs(gyp_dir)
  gyp_file_list = ['./app/tools/android/gyp/dex.py',
                   './app/tools/android/gyp/finalize_apk.py',
                   './app/tools/android/gyp/jar.py',
                   './app/tools/android/gyp/javac.py']
  for gyp_file in gyp_file_list:
    shutil.copy(gyp_file, gyp_dir)

  util_dir = os.path.join(gyp_dir, 'util')
  if not os.path.exists(util_dir):
    os.makedirs(util_dir)
  shutil.copy('./app/tools/android/gyp/util/build_utils.py', util_dir)

  app_src_dir = os.path.join(target_dir, 'app_src')
  if not os.path.exists(app_src_dir):
    os.makedirs(app_src_dir)
  shutil.copy('./app/android/app_template/AndroidManifest.xml', app_src_dir)
  app_src_folder_list = ['./app/android/app_template/assets',
                         './app/android/app_template/res',
                         './app/android/app_template/src']
  for folder in app_src_folder_list:
    shutil.copytree(folder, os.path.join(app_src_dir, os.path.basename(folder)))

  packaging_tool_list = ['./app/tools/android/customize.py',
                         './app/tools/android/make_apk.py',
                         './app/tools/android/manifest_json_parser.py']
  for packaging_tool in packaging_tool_list:
    shutil.copy(packaging_tool, target_dir)


def main(args):
  if len(args) != 1:
    print 'You must provide only one argument: folder to update'
    return 1
  target_dir = args[0]
  Clean(target_dir)
  PrepareFromChromium(target_dir)
  PrepareFromXwalk(target_dir)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
