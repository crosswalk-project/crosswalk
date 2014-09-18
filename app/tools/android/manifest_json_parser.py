#!/usr/bin/env python

# Copyright (c) 2013, 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Parse JSON-format manifest configuration file and
provide the specific fields, which have to be integrated with
packaging tool(e.g. make_apk.py) to generate xml-format manifest file.

Sample usage from shell script:
python manifest_json_parser.py --jsonfile=/path/to/manifest.json
"""

import json
import optparse
import os
import re
import sys


def HandlePermissionList(permission_list):
  """This function is used to handle the permission list and return the string
  of permissions.

  Args:
    permission_list: the permission list, e.g.["permission1", "permission2"].

  Returns:
    The string of permissions with ':' as separator.
    e.g. "permission1:permission2".
  """
  permissions = list(permission_list)
  reg_permission = re.compile(r'^[a-zA-Z\.]*$')
  for permission in permissions:
    if not reg_permission.match(permission):
      print('\'Permissions\' field error, only alphabets and '
            '\'.\' are allowed.')
      sys.exit(1)
  return ':'.join(permissions)


def ParseLaunchScreen(ret_dict, launch_screen_dict, orientation):
  if orientation in launch_screen_dict:
    sub_dict = launch_screen_dict[orientation]
    if 'background_color' in sub_dict:
      ret_dict['launch_screen_background_color_' + orientation] = (
          sub_dict['background_color'])
    if 'background_image' in sub_dict:
      ret_dict['launch_screen_background_image_' + orientation] = (
          sub_dict['background_image'])
    if 'image' in sub_dict:
      ret_dict['launch_screen_image_' + orientation] = (
          sub_dict['image'])
    if 'image_border' in sub_dict:
      ret_dict['launch_screen_image_border_' + orientation] = (
          sub_dict['image_border'])


def PrintDeprecationWarning(deprecated_items):
  if len(deprecated_items) > 0:
    print ('  Warning: The following fields have been deprecated for '
           'Crosswalk:\n   %s' %
           ', '.join([str(item) for item in deprecated_items]))
    print ('  Please follow: https://www.crosswalk-project.org/#documentation/'
           'manifest.')


class ManifestJsonParser(object):
  """ The class is used to parse json-format manifest file, recompose the
  fields and provide the field interfaces required by the packaging tool.

  Args:
    input_path: the full path of the json-format manifest file.
  """
  def __init__(self, input_path):
    self.input_path = input_path
    input_file = open(self.input_path)
    try:
      input_src = input_file.read()
      self.data_src = json.JSONDecoder().decode(input_src)
      self.ret_dict = self._output_items()
    except (TypeError, ValueError, IOError) as error:
      print('There is a parser error in manifest.json file: %s' % error)
      sys.exit(1)
    except KeyError as error:
      print('There is a field error in manifest.json file: %s' % error)
      sys.exit(1)
    finally:
      input_file.close()

  def _output_items(self):
    """ The manifest field items are reorganized and returned as a
    dictionary to support single or multiple values of keys.

    Returns:
      A dictionary to the corresponding items. the dictionary keys are
      described as follows, the value is set to "" if the value of the
      key is not set.
    app_name:         The application name.
    version:          The version number.
    icons:            An array of icons.
    app_url:          The url of application, e.g. hosted app.
    description:      The description of application.
    app_root:         The root path of the web, this flag allows to package
                      local web application as apk.
    app_local_path:   The relative path of entry file based on app_root,
                      this flag should work with "--app-root" together.
    permissions:      The permission list.
    orientation       The default allowed orientations.
    fullscreen:       The fullscreen flag of the application.
    launch_screen:    The launch screen configuration.
    """
    print ("Checking manifest file")
    ret_dict = {}
    deprecated_items = []
    if 'name' not in self.data_src:
      print('Error: no \'name\' field in manifest.json file.')
      sys.exit(1)
    ret_dict['app_name'] = self.data_src['name']
    ret_dict['version'] = ''
    if 'version' in self.data_src and 'xwalk_version' in self.data_src:
      print('WARNING: the value in "version" will be ignored and support '
            'for it will be removed in the future.')
      ret_dict['version'] = self.data_src['xwalk_version']
    elif 'xwalk_version' in self.data_src:
      ret_dict['version'] = self.data_src['xwalk_version']
    elif 'version' in self.data_src:
      deprecated_items.append('version')
      ret_dict['version'] = self.data_src['version']
    if 'start_url' in self.data_src:
      app_url = self.data_src['start_url']
    elif 'launch_path' in self.data_src:
      deprecated_items.append('launch_path')
      app_url = self.data_src['launch_path']
    elif ('app' in self.data_src and
          'launch' in self.data_src['app'] and
          'local_path' in self.data_src['app']['launch']):
      deprecated_items.append('app.launch.local_path')
      app_url = self.data_src['app']['launch']['local_path']
    else:
      app_url = ''
    if app_url.lower().startswith(('http://', 'https://')):
      app_local_path = ''
    else:
      app_local_path = app_url
      app_url = ''
    file_path_prefix = os.path.split(self.input_path)[0]
    if 'icons' in self.data_src:
      icons = self.data_src['icons']
      if type(icons) == dict:
        deprecated_items.append('icons defined as index:value')
        ret_dict['icons'] = icons
      elif type(icons) == list:
        icons_dict = {}
        for icon in icons:
          if 'sizes' in icon and 'src' in icon:
            icons_dict[icon['sizes'].split('x')[0]] = icon['src']
        ret_dict['icons'] = icons_dict
      else:
        ret_dict['icons'] = {}
    else:
      ret_dict['icons'] = {}
    app_root = file_path_prefix
    ret_dict['description'] = ''
    if 'description' in self.data_src and 'xwalk_description' in self.data_src:
      print('WARNING: the value in "description" will be ignored and support '
            'for it will be removed in the future.')
      ret_dict['description'] = self.data_src['xwalk_description']
    elif 'xwalk_description' in self.data_src:
      ret_dict['description'] = self.data_src['xwalk_description']
    elif 'description' in self.data_src:
      deprecated_items.append('description')
      ret_dict['description'] = self.data_src['description']
    ret_dict['app_url'] = app_url
    ret_dict['app_root'] = app_root
    ret_dict['app_local_path'] = app_local_path
    ret_dict['permissions'] = ''
    if 'xwalk_permissions' in self.data_src:
      try:
        permission_list = self.data_src['xwalk_permissions']
        ret_dict['permissions'] = HandlePermissionList(permission_list)
      except (TypeError, ValueError, IOError):
        print('\'Permissions\' field error in manifest.json file.')
        sys.exit(1)
    elif 'permissions' in self.data_src:
      deprecated_items.append('permissions')
      try:
        permission_list = self.data_src['permissions']
        ret_dict['permissions'] = HandlePermissionList(permission_list)
      except (TypeError, ValueError, IOError):
        print('\'Permissions\' field error in manifest.json file.')
        sys.exit(1)
    orientation = {'landscape':'landscape',
                   'landscape-primary':'landscape',
                   'landscape-secondary':'reverseLandscape',
                   'portrait':'portrait',
                   'portrait-primary':'portrait',
                   'portrait-secondary':'reversePortrait',
                   'any':'unspecified',
                   'natural':'unspecified'}
    if 'orientation' in self.data_src:
      if self.data_src['orientation'] in orientation:
        ret_dict['orientation'] = orientation[self.data_src['orientation']]
      else:
        ret_dict['orientation'] = 'unspecified'
    else:
      ret_dict['orientation'] = 'unspecified'
    if 'display' in self.data_src and 'fullscreen' in self.data_src['display']:
      ret_dict['fullscreen'] = 'true'
    else:
      ret_dict['fullscreen'] = ''
    if 'xwalk_launch_screen' in self.data_src:
      launch_screen_dict = self.data_src['xwalk_launch_screen']
      ParseLaunchScreen(ret_dict, launch_screen_dict, 'default')
      ParseLaunchScreen(ret_dict, launch_screen_dict, 'portrait')
      ParseLaunchScreen(ret_dict, launch_screen_dict, 'landscape')
    elif 'launch_screen' in self.data_src:
      deprecated_items.append('launch_screen')
      launch_screen_dict = self.data_src['launch_screen']
      ParseLaunchScreen(ret_dict, launch_screen_dict, 'default')
      ParseLaunchScreen(ret_dict, launch_screen_dict, 'portrait')
      ParseLaunchScreen(ret_dict, launch_screen_dict, 'landscape')

    PrintDeprecationWarning(deprecated_items)
    return ret_dict

  def ShowItems(self):
    """Show the processed results, it is used for command-line
    internal debugging."""
    print("app_name: %s" % self.GetAppName())
    print("version: %s" % self.GetVersion())
    print("description: %s" % self.GetDescription())
    print("icons: %s" % self.GetIcons())
    print("app_url: %s" % self.GetAppUrl())
    print("app_root: %s" % self.GetAppRoot())
    print("app_local_path: %s" % self.GetAppLocalPath())
    print("permissions: %s" % self.GetPermissions())
    print("orientation: %s" % self.GetOrientation())
    print("fullscreen: %s" % self.GetFullScreenFlag())
    print('launch_screen.default.background_color: %s' %
          self.GetLaunchScreenBackgroundColor('default'))
    print('launch_screen.default.background_image: %s' %
          self.GetLaunchScreenBackgroundImage('default'))
    print('launch_screen.default.image: %s' %
          self.GetLaunchScreenImage('default'))
    print('launch_screen.default.image_border: %s' %
          self.GetLaunchScreenImageBorder('default'))
    print('launch_screen.portrait.background_color: %s' %
          self.GetLaunchScreenBackgroundColor('portrait'))
    print('launch_screen.portrait.background_image: %s' %
          self.GetLaunchScreenBackgroundImage('portrait'))
    print('launch_screen.portrait.image: %s' %
          self.GetLaunchScreenImage('portrait'))
    print('launch_screen.portrait.image_border: %s' %
          self.GetLaunchScreenImageBorder('portrait'))
    print('launch_screen.landscape.background_color: %s' %
          self.GetLaunchScreenBackgroundColor('landscape'))
    print('launch_screen.landscape.background_image: %s' %
          self.GetLaunchScreenBackgroundImage('landscape'))
    print('launch_screen.landscape.image: %s' %
          self.GetLaunchScreenImage('landscape'))
    print('launch_screen.landscape.image_border: %s' %
          self.GetLaunchScreenImageBorder('landscape'))

  def GetAppName(self):
    """Return the application name."""
    return self.ret_dict['app_name']

  def GetVersion(self):
    """Return the version number."""
    return self.ret_dict['version']

  def GetIcons(self):
    """Return the icons."""
    return self.ret_dict['icons']

  def GetAppUrl(self):
    """Return the URL of the application."""
    return self.ret_dict['app_url']

  def GetDescription(self):
    """Return the description of the application."""
    return self.ret_dict['description']

  def GetAppRoot(self):
    """Return the root path of the local web application."""
    return self.ret_dict['app_root']

  def GetAppLocalPath(self):
    """Return the local relative path of the local web application."""
    return self.ret_dict['app_local_path']

  def GetPermissions(self):
    """Return the permissions."""
    return self.ret_dict['permissions']

  def GetOrientation(self):
    """Return the default allowed orientations"""
    return self.ret_dict['orientation']

  def GetFullScreenFlag(self):
    """Return the set fullscreen flag of the application."""
    return self.ret_dict['fullscreen']

  def GetLaunchScreenBackgroundColor(self, orientation):
    """Return the background color for launch_screen."""
    key = 'launch_screen_background_color_' + orientation
    return self.ret_dict.get(key, '')

  def GetLaunchScreenBackgroundImage(self, orientation):
    """Return the background image for launch_screen."""
    key = 'launch_screen_background_image_' + orientation
    return self.ret_dict.get(key, '')

  def GetLaunchScreenImage(self, orientation):
    """Return the image for launch_screen."""
    key = 'launch_screen_image_' + orientation
    return self.ret_dict.get(key, '')

  def GetLaunchScreenImageBorder(self, orientation):
    """Return the image border for launch_screen."""
    key = 'launch_screen_image_border_' + orientation
    return self.ret_dict.get(key, '')


def main(argv):
  """Respond to command mode and show the processed field values."""
  parser = optparse.OptionParser()
  info = ('The input json-format file name. Such as: '
          '--jsonfile=manifest.json')
  parser.add_option('-j', '--jsonfile', action='store', dest='jsonfile',
                    help=info)
  opts, _ = parser.parse_args()
  if len(argv) == 1:
    parser.print_help()
    return 0
  json_parser = ManifestJsonParser(opts.jsonfile)
  json_parser.ShowItems()
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
