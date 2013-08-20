#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import subprocess
import sys

def Which(name):
  """Search PATH for executable files with the given name."""
  result = []
  exts = filter(None, os.environ.get('PATHEXT', '').split(os.pathsep))
  path = os.environ.get('PATH', None)
  if path is None:
    return []
  for p in os.environ.get('PATH', '').split(os.pathsep):
    p = os.path.join(p, name)
    if os.access(p, os.X_OK):
      result.append(p)
    for e in exts:
      pext = p + e
      if os.access(pext, os.X_OK):
        result.append(pext)
  return result


def Execution():
  android_path_array = Which("android")
  if not android_path_array:
    print "Please install Android SDK first."
    return

  sdk_root_path = os.path.dirname(os.path.dirname(android_path_array[0]))
  sdk_jar_path = '%s/platforms/android-17/android.jar' % sdk_root_path
  apk_name = 'XWalkAppTemplate'
  key_store = 'scripts/ant/chromium-debug.keystore'

  if not os.path.exists("out/"):
    os.mkdir("out")

  # Make sure to use ant-tasks.jar correctly.
  # Default Android SDK names it as ant-tasks.jar
  # Chrome third party Android SDk names it as anttasks.jar
  ant_tasks_jar_path = '%s/tools/lib/ant-tasks.jar' % sdk_root_path
  if not os.path.exists(ant_tasks_jar_path):
    ant_tasks_jar_path = '%s/tools/lib/anttasks.jar' % sdk_root_path

  # Check whether ant is installed.
  try:
    proc = subprocess.Popen(['ant', '-version'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  except EnvironmentError:
    print "Please install ant first."
    sys.exit()

  proc = subprocess.Popen(['python', 'scripts/gyp/ant.py',
                           '-DADDITIONAL_RES_DIRS=',
                           '-DADDITIONAL_RES_PACKAGES=',
                           '-DADDITIONAL_R_TEXT_FILES=',
                           '-DANDROID_MANIFEST=app_src/AndroidManifest.xml',
                           '-DANDROID_SDK_JAR=%s' % sdk_jar_path,
                           '-DANDROID_SDK_ROOT=%s' % sdk_root_path,
                           '-DANDROID_SDK_VERSION=17',
                           '-DANT_TASKS_JAR=%s' % ant_tasks_jar_path,
                           '-DLIBRARY_MANIFEST_PATHS=',
                           '-DOUT_DIR=out',
                           '-DRESOURCE_DIR=app_src/res',
                           '-DSTAMP=codegen.stamp',
                           '-Dbasedir=.',
                           '-buildfile',
                           'scripts/ant/apk-codegen.xml'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  # Check whether java is installed.
  try:
    proc = subprocess.Popen(['java', '-version'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  except EnvironmentError:
    print "Please install Oracle JDK first."
    sys.exit()

  classpath = ('--classpath=\"libs/xwalk_app_runtime_activity_java.jar\"'
               ' \"libs/xwalk_app_runtime_client_java.jar\" %s' % sdk_jar_path)
  proc = subprocess.Popen(['python', 'scripts/gyp/javac.py',
                           '--output-dir=out/classes',
                           classpath,
                           '--src-dirs=app_src/src \"out/gen\"',
                           '--javac-includes=',
                           '--chromium-code=0',
                           '--stamp=compile.stam'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  proc = subprocess.Popen(['python', 'scripts/gyp/ant.py',
                           '-DADDITIONAL_RES_DIRS=',
                           '-DADDITIONAL_RES_PACKAGES=',
                           '-DADDITIONAL_R_TEXT_FILES=',
                           '-DANDROID_SDK_JAR=%s' % sdk_jar_path,
                           '-DANDROID_SDK_ROOT=%s' % sdk_root_path,
                           '-DANT_TASKS_JAR=%s' % ant_tasks_jar_path,
                           '-DAPK_NAME=%s' % apk_name,
                           '-DAPP_MANIFEST_VERSION_CODE=0',
                           '-DAPP_MANIFEST_VERSION_NAME=Developer Build',
                           '-DASSET_DIR=app_src/assets',
                           '-DCONFIGURATION_NAME=Release',
                           '-DOUT_DIR=out',
                           '-DRESOURCE_DIR=app_src/res',
                           '-DSTAMP=package_resources.stamp',
                           '-Dbasedir=.',
                           '-buildfile',
                           'scripts/ant/apk-package-resources.xml'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  proc = subprocess.Popen(['python', 'scripts/gyp/jar.py',
                           '--classes-dir=out/classes',
                           '--jar-path=libs/app_apk.jar',
                           '--excluded-classes=',
                           '--stamp=jar.stamp'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  proc = subprocess.Popen(['python', 'scripts/gyp/dex.py',
                           '--dex-path=out/classes.dex',
                           '--android-sdk-root=%s' % sdk_root_path,
                           'libs/xwalk_app_runtime_activity_java.dex.jar',
                           'libs/xwalk_app_runtime_client_java.dex.jar',
                           'out/classes'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  proc = subprocess.Popen(['python', 'scripts/gyp/ant.py',
                           '-DANDROID_SDK_ROOT=%s' % sdk_root_path,
                           '-DANT_TASKS_JAR=%s' % ant_tasks_jar_path,
                           '-DAPK_NAME=%s' % apk_name,
                           '-DCONFIGURATION_NAME=Release',
                           '-DOUT_DIR=out',
                           '-DSOURCE_DIR=app_src/src',
                           '-DUNSIGNED_APK_PATH=out/app-unsigned.apk',
                           '-Dbasedir=.',
                           '-buildfile',
                           'scripts/ant/apk-package.xml'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  proc = subprocess.Popen(['python', 'scripts/gyp/finalize_apk.py',
                           '--android-sdk-root=%s' % sdk_root_path,
                           '--unsigned-apk-path=out/app-unsigned.apk',
                           '--final-apk-path=out/%s.apk' % apk_name,
                           '--keystore-path=%s' % key_store],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  src_file = 'out/%s.apk' % apk_name
  dst_file = '%s.apk' % apk_name
  shutil.copyfile(src_file, dst_file)


def main():
  Execution()


if __name__ == '__main__':
  sys.exit(main())
