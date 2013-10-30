#nH!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
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
  permissions = permission_list
  if len(permission_list) > 1:
    permissions = permission_list[0]
    index = 1
    while index < len(permission_list):
      permissions = permissions + ":" + permission_list[index]
      index += 1
  return permissions


class ManifestJsonParser(object):
  """ The class is used to parse json-format manifest file, recompose the fields
  and provide the field interfaces required by the packaging tool.

  Args:
    input_path: the full path of the json-format manifest file.
  """
  def __init__(self, input_path):
    self.input_path = input_path
    input_file = file(self.input_path)
    try:
      input_src = input_file.read()
      self.data_src = json.JSONDecoder().decode(input_src)
      self.ret_dict = self._output_items()
    except IOError:
      raise Exception('Error in decoding json-format manifest file.')
    except KeyError:
      raise Exception('Wrong keyword in json-format manifest file.')
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
    required_version: The required crosswalk runtime version.
    plugin:           The plug-in information.
    fullscreen:       The fullscreen flag of the application.
    """
    ret_dict = {}
    ret_dict['app_name'] = self.data_src['name']
    ret_dict['version'] = self.data_src['version']
    if self.data_src.has_key('launch_path'):
      app_url = self.data_src['launch_path']
    elif self.data_src.has_key('local_path'):
      app_url = self.data_src['app']['launch']['local_path']
    else:
      app_url = ''
    if app_url.lower().startswith(('http', 'https')):
      app_local_path = ''
    else:
      app_local_path = app_url
      app_url = ''
    file_path_prefix = os.path.split(self.input_path)[0]
    if self.data_src.has_key('icons'):
      ret_dict['icons'] = self.data_src['icons']
    else:
      ret_dict['icons'] = ''
    app_root = file_path_prefix
    ret_dict['description'] = ''
    if self.data_src.has_key('description'):
      ret_dict['description'] = self.data_src['description']
    ret_dict['app_url'] = app_url
    ret_dict['app_root'] = app_root
    ret_dict['app_local_path'] = app_local_path
    ret_dict['permissions'] = ''
    if self.data_src.has_key('permissions'):
      permission_list = self.data_src['permissions']
      ret_dict['permissions'] = HandlePermissionList(permission_list)
    ret_dict['required_version'] = ''
    if self.data_src.has_key('required_version'):
      ret_dict['required_version'] = self.data_src['required_version']
    ret_dict['plugin'] = ''
    if self.data_src.has_key('plugin'):
      ret_dict['plugin'] = self.data_src['plugin']
    if self.data_src.has_key('fullscreen'):
      ret_dict['fullscreen'] = self.data_src['fullscreen']
    else:
      ret_dict['fullscreen'] = 'False'
    return ret_dict

  def ShowItems(self):
    """Show the processed results, it is used for command-line
    internal debugging."""
    print "app_name: %s" % self.GetAppName()
    print "version: %s" % self.GetVersion()
    print "description: %s" % self.GetDescription()
    print "icons: %s" % self.GetIcons()
    print "app_url: %s" % self.GetAppUrl()
    print "app_root: %s" % self.GetAppRoot()
    print "app_local_path: %s" % self.GetAppLocalPath()
    print "permissions: %s" % self.GetPermissions()
    print "required_version: %s" % self.GetRequiredVersion()
    print "plugins: %s" % self.GetPlugins()
    print "fullscreen: %s" % self.GetFullScreenFlag()

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

  def GetRequiredVersion(self):
    """Return the required crosswalk runtime version."""
    return self.ret_dict['required_version']

  def GetPlugins(self):
    """Return the plug-in path and file name."""
    return self.ret_dict['plugin']

  def GetFullScreenFlag(self):
    """Return the set fullscreen flag of the application."""
    return self.ret_dict['fullscreen']


def main():
  """Respond to command mode and show the processed field values."""
  parser = optparse.OptionParser()
  info = ('The input json-format file name. Such as: '
          '--jsonfile=manifest.json')
  parser.add_option('-j', '--jsonfile', action='store', dest='jsonfile',
                    help=info)
  opts, _ = parser.parse_args()
  json_parser = ManifestJsonParser(opts.jsonfile)
  json_parser.ShowItems()
  return 0


if __name__ == '__main__':
  sys.exit(main())
