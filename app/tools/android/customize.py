#!/usr/bin/env python

# Copyright (c) 2013,2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import compress_js_and_css
import fnmatch
import json
import optparse
import os
import re
import shutil
import stat
import sys

# get xwalk absolute path so we can run this script from any location
xwalk_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(xwalk_dir)

from app_info import AppInfo
from customize_launch_screen import CustomizeLaunchScreen
from handle_xml import AddElementAttribute
from handle_xml import AddElementAttributeAndText
from handle_xml import EditElementAttribute
from handle_xml import EditElementValueByNodeName
from handle_xml import MergeNodes
from handle_permissions import HandlePermissions
from util import CleanDir, CreateAndCopyDir, GetBuildDir
from xml.dom import minidom


TEMPLATE_DIR_NAME = 'template'


def VerifyPackageName(value):
  regex = r'^[a-z][a-z0-9_]*(\.[a-z][a-z0-9_]*)+$'
  descrpt = 'Each part of package'
  sample = 'org.xwalk.example, org.xwalk.example_'

  if len(value) >= 128:
    print('To be safe, the length of package name or app name '
          'should be less than 128.')
    sys.exit(6)

  if not re.match(regex, value):
    print('Error: %s name should be started with letters and should not '
          'contain invalid characters.\n'
          'It may contain lowercase letters, numbers, blank spaces and '
          'underscores\n'
          'Sample: %s' % (descrpt, sample))
    sys.exit(6)


def ReplaceSpaceWithUnderscore(value):
  return value.replace(' ', '_')


def ReplaceInvalidChars(value, mode='default'):
  """ Replace the invalid chars with '_' for input string.
  Args:
    value: the original string.
    mode: the target usage mode of original string.
  """
  if mode == 'default':
    invalid_chars = '\/:*?"<>|- '
  elif mode == 'apkname':
    invalid_chars = '\/:.*?"<>|- '
  for c in invalid_chars:
    if mode == 'apkname' and c in value:
      print('Illegal character: "%s" replaced with "_"' % c)
    value = value.replace(c, '_')
  return value


def GetFilesByExt(path, ext, sub_dir=True):
  if os.path.exists(path):
    file_list = []
    for name in os.listdir(path):
      full_name = os.path.join(path, name)
      st = os.lstat(full_name)
      if stat.S_ISDIR(st.st_mode) and sub_dir:
        file_list += GetFilesByExt(full_name, ext)
      elif os.path.isfile(full_name):
        if fnmatch.fnmatch(full_name, ext):
          file_list.append(full_name)
    return file_list
  else:
    return []


def ParseParameterForCompressor(option, value, values, parser):
  if ((not values or values.startswith('-'))
      and value.find('--compressor') != -1):
    values = 'all'
  val = values
  if parser.rargs and not parser.rargs[0].startswith('-'):
    val = parser.rargs[0]
    parser.rargs.pop(0)
  setattr(parser.values, option.dest, val)


def CompressSourceFiles(app_root, compressor):
  js_list = []
  css_list = []
  js_ext = '*.js'
  css_ext = '*.css'

  if compressor == 'all' or compressor == 'js':
    js_list = GetFilesByExt(app_root, js_ext)
    compress_js_and_css.CompressJavaScript(js_list)

  if compressor == 'all' or compressor == 'css':
    css_list = GetFilesByExt(app_root, css_ext)
    compress_js_and_css.CompressCss(css_list)


def Prepare(app_info, compressor):
  """Copy the Android template project to a new app project
     named app_info.app_name
  """
  # create new app_dir in temp dir
  app_name = app_info.android_name
  app_dir = GetBuildDir(app_name)
  app_package = app_info.package
  app_root = app_info.app_root
  template_app_dir = os.path.join(xwalk_dir, TEMPLATE_DIR_NAME)

  # 1) copy template project to app_dir
  CleanDir(app_dir)
  if not os.path.isdir(template_app_dir):
    print('Error: The template directory could not be found (%s).' %
          template_app_dir)
    sys.exit(7)
  shutil.copytree(template_app_dir, app_dir)

  # 2) replace app_dir 'src' dir with template 'src' dir
  CleanDir(os.path.join(app_dir, 'src'))
  template_src_root = os.path.join(template_app_dir, 'src', 'org', 'xwalk',
                                   'app', 'template')

  # 3) Create directory tree from app package (org.xyz.foo -> src/org/xyz/foo)
  #    and copy AppTemplateActivity.java to <app_name>Activity.java
  template_activity_file = os.path.join(template_src_root,
                                        'AppTemplateActivity.java')
  if not os.path.isfile(template_activity_file):
    print ('Error: The template file %s was not found. '
           'Please make sure this file exists.' % template_activity_file)
    sys.exit(7)
  app_pkg_dir = os.path.join(app_dir, 'src',
                             app_package.replace('.', os.path.sep))
  if not os.path.exists(app_pkg_dir):
    os.makedirs(app_pkg_dir)
  app_activity_file = app_name + 'Activity.java'
  shutil.copyfile(template_activity_file,
                  os.path.join(app_pkg_dir, app_activity_file))

  # 4) Copy all HTML source from app_root to app_dir
  if app_root:
    app_assets_dir = os.path.join(app_dir, 'assets', 'www')
    CleanDir(app_assets_dir)
    shutil.copytree(app_root, app_assets_dir)
    if compressor:
      CompressSourceFiles(app_assets_dir, compressor)


def EncodingUnicodeValue(value):
  try:
    if isinstance(value, unicode):
      value = value.encode("utf-8")
  except NameError:
    pass
  return value


def CustomizeStringXML(name, description):
  strings_path = os.path.join(GetBuildDir(name), 'res', 'values',
                              'strings.xml')
  if not os.path.isfile(strings_path):
    print ('Please make sure strings_xml'
           ' exists under template folder.')
    sys.exit(6)

  if description:
    description = EncodingUnicodeValue(description)
    xmldoc = minidom.parse(strings_path)
    AddElementAttributeAndText(xmldoc, 'string', 'name', 'description',
                               description)
    strings_file = open(strings_path, 'w')
    xmldoc.writexml(strings_file, encoding='utf-8')
    strings_file.close()


def CustomizeThemeXML(name, fullscreen, manifest):
  theme_path = os.path.join(GetBuildDir(name), 'res', 'values-v14', 'theme.xml')
  if not os.path.isfile(theme_path):
    print('Error: theme.xml is missing in the build tool.')
    sys.exit(6)

  theme_xmldoc = minidom.parse(theme_path)
  if fullscreen:
    EditElementValueByNodeName(theme_xmldoc, 'item',
                               'android:windowFullscreen', 'true')
  has_background = CustomizeLaunchScreen(manifest, name)
  if has_background:
    EditElementValueByNodeName(theme_xmldoc, 'item',
                               'android:windowBackground',
                               '@drawable/launchscreen_bg')
  theme_file = open(theme_path, 'w')
  theme_xmldoc.writexml(theme_file, encoding='utf-8')
  theme_file.close()


def CustomizeXML(app_info, description, icon_dict, manifest, permissions):
  app_version = app_info.app_version
  app_versionCode = app_info.app_versionCode
  name = app_info.android_name
  orientation = app_info.orientation
  package = app_info.package
  app_name = app_info.app_name
  app_dir = GetBuildDir(name)
  # Chinese character with unicode get from 'manifest.json' will cause
  # 'UnicodeEncodeError' when finally wrote to 'AndroidManifest.xml'.
  app_name = EncodingUnicodeValue(app_name)
  # If string start with '@' or '?', it will be treated as Android resource,
  # which will cause 'No resource found' error,
  # append a space before '@' or '?' to fix that.
  if app_name.startswith('@') or app_name.startswith('?'):
    app_name = ' ' + app_name
  manifest_path = os.path.join(app_dir, 'AndroidManifest.xml')
  if not os.path.isfile(manifest_path):
    print ('Please make sure AndroidManifest.xml'
           ' exists under template folder.')
    sys.exit(6)

  CustomizeStringXML(name, description)
  CustomizeThemeXML(name, app_info.fullscreen_flag, manifest)
  xmldoc = minidom.parse(manifest_path)
  EditElementAttribute(xmldoc, 'manifest', 'package', package)
  if app_versionCode:
    EditElementAttribute(xmldoc, 'manifest', 'android:versionCode',
                         str(app_versionCode))
  if app_version:
    EditElementAttribute(xmldoc, 'manifest', 'android:versionName',
                         app_version)
  if description:
    EditElementAttribute(xmldoc, 'manifest', 'android:description',
                         "@string/description")
  HandlePermissions(permissions, xmldoc)
  EditElementAttribute(xmldoc, 'application', 'android:label', app_name)
  activity_name = package + '.' + name + 'Activity'
  EditElementAttribute(xmldoc, 'activity', 'android:name', activity_name)
  EditElementAttribute(xmldoc, 'activity', 'android:label', app_name)
  if orientation:
    EditElementAttribute(xmldoc, 'activity', 'android:screenOrientation',
                         orientation)
  icon_name = CustomizeIcon(name, app_info.app_root, app_info.icon, icon_dict)
  if icon_name:
    EditElementAttribute(xmldoc, 'application', 'android:icon',
                         '@drawable/%s' % icon_name)

  file_handle = open(os.path.join(app_dir, 'AndroidManifest.xml'), 'w')
  xmldoc.writexml(file_handle, encoding='utf-8')
  file_handle.close()


def ReplaceString(file_path, src, dest):
  file_handle = open(file_path, 'r')
  src_content = file_handle.read()
  file_handle.close()
  file_handle = open(file_path, 'w')
  dest_content = src_content.replace(src, dest)
  file_handle.write(dest_content)
  file_handle.close()


def SetVariable(file_path, string_line, variable, value):
  function_string = ('%sset%s(%s);\n' %
                     ('        ', variable, value))
  temp_file_path = file_path + '.backup'
  file_handle = open(temp_file_path, 'w+')
  for line in open(file_path):
    file_handle.write(line)
    if (line.find(string_line) >= 0):
      file_handle.write(function_string)
  file_handle.close()
  shutil.move(temp_file_path, file_path)


def CustomizeJava(app_info, app_url, app_local_path, keep_screen_on):
  name = app_info.android_name
  package = app_info.package
  app_dir = GetBuildDir(name)
  app_pkg_dir = os.path.join(app_dir, 'src', package.replace('.', os.path.sep))
  dest_activity = os.path.join(app_pkg_dir, name + 'Activity.java')
  ReplaceString(dest_activity, 'org.xwalk.app.template', package)
  ReplaceString(dest_activity, 'AppTemplate', name)
  manifest_file = os.path.join(app_dir, 'assets', 'www', 'manifest.json')
  if os.path.isfile(manifest_file):
    ReplaceString(
        dest_activity,
        'loadAppFromUrl("file:///android_asset/www/index.html")',
        'loadAppFromManifest("file:///android_asset/www/manifest.json")')
  else:
    if app_url:
      if re.search(r'^http(|s)', app_url):
        ReplaceString(dest_activity, 'file:///android_asset/www/index.html',
                      app_url)
    elif app_local_path:
      if os.path.isfile(os.path.join(app_dir, 'assets', 'www', app_local_path)):
        ReplaceString(dest_activity, 'file:///android_asset/www/index.html',
                      'app://' + package + '/' + app_local_path)
      else:
        print ('Please make sure that the relative path of entry file'
               ' is correct.')
        sys.exit(8)

  if app_info.remote_debugging:
    SetVariable(dest_activity,
                'public void onCreate(Bundle savedInstanceState)',
                'RemoteDebugging', 'true')
  if app_info.use_animatable_view:
    SetVariable(dest_activity,
                'public void onCreate(Bundle savedInstanceState)',
                'UseAnimatableView', 'true')
  if app_info.fullscreen_flag:
    SetVariable(dest_activity,
                'super.onCreate(savedInstanceState)',
                'IsFullscreen', 'true')
  if keep_screen_on:
    ReplaceString(
        dest_activity,
        'super.onCreate(savedInstanceState);',
        'super.onCreate(savedInstanceState);\n        ' +
        'getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);')


def CopyExtensionFile(extension_name, suffix, src_path, dest_path):
  # Copy the file from src_path into dest_path.
  dest_extension_path = os.path.join(dest_path, extension_name)
  if os.path.exists(dest_extension_path):
    # TODO: Refine it by renaming it internally.
    print('Error: duplicate extension names were found (%s). Please rename it.'
          % extension_name)
    sys.exit(9)
  else:
    os.mkdir(dest_extension_path)

  file_name = extension_name + suffix
  src_file = os.path.join(src_path, file_name)
  dest_file = os.path.join(dest_extension_path, file_name)
  if not os.path.isfile(src_file):
    print('Error: %s was not found in %s.' % (file_name, src_path))
    sys.exit(9)
  else:
    shutil.copyfile(src_file, dest_file)


def CustomizeExtensions(app_info, extensions):
  """Copy the files from external extensions and merge them into APK.

  The directory of one external extension should be like:
    myextension/
      myextension.jar
      myextension.js
      myextension.json
  That means the name of the internal files should be the same as the
  directory name.
  For .jar files, they'll be copied to xwalk-extensions/ and then
  built into classes.dex in make_apk.py.
  For .js files, they'll be copied into assets/xwalk-extensions/.
  For .json files, the'll be merged into one file called
  extensions-config.json and copied into assets/.
  """
  if not extensions:
    return
  name = app_info.android_name
  app_dir = GetBuildDir(name)
  apk_assets_path = os.path.join(app_dir, 'assets')
  extensions_string = 'xwalk-extensions'

  # Set up the target directories and files.
  dest_jar_path = os.path.join(app_dir, extensions_string)
  os.mkdir(dest_jar_path)
  dest_js_path = os.path.join(apk_assets_path, extensions_string)
  os.mkdir(dest_js_path)
  apk_extensions_json_path = os.path.join(apk_assets_path,
                                          'extensions-config.json')

  # Split the paths into a list.
  extension_paths = extensions.split(os.pathsep)
  extension_json_list = []
  for source_path in extension_paths:
    if not os.path.exists(source_path):
      print('Error: can not find the extension directory \'%s\'.' % source_path)
      sys.exit(9)
    # Remove redundant separators to avoid empty basename.
    source_path = os.path.normpath(source_path)
    extension_name = os.path.basename(source_path)

    # Copy .jar file into xwalk-extensions.
    CopyExtensionFile(extension_name, '.jar', source_path, dest_jar_path)

    # Copy .js file into assets/xwalk-extensions.
    CopyExtensionFile(extension_name, '.js', source_path, dest_js_path)

    # Merge .json file into assets/xwalk-extensions.
    file_name = extension_name + '.json'
    src_file = os.path.join(source_path, file_name)
    if not os.path.isfile(src_file):
      print('Error: %s was not found in %s.' % (file_name, source_path))
      sys.exit(9)
    else:
      src_file_handle = open(src_file)
      src_file_content = src_file_handle.read()
      json_output = json.JSONDecoder().decode(src_file_content)
      # Below 3 properties are used by runtime. See extension manager.
      # And 'permissions' will be merged.
      if not ('name' in json_output and
              'class' in json_output and
              'jsapi' in json_output):
        print ('Error: properties \'name\', \'class\' and \'jsapi\' in a json '
               'file are mandatory.')
        sys.exit(9)
      # Reset the path for JavaScript.
      js_path_prefix = extensions_string + '/' + extension_name + '/'
      json_output['jsapi'] = js_path_prefix + json_output['jsapi']
      extension_json_list.append(json_output)
      # Merge the permissions of extensions into AndroidManifest.xml.
      manifest_path = os.path.join(app_dir, 'AndroidManifest.xml')
      xmldoc = minidom.parse(manifest_path)
      if ('permissions' in json_output):
        # Get used permission list to avoid repetition as "--permissions"
        # option can also be used to declare used permissions.
        existingList = []
        usedPermissions = xmldoc.getElementsByTagName("uses-permission")
        for used in usedPermissions:
          existingList.append(used.getAttribute("android:name"))

        # Add the permissions to manifest file if not used yet.
        for p in json_output['permissions']:
          if p in existingList:
            continue
          AddElementAttribute(xmldoc, 'uses-permission', 'android:name', p)
          existingList.append(p)

        # Write to the manifest file to save the update.
        file_handle = open(manifest_path, 'w')
        xmldoc.writexml(file_handle, encoding='utf-8')
        file_handle.close()
      if 'manifest' in json_output:
        manifest_merge_path = os.path.join(source_path, json_output['manifest'])
        if not os.path.isfile(manifest_merge_path):
          print('Error: %s specified in the extension\'s JSON '
                'could not be found.' % manifest_merge_path)
          sys.exit(9)
        xmldoc_merge = minidom.parse(manifest_merge_path)
        manifest_nodes = xmldoc.getElementsByTagName('manifest')
        manifest_nodes_merge = xmldoc_merge.getElementsByTagName('manifest')
        if not manifest_nodes:
          print('Error: %s does not have a <manifest> node.' % manifest_path)
          sys.exit(9)
        if not manifest_nodes_merge:
          print('Error: %s does not have a <manifest> node.'
                % manifest_merge_path)
          sys.exit(9)
        MergeNodes(manifest_nodes[0], manifest_nodes_merge[0])
        with open(manifest_path, 'w') as file_handle:
          xmldoc.writexml(file_handle, encoding='utf-8')

  # Write configuration of extensions into the target extensions-config.json.
  if extension_json_list:
    extensions_string = json.JSONEncoder().encode(extension_json_list)
    extension_json_file = open(apk_extensions_json_path, 'w')
    extension_json_file.write(extensions_string)
    extension_json_file.close()


def GenerateCommandLineFile(app_info, xwalk_command_line):
  if xwalk_command_line == '':
    return
  assets_path = os.path.join(GetBuildDir(app_info.android_name), 'assets')
  file_path = os.path.join(assets_path, 'xwalk-command-line')
  command_line_file = open(file_path, 'w')
  command_line_file.write('xwalk ' + xwalk_command_line)


def CustomizeIconByDict(name, app_root, icon_dict):
  app_dir = GetBuildDir(name)
  icon_name = None
  drawable_dict = {'ldpi': [1, 37], 'mdpi': [37, 72], 'hdpi': [72, 96],
                   'xhdpi': [96, 120], 'xxhdpi': [120, 144],
                   'xxxhdpi': [144, 168]}
  if not icon_dict:
    return icon_name

  try:
    icon_dict = dict((int(k), v) for k, v in icon_dict.items())
  except ValueError:
    print('The key of icon in the manifest file should be a number.')

  if len(icon_dict) > 0:
    icon_list = sorted(icon_dict.items(), key=lambda d: d[0])
    for kd, vd in drawable_dict.items():
      for item in icon_list:
        if item[0] >= vd[0] and item[0] < vd[1]:
          drawable_path = os.path.join(app_dir, 'res', 'drawable-' + kd)
          if not os.path.exists(drawable_path):
            os.makedirs(drawable_path)
          icon = os.path.join(app_root, item[1])
          if icon and os.path.isfile(icon):
            icon_name = os.path.basename(icon)
            icon_suffix = icon_name.split('.')[-1]
            shutil.copyfile(icon, os.path.join(drawable_path,
                                               'icon.' + icon_suffix))
            icon_name = 'icon'
          elif icon and (not os.path.isfile(icon)):
            print('Error: "%s" does not exist.' % icon)
            sys.exit(6)
          break
  return icon_name


def CustomizeIconByOption(name, icon):
  if os.path.isfile(icon):
    drawable_path = os.path.join(GetBuildDir(name), 'res', 'drawable')
    if not os.path.exists(drawable_path):
      os.makedirs(drawable_path)
    icon_file = os.path.basename(icon)
    icon_file = ReplaceInvalidChars(icon_file)
    shutil.copyfile(icon, os.path.join(drawable_path, icon_file))
    icon_name = os.path.splitext(icon_file)[0]
    return icon_name
  else:
    print('Error: "%s" does not exist.' % icon)
    sys.exit(6)


def CustomizeIcon(name, app_root, icon, icon_dict):
  icon_name = None
  if icon:
    icon_name = CustomizeIconByOption(name, icon)
  else:
    icon_name = CustomizeIconByDict(name, app_root, icon_dict)
  return icon_name


def CustomizeAll(app_info, description, icon_dict, permissions, app_url,
                 app_local_path, keep_screen_on, extensions, manifest,
                 xwalk_command_line='', compressor=None):
  try:
    Prepare(app_info, compressor)
    CustomizeXML(app_info, description, icon_dict, manifest, permissions)
    CustomizeJava(app_info, app_url, app_local_path, keep_screen_on)
    CustomizeExtensions(app_info, extensions)
    GenerateCommandLineFile(app_info, xwalk_command_line)
  except SystemExit as ec:
    print('Exiting with error code: %d' % ec.code)
    sys.exit(ec.code)


def main():
  parser = optparse.OptionParser()
  info = ('The package name. Such as: '
          '--package=com.example.YourPackage')
  parser.add_option('--package', help=info)
  info = ('The apk name. Such as: --name="Your Application Name"')
  parser.add_option('--name', help=info)
  info = ('The version of the app. Such as: --app-version=TheVersionNumber')
  parser.add_option('--app-version', help=info)
  info = ('The versionCode of the app. Such as: --app-versionCode=24')
  parser.add_option('--app-versionCode', type='int', help=info)
  info = ('The application description. Such as:'
          '--description=YourApplicationdDescription')
  parser.add_option('--description', help=info)
  info = ('The permission list. Such as: --permissions="geolocation"'
          'For more permissions, such as:'
          '--permissions="geolocation:permission2"')
  parser.add_option('--permissions', help=info)
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
  parser.add_option('--enable-remote-debugging', action='store_true',
                    dest='enable_remote_debugging', default=False,
                    help='Enable remote debugging.')
  parser.add_option('--use-animatable-view', action='store_true',
                    dest='use_animatable_view', default=False,
                    help='Enable using animatable view (TextureView).')
  parser.add_option('-f', '--fullscreen', action='store_true',
                    dest='fullscreen', default=False,
                    help='Make application fullscreen.')
  parser.add_option('--keep-screen-on', action='store_true', default=False,
                    help='Support keeping screen on')
  info = ('The path list for external extensions separated by os separator.'
          'On Linux and Mac, the separator is ":". On Windows, it is ";".'
          'Such as: --extensions="/path/to/extension1:/path/to/extension2"')
  parser.add_option('--extensions', help=info)
  info = ('The orientation of the web app\'s display on the device. '
          'Such as: --orientation=landscape. The default value is "unspecified"'
          'The value options are the same as those on the Android: '
          'http://developer.android.com/guide/topics/manifest/'
          'activity-element.html#screen')
  parser.add_option('--orientation', help=info)
  parser.add_option('--manifest', help='The manifest path')
  info = ('Use command lines.'
          'Crosswalk is powered by Chromium and supports Chromium command line.'
          'For example, '
          '--xwalk-command-line=\'--chromium-command-1 --xwalk-command-2\'')
  info = ('Create an Android project directory at this location. ')
  parser.add_option('--project-dir', help=info)
  parser.add_option('--xwalk-command-line', default='', help=info)
  info = ('Minify and obfuscate javascript and css.'
          '--compressor: compress javascript and css.'
          '--compressor=js: compress javascript.'
          '--compressor=css: compress css.')
  parser.add_option('--compressor', dest='compressor', action='callback',
                    callback=ParseParameterForCompressor,
                    type='string', nargs=0, help=info)
  options, _ = parser.parse_args()
  try:
    icon_dict = {144: 'icons/icon_144.png',
                 72: 'icons/icon_72.png',
                 96: 'icons/icon_96.png',
                 48: 'icons/icon_48.png'}
    app_info = AppInfo()
    if options.name is not None:
      app_info.android_name = options.name
    if options.app_root is None:
      app_info.app_root = os.path.join(xwalk_dir, 'test_data', 'manifest')
    else:
      app_info.app_root = options.app_root
    if options.package is not None:
      app_info.package = options.package
    if options.orientation is not None:
      app_info.orientation = options.orientation
    if options.app_version is not None:
      app_info.app_version = options.app_version
    if options.enable_remote_debugging is not None:
      app_info.remote_debugging = options.enable_remote_debugging
    if options.fullscreen is not None:
      app_info.fullscreen_flag = options.fullscreen
    app_info.icon = os.path.join('test_data', 'manifest', 'icons',
                                 'icon_96.png')
    CustomizeAll(app_info, options.description, icon_dict,
                 options.permissions, options.app_url, options.app_local_path,
                 options.keep_screen_on, options.extensions, None,
                 options.xwalk_command_line, options.compressor)

    # build project is now in /tmp/<name>. Copy to project_dir
    if options.project_dir:
      src_dir = GetBuildDir(app_info.android_name)
      dest_dir = os.path.join(options.project_dir, app_info.android_name)
      CreateAndCopyDir(src_dir, dest_dir, True)

  except SystemExit as ec:
    print('Exiting with error code: %d' % ec.code)
    return ec.code
  finally:
    CleanDir(GetBuildDir(app_info.android_name))
  return 0


if __name__ == '__main__':
  sys.exit(main())
