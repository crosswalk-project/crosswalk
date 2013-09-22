#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

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
    invalid_chars = '\/:*?"<>|-'
  for c in invalid_chars:
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


def ReplaceNodeValue(doc, node, name, value):
  item = doc.getElementsByTagName(node)[0]
  item.attributes[name].value = value


def AddAttribute(doc, node, name, value):
  item = doc.getElementsByTagName(node)[0]
  item.setAttribute(name, value)


def AddThemeStyle(doc, node, name, value):
  item = doc.getElementsByTagName(node)[0]
  src_str = item.attributes[name].value
  src_str = src_str + '.' + value
  item.attributes[name].value = src_str


def RemoveThemeStyle(doc, node, name, value):
  item = doc.getElementsByTagName(node)[0]
  dest_str = item.attributes[name].value.replace('.' + value, '')
  item.attributes[name].value = dest_str


def CustomizeXML(options, sanitized_name):
  manifest_path = os.path.join(sanitized_name, 'AndroidManifest.xml')
  if not os.path.isfile(manifest_path):
    print ('Please make sure AndroidManifest.xml'
           ' exists under app_src folder.')
    sys.exit(6)

  xmldoc = minidom.parse(manifest_path)
  ReplaceNodeValue(xmldoc, 'manifest', 'package', options.package)
  ReplaceNodeValue(xmldoc, 'application', 'android:label', options.name)
  activity_name = options.package + '.' + sanitized_name + 'Activity'
  ReplaceNodeValue(xmldoc, 'activity', 'android:name', activity_name)
  ReplaceNodeValue(xmldoc, 'activity', 'android:label', options.name)
  if options.fullscreen:
    AddThemeStyle(xmldoc, 'activity', 'android:theme', 'Fullscreen')
  else:
    RemoveThemeStyle(xmldoc, 'activity', 'android:theme', 'Fullscreen')
  if options.icon:
    drawable_path = os.path.join(sanitized_name, 'res', 'drawable')
    if not os.path.exists(drawable_path):
      os.makedirs(drawable_path)
    icon_file = os.path.basename(options.icon)
    icon_file = ReplaceInvalidChars(icon_file)
    shutil.copyfile(options.icon, os.path.join(drawable_path, icon_file))
    icon_name = os.path.splitext(icon_file)[0]
    AddAttribute(xmldoc, 'application',
                 'android:icon', '@drawable/%s' % icon_name)

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
  if options.app_url:
    if re.search(r'^http(|s)', options.app_url):
      ReplaceString(dest_activity, 'file:///android_asset/index.html',
                    options.app_url)
  elif options.app_local_path:
    if os.path.isfile(os.path.join(sanitized_name, 'assets',
                                   options.app_local_path)):
      ReplaceString(dest_activity, 'index.html', options.app_local_path)
    else:
      print ('Please make sure that the reletive path of entry file'
             ' is correct.')
      sys.exit(8)
  if options.enable_remote_debugging:
    SetVariable(dest_activity, 'RemoteDebugging', 'true')


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
  parser.add_option('--enable-remote-debugging', action='store_true',
                    dest='enable_remote_debugging', default=False,
                    help = 'Enable remote debugging.')
  parser.add_option('-f', '--fullscreen', action='store_true',
                    dest='fullscreen', default=False,
                    help='Make application fullscreen.')
  options, _ = parser.parse_args()
  sanitized_name = ReplaceInvalidChars(options.name)
  try:
    Prepare(options, sanitized_name)
    CustomizeXML(options, sanitized_name)
    CustomizeJava(options, sanitized_name)
  except SystemExit, ec:
    print 'Exiting with error code: %d' % ec.code
    return ec.code
  return 0


if __name__ == '__main__':
  sys.exit(main())
