#!/usr/bin/env python

# Copyright (c) 2013, 2014 Intel Corporation. All rights reserved.
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

from customize import VerifyAppName, CustomizeAll, \
                      ParseParameterForCompressor, ReplaceSpaceWithUnderscore
from dex import AddExeExtensions
from handle_permissions import permission_mapping_table
from manifest_json_parser import HandlePermissionList
from manifest_json_parser import ManifestJsonParser


def CleanDir(path):
  if os.path.exists(path):
    shutil.rmtree(path)

def AllArchitectures():
  return ("x86", "arm")

def RunCommand(command, verbose=False, shell=False):
  """Runs the command list, print the output, and propagate its result."""
  proc = subprocess.Popen(command, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, shell=shell)
  if not shell:
    output = proc.communicate()[0]
    result = proc.returncode
    if verbose:
      print(output.decode("utf-8").strip())
    if result != 0:
      print ('Command "%s" exited with non-zero exit code %d'
             % (' '.join(command), result))
      sys.exit(result)


def Which(name):
  """Search PATH for executable files with the given name."""
  result = []
  exts = [_f for _f in os.environ.get('PATHEXT', '').split(os.pathsep) if _f]
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
  return max(iter(result.items()), key=operator.itemgetter(1))[0]


def GetVersion(path):
  """Get the version of this python tool."""
  version_str = 'Crosswalk app packaging tool version is '
  file_handle = open(path, 'r')
  src_content = file_handle.read()
  version_nums = re.findall(r'\d+', src_content)
  version_str += ('.').join(version_nums)
  file_handle.close()
  return version_str


def ParseManifest(options):
  parser = ManifestJsonParser(os.path.expanduser(options.manifest))
  app_name = parser.GetAppName()
  if options.package:
    VerifyAppName(options.package, 'packagename')
  else:
    VerifyAppName(app_name)
    app_name = ReplaceSpaceWithUnderscore(app_name)
    options.package = 'org.xwalk.' + app_name.lower()
  if options.name:
    VerifyAppName(options.name)
    options.name = ReplaceSpaceWithUnderscore(options.name)
  else:
    VerifyAppName(app_name)
    options.name = ReplaceSpaceWithUnderscore(app_name)
  if not options.app_version:
    options.app_version = parser.GetVersion()
  if not options.app_versionCode and not options.app_versionCodeBase:
    options.app_versionCode = 1
  if parser.GetDescription():
    options.description = parser.GetDescription()
  if parser.GetPermissions():
    options.permissions = parser.GetPermissions()
  if parser.GetAppUrl():
    options.app_url = parser.GetAppUrl()
  elif parser.GetAppLocalPath():
    options.app_local_path = parser.GetAppLocalPath()
  else:
    print('Error: there is no app launch path defined in manifest.json.')
    sys.exit(9)
  if parser.GetAppRoot():
    options.app_root = parser.GetAppRoot()
    options.icon_dict = parser.GetIcons()
  if parser.GetFullScreenFlag().lower() == 'true':
    options.fullscreen = True
  elif parser.GetFullScreenFlag().lower() == 'false':
    options.fullscreen = False


def ParseXPK(options, out_dir):
  cmd = ['python', 'parse_xpk.py',
         '--file=%s' % os.path.expanduser(options.xpk),
         '--out=%s' % out_dir]
  RunCommand(cmd)
  if options.manifest:
    print ('Use the manifest from XPK by default '
           'when "--xpk" option is specified, and '
           'the "--manifest" option would be ignored.')
    sys.exit(7)

  if os.path.isfile(os.path.join(out_dir, 'manifest.json')):
    options.manifest = os.path.join(out_dir, 'manifest.json')
  else:
    print('XPK doesn\'t contain manifest file.')
    sys.exit(8)


def FindExtensionJars(root_path):
  ''' Find all .jar files for external extensions. '''
  extension_jars = []
  if not os.path.exists(root_path):
    return extension_jars

  for afile in os.listdir(root_path):
    if os.path.isdir(os.path.join(root_path, afile)):
      base_name = os.path.basename(afile)
      extension_jar = os.path.join(root_path, afile, base_name + '.jar')
      if os.path.isfile(extension_jar):
        extension_jars.append(extension_jar)
  return extension_jars


# Follows the recommendation from
# http://software.intel.com/en-us/blogs/2012/11/12/how-to-publish-
# your-apps-on-google-play-for-x86-based-android-devices-using
def MakeVersionCode(options):
  ''' Construct a version code'''
  if options.app_versionCode:
    return options.app_versionCode

  # First digit is ABI, ARM=2, x86=6
  abi = '0'
  if options.arch == 'arm':
    abi = '2'
  if options.arch == 'x86':
    abi = '6'
  b = '0'
  if options.app_versionCodeBase:
    b = str(options.app_versionCodeBase)
    if len(b) > 7:
      print('Version code base must be 7 digits or less: '
            'versionCodeBase=%s' % (b))
      sys.exit(12)
  # zero pad to 7 digits, middle digits can be used for other
  # features, according to recommendation in URL
  return '%s%s' % (abi, b.zfill(7))


def Customize(options):
  package = 'org.xwalk.app.template'
  if options.package:
    package = options.package
  name = 'AppTemplate'
  if options.name:
    name = options.name
  app_version = ''
  if options.app_version:
    app_version = options.app_version
  app_versionCode = MakeVersionCode(options)
  app_root = ''
  if options.app_root:
    app_root = os.path.expanduser(options.app_root)
  remote_debugging = ''
  if options.enable_remote_debugging:
    remote_debugging = '--enable-remote-debugging'
  fullscreen_flag = ''
  if options.fullscreen:
    fullscreen_flag = '-f'
  orientation = 'unspecified'
  if options.orientation:
    orientation = options.orientation
  icon = ''
  if options.icon:
    icon = '%s' % os.path.expanduser(options.icon)
  CustomizeAll(app_versionCode, options.description, options.icon_dict,
               options.permissions, options.app_url, app_root,
               options.app_local_path, remote_debugging,
               fullscreen_flag, options.keep_screen_on, options.extensions,
               options.manifest, icon, package, name, app_version,
               orientation, options.xwalk_command_line, options.compressor)


def Execution(options, name):
  android_path_array = Which('android')
  if not android_path_array:
    print('Please install Android SDK first.')
    sys.exit(1)

  sdk_root_path = os.path.dirname(os.path.dirname(android_path_array[0]))

  try:
    sdk_jar_path = Find('android.jar',
                        os.path.join(sdk_root_path, 'platforms'))
  except Exception:
    print('Your Android SDK may be ruined, please reinstall it.')
    sys.exit(2)

  level_string = os.path.basename(os.path.dirname(sdk_jar_path))
  api_level = int(re.search(r'\d+', level_string).group())
  if api_level < 14:
    print('Please install Android API level (>=14) first.')
    sys.exit(3)

  if options.keystore_path:
    key_store = os.path.expanduser(options.keystore_path)
    if options.keystore_alias:
      key_alias = options.keystore_alias
    else:
      print('Please provide an alias name of the developer key.')
      sys.exit(6)
    if options.keystore_passcode:
      key_code = options.keystore_passcode
    else:
      print('Please provide the passcode of the developer key.')
      sys.exit(6)
  else:
    print ('Use xwalk\'s keystore by default for debugging. '
           'Please switch to your keystore when distributing it to app market.')
    key_store = 'scripts/ant/xwalk-debug.keystore'
    key_alias = 'xwalkdebugkey'
    key_code = 'xwalkdebug'

  if not os.path.exists('out'):
    os.mkdir('out')

  # Make sure to use ant-tasks.jar correctly.
  # Default Android SDK names it as ant-tasks.jar
  # Chrome third party Android SDk names it as anttasks.jar
  ant_tasks_jar_path = os.path.join(sdk_root_path,
                                    'tools', 'lib', 'ant-tasks.jar')
  if not os.path.exists(ant_tasks_jar_path):
    ant_tasks_jar_path = os.path.join(sdk_root_path,
                                      'tools', 'lib', 'anttasks.jar')

  aapt_path = ''
  for aapt_str in AddExeExtensions('aapt'):
    try:
      aapt_path = Find(aapt_str, sdk_root_path)
      print('Use %s in %s.' % (aapt_str, sdk_root_path))
      break
    except Exception:
      print('There doesn\'t exist %s in %s.' % (aapt_str, sdk_root_path))
  if not aapt_path:
    print('Your Android SDK may be ruined, please reinstall it.')
    sys.exit(2)

  # Check whether ant is installed.
  try:
    cmd = ['ant', '-version']
    RunCommand(cmd, shell=True)
  except EnvironmentError:
    print('Please install ant first.')
    sys.exit(4)

  res_dirs = '-DADDITIONAL_RES_DIRS=\'\''
  res_packages = '-DADDITIONAL_RES_PACKAGES=\'\''
  res_r_text_files = '-DADDITIONAL_R_TEXT_FILES=\'\''
  if options.mode == 'embedded':
    # Prepare the .pak file for embedded mode.
    pak_src_path = os.path.join('native_libs_res', 'xwalk.pak')
    pak_des_path = os.path.join(name, 'assets', 'xwalk.pak')
    shutil.copy(pak_src_path, pak_des_path)

    # Prepare the icudtl.dat for embedded mode.
    icudtl_src_path = os.path.join('native_libs_res', 'icudtl.dat')
    icudtl_des_path = os.path.join(name, 'assets', 'icudtl.dat')
    shutil.copy(icudtl_src_path, icudtl_des_path)

    js_src_dir = os.path.join('native_libs_res', 'jsapi')
    js_des_dir = os.path.join(name, 'assets', 'jsapi')
    if os.path.exists(js_des_dir):
      shutil.rmtree(js_des_dir)
    shutil.copytree(js_src_dir, js_des_dir)

    res_ui_java = os.path.join('gen', 'ui_java')
    res_content_java = os.path.join('gen', 'content_java')
    res_xwalk_java = os.path.join('gen', 'xwalk_core_java')
    res_dirs = ('-DADDITIONAL_RES_DIRS='
                + os.path.join(res_ui_java, 'res_crunched') + ' '
                + os.path.join(res_ui_java, 'res_v14_compatibility') + ' '
                + os.path.join(res_ui_java, 'res_grit') + ' '
                + os.path.join('libs_res', 'ui') + ' '
                + os.path.join(res_content_java, 'res_crunched') + ' '
                + os.path.join(res_content_java, 'res_v14_compatibility') + ' '
                + os.path.join('libs_res', 'content') + ' '
                + os.path.join(res_content_java, 'res_grit') + ' '
                + os.path.join(res_xwalk_java, 'res_crunched') + ' '
                + os.path.join(res_xwalk_java, 'res_v14_compatibility') + ' '
                + os.path.join('libs_res', 'runtime') + ' '
                + os.path.join(res_xwalk_java, 'res_grit'))
    res_packages = ('-DADDITIONAL_RES_PACKAGES=org.chromium.ui '
                    'org.xwalk.core org.chromium.content')
    res_r_text_files = ('-DADDITIONAL_R_TEXT_FILES='
                        + os.path.join(res_ui_java, 'java_R', 'R.txt') + ' '
                        + os.path.join(res_xwalk_java, 'java_R', 'R.txt') + ' '
                        + os.path.join(res_content_java, 'java_R', 'R.txt'))

  resource_dir = '-DRESOURCE_DIR=' + os.path.join(name, 'res')
  manifest_path = os.path.join(name, 'AndroidManifest.xml')
  cmd = ['python', os.path.join('scripts', 'gyp', 'ant.py'),
         '-DAAPT_PATH=%s' % aapt_path,
         res_dirs,
         res_packages,
         res_r_text_files,
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
         os.path.join('scripts', 'ant', 'apk-codegen.xml')]
  RunCommand(cmd, options.verbose)

  # Check whether java is installed.
  try:
    cmd = ['java', '-version']
    RunCommand(cmd, shell=True)
  except EnvironmentError:
    print('Please install Oracle JDK first.')
    sys.exit(5)

  # Compile App source code with app runtime code.
  classpath = '--classpath='
  classpath += os.path.join(os.getcwd(), 'libs',
                            'xwalk_app_runtime_java.jar')
  classpath += ' ' + sdk_jar_path
  src_dirs = '--src-dirs=' + os.path.join(os.getcwd(), name, 'src') +\
             ' ' + os.path.join(os.getcwd(), 'out', 'gen')
  cmd = ['python', os.path.join('scripts', 'gyp', 'javac.py'),
         '--output-dir=%s' % os.path.join('out', 'classes'),
         classpath,
         src_dirs,
         '--javac-includes=',
         '--chromium-code=0',
         '--stamp=compile.stam']
  RunCommand(cmd, options.verbose)

  # Package resources.
  asset_dir = '-DASSET_DIR=%s' % os.path.join(name, 'assets')
  xml_path = os.path.join('scripts', 'ant', 'apk-package-resources.xml')
  cmd = ['python', os.path.join('scripts', 'gyp', 'ant.py'),
         '-DAAPT_PATH=%s' % aapt_path,
         res_dirs,
         res_packages,
         res_r_text_files,
         '-DANDROID_SDK_JAR=%s' % sdk_jar_path,
         '-DANDROID_SDK_ROOT=%s' % sdk_root_path,
         '-DANT_TASKS_JAR=%s' % ant_tasks_jar_path,
         '-DAPK_NAME=%s' % name,
         '-DAPP_MANIFEST_VERSION_CODE=0',
         '-DAPP_MANIFEST_VERSION_NAME=Developer Build',
         asset_dir,
         '-DCONFIGURATION_NAME=Release',
         '-DOUT_DIR=out',
         resource_dir,
         '-DSTAMP=package_resources.stamp',
         '-Dbasedir=.',
         '-buildfile',
         xml_path]
  RunCommand(cmd, options.verbose)

  dex_path = '--dex-path=' + os.path.join(os.getcwd(), 'out', 'classes.dex')
  app_runtime_jar = os.path.join(os.getcwd(),
                                 'libs', 'xwalk_app_runtime_java.jar')

  # Check whether external extensions are included.
  extensions_string = 'xwalk-extensions'
  extensions_dir = os.path.join(os.getcwd(), name, extensions_string)
  external_extension_jars = FindExtensionJars(extensions_dir)
  input_jars = []
  if options.mode == 'embedded':
    input_jars.append(os.path.join(os.getcwd(), 'libs',
                                   'xwalk_runtime_embedded.dex.jar'))
  dex_command_list = ['python', os.path.join('scripts', 'gyp', 'dex.py'),
                      dex_path,
                      '--android-sdk-root=%s' % sdk_root_path,
                      app_runtime_jar,
                      os.path.join(os.getcwd(), 'out', 'classes')]
  dex_command_list.extend(external_extension_jars)
  dex_command_list.extend(input_jars)
  RunCommand(dex_command_list)

  src_dir = '-DSOURCE_DIR=' + os.path.join(name, 'src')
  apk_path = '-DUNSIGNED_APK_PATH=' + os.path.join('out', 'app-unsigned.apk')
  native_lib_path = '-DNATIVE_LIBS_DIR='
  if options.mode == 'embedded':
    if options.arch == 'x86':
      x86_native_lib_path = os.path.join('native_libs', 'x86', 'libs',
                                         'x86', 'libxwalkcore.so')
      if os.path.isfile(x86_native_lib_path):
        native_lib_path += os.path.join('native_libs', 'x86', 'libs')
      else:
        print('Missing x86 native library for Crosswalk embedded APK. Abort!')
        sys.exit(10)
    elif options.arch == 'arm':
      arm_native_lib_path = os.path.join('native_libs', 'armeabi-v7a', 'libs',
                                         'armeabi-v7a', 'libxwalkcore.so')
      if os.path.isfile(arm_native_lib_path):
        native_lib_path += os.path.join('native_libs', 'armeabi-v7a', 'libs')
      else:
        print('Missing ARM native library for Crosswalk embedded APK. Abort!')
        sys.exit(10)
  # A space is needed for Windows.
  native_lib_path += ' '
  cmd = ['python', 'scripts/gyp/ant.py',
         '-DANDROID_SDK_ROOT=%s' % sdk_root_path,
         '-DANT_TASKS_JAR=%s' % ant_tasks_jar_path,
         '-DAPK_NAME=%s' % name,
         '-DCONFIGURATION_NAME=Release',
         native_lib_path,
         '-DOUT_DIR=out',
         src_dir,
         apk_path,
         '-Dbasedir=.',
         '-buildfile',
         'scripts/ant/apk-package.xml']
  RunCommand(cmd, options.verbose)

  apk_path = '--unsigned-apk-path=' + os.path.join('out', 'app-unsigned.apk')
  final_apk_path = '--final-apk-path=' + \
                   os.path.join('out', name + '.apk')
  cmd = ['python', 'scripts/gyp/finalize_apk.py',
         '--android-sdk-root=%s' % sdk_root_path,
         apk_path,
         final_apk_path,
         '--keystore-path=%s' % key_store,
         '--keystore-alias=%s' % key_alias,
         '--keystore-passcode=%s' % key_code]
  RunCommand(cmd)

  src_file = os.path.join('out', name + '.apk')
  package_name = options.name
  if options.app_version:
    package_name += ('_' + options.app_version)
  if options.mode == 'shared':
    dst_file = os.path.join(options.target_dir, '%s.apk' % package_name)
  elif options.mode == 'embedded':
    dst_file = os.path.join(options.target_dir,
                            '%s_%s.apk' % (package_name, options.arch))
  shutil.copyfile(src_file, dst_file)
  CleanDir('out')
  if options.mode == 'embedded':
    os.remove(pak_des_path)


def PrintPackageInfo(options, packaged_archs):
  package_name_version = os.path.join(options.target_dir, options.name)
  if options.app_version:
    package_name_version += '_' + options.app_version

  if len(packaged_archs) == 0:
    print ('A non-platform specific APK for the web application "%s" was '
           'generated successfully at\n%s.apk. It requires a shared Crosswalk '
           'Runtime to be present.'
           % (options.name, package_name_version))
    return

  for arch in packaged_archs:
    print ('An APK for the web application "%s" including the Crosswalk '
           'Runtime built for %s was generated successfully, which can be '
           'found at\n%s_%s.apk.'
           % (options.name, arch, package_name_version, arch))

  all_archs = set(AllArchitectures())

  if len(packaged_archs) != len(all_archs):
    missed_archs = all_archs - set(packaged_archs)
    print ('\n\nWARNING: ')
    print ('This APK will only work on %s based Android devices. Consider '
           'building for %s as well.' %
           (', '.join(packaged_archs), ', '.join(missed_archs)))
  else:
    print ('\n\n%d APKs were created for %s devices. '
           % (len(all_archs), ', '.join(all_archs)))
    print ('Please install the one that matches the processor architecture '
           'of your device.\n\n')
    print ('If you are going to submit this application to an application '
           'store, please make sure you submit both packages.\nInstructions '
           'for submitting multiple APKs to Google Play Store are available '
           'here:\nhttps://software.intel.com/en-us/html5/articles/submitting'
           '-multiple-crosswalk-apk-to-google-play-store')

def MakeApk(options):
  Customize(options)
  name = options.name
  packaged_archs = []
  if options.mode == 'shared':
    Execution(options, name)
  elif options.mode == 'embedded':
    if options.arch:
      Execution(options, name)
      packaged_archs.append(options.arch)
    else:
      # If the arch option is unspecified, all of available platform APKs
      # will be generated.
      valid_archs = ['x86', 'armeabi-v7a']
      for arch in valid_archs:
        lib_path = os.path.join('native_libs', arch, 'libs',
                                arch, 'libxwalkcore.so')
        if os.path.isfile(lib_path):
          if arch.find('x86') != -1:
            options.arch = 'x86'
          elif arch.find('arm') != -1:
            options.arch = 'arm'
          Execution(options, name)
          packaged_archs.append(options.arch)
        else:
          print('Warning: failed to create package for arch "%s" '
                'due to missing library %s' %
                (arch, lib_path))

      if len(packaged_archs) == 0:
        print('No packages created, aborting')
        sys.exit(13)
  else:
    print('Unknown mode for packaging the application. Abort!')
    sys.exit(11)

  PrintPackageInfo(options, packaged_archs)

def main(argv):
  parser = optparse.OptionParser()
  parser.add_option('-v', '--version', action='store_true',
                    dest='version', default=False,
                    help='The version of this python tool.')
  parser.add_option('--verbose', action="store_true",
                    dest='verbose', default=False,
                    help='Print debug messages.')
  info = ('The packaging mode of the web application. The value \'shared\' '
          'means that the runtime is shared across multiple application '
          'instances and that the runtime needs to be distributed separately. '
          'The value \'embedded\' means that the runtime is embedded into the '
          'application itself and distributed along with it.'
          'Set the default mode as \'embedded\'. For example: --mode=embedded')
  parser.add_option('--mode', default='embedded', help=info)
  info = ('The target architecture of the embedded runtime. Supported values '
          'are \'x86\' and \'arm\'. Note, if undefined, APKs for all possible '
          'architestures will be generated.')
  parser.add_option('--arch', choices=AllArchitectures(), help=info)
  group = optparse.OptionGroup(parser, 'Application Source Options',
      'This packaging tool supports 3 kinds of web application source: '
      '1) XPK package; 2) manifest.json; 3) various command line options, '
      'for example, \'--app-url\' for website, \'--app-root\' and '
      '\'--app-local-path\' for local web application.')
  info = ('The path of the XPK package. For example, --xpk=/path/to/xpk/file')
  group.add_option('--xpk', help=info)
  info = ('The manifest file with the detail description of the application. '
          'For example, --manifest=/path/to/your/manifest/file')
  group.add_option('--manifest', help=info)
  info = ('The url of application. '
          'This flag allows to package website as apk. For example, '
          '--app-url=http://www.intel.com')
  group.add_option('--app-url', help=info)
  info = ('The root path of the web app. '
          'This flag allows to package local web app as apk. For example, '
          '--app-root=/root/path/of/the/web/app')
  group.add_option('--app-root', help=info)
  info = ('The relative path of entry file based on the value from '
          '\'app_root\'. This flag should work with \'--app-root\' together. '
          'For example, --app-local-path=/relative/path/of/entry/file')
  group.add_option('--app-local-path', help=info)
  parser.add_option_group(group)
  group = optparse.OptionGroup(parser, 'Mandatory arguments',
      'They are used for describing the APK information through '
      'command line options.')
  info = ('The apk name. For example, --name="Your Application Name"')
  group.add_option('--name', help=info)
  info = ('The package name. For example, '
          '--package=com.example.YourPackage')
  group.add_option('--package', help=info)
  parser.add_option_group(group)
  group = optparse.OptionGroup(parser, 'Optional arguments',
      'They are used for various settings for applications through '
      'command line options.')
  info = ('The version name of the application. '
          'For example, --app-version=1.0.0')
  group.add_option('--app-version', help=info)
  info = ('The version code of the application. '
          'For example, --app-versionCode=24')
  group.add_option('--app-versionCode', type='int', help=info)
  info = ('The version code base of the application. Version code will '
          'be made by adding a prefix based on architecture to the version '
          'code base. For example, --app-versionCodeBase=24')
  group.add_option('--app-versionCodeBase', type='int', help=info)
  info = ('Use command lines.'
          'Crosswalk is powered by Chromium and supports Chromium command line.'
          'For example, '
          '--xwalk-command-line=\'--chromium-command-1 --xwalk-command-2\'')
  group.add_option('--xwalk-command-line', default='', help=info)
  info = ('The description of the application. For example, '
          '--description=YourApplicationDescription')
  group.add_option('--description', help=info)
  group.add_option('--enable-remote-debugging', action='store_true',
                   dest='enable_remote_debugging', default=False,
                   help='Enable remote debugging.')
  info = ('The list of external extension paths splitted by OS separators. '
          'The separators are \':\' , \';\' and \':\' on Linux, Windows and '
          'Mac OS respectively. For example, '
          '--extensions=/path/to/extension1:/path/to/extension2.')
  group.add_option('--extensions', help=info)
  group.add_option('-f', '--fullscreen', action='store_true',
                   dest='fullscreen', default=False,
                   help='Make application fullscreen.')
  group.add_option('--keep-screen-on', action='store_true', default=False,
                   help='Support keeping screen on')
  info = ('The path of application icon. '
          'Such as: --icon=/path/to/your/customized/icon')
  group.add_option('--icon', help=info)
  info = ('The orientation of the web app\'s display on the device. '
          'For example, --orientation=landscape. The default value is '
          '\'unspecified\'. The permitted values are from Android: '
          'http://developer.android.com/guide/topics/manifest/'
          'activity-element.html#screen')
  group.add_option('--orientation', help=info)
  info = ('The list of permissions to be used by web application. For example, '
          '--permissions=geolocation:webgl')
  group.add_option('--permissions', help=info)
  info = ('Packaging tool will move the output APKS to the target directory')
  group.add_option('--target-dir', default=os.getcwd(), help=info)
  parser.add_option_group(group)
  group = optparse.OptionGroup(parser, 'Keystore Options',
      'The keystore is a signature from web developer, it\'s used when '
      'developer wants to distribute the applications.')
  info = ('The path to the developer keystore. For example, '
          '--keystore-path=/path/to/your/developer/keystore')
  group.add_option('--keystore-path', help=info)
  info = ('The alias name of keystore. For example, --keystore-alias=name')
  group.add_option('--keystore-alias', help=info)
  info = ('The passcode of keystore. For example, --keystore-passcode=code')
  group.add_option('--keystore-passcode', help=info)
  info = ('Minify and obfuscate javascript and css.'
          '--compressor: compress javascript and css.'
          '--compressor=js: compress javascript.'
          '--compressor=css: compress css.')
  group.add_option('--compressor', dest='compressor', action='callback',
                   callback=ParseParameterForCompressor, type='string',
                   nargs=0, help=info)
  parser.add_option_group(group)
  options, _ = parser.parse_args()
  if len(argv) == 1:
    parser.print_help()
    return 0

  if options.version:
    if os.path.isfile('VERSION'):
      print(GetVersion('VERSION'))
      return 0
    else:
      parser.error('Can\'t get version due to the VERSION file missing!')

  xpk_temp_dir = ''
  if options.xpk:
    xpk_name = os.path.splitext(os.path.basename(options.xpk))[0]
    xpk_temp_dir = xpk_name + '_xpk'
    ParseXPK(options, xpk_temp_dir)

  if options.app_root and not options.manifest:
    manifest_path = os.path.join(options.app_root, 'manifest.json')
    if os.path.exists(manifest_path):
      print('Using manifest.json distributed with the application.')
      options.manifest = manifest_path

  if not options.manifest:
    if options.package:
      VerifyAppName(options.package, 'packagename')
    else:
      parser.error('The package name is required! '
                   'Please use "--package" option.')
    if options.name:
      VerifyAppName(options.name)
      options.name = ReplaceSpaceWithUnderscore(options.name)
    else:
      parser.error('The APK name is required! Please use "--name" option.')
    if not ((options.app_url and
             not options.app_root and
             not options.app_local_path) or
            (not options.app_url and
             options.app_root and
             options.app_local_path)):
      parser.error('The entry is required. If the entry is a remote url, '
                   'please use "--app-url" option; If the entry is local, '
                   'please use "--app-root" and '
                   '"--app-local-path" options together!')
    if options.permissions:
      permission_list = options.permissions.split(':')
    else:
      print('Warning: all supported permissions on Android port are added. '
            'Refer to https://github.com/crosswalk-project/'
            'crosswalk-website/wiki/Crosswalk-manifest')
      permission_list = permission_mapping_table.keys()
    options.permissions = HandlePermissionList(permission_list)
    options.icon_dict = {}
  else:
    try:
      ParseManifest(options)
    except SystemExit as ec:
      return ec.code

  if (options.app_root and options.app_local_path and
      not os.path.isfile(os.path.join(options.app_root,
                                      options.app_local_path))):
    print('Please make sure that the local path file of launching app '
          'does exist.')
    sys.exit(7)

  if os.path.exists(options.name):
    print ('Folder %s existing in current path, '
           'please move it' % options.name)
    return 1

  if options.target_dir:
    target_dir = os.path.abspath(os.path.expanduser(options.target_dir))
    options.target_dir = target_dir
    if not os.path.isdir(target_dir):
      os.makedirs(target_dir)

  try:
    MakeApk(options)
  except SystemExit as ec:
    CleanDir(options.name)
    CleanDir('out')
    CleanDir(xpk_temp_dir)
    return ec.code
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
