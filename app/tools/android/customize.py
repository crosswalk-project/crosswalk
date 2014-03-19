#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import optparse
import os
import re
import shutil
import sys

from handle_xml import AddElementAttribute
from handle_xml import AddElementAttributeAndText
from handle_xml import EditElementAttribute
from handle_xml import EditElementValueByNodeName
from handle_permissions import HandlePermissions
from xml.dom import minidom

def ReplaceInvalidChars(value, mode='default'):
  """ Replace the invalid chars with '_' for input string.
  Args:
    value: the original string.
    mode: the target usage mode of original string.
  """
  if mode == 'default':
    invalid_chars = '\/:*?"<>|- '
  elif mode == 'apkname':
    invalid_chars = '\/:.*?"<>|-'
  for c in invalid_chars:
    if mode == 'apkname' and c in value:
      print("Illegal character: '%s' is replaced with '_'" % c)
    value = value.replace(c,'_')
  return value


def Prepare(sanitized_name, package, app_root):
  if os.path.exists(sanitized_name):
    shutil.rmtree(sanitized_name)
  shutil.copytree('app_src', sanitized_name)
  shutil.rmtree(os.path.join(sanitized_name, 'src'))
  src_root = os.path.join('app_src', 'src', 'org', 'xwalk', 'app', 'template')
  src_activity = os.path.join(src_root, 'AppTemplateActivity.java')
  if not os.path.isfile(src_activity):
    print ('Please make sure that the java file'
           ' of activity does exist.')
    sys.exit(7)
  root_path =  os.path.join(sanitized_name, 'src',
                            package.replace('.', os.path.sep))
  if not os.path.exists(root_path):
    os.makedirs(root_path)
  dest_activity = sanitized_name + 'Activity.java'
  shutil.copyfile(src_activity, os.path.join(root_path, dest_activity))
  if app_root:
    assets_path = os.path.join(sanitized_name, 'assets')
    shutil.rmtree(assets_path)
    os.makedirs(assets_path)
    app_src_path = os.path.join(assets_path, 'www')
    shutil.copytree(app_root, app_src_path)


def CustomizeStringXML(sanitized_name, description):
  strings_path = os.path.join(sanitized_name, 'res', 'values', 'strings.xml')
  if not os.path.isfile(strings_path):
    print ('Please make sure strings_xml'
           ' exists under app_src folder.')
    sys.exit(6)

  if description:
    xmldoc = minidom.parse(strings_path)
    AddElementAttributeAndText(xmldoc, 'string', 'name', 'description',
                               description)
    strings_file = open(strings_path, 'w')
    xmldoc.writexml(strings_file, encoding='utf-8')
    strings_file.close()


def CustomizeThemeXML(sanitized_name, fullscreen, launch_screen_img):
  theme_path = os.path.join(sanitized_name, 'res', 'values', 'theme.xml')
  if not os.path.isfile(theme_path):
    print('Error: theme.xml is missing in the build tool.')
    sys.exit(6)

  xmldoc = minidom.parse(theme_path)
  if fullscreen:
    EditElementValueByNodeName(xmldoc, 'item',
                               'android:windowFullscreen', 'true')
  if launch_screen_img:
    EditElementValueByNodeName(xmldoc, 'item',
                               'android:windowBackground',
                               '@drawable/launchscreen')
    default_image = launch_screen_img
    if os.path.isfile(default_image):
      drawable_path = os.path.join(sanitized_name, 'res', 'drawable')
      if not os.path.exists(drawable_path):
        os.makedirs(drawable_path)
      # Get the extension of default_image.
      # Need to take care of special case, such as 'img.9.png'
      name = os.path.basename(default_image)
      extlist = name.split('.')
      # Remove the file name from the list.
      extlist.pop(0)
      ext = '.' + '.'.join(extlist)
      final_launch_screen_path = os.path.join(drawable_path,
                                              'launchscreen' + ext)
      shutil.copyfile(default_image, final_launch_screen_path)
    else:
      print('Error: Please make sure \"' + default_image + '\" exists!')
      sys.exit(6)
  theme_file = open(theme_path, 'w')
  xmldoc.writexml(theme_file, encoding='utf-8')
  theme_file.close()


def CustomizeXML(sanitized_name, package, app_versionCode, app_version,
                 description, name, orientation, icon, fullscreen,
                 launch_screen_img, permissions):
  manifest_path = os.path.join(sanitized_name, 'AndroidManifest.xml')
  if not os.path.isfile(manifest_path):
    print ('Please make sure AndroidManifest.xml'
           ' exists under app_src folder.')
    sys.exit(6)

  CustomizeStringXML(sanitized_name, description)
  CustomizeThemeXML(sanitized_name, fullscreen, launch_screen_img)
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
  EditElementAttribute(xmldoc, 'application', 'android:label', name)
  activity_name = package + '.' + sanitized_name + 'Activity'
  EditElementAttribute(xmldoc, 'activity', 'android:name', activity_name)
  EditElementAttribute(xmldoc, 'activity', 'android:label', name)
  if orientation:
    EditElementAttribute(xmldoc, 'activity', 'android:screenOrientation',
                         orientation)
  if icon and os.path.isfile(icon):
    drawable_path = os.path.join(sanitized_name, 'res', 'drawable')
    if not os.path.exists(drawable_path):
      os.makedirs(drawable_path)
    icon_file = os.path.basename(icon)
    icon_file = ReplaceInvalidChars(icon_file)
    shutil.copyfile(icon, os.path.join(drawable_path, icon_file))
    icon_name = os.path.splitext(icon_file)[0]
    EditElementAttribute(xmldoc, 'application',
                         'android:icon', '@drawable/%s' % icon_name)
  elif icon and (not os.path.isfile(icon)):
    print ('Please make sure the icon file does exist!')
    sys.exit(6)

  file_handle = open(os.path.join(sanitized_name, 'AndroidManifest.xml'), 'w')
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


def CustomizeJava(sanitized_name, package, app_url, app_local_path,
                  enable_remote_debugging, display_as_fullscreen):
  root_path =  os.path.join(sanitized_name, 'src',
                            package.replace('.', os.path.sep))
  dest_activity = os.path.join(root_path, sanitized_name + 'Activity.java')
  ReplaceString(dest_activity, 'org.xwalk.app.template', package)
  ReplaceString(dest_activity, 'AppTemplate', sanitized_name)
  manifest_file = os.path.join(sanitized_name, 'assets/www', 'manifest.json')
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
      if os.path.isfile(os.path.join(sanitized_name, 'assets/www',
                                     app_local_path)):
        ReplaceString(dest_activity, 'file:///android_asset/www/index.html',
                      'app://' + package + '/' + app_local_path)
      else:
        print ('Please make sure that the relative path of entry file'
               ' is correct.')
        sys.exit(8)

  if enable_remote_debugging:
    SetVariable(dest_activity,
                'public void onCreate(Bundle savedInstanceState)',
                'RemoteDebugging', 'true')
  if display_as_fullscreen:
    SetVariable(dest_activity,
                'super.onCreate(savedInstanceState)',
                'IsFullscreen', 'true')


def CopyExtensionFile(extension_name, suffix, src_path, dest_path):
  # Copy the file from src_path into dest_path.
  dest_extension_path = os.path.join(dest_path, extension_name)
  if os.path.exists(dest_extension_path):
    # TODO: Refine it by renaming it internally.
    print('Error: duplicated extension names "%s" are found. Please rename it.'
          % extension_name)
    sys.exit(9)
  else:
    os.mkdir(dest_extension_path)

  file_name = extension_name + suffix
  src_file = os.path.join(src_path, file_name)
  dest_file = os.path.join(dest_extension_path, file_name)
  if not os.path.isfile(src_file):
    sys.exit(9)
    print('Error: %s is not found in %s.' % (file_name, src_path))
  else:
    shutil.copyfile(src_file, dest_file)


def CustomizeExtensions(sanitized_name, name, extensions):
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
  apk_path = name
  apk_assets_path = os.path.join(apk_path, 'assets')
  extensions_string = 'xwalk-extensions'

  # Set up the target directories and files.
  dest_jar_path = os.path.join(apk_path, extensions_string)
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
      print('Error: can\'t find the extension directory \'%s\'.' % source_path)
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
      print('Error: %s is not found in %s.' % (file_name, source_path))
      sys.exit(9)
    else:
      src_file_handle = file(src_file)
      src_file_content = src_file_handle.read()
      json_output = json.JSONDecoder().decode(src_file_content)
      # Below 3 properties are used by runtime. See extension manager.
      # And 'permissions' will be merged.
      if ((not 'name' in json_output) or (not 'class' in json_output)
          or (not 'jsapi' in json_output)):
        print ('Error: properties \'name\', \'class\' and \'jsapi\' in a json '
               'file are mandatory.')
        sys.exit(9)
      # Reset the path for JavaScript.
      js_path_prefix = extensions_string + '/' + extension_name + '/'
      json_output['jsapi'] = js_path_prefix + json_output['jsapi']
      extension_json_list.append(json_output)
      # Merge the permissions of extensions into AndroidManifest.xml.
      manifest_path = os.path.join(sanitized_name, 'AndroidManifest.xml')
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
        xmldoc.writexml(file_handle)
        file_handle.close()

  # Write configuration of extensions into the target extensions-config.json.
  if extension_json_list:
    extensions_string = json.JSONEncoder().encode(extension_json_list)
    extension_json_file = open(apk_extensions_json_path, 'w')
    extension_json_file.write(extensions_string)
    extension_json_file.close()


def CustomizeAll(app_versionCode, description, icon, permissions, app_url,
                 app_root, app_local_path, enable_remote_debugging,
                 display_as_fullscreen, extensions, launch_screen_img,
                 package='org.xwalk.app.template', name='AppTemplate',
                 app_version='1.0.0', orientation='unspecified') :
  sanitized_name = ReplaceInvalidChars(name, 'apkname')
  try:
    Prepare(sanitized_name, package, app_root)
    CustomizeXML(sanitized_name, package, app_versionCode, app_version,
                 description, name, orientation, icon, display_as_fullscreen,
                 launch_screen_img, permissions)
    CustomizeJava(sanitized_name, package, app_url, app_local_path,
                  enable_remote_debugging, display_as_fullscreen)
    CustomizeExtensions(sanitized_name, name, extensions)
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
  info = ('The path of icon. Such as: --icon=/path/to/your/customized/icon')
  parser.add_option('--icon', help=info)
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
                    help = 'Enable remote debugging.')
  parser.add_option('-f', '--fullscreen', action='store_true',
                    dest='fullscreen', default=False,
                    help='Make application fullscreen.')
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
  parser.add_option('--launch-screen-img',
                    help='The fallback image for launch_screen')
  options, _ = parser.parse_args()
  try:
    CustomizeAll(options.app_versionCode, options.description, options.icon,
                 options.permissions, options.app_url, options.app_root,
                 options.app_local_path, options.enable_remote_debugging,
                 options.fullscreen, options.extensions,
                 options.launch_screen_img, options.package, options.name,
                 options.app_version, options.orientation)
  except SystemExit as ec:
    print('Exiting with error code: %d' % ec.code)
    return ec.code
  return 0


if __name__ == '__main__':
  sys.exit(main())
