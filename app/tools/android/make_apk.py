#!/usr/bin/env python

# Copyright (c) 2013, 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import optparse
import os
import re
import shutil
import subprocess
import sys

# get xwalk absolute path so we can run this script from any location
xwalk_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(xwalk_dir)

from app_info import AppInfo
from customize import VerifyPackageName, CustomizeAll, \
                      ParseParameterForCompressor
from handle_permissions import permission_mapping_table
from manifest_json_parser import HandlePermissionList
from manifest_json_parser import ManifestJsonParser


NATIVE_LIBRARY = 'libxwalkcore.so'


def CleanDir(path):
  if os.path.exists(path):
    shutil.rmtree(path)


def AllArchitectures():
  return ("x86", "arm")


def ConvertArchNameToArchFolder(arch):
  arch_dict = {
      'x86': 'x86',
      'arm': 'armeabi-v7a'
  }
  return arch_dict.get(arch, None)


def AddExeExtensions(name):
  exts_str = os.environ.get('PATHEXT', '').lower()
  exts = [_f for _f in exts_str.split(os.pathsep) if _f]
  result = []
  result.append(name)
  for e in exts:
    result.append(name + e)
  return result


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
    return output.decode("utf-8")


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


def GetAndroidApiLevel():
  """Get Highest Android target level installed.
     return -1 if no targets have been found.
  """
  target_output = RunCommand(['android', 'list', 'target', '-c'])
  target_regex = re.compile(r'android-(\d+)')
  targets = [int(i) for i in target_regex.findall(target_output)]
  targets.extend([-1])
  return max(targets)


def GetVersion(path):
  """Get the version of this python tool."""
  version_str = 'Crosswalk app packaging tool version is '
  file_handle = open(path, 'r')
  src_content = file_handle.read()
  version_nums = re.findall(r'\d+', src_content)
  version_str += ('.').join(version_nums)
  file_handle.close()
  return version_str


def ContainsNativeLibrary(path):
  return os.path.isfile(os.path.join(path, NATIVE_LIBRARY))


def ParseManifest(options):
  parser = ManifestJsonParser(os.path.expanduser(options.manifest))
  if not options.name:
    options.name = parser.GetAppName()
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
  if parser.GetOrientation():
    options.orientation = parser.GetOrientation()
  if parser.GetFullScreenFlag().lower() == 'true':
    options.fullscreen = True
  elif parser.GetFullScreenFlag().lower() == 'false':
    options.fullscreen = False
  return parser


def ParseXPK(options, out_dir):
  cmd = ['python', os.path.join (xwalk_dir, 'parse_xpk.py'),
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


def Customize(options, app_info, manifest):
  app_info.package = options.package
  app_info.app_name = options.name
  # 'org.xwalk.my_first_app' => 'MyFirstApp'
  android_name = options.package.split('.')[-1].split('_')
  app_info.android_name = ''.join([i.capitalize() for i in android_name if i])
  if options.app_version:
    app_info.app_version = options.app_version
  app_info.app_versionCode = MakeVersionCode(options)
  if options.app_root:
    app_info.app_root = os.path.expanduser(options.app_root)
  if options.enable_remote_debugging:
    app_info.remote_debugging = '--enable-remote-debugging'
  if options.fullscreen:
    app_info.fullscreen_flag = '-f'
  if options.orientation:
    app_info.orientation = options.orientation
  if options.icon:
    app_info.icon = '%s' % os.path.expanduser(options.icon)
  CustomizeAll(app_info, options.description, options.icon_dict,
               options.permissions, options.app_url, options.app_local_path,
               options.keep_screen_on, options.extensions, manifest,
               options.xwalk_command_line, options.compressor)


def Execution(options, name):
  android_path = Which('android')
  if android_path is None:
    print('The "android" binary could not be found. Check your Android SDK '
          'installation and your PATH environment variable.')
    sys.exit(1)

  api_level = GetAndroidApiLevel()
  if api_level < 14:
    print('Please install Android API level (>=14) first.')
    sys.exit(3)
  target_string = 'android-%d' % api_level

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
      key_code = None
    if options.keystore_alias_passcode:
      key_alias_code = options.keystore_alias_passcode
    else:
      key_alias_code = None
  else:
    print ('Use xwalk\'s keystore by default for debugging. '
           'Please switch to your keystore when distributing it to app market.')
    key_store = os.path.join(xwalk_dir, 'xwalk-debug.keystore')
    key_alias = 'xwalkdebugkey'
    key_code = 'xwalkdebug'
    key_alias_code = 'xwalkdebug'

  # Check whether ant is installed.
  try:
    cmd = ['ant', '-version']
    RunCommand(cmd, shell=True)
  except EnvironmentError:
    print('Please install ant first.')
    sys.exit(4)

  # Update android project for app and xwalk_core_library.
  update_project_cmd = ['android', 'update', 'project',
                        '--path', os.path.join (xwalk_dir, name),
                        '--target', target_string,
                        '--name', name]
  if options.mode == 'embedded':
    RunCommand(['android', 'update', 'lib-project',
                '--path', os.path.join(xwalk_dir, name, 'xwalk_core_library'),
                '--target', target_string])
    update_project_cmd.extend(['-l', 'xwalk_core_library'])
  else:
    # Shared mode doesn't need xwalk_runtime_java.jar.
    os.remove(os.path.join(xwalk_dir, name, 'libs', 'xwalk_runtime_java.jar'))

  RunCommand(update_project_cmd)

  # Check whether external extensions are included.
  extensions_string = 'xwalk-extensions'
  extensions_dir = os.path.join(xwalk_dir, name, extensions_string)
  external_extension_jars = FindExtensionJars(extensions_dir)
  for external_extension_jar in external_extension_jars:
    shutil.copyfile(external_extension_jar,
                    os.path.join(xwalk_dir, name, 'libs',
                                 os.path.basename(external_extension_jar)))

  if options.mode == 'embedded':
    # Remove existing native libraries in xwalk_core_library, they are probably
    # for the last execution to make apk for another CPU arch.
    # And then copy the native libraries for the specified arch into
    # xwalk_core_library.
    arch = ConvertArchNameToArchFolder(options.arch)
    if not arch:
      print ('Invalid CPU arch: %s.' % arch)
      sys.exit(10)
    library_lib_path = os.path.join(xwalk_dir, name, 'xwalk_core_library',
                                    'libs')
    for dir_name in os.listdir(library_lib_path):
      lib_dir = os.path.join(library_lib_path, dir_name)
      if ContainsNativeLibrary(lib_dir):
        shutil.rmtree(lib_dir)
    native_lib_path = os.path.join(xwalk_dir, name, 'native_libs', arch)
    if ContainsNativeLibrary(native_lib_path):
      shutil.copytree(native_lib_path, os.path.join(library_lib_path, arch))
    else:
      print('No %s native library has been found for creating a Crosswalk '
            'embedded APK.' % arch)
      sys.exit(10)

  ant_cmd = ['ant', 'release', '-f', os.path.join(xwalk_dir, name, 'build.xml')]
  if not options.verbose:
    ant_cmd.extend(['-quiet'])
  ant_cmd.extend(['-Dkey.store="%s"' % os.path.abspath(key_store)])
  ant_cmd.extend(['-Dkey.alias="%s"' % key_alias])
  if key_code:
    ant_cmd.extend(['-Dkey.store.password="%s"' % key_code])
  if key_alias_code:
    ant_cmd.extend(['-Dkey.alias.password="%s"' % key_alias_code])
  ant_result = subprocess.call(ant_cmd)
  if ant_result != 0:
    print('Command "%s" exited with non-zero exit code %d'
          % (' '.join(ant_cmd), ant_result))
    sys.exit(ant_result)

  src_file = os.path.join(xwalk_dir, name, 'bin', '%s-release.apk' % name)
  package_name = name
  if options.app_version:
    package_name += ('_' + options.app_version)
  if options.mode == 'shared':
    dst_file = os.path.join(options.target_dir, '%s.apk' % package_name)
  elif options.mode == 'embedded':
    dst_file = os.path.join(options.target_dir,
                            '%s_%s.apk' % (package_name, options.arch))
  shutil.copyfile(src_file, dst_file)


def PrintPackageInfo(options, name, packaged_archs):
  package_name_version = os.path.join(options.target_dir, name)
  if options.app_version:
    package_name_version += '_' + options.app_version

  if len(packaged_archs) == 0:
    print ('A non-platform specific APK for the web application "%s" was '
           'generated successfully at\n%s.apk. It requires a shared Crosswalk '
           'Runtime to be present.'
           % (name, package_name_version))
    return

  for arch in packaged_archs:
    print ('An APK for the web application "%s" including the Crosswalk '
           'Runtime built for %s was generated successfully, which can be '
           'found at\n%s_%s.apk.'
           % (name, arch, package_name_version, arch))

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

def MakeApk(options, app_info, manifest):
  Customize(options, app_info, manifest)
  name = app_info.android_name
  packaged_archs = []
  if options.mode == 'shared':
    Execution(options, name)
  elif options.mode == 'embedded':
    # Copy xwalk_core_library into app folder and move the native libraries
    # out.
    # When making apk for specified CPU arch, will only include the
    # corresponding native library by copying it back into xwalk_core_library.
    target_library_path = os.path.join(xwalk_dir, name, 'xwalk_core_library')
    shutil.copytree(os.path.join(xwalk_dir, 'xwalk_core_library'),
                    target_library_path)
    library_lib_path = os.path.join(target_library_path, 'libs')
    native_lib_path = os.path.join(xwalk_dir, name, 'native_libs')
    os.makedirs(native_lib_path)
    available_archs = []
    for dir_name in os.listdir(library_lib_path):
      lib_dir = os.path.join(library_lib_path, dir_name)
      if ContainsNativeLibrary(lib_dir):
        shutil.move(lib_dir, os.path.join(native_lib_path, dir_name))
        available_archs.append(dir_name)
    if options.arch:
      Execution(options, name)
      packaged_archs.append(options.arch)
    else:
      # If the arch option is unspecified, all of available platform APKs
      # will be generated.
      valid_archs = ['x86', 'armeabi-v7a']
      for arch in valid_archs:
        if arch in available_archs:
          if arch.find('x86') != -1:
            options.arch = 'x86'
          elif arch.find('arm') != -1:
            options.arch = 'arm'
          Execution(options, name)
          packaged_archs.append(options.arch)
        else:
          print('Warning: failed to create package for arch "%s" '
                'due to missing native library' % arch)

      if len(packaged_archs) == 0:
        print('No packages created, aborting')
        sys.exit(13)

  PrintPackageInfo(options, name, packaged_archs)

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
  parser.add_option('--mode', choices=('embedded', 'shared'),
                    default='embedded', help=info)
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
  info = ('Passcode for alias\'s private key in the keystore, '
          'For example, --keystore-alias-passcode=alias-code')
  group.add_option('--keystore-alias-passcode', help=info)
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
      parser.error('VERSION was not found, so Crosswalk\'s version could not '
                   'be determined.')

  xpk_temp_dir = ''
  if options.xpk:
    xpk_name = os.path.splitext(os.path.basename(options.xpk))[0]
    xpk_temp_dir = os.path.join(xwalk_dir, xpk_name + '_xpk')
    ParseXPK(options, xpk_temp_dir)

  if options.app_root and not options.manifest:
    manifest_path = os.path.join(options.app_root, 'manifest.json')
    if os.path.exists(manifest_path):
      print('Using manifest.json distributed with the application.')
      options.manifest = manifest_path

  app_info = AppInfo()
  manifest = None
  if not options.manifest:
    # The checks here are really convoluted, but at the moment make_apk
    # misbehaves any of the following conditions is true.
    if options.app_url:
      # 1) --app-url must be passed without either --app-local-path or
      #    --app-root.
      if options.app_root or options.app_local_path:
        parser.error('You must pass either "--app-url" or "--app-local-path" '
                     'with "--app-root", but not all.')
    else:
      # 2) --app-url is not passed but only one of --app-local-path and
      #    --app-root is set.
      if bool(options.app_root) != bool(options.app_local_path):
        parser.error('You must specify both "--app-local-path" and '
                     '"--app-root".')
      # 3) None of --app-url, --app-local-path and --app-root are passed.
      elif not options.app_root and not options.app_local_path:
        parser.error('You must pass either "--app-url" or "--app-local-path" '
                     'with "--app-root".')

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
      manifest = ParseManifest(options)
    except SystemExit as ec:
      return ec.code

  if not options.name:
    parser.error('An APK name is required. Please use the "--name" option.')

  if not options.package:
    parser.error('A package name is required. Please use the "--package" '
                 'option.')
  VerifyPackageName(options.package)

  if (options.app_root and options.app_local_path and
      not os.path.isfile(os.path.join(options.app_root,
                                      options.app_local_path))):
    print('Please make sure that the local path file of launching app '
          'does exist.')
    sys.exit(7)

  if options.target_dir:
    target_dir = os.path.abspath(os.path.expanduser(options.target_dir))
    options.target_dir = target_dir
    if not os.path.isdir(target_dir):
      os.makedirs(target_dir)

  try:
    MakeApk(options, app_info, manifest)
  except SystemExit as ec:
    CleanDir(app_info.android_name)
    CleanDir('out')
    CleanDir(xpk_temp_dir)
    return ec.code
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
