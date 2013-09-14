#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Generate Tizen pkginfo compatible XML data file from Crosswalk manifest.json.
Sample usage:
install_into_pkginfo_db.py --install --pkgid |package_id| \
[--datapath |data_path|]
"""
import argparse
import json
import os
import pwd
import re
import shutil
import string
import traceback
from xml.dom import minidom

class InstallHelper(object):
  def __init__(self, package_id, data_path):
    """
    package_id_    : Crosswalk application ID.
    data_path_     : directory header of all Crosswalk applications.
    app_path_      : directory of installed Crosswalk application.
    xml_path_      : path where the pkginfo compatible XML should be written.
                     /opt/share/packages/|package_id_|.xml
    stripped_name_ : Crosswalk application name with white spaces stripped.
    icon_path_     : location of application icon,
                     /opt/share/icons/default/small/|package_id_| \
                     .|stripped_name_|.png
    execute_path_  : symbol link to Crosswalk binary,
                     /opt/usr/apps/applications/|package_id_|/bin/|package_id_|
    """
    self.package_id_ = package_id
    self.data_path_ = data_path
    self.app_path_ = data_path + '/applications/' + package_id + '/'
    self.xml_path_ = '/opt/share/packages/' + self.package_id_ + '.xml'
    self.execute_path_ = '/opt/usr/apps/applications/' + self.package_id_ + \
                         '/bin/' + self.package_id_
    try:
      manifest_path = self.app_path_ + '/manifest.json'
      input_file = file(manifest_path)
      input_source = input_file.read()
      self.data_ = json.JSONDecoder().decode(input_source)
    except IOError:
      traceback.print_exc()
    finally:
      input_file.close()

    if 'name' in self.data_:
      self.stripped_name_ = string.replace(self.data_['name'], ' ', '')
      self.icon_path_ = '/opt/share/icons/default/small/' + \
                        self.package_id_ + '.' + self.stripped_name_ + '.png'
    else:
      self.icon_path_ = '/opt/share/icons/default/small/' + \
                        self.package_id_ + '.png'

  def Show(self):
    print('Package with ID: %s, under directory: %s'
          % (self.package_id_, self.app_path_))

  def GeneratePkgInfoXML(self):
    """Generate XML file from manifest.json,
    package  : |package_id_|
    exec     : /opt/usr/apps/applications/|package_id_|/bin/|package_id_|
    icon     : value is from |icons| with key |'128'|,
               key '128' is required by manifest.json.
    type     : c++app
    """
    try:
      dir_output = os.path.dirname(self.xml_path_)
      if not os.path.exists(dir_output):
        os.makedirs(dir_output)
      output_file = open(self.xml_path_, 'w')
      document = minidom.Document()
      manifest = self.__CreateNode(document, document, 'manifest')
      self.__SetAttribute(manifest, 'xmlns', 'http://tizen.org/ns/packages')
      self.__SetAttribute(manifest, 'package', self.package_id_)
      if 'version' in self.data_:
        self.__SetAttribute(manifest, 'version', self.data_['version'])

      label = self.__CreateNode(document, manifest, 'label')
      if 'name' in self.data_:
        self.__CreateTextNode(document, label, self.data_['name'])

      description = self.__CreateNode(document, manifest, 'description')
      if 'description' in self.data_:
        self.__CreateTextNode(document, description, self.data_['description'])

      ui_application = self.__CreateNode(document, manifest, 'ui-application')
      self.__SetAttribute(ui_application, 'appid',
                          self.package_id_ + '.' + self.stripped_name_)
      self.__SetAttribute(ui_application, 'exec', self.execute_path_)
      # Set application type to 'c++app' for now,
      # to differentiate from 'webapp' used by legacy Tizen web applications.
      self.__SetAttribute(ui_application, 'type', 'c++app')
      self.__SetAttribute(ui_application, 'taskmanage', 'true')

      ui_label = self.__CreateNode(document, ui_application, 'label')
      if 'name' in self.data_:
        self.__CreateTextNode(document, ui_label, self.data_['name'])

      if ('name' in self.data_ and
          'icons' in self.data_ and
          '128' in self.data_['icons'] and
          os.path.exists(self.app_path_ + self.data_['icons']['128'])):
        icon = self.__CreateNode(document, ui_application, 'icon')
        self.__CreateTextNode(
            document,
            icon,
            self.package_id_ + '.' + self.stripped_name_ + '.png')

      text_re = re.compile('>\n\s+([^<>\s].*?)\n\s+</', re.DOTALL)
      pretty_xml = text_re.sub('>\g<1></', document.toprettyxml(indent='  '))
      output_file.write(pretty_xml)
      print('Converting manifest.json into %s.xml for installation[DONE]'
            % self.package_id_)
    except IOError:
      traceback.print_exc()

  def CopyOrLinkResources(self):
    """Prepare required resources for home screen launch.
    """
    try:
      if ('icons' in self.data_ and
          '128' in self.data_['icons']):
        icon = self.app_path_ + '/' + self.data_['icons']['128']
        if os.path.exists(icon):
          shutil.copy2(icon, self.icon_path_)

      xwalk_path = '/usr/lib/xwalk/xwalk'
      exec_dir = os.path.dirname(self.execute_path_)
      if not os.path.exists(exec_dir):
        os.makedirs(exec_dir)
      if not os.path.lexists(self.execute_path_):
        os.symlink(xwalk_path, self.execute_path_)

      print('Copying and linking files into correct locations [DONE]')
    except IOError:
      traceback.print_exc()

  def InstallPkgInfoDB(self):
    """Install pkginfo compatible XML data into database."""
    try:
      (user_id, group_id) = self.__GetNumericID('app', 'app')
      # Application is installed under |args.datapath|/applications
      # and |args.datapath|/applications_db
      self.__ChangeOwner(self.data_path_ + '/applications',
                         user_id, group_id)
      self.__ChangeOwner(self.data_path_ + '/applications_db',
                         user_id, group_id)
      if os.path.exists(self.xml_path_):
        command = '/usr/bin/pkginfo --imd ' + self.xml_path_
        os.system(command)
    except OSError:
      traceback.print_exc()

  def Uninstall(self):
    if os.path.exists(self.xml_path_):
      command = '/usr/bin/pkginfo --rmd ' + self.xml_path_
      os.system(command)
    try:
      for file_path in (self.icon_path_, self.execute_path_, self.xml_path_):
        if os.path.exists(file_path):
          os.remove(file_path)
      print('Removing and unlinking files from installed locations [DONE]')
    except OSError:
      traceback.print_exec()

  @classmethod
  def __CreateNode(cls, document, parentNode, nodeName):
    node = document.createElement(nodeName)
    if not parentNode is None:
      parentNode.appendChild(node)
      return node

  @classmethod
  def __CreateTextNode(cls, document, parentNode, nodeName):
    node = document.createTextNode(nodeName)
    if not parentNode is None:
      parentNode.appendChild(node)
      return node

  @classmethod
  def __SetAttribute(cls, node, attrName, attrValue):
    if not node is None:
      node.setAttribute(attrName, attrValue)

  @classmethod
  def __GetNumericID(cls, user, group):
    user_id = pwd.getpwnam(user).pw_uid
    group_id = pwd.getpwnam(group).pw_gid
    return (user_id, group_id)

  @classmethod
  def __ChangeOwner(cls, path, uid, gid):
    os.lchown(path, uid, gid)
    if os.path.isdir(path):
      for item in os.listdir(path):
        itempath = os.path.join(path, item)
        cls.__ChangeOwner(itempath, uid, gid)


def main():
  parser = argparse.ArgumentParser(
      description='pass arguments from schell script to InstallHelper')
  parser.add_argument('-i', '--install', action='store_true')
  parser.add_argument('-u', '--uninstall', action='store_true')
  parser.add_argument('-p', '--pkgid', help='Package ID', required=True)
  parser.add_argument(
      '-d', '--datapath',
      help='Directory path to Crosswalk data',
      default='/opt/usr/apps/')
  args = parser.parse_args()

  installer = InstallHelper(args.pkgid, args.datapath)
  installer.Show()
  if args.install:
    installer.GeneratePkgInfoXML()
    installer.CopyOrLinkResources()
    if os.path.exists(args.datapath):
      installer.InstallPkgInfoDB()
  if args.uninstall:
    installer.Uninstall()


if __name__ == '__main__':
  main()
