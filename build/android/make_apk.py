#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import operator
import optparse
import os
import re
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


def Find(name, path):
  """Find executable file with the given name
  and maximum API level under specific path."""
  result = {}
  for root, _, files in os.walk(path):
    if name in files:
      key = os.path.join(root, name)
      str_num = re.search(r'\d+', key.split('/')[-2])
      if str_num:
        result[key] = int(str_num.group())
      else:
        result[key] = 0
  if not result:
    raise Exception()
  return max(result.iteritems(), key=operator.itemgetter(1))[0]


def Customize(options):
  package = '--package=org.xwalk.app.template'
  if options.package:
    package = '--package=%s' % options.package
  name = '--name=AppTemplate'
  if options.name:
    name = '--name=%s' % options.name
  icon = ''
  if options.icon:
    icon = '--icon=%s' % os.path.expanduser(options.icon)
  app_url =  ''
  if options.app_url:
    app_url = '--app-url=%s' % options.app_url
  app_root = ''
  if options.app_root:
    app_root = '--app-root=%s' % os.path.expanduser(options.app_root)
  app_local_path = 'index.html'
  if options.app_local_path:
    app_local_path = '--app-local-path=%s' % options.app_local_path
  fullscreen_flag = ''
  if options.fullscreen:
    fullscreen_flag = '-f'
  proc = subprocess.Popen(['python', 'customize.py', package,
                           name, icon, app_url,
                           app_root, app_local_path, fullscreen_flag],
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out


def Execution(options):
  apk_name = options.name
  android_path_array = Which("android")
  if not android_path_array:
    print "Please install Android SDK first."
    sys.exit(1)

  sdk_root_path = os.path.dirname(os.path.dirname(android_path_array[0]))

  try:
    sdk_jar_path = Find('android.jar', '%s/platforms/' % sdk_root_path)
  except Exception:
    print "Your Android SDK may be ruined, please reinstall it."
    sys.exit(2)

  api_level = int(re.search(r'\d+', sdk_jar_path.split('/')[-2]).group())
  if api_level < 14:
    print "Please install Android API level (>=14) first."
    sys.exit(3)

  if options.keystore_path:
    key_store = os.path.expanduser(options.keystore_path)
    if options.keystore_alias:
      key_alias = options.keystore_alias
    else:
      print "Please provide an alias name of the developer key."
      sys.exit(6)
    if options.keystore_passcode:
      key_code = options.keystore_passcode
    else:
      print "Please provide the passcode of the developer key."
      sys.exit(6)
  else:
    print ('Use xwalk\'s keystore by default for debugging. '
           'Please switch to your keystore when distributing it to app market.')
    key_store = 'scripts/ant/xwalk-debug.keystore'
    key_alias = 'xwalkdebugkey'
    key_code = 'xwalkdebug'

  if not os.path.exists("out/"):
    os.mkdir("out")

  # Make sure to use ant-tasks.jar correctly.
  # Default Android SDK names it as ant-tasks.jar
  # Chrome third party Android SDk names it as anttasks.jar
  ant_tasks_jar_path = '%s/tools/lib/ant-tasks.jar' % sdk_root_path
  if not os.path.exists(ant_tasks_jar_path):
    ant_tasks_jar_path = '%s/tools/lib/anttasks.jar' % sdk_root_path

  try:
    aapt_path = Find('aapt', sdk_root_path)
  except Exception:
    print "Your Android SDK may be ruined, please reinstall it."
    sys.exit(2)

  # Check whether ant is installed.
  try:
    proc = subprocess.Popen(['ant', '-version'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  except EnvironmentError:
    print "Please install ant first."
    sys.exit(4)

  manifest_path = '%s/AndroidManifest.xml' % apk_name
  proc = subprocess.Popen(['python', 'scripts/gyp/ant.py',
                           '-DAAPT_PATH=%s' % aapt_path,
                           '-DADDITIONAL_RES_DIRS=',
                           '-DADDITIONAL_RES_PACKAGES=',
                           '-DADDITIONAL_R_TEXT_FILES=',
                           '-DANDROID_MANIFEST=%s' % manifest_path,
                           '-DANDROID_SDK_JAR=%s' % sdk_jar_path,
                           '-DANDROID_SDK_ROOT=%s' % sdk_root_path,
                           '-DANDROID_SDK_VERSION=%d' % api_level,
                           '-DANT_TASKS_JAR=%s' % ant_tasks_jar_path,
                           '-DLIBRARY_MANIFEST_PATHS=',
                           '-DOUT_DIR=out',
                           '-DRESOURCE_DIR=%s/res' % apk_name,
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
    sys.exit(5)

  classpath = ('--classpath=\"libs/xwalk_app_runtime_activity_java.jar\"'
               ' \"libs/xwalk_app_runtime_client_java.jar\" %s' % sdk_jar_path)
  proc = subprocess.Popen(['python', 'scripts/gyp/javac.py',
                           '--output-dir=out/classes',
                           classpath,
                           '--src-dirs=%s/src \"out/gen\"' % apk_name,
                           '--javac-includes=',
                           '--chromium-code=0',
                           '--stamp=compile.stam'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  proc = subprocess.Popen(['python', 'scripts/gyp/ant.py',
                           '-DAAPT_PATH=%s' % aapt_path,
                           '-DADDITIONAL_RES_DIRS=',
                           '-DADDITIONAL_RES_PACKAGES=',
                           '-DADDITIONAL_R_TEXT_FILES=',
                           '-DANDROID_SDK_JAR=%s' % sdk_jar_path,
                           '-DANDROID_SDK_ROOT=%s' % sdk_root_path,
                           '-DANT_TASKS_JAR=%s' % ant_tasks_jar_path,
                           '-DAPK_NAME=%s' % apk_name,
                           '-DAPP_MANIFEST_VERSION_CODE=0',
                           '-DAPP_MANIFEST_VERSION_NAME=Developer Build',
                           '-DASSET_DIR=%s/assets' % apk_name,
                           '-DCONFIGURATION_NAME=Release',
                           '-DOUT_DIR=out',
                           '-DRESOURCE_DIR=%s/res' % apk_name,
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
                           '-DSOURCE_DIR=%s/src' % apk_name,
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
                           '--keystore-path=%s' % key_store,
                           '--keystore-alias=%s' % key_alias,
                           '--keystore-passcode=%s' % key_code],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  src_file = 'out/%s.apk' % apk_name
  dst_file = '%s.apk' % apk_name
  shutil.copyfile(src_file, dst_file)

  if os.path.exists('out/'):
    shutil.rmtree('out/')


def main():
  parser = optparse.OptionParser()
  info = ('The package name. Such as: '
          '--package=com.example.YourPackage')
  parser.add_option('--package', help=info)
  info = ('The apk name. Such as: --name=YourApplicationName')
  parser.add_option('--name', help=info)
  info = ('The path of icon. Such as: --icon=/path/to/your/customized/icon')
  parser.add_option('--icon', help=info)
  info = ('The url of application. '
          'This flag allows to package website as apk. Such as: '
          '--app-url=http://www.intel.com')
  parser.add_option('--app-url', help=info)
  info = ('The root path of the web app. '
          'This flag allows to package local web app as apk. Such as: '
          '--app-root=/root/path/of/the/web/app')
  parser.add_option('--app-root', help=info)
  info = ('The reletive path of entry file based on |app_root|. '
          'This flag should work with "--app-root" together. '
          'Such as: --app-local-path=/reletive/path/of/entry/file')
  parser.add_option('--app-local-path', help=info)
  info = ('The path of the developer keystore, Such as: '
          '--keystore-path=/path/to/your/developer/keystore')
  parser.add_option('--keystore-path', help=info)
  info = ('The alias name of keystore, Such as: --keystore-alias=alias_name')
  parser.add_option('--keystore-alias', help=info)
  info = ('The passcode of keystore, Such as: --keystore-passcode=code')
  parser.add_option('--keystore-passcode', help=info)
  parser.add_option('-f', '--fullscreen', action='store_true',
                    dest='fullscreen', default=False,
                    help='Make application fullscreen.')
  options, _ = parser.parse_args()
  try:
    Customize(options)
    Execution(options)
  except SystemExit, ec:
    print 'Exiting with error code: %d' % ec.code
    return ec.code
  return 0


if __name__ == '__main__':
  sys.exit(main())
