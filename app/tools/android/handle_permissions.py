#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Provide the function interface of mapping the permissions
from manifest.json to AndroidManifest.xml.
It suppports the mapping of permission fields both defined in Crosswalk
and supported by Android Manifest specification.

Sample usage from shell script:
python permissions_mapping.py --jsonfile=/path/to/manifest.json
--manifest=/path/to/AndroidManifest.xml
"""

import optparse
import os
import sys

from handle_xml import AddElementAttribute
from manifest_json_parser import ManifestJsonParser
from xml.dom import minidom
from xml.parsers.expat import ExpatError

# The global permission mapping table.
# TODO: update the permission mapping table for added permission.
permission_mapping_table = {
  'contacts' : ['android.permission.READ_CONTACTS',
                'android.permission.WRITE_CONTACTS'],
  'geolocation' : ['android.permission.ACCESS_FINE_LOCATION'],
  'messaging' : ['android.permission.READ_SMS',
                 'android.permission.READ_PHONE_STATE',
                 'android.permission.RECEIVE_SMS',
                 'android.permission.SEND_SMS',
                 'android.permission.WRITE_SMS'],
  'devicecapabilities' : [],
  'fullscreen' : [],
  'presentation' : [],
  'rawsockets' : [],
  'screenorientation' : []
}


def HandlePermissions(options, xmldoc):
  """ Implement the mapping of permission list to the AndroidManifest.xml file.
  Args:
    options: the parsed options with permissions.
    xmldoc: the parsed xmldoc of the AndroidManifest.xml file, used for
        reading and writing.
  """
  if options.permissions:
    existing_permission_list = []
    used_permissions = xmldoc.getElementsByTagName("uses-permission")
    for item in used_permissions:
      existing_permission_list.append(item.getAttribute("android:name"))

    for permission in options.permissions.split(':'):
      if permission.lower() not in permission_mapping_table.keys():
        print 'Error: permission \'%s\' related API is not supported.' \
            % permission
        sys.exit(1)
      permission_item = permission_mapping_table.get(permission.lower())
      if permission_item:
        for android_permission in permission_item:
          if android_permission not in existing_permission_list:
            existing_permission_list.append(android_permission)
            AddElementAttribute(xmldoc, 'uses-permission',
                                'android:name', android_permission)


def main(argv):
  """Respond to command mode of the mapping permission list."""
  parser = optparse.OptionParser()
  info = ('The input json-format file name. Such as: '
          '--jsonfile=manifest.json')
  parser.add_option('-j', '--jsonfile', action='store', dest='jsonfile',
                    help=info)
  info = ('The destination android manifest file name. Such as: '
          '--manifest=AndroidManifest.xml')
  parser.add_option('-m', '--manifest', action='store', dest='manifest',
                    help=info)
  options, _ = parser.parse_args()
  if len(argv) == 1:
    parser.print_help()
    return 0
  if not options.jsonfile:
    print 'Please set the manifest.json file.'
    return 1
  if not options.manifest:
    print 'Please set the AndroidManifest.xml file.'
    return 1
  try:
    json_parser = ManifestJsonParser(os.path.expanduser(options.jsonfile))
    if json_parser.GetPermissions():
      options.permissions = json_parser.GetPermissions()
  except SystemExit, ec:
    print 'Exiting with error code: %d' % ec.code
    return ec.code
  try:
    xmldoc = minidom.parse(options.manifest)
    HandlePermissions(options, xmldoc)
    file_handle = open(options.manifest, 'wb')
    xmldoc.writexml(file_handle)
    file_handle.close()
  except (ExpatError, IOError):
    print 'There is an error in AndroidManifest.xml.'
    return 1
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
