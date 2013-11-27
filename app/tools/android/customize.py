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
    invalid_chars = '\/:.*?"<>|- '
  for c in invalid_chars:
    if mode == 'apkname' and c in value:
      print "Illegal character: '%s' is replaced with '_'" % c
    value = value.replace(c,'_')
  return value


def Prepare(options, sanitized_name):
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
                            options.package.replace('.', os.path.sep))
  if not os.path.exists(root_path):
    os.makedirs(root_path)
  dest_activity = sanitized_name + 'Activity.java'
  shutil.copyfile(src_activity, os.path.join(root_path, dest_activity))
  if options.app_root:
    assets_path = os.path.join(sanitized_name, 'assets')
    shutil.rmtree(assets_path)
    shutil.copytree(options.app_root, assets_path)


def EditElementAttribute(doc, node, name, value):
  item = doc.getElementsByTagName(node)[0]
  if item.hasAttribute(name):
    item.attributes[name].value = value
  else:
    item.setAttribute(name, value)


def AddElementAttribute(doc, node, name, value):
  root = doc.documentElement
  item = doc.createElement(node)
  item.setAttribute(name, value)
  root.appendChild(item)


def AddElementAttributeAndText(doc, node, name, value, data):
  root = doc.documentElement
  item = doc.createElement(node)
  item.setAttribute(name, value)
  text = doc.createTextNode(data)
  item.appendChild(text)
  root.appendChild(item)


def AddThemeStyle(doc, node, name, value):
  item = doc.getElementsByTagName(node)[0]
  src_str = item.attributes[name].value
  src_str = src_str + '.' + value
  item.attributes[name].value = src_str


def RemoveThemeStyle(doc, node, name, value):
  item = doc.getElementsByTagName(node)[0]
  dest_str = item.attributes[name].value.replace('.' + value, '')
  item.attributes[name].value = dest_str


def CustomizeStringXML(options, sanitized_name):
  strings_path = os.path.join(sanitized_name, 'res', 'values', 'strings.xml')
  if not os.path.isfile(strings_path):
    print ('Please make sure strings_xml'
           ' exists under app_src folder.')
    sys.exit(6)

  if options.description:
    xmldoc = minidom.parse(strings_path)
    AddElementAttributeAndText(xmldoc, 'string', 'name', 'description',
                               options.description)
    strings_file = open(strings_path, 'wb')
    xmldoc.writexml(strings_file)
    strings_file.close()


def CustomizeXML(options, sanitized_name):
  manifest_path = os.path.join(sanitized_name, 'AndroidManifest.xml')
  if not os.path.isfile(manifest_path):
    print ('Please make sure AndroidManifest.xml'
           ' exists under app_src folder.')
    sys.exit(6)

  CustomizeStringXML(options, sanitized_name)
  xmldoc = minidom.parse(manifest_path)
  EditElementAttribute(xmldoc, 'manifest', 'package', options.package)
  if options.app_version:
    EditElementAttribute(xmldoc, 'manifest', 'android:versionName',
                         options.app_version)
  if options.description:
    EditElementAttribute(xmldoc, 'manifest', 'android:description',
                         "@string/description")
  # TODO: Update the permission list after the permission
  # specification is defined.
  if options.permissions:
    if 'geolocation' in options.permissions:
      AddElementAttribute(xmldoc, 'uses-permission', 'android:name',
                          'android.permission.LOCATION_HARDWARE')
  EditElementAttribute(xmldoc, 'application', 'android:label', options.name)
  activity_name = options.package + '.' + sanitized_name + 'Activity'
  EditElementAttribute(xmldoc, 'activity', 'android:name', activity_name)
  EditElementAttribute(xmldoc, 'activity', 'android:label', options.name)
  if options.orientation:
    EditElementAttribute(xmldoc, 'activity', 'android:screenOrientation',
                         options.orientation)
  if options.fullscreen:
    AddThemeStyle(xmldoc, 'activity', 'android:theme', 'Fullscreen')
  else:
    RemoveThemeStyle(xmldoc, 'activity', 'android:theme', 'Fullscreen')
  if options.icon and os.path.isfile(options.icon):
    drawable_path = os.path.join(sanitized_name, 'res', 'drawable')
    if not os.path.exists(drawable_path):
      os.makedirs(drawable_path)
    icon_file = os.path.basename(options.icon)
    icon_file = ReplaceInvalidChars(icon_file)
    shutil.copyfile(options.icon, os.path.join(drawable_path, icon_file))
    icon_name = os.path.splitext(icon_file)[0]
    EditElementAttribute(xmldoc, 'application',
                         'android:icon', '@drawable/%s' % icon_name)
  elif options.icon and (not os.path.isfile(options.icon)):
    print ('Please make sure the icon file does exist!')
    sys.exit(6)

  file_handle = open(os.path.join(sanitized_name, 'AndroidManifest.xml'), 'wb')
  xmldoc.writexml(file_handle)
  file_handle.close()


def ReplaceString(file_path, src, dest):
  file_handle = open(file_path, 'r')
  src_content = file_handle.read()
  file_handle.close()
  file_handle = open(file_path, 'w')
  dest_content = src_content.replace(src, dest)
  file_handle.write(dest_content)
  file_handle.close()


def SetVariable(file_path, variable, value):
  function_string = ('%sset%s(%s);\n' %
                    ('        ', variable, value))
  temp_file_path = file_path + '.backup'
  file_handle = open(temp_file_path, 'w+')
  for line in open(file_path):
    file_handle.write(line)
    if (line.find('public void onCreate(Bundle savedInstanceState)') >= 0):
      file_handle.write(function_string)
  file_handle.close()
  shutil.move(temp_file_path, file_path)


def CustomizeJava(options, sanitized_name):
  root_path =  os.path.join(sanitized_name, 'src',
                            options.package.replace('.', os.path.sep))
  dest_activity = os.path.join(root_path, sanitized_name + 'Activity.java')
  ReplaceString(dest_activity, 'org.xwalk.app.template', options.package)
  ReplaceString(dest_activity, 'AppTemplate', sanitized_name)
  manifest_file = os.path.join(sanitized_name, 'assets', 'manifest.json')
  if os.path.isfile(manifest_file):
    ReplaceString(dest_activity,
                  'loadAppFromUrl("file:///android_asset/index.html")',
                  'loadAppFromManifest("file:///android_asset/manifest.json")')
  else:
    if options.app_url:
      if re.search(r'^http(|s)', options.app_url):
        ReplaceString(dest_activity, 'file:///android_asset/index.html',
                      options.app_url)
    elif options.app_local_path:
      if os.path.isfile(os.path.join(sanitized_name, 'assets',
                                     options.app_local_path)):
        ReplaceString(dest_activity, 'index.html', options.app_local_path)
      else:
        print ('Please make sure that the relative path of entry file'
               ' is correct.')
        sys.exit(8)

  if options.enable_remote_debugging:
    SetVariable(dest_activity, 'RemoteDebugging', 'true')


def CopyExtensionFile(extension_name, suffix, src_path, dest_path):
  # Copy the file from src_path into dest_path.
  dest_extension_path = os.path.join(dest_path, extension_name)
  if os.path.exists(dest_extension_path):
    # TODO: Refine it by renaming it internally.
    print ('Error: duplicated extension names "%s" are found. Please rename it.'
           % extension_name)
    sys.exit(9)
  else:
    os.mkdir(dest_extension_path)

  file_name = extension_name + suffix
  src_file = os.path.join(src_path, file_name)
  dest_file = os.path.join(dest_extension_path, file_name)
  if not os.path.isfile(src_file):
    sys.exit(9)
    print 'Error: %s is not found in %s.' % (file_name, src_path)
  else:
    shutil.copyfile(src_file, dest_file)


def CustomizeExtensions(options, sanitized_name):
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
  if not options.extensions:
    return
  apk_path = options.name
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
  extension_paths = options.extensions.split(os.pathsep)
  extension_json_list = []
  for source_path in extension_paths:
    if not os.path.exists(source_path):
      print 'Error: can\'t find the extension directory \'%s\'.' % source_path
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
      print 'Error: %s is not found in %s.' % (file_name, source_path)
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
        file_handle = open(manifest_path, 'wb')
        xmldoc.writexml(file_handle)
        file_handle.close()

  # Write configuration of extensions into the target extensions-config.json.
  if extension_json_list:
    extensions_string = json.JSONEncoder().encode(extension_json_list)
    extension_json_file = open(apk_extensions_json_path, 'w')
    extension_json_file.write(extensions_string)
    extension_json_file.close()


def main():
  parser = optparse.OptionParser()
  info = ('The package name. Such as: '
          '--package=com.example.YourPackage')
  parser.add_option('--package', help=info)
  info = ('The apk name. Such as: --name=YourApplicationName')
  parser.add_option('--name', help=info)
  info = ('The version of the app. Such as: --app-version=TheVersionNumber')
  parser.add_option('--app-version', help=info)
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
  options, _ = parser.parse_args()
  sanitized_name = ReplaceInvalidChars(options.name, 'apkname')
  try:
    Prepare(options, sanitized_name)
    CustomizeXML(options, sanitized_name)
    CustomizeJava(options, sanitized_name)
    CustomizeExtensions(options, sanitized_name)
  except SystemExit, ec:
    print 'Exiting with error code: %d' % ec.code
    return ec.code
  return 0


if __name__ == '__main__':
  sys.exit(main())
