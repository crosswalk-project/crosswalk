#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" 
Parse json-format manifest configuration file and 
provide the specific fields, which have to be integrated with 
packaging tool(e.g. make_apk.py) to generate xml-format manifest file.

Sample usage from shell script:
python manifest_json_parser.py --jsonfile=/path/to/manifest.json
"""

import json
import optparse
import os
import sys


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
    app_version:      The application version.
    icon_path:        The path of icon.
    app_url:          The url of application, e.g. hosted app.
    app_root:         The root path of the web, this flag allows to package
                      local web application as apk.
    app_local_path:   The relative path of entry file based on app_root,
                      this flag should work with "--app-root" together. 
    required_version: The required crosswalk runtime version.
    plugin:           The plug-in information.
    fullscreen:       The fullscreen flag of the application.
    """                
    ret_dict = {}
    ret_dict['app_name'] = self.data_src['name']
    ret_dict['app_version'] = self.data_src['version']
    file_path_prefix = os.path.split(self.input_path)[0]
    origin_icon_path = file_path_prefix
    # Get the relative path of icons.
    for key in self.data_src['icons']:
      icon_rel_path = (os.path.split(self.data_src['icons'][key]))[0]
      if len(icon_rel_path) != 0:
        icon_path = os.path.join(origin_icon_path, icon_rel_path)
      else:
        icon_path = origin_icon_path
    ret_dict['icon_path'] = icon_path
    app_root = file_path_prefix
    app_url = self.data_src['launch_path']
    if app_url.lower().startswith(('http', 'https')):
      app_local_path = ''
    else:
      app_local_path = app_url
      app_url = ''
    ret_dict['app_url'] = app_url
    ret_dict['app_root'] = app_root
    ret_dict['app_local_path'] = app_local_path
    ret_dict['required_version'] = self.data_src['required_version']
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
    print "app_version: %s" % self.GetAppVersion()
    print "icon_path: %s" % self.GetIconPath()
    print "app_url: %s" % self.GetAppUrl()
    print "app_root: %s" % self.GetAppRoot()
    print "app_local_path: %s" % self.GetAppLocalPath()
    print "required_version: %s" % self.GetRequiredVersion()
    print "plugins: %s" % self.GetPlugins()
    print "fullscreen: %s" % self.GetFullScreenFlag()
    
  def GetAppName(self):
    """Return the application name."""
    return self.ret_dict['app_name']
    
  def GetAppVersion(self):
    """Return the application version."""
    return self.ret_dict['app_version']
    
  def GetIconPath(self):
    """Return the icon path."""
    return self.ret_dict['icon_path']
    
  def GetAppUrl(self):
    """Return the URL of the application."""
    return self.ret_dict['app_url']
    
  def GetAppRoot(self):
    """Return the root path of the local web application."""
    return self.ret_dict['app_root']
  
  def GetAppLocalPath(self):
    """Return the local relative path of the local web application."""
    return self.ret_dict['app_local_path']
    
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