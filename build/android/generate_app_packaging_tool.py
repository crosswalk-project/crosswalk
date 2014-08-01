#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import os
import shutil
import sys
import re
import operator
from common_function import RemoveUnusedFilesInReleaseMode

def AddExeExtensions(name):
  exts_str = os.environ.get('PATHEXT', '').lower()
  exts = [_f for _f in exts_str.split(os.pathsep) if _f]
  result = []
  result.append(name)
  for e in exts:
    result.append(name + e)
  return result


def Clean(dir_to_clean):
  if os.path.isdir(dir_to_clean):
    shutil.rmtree(dir_to_clean)


def PrepareFromChromium(target_dir):
  gyp_dir =  os.path.join(target_dir, 'scripts', 'gyp')
  if not os.path.exists(gyp_dir):
    os.makedirs(gyp_dir)
  shutil.copytree('../build/android/gyp/util', os.path.join(gyp_dir, 'util'))


def Find(name, path):
  """Find executable file with the given name
  and maximum API level under specific path."""
  result = {}
  for root, _, files in os.walk(path):
    if name in files:
      key = os.path.join(root, name)
      sdk_version = os.path.basename(os.path.dirname(key))
      str_num = re.search(r'\d+', sdk_version)
      if str_num:
        result[key] = int(str_num.group())
      else:
        result[key] = 0
  if not result:
    raise Exception()
  return max(iter(result.items()), key=operator.itemgetter(1))[0]


def Which(name):
  """Searches PATH for executable files with the given name, also taking
  PATHEXT into account. Returns the first existing match, or None if no matches
  are found."""
  for path in os.environ.get('PATH', '').split(os.pathsep):
    for filename in AddExeExtensions(name):
      full_path = os.path.join(path, filename)
      if os.path.isfile(full_path) and os.access(full_path, os.X_OK):
        return full_path
  return None


def FindSdkPath():
  android_path = Which('android')
  sdk_root_path = os.path.dirname(os.path.dirname(android_path))
  return sdk_root_path


def PrepareFromXwalk(src_dir, target_dir):
  '''Prepare different files for app packaging tools. All resources are used by
  make_apk.py.
  '''
  # The version of yui compressor.
  yui_compressor_version = '2.4.8'

  # Get the dir of source code from src_dir: ../../.
  source_code_dir = os.path.dirname(os.path.dirname(src_dir))

  # The directories for source and target .jar files.
  jar_src_dir = os.path.join(src_dir, 'lib.java')
  jar_target_dir = os.path.join(target_dir, 'libs')

  # The directories for generated resources.
  gen_res_src_dir = os.path.join(src_dir, 'gen')
  gen_res_target_dir = os.path.join(target_dir, 'gen')
  xwalk_core_library_res_dir = os.path.join(
      src_dir, 'xwalk_core_library', 'res')

  # The directory for source packaging tools.
  tools_src_dir = os.path.join(source_code_dir, 'xwalk/app/tools/android')

  # The directories for source and target gyp.
  gyp_src_dir =  os.path.join(tools_src_dir, 'gyp')
  gyp_target_dir = os.path.join(target_dir, 'scripts/gyp')

  # The source file/directory list to be copied and the target directory list.
  source_target_list = [
    (os.path.join(source_code_dir, 'xwalk/VERSION'), target_dir),

    # This jar is needed for minifying and obfuscating the javascript and css.
    (os.path.join(tools_src_dir,
      'libs/yuicompressor-' + yui_compressor_version + '.jar'),
        jar_target_dir),

    # This jar is needed for 'javac' compile.
    (os.path.join(jar_src_dir, 'xwalk_app_runtime_java.jar'), jar_target_dir),
    (os.path.join(jar_src_dir, 'xwalk_runtime_embedded.dex.jar'),
      jar_target_dir),

    # Native library, like libxwalkcore.so.
    (os.path.join(src_dir, 'xwalk_runtime_lib_apk/libs/x86'),
     os.path.join(target_dir, 'native_libs/x86/libs/x86')),
    (os.path.join(src_dir, 'xwalk_runtime_lib_apk/libs/armeabi-v7a'),
     os.path.join(target_dir, 'native_libs/armeabi-v7a/libs/armeabi-v7a')),

    # Native source package(xwalk.pak) and related js files for extension.
    (os.path.join(src_dir, 'xwalk_runtime_lib/assets'),
     os.path.join(target_dir, 'native_libs_res')),

    # Various Java resources.
    # FIXME(wang16): Copy from the xwalk_core_library first. We need to
    # consider refine the whole process of make_apk.
    (os.path.join(gen_res_src_dir, 'ui_java/java_R'),
     os.path.join(gen_res_target_dir, 'ui_java/java_R')),
    (os.path.join(gen_res_src_dir, 'content_java/java_R'),
     os.path.join(gen_res_target_dir, 'content_java/java_R')),
    (os.path.join(gen_res_src_dir, 'xwalk_core_internal_java/java_R'),
     os.path.join(gen_res_target_dir, 'xwalk_core_internal_java/java_R')),
    (xwalk_core_library_res_dir,
     os.path.join(target_dir, 'libs_res/xwalk_core_library')),

    # The app wrapper code. It's the template Java code.
    (os.path.join(source_code_dir, 'xwalk/app/android/app_template'),
     os.path.join(target_dir, 'app_src')),

    # Copy below 7 files to overwrite the existing ones from Chromium.
    (os.path.join(gyp_src_dir, 'util/build_utils.py'),
     os.path.join(gyp_target_dir, 'util')),
    (os.path.join(gyp_src_dir, 'util/md5_check.py'),
     os.path.join(gyp_target_dir, 'util')),
    (os.path.join(gyp_src_dir, 'ant.py'), gyp_target_dir),
    (os.path.join(gyp_src_dir, 'dex.py'), gyp_target_dir),
    (os.path.join(gyp_src_dir, 'finalize_apk.py'), gyp_target_dir),
    (os.path.join(gyp_src_dir, 'jar.py'), gyp_target_dir),
    (os.path.join(gyp_src_dir, 'javac.py'), gyp_target_dir),

    # Build and python tools.
    (os.path.join(tools_src_dir, 'ant'),
     os.path.join(target_dir, 'scripts/ant')),
    (os.path.join(tools_src_dir, 'app_info.py'), target_dir),
    (os.path.join(tools_src_dir, 'compress_js_and_css.py'), target_dir),
    (os.path.join(tools_src_dir, 'customize.py'), target_dir),
    (os.path.join(tools_src_dir, 'customize_launch_screen.py'), target_dir),
    (os.path.join(tools_src_dir, 'handle_permissions.py'), target_dir),
    (os.path.join(tools_src_dir, 'handle_xml.py'), target_dir),
    (os.path.join(tools_src_dir, 'make_apk.py'), target_dir),
    (os.path.join(tools_src_dir, 'manifest_json_parser.py'), target_dir),
    (os.path.join(tools_src_dir, 'parse_xpk.py'), target_dir)
  ]

  # Package with the necessary tools&libs from Android SDK
  package_sdk = os.environ.get('PACKAGE_WITH_DEPENDENT_TOOLS', '')
  if package_sdk == '1':
    sdk_root_path = FindSdkPath()
    sdk_jar_path = Find('android.jar', \
                        os.path.join(sdk_root_path, 'platforms'))
    level_string = os.path.basename(os.path.dirname(sdk_jar_path))
    sdk_platforms_dir = os.path.join(target_dir, 'sdk', 'platforms', \
                                     level_string)
    source_target_list.append((os.path.join(sdk_root_path, 'platforms', \
                              level_string, 'android.jar'), sdk_platforms_dir))
    aapt_path = ''
    for aapt_str in AddExeExtensions('aapt'):
      try:
        aapt_path = Find(aapt_str, sdk_root_path)
        break
      except Exception:
        pass
    aapt_folder = os.path.basename(os.path.dirname(aapt_path))
    sdk_build_dir = os.path.join(target_dir, 'sdk', 'build-tools', aapt_folder)
    source_target_list.append((os.path.join(sdk_root_path, 'build-tools', \
                              aapt_folder), sdk_build_dir))

    # Include building-related items from "tools" path of Android SDK
    sdk_tools_dir = os.path.join(target_dir, 'sdk','tools')
    f = open('build/android/tools-list.txt')
    lines = f.readlines()
    for line in lines:
      line = line.strip('\n')
      sdk_tools_src = os.path.join(sdk_root_path, line)
      sdk_bash_tools = os.path.basename(os.path.dirname(sdk_tools_src))
      if (sdk_bash_tools == 'lib'):
        sdk_tool_target = os.path.join(sdk_tools_dir, 'lib')
      else:
        sdk_tool_target = sdk_tools_dir
      source_target_list.append((sdk_tools_src, sdk_tool_target))

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
