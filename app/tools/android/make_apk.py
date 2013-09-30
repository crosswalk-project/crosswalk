#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import operator
import optparse
import os
import re
import shutil
import subprocess
import sys

sys.path.append('scripts/gyp')
from customize import ReplaceInvalidChars
from dex import AddExeExtensions
from manifest_json_parser import ManifestJsonParser

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
      sdk_version = os.path.basename(os.path.dirname(key))
      str_num = re.search(r'\d+', sdk_version)
      if str_num:
        result[key] = int(str_num.group())
      else:
        result[key] = 0
  if not result:
    raise Exception()
  return max(result.iteritems(), key=operator.itemgetter(1))[0]


def ParseManifest(options):
  parser = ManifestJsonParser(os.path.expanduser(options.manifest))
  if not options.package:
    options.package = 'org.xwalk.' + parser.GetAppName().lower()
  if not options.name:
    options.name = parser.GetAppName()
  if parser.GetAppUrl():
    options.app_url = parser.GetAppUrl()
  if parser.GetAppRoot():
    options.app_root = parser.GetAppRoot()
    temp_dict = parser.GetIcons()
    try:
      icon_dict = dict((int(k), v) for k, v in temp_dict.iteritems())
    except ValueError:
      print 'The key of icon in the manifest file should be a number.'
    # TODO(junmin): add multiple icons support.
    if icon_dict:
      icon_file = max(icon_dict.iteritems(), key=operator.itemgetter(0))[1]
      options.icon = os.path.join(options.app_root, icon_file)
  if parser.GetAppLocalPath():
    options.app_local_path = parser.GetAppLocalPath()
  options.enable_remote_debugging = False
  if parser.GetFullScreenFlag().lower() == 'true':
    options.fullscreen = True
  elif parser.GetFullScreenFlag().lower() == 'false':
    options.fullscreen = False


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
  remote_debugging = ''
  if options.enable_remote_debugging:
    remote_debugging = '--enable-remote-debugging'
  fullscreen_flag = ''
  if options.fullscreen:
    fullscreen_flag = '-f'
  proc = subprocess.Popen(['python', 'customize.py', package,
                           name, icon, app_url, remote_debugging,
                           app_root, app_local_path, fullscreen_flag],
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out


def Execution(options):
  sanitized_name = ReplaceInvalidChars(options.name)
  android_path_array = Which("android")
  if not android_path_array:
    print "Please install Android SDK first."
    sys.exit(1)

  sdk_root_path = os.path.dirname(os.path.dirname(android_path_array[0]))

  try:
    sdk_jar_path = Find('android.jar', os.path.join(sdk_root_path, 'platforms'))
  except Exception:
    print "Your Android SDK may be ruined, please reinstall it."
    sys.exit(2)

  level_string = os.path.basename(os.path.dirname(sdk_jar_path))
  api_level = int(re.search(r'\d+', level_string).group())
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

  if not os.path.exists("out"):
    os.mkdir("out")

  # Make sure to use ant-tasks.jar correctly.
  # Default Android SDK names it as ant-tasks.jar
  # Chrome third party Android SDk names it as anttasks.jar
  ant_tasks_jar_path = os.path.join(sdk_root_path,
                                    'tools', 'lib', 'ant-tasks.jar')
  if not os.path.exists(ant_tasks_jar_path):
    ant_tasks_jar_path = os.path.join(sdk_root_path,
                                      'tools', 'lib' ,'anttasks.jar')

  aapt_path = ''
  for aapt_str in AddExeExtensions('aapt'):
    try:
      aapt_path = Find(aapt_str, sdk_root_path)
      print "Use %s in %s." % (aapt_str, sdk_root_path)
      break
    except Exception:
      print "There doesn't exist %s in %s." % (aapt_str, sdk_root_path)
  if not aapt_path:
    print "Your Android SDK may be ruined, please reinstall it."
    sys.exit(2)

  # Check whether ant is installed.
  try:
    proc = subprocess.Popen(['ant', '-version'],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, shell=True)
  except EnvironmentError:
    print "Please install ant first."
    sys.exit(4)

  resource_dir = '-DRESOURCE_DIR=' + os.path.join(sanitized_name, 'res')
  manifest_path = os.path.join(sanitized_name, 'AndroidManifest.xml')
  proc = subprocess.Popen(['python', os.path.join('scripts', 'gyp', 'ant.py'),
                           '-DAAPT_PATH=%s' % aapt_path,
                           '-DADDITIONAL_RES_DIRS=\'\'',
                           '-DADDITIONAL_RES_PACKAGES=\'\'',
                           '-DADDITIONAL_R_TEXT_FILES=\'\'',
                           '-DANDROID_MANIFEST=%s' % manifest_path,
                           '-DANDROID_SDK_JAR=%s' % sdk_jar_path,
                           '-DANDROID_SDK_ROOT=%s' % sdk_root_path,
                           '-DANDROID_SDK_VERSION=%d' % api_level,
                           '-DANT_TASKS_JAR=%s' % ant_tasks_jar_path,
                           '-DLIBRARY_MANIFEST_PATHS= ',
                           '-DOUT_DIR=out',
                           resource_dir,
                           '-DSTAMP=codegen.stamp',
                           '-Dbasedir=.',
                           '-buildfile',
                           os.path.join('scripts', 'ant', 'apk-codegen.xml')],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  # Check whether java is installed.
  try:
    proc = subprocess.Popen(['java', '-version'],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, shell=True)
  except EnvironmentError:
    print "Please install Oracle JDK first."
    sys.exit(5)

  classpath = '--classpath='
  classpath += os.path.join(os.getcwd(), 'libs',
                            'xwalk_app_runtime_activity_java.jar')
  classpath += ' ' + os.path.join(os.getcwd(),
                                  'libs', 'xwalk_app_runtime_client_java.jar')
  classpath += ' ' + sdk_jar_path
  src_dirs = '--src-dirs=' + os.path.join(os.getcwd(), sanitized_name, 'src') +\
             ' ' + os.path.join(os.getcwd(), 'out', 'gen')
  proc = subprocess.Popen(['python', os.path.join('scripts', 'gyp', 'javac.py'),
                           '--output-dir=%s' % os.path.join('out', 'classes'),
                           classpath,
                           src_dirs,
                           '--javac-includes=',
                           '--chromium-code=0',
                           '--stamp=compile.stam'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  asset_dir = '-DASSET_DIR=%s' % os.path.join(sanitized_name, 'assets')
  xml_path = os.path.join('scripts', 'ant', 'apk-package-resources.xml')
  proc = subprocess.Popen(['python', os.path.join('scripts', 'gyp', 'ant.py'),
                           '-DAAPT_PATH=%s' % aapt_path,
                           '-DADDITIONAL_RES_DIRS=\'\'',
                           '-DADDITIONAL_RES_PACKAGES=\'\'',
                           '-DADDITIONAL_R_TEXT_FILES=\'\'',
                           '-DANDROID_SDK_JAR=%s' % sdk_jar_path,
                           '-DANDROID_SDK_ROOT=%s' % sdk_root_path,
                           '-DANT_TASKS_JAR=%s' % ant_tasks_jar_path,
                           '-DAPK_NAME=%s' % sanitized_name,
                           '-DAPP_MANIFEST_VERSION_CODE=0',
                           '-DAPP_MANIFEST_VERSION_NAME=Developer Build',
                           asset_dir,
                           '-DCONFIGURATION_NAME=Release',
                           '-DOUT_DIR=out',
                           resource_dir,
                           '-DSTAMP=package_resources.stamp',
                           '-Dbasedir=.',
                           '-buildfile',
                           xml_path],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  dex_path = '--dex-path=' + os.path.join(os.getcwd(), 'out', 'classes.dex')
  activity_jar = os.path.join(os.getcwd(),
                              'libs', 'xwalk_app_runtime_activity_java.dex.jar')
  client_jar = os.path.join(os.getcwd(),
                            'libs', 'xwalk_app_runtime_client_java.dex.jar')
  proc = subprocess.Popen(['python', os.path.join('scripts', 'gyp', 'dex.py'),
                           dex_path,
                           '--android-sdk-root=%s' % sdk_root_path,
                           activity_jar,
                           client_jar,
                           os.path.join(os.getcwd(), 'out', 'classes')],
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  src_dir = '-DSOURCE_DIR=' + os.path.join(sanitized_name, 'src')
  apk_path = '-DUNSIGNED_APK_PATH=' + os.path.join('out', 'app-unsigned.apk')
  proc = subprocess.Popen(['python', 'scripts/gyp/ant.py',
                           '-DANDROID_SDK_ROOT=%s' % sdk_root_path,
                           '-DANT_TASKS_JAR=%s' % ant_tasks_jar_path,
                           '-DAPK_NAME=%s' % sanitized_name,
                           '-DCONFIGURATION_NAME=Release',
                           '-DOUT_DIR=out',
                           src_dir,
                           apk_path,
                           '-Dbasedir=.',
                           '-buildfile',
                           'scripts/ant/apk-package.xml'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  apk_path = '--unsigned-apk-path=' + os.path.join('out', 'app-unsigned.apk')
  final_apk_path = '--final-apk-path=' + \
                   os.path.join('out', sanitized_name + '.apk')
  proc = subprocess.Popen(['python', 'scripts/gyp/finalize_apk.py',
                           '--android-sdk-root=%s' % sdk_root_path,
                           apk_path,
                           final_apk_path,
                           '--keystore-path=%s' % key_store,
                           '--keystore-alias=%s' % key_alias,
                           '--keystore-passcode=%s' % key_code],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  out, _ = proc.communicate()
  print out

  src_file = os.path.join('out', sanitized_name + '.apk')
  dst_file = '%s.apk' % options.name
  shutil.copyfile(src_file, dst_file)

  if os.path.exists('out'):
    shutil.rmtree('out')


def main(argv):
  parser = optparse.OptionParser()
  info = ('The manifest file with the detail of the app.'
          'Such as: --manifest=/path/to/your/manifest/file')
  parser.add_option('--manifest', help=info)
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
  parser.add_option('--enable-remote-debugging', action='store_true',
                    dest='enable_remote_debugging', default=False,
                    help = 'Enable remote debugging.')
  parser.add_option('-f', '--fullscreen', action='store_true',
                    dest='fullscreen', default=False,
                    help='Make application fullscreen.')
  options, _ = parser.parse_args()
  if len(argv) == 1:
    parser.print_help()
    return 0

  if not options.manifest:
    if not options.package:
      parser.error('The package name is required! '
                   'Please use "--package" option.')
    if not options.name:
      parser.error('The APK name is required! Pleaes use "--name" option.')
    if not ((options.app_url and not options.app_root
        and not options.app_local_path) or ((not options.app_url)
            and options.app_root and options.app_local_path)):
      parser.error('The entry is required. If the entry is a remote url, '
                   'please use "--app-url" option; If the entry is local, '
                   'please use "--app-root" and '
                   '"--app-local-path" options together!')
  else:
    try:
      ParseManifest(options)
    except KeyError, ec:
      print 'The manifest file contains syntax errors.'
      return ec.code

  options.name = ReplaceInvalidChars(options.name, 'apkname')
  options.package = ReplaceInvalidChars(options.package)

  try:
    Customize(options)
    Execution(options)
  except SystemExit, ec:
    print 'Exiting with error code: %d' % ec.code
    return ec.code
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
