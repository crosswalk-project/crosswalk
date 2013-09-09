#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Generate Tizen pkginfo compatible XML data file from Crosswalk manifest.json.
Sample usage:
install_into_pkginfo_db.py -i |input_path| -p |package_id| [-d |data_path|]
"""
import argparse
import json
import os
import pwd
import re
import shutil
import traceback
from xml.dom import minidom


class InstallHelper(object):
  def __init__(self, package_id, input_path, data_path):
    """
    package_id_  : Crosswalk application ID.
    input_path_  : file path to the Crosswalk compatible manifest.json.
    output_path_ : path where the pkginfo compatible XML should be written.
    data_path_   : directory of installed Crosswalk application.
    """
    self.package_id_ = package_id
    self.input_path_ = input_path
    self.output_path_ = "/opt/share/packages/" + self.package_id_ + ".xml"
    self.data_path_ = data_path + "/applications/" + package_id + "/"
    try:
      input_file = file(self.input_path_)
      input_source = input_file.read()
      self.data_ = json.JSONDecoder().decode(input_source)
    except IOError:
      traceback.print_exc()
    finally:
      input_file.close()

  def show(self):
    print("Installing package with ID: %s, under directory: %s"
          % (self.package_id_, self.data_path_))

  def generatePkgInfoXML(self):
    """Generate XML file from manifest.json,
    package  : |package_id_|
    exec     : /opt/usr/apps/applications/|package_id_|/bin/|package_id_|
    icon     : value is from |icons| with key |'128'|,
               key '128' is required by manifest.json.
    type     : c++app
    """
    try:
      dir_output = os.path.dirname(self.output_path_)
      if not os.path.exists(dir_output):
        os.makedirs(dir_output)
      output_file = open(self.output_path_, "w")
      document = minidom.Document()
      manifest = self.__createNode(document, document, "manifest")
      self.__setAttribute(manifest, "xmlns", "http://tizen.org/ns/packages")
      self.__setAttribute(manifest, "package", self.package_id_)
      if 'version' in self.data_:
        self.__setAttribute(manifest, "version", self.data_['version'])

      label = self.__createNode(document, manifest, "label")
      if 'name' in self.data_:
        self.__createTextNode(document, label, self.data_['name'])

      description = self.__createNode(document, manifest, "description")
      if 'description' in self.data_:
        self.__createTextNode(document, description, self.data_['description'])

      ui_application = self.__createNode(document, manifest, "ui-application")
      self.__setAttribute(ui_application, "appid", self.package_id_)
      self.__setAttribute(ui_application, "exec", "/opt/usr/apps/applications/"
                          + self.package_id_ + "/bin/" + self.package_id_)
      # Set application type to "c++app" for now,
      # to differentiate from "webapp" used by legacy Tizen web applications.
      self.__setAttribute(ui_application, "type", "c++app")
      self.__setAttribute(ui_application, "taskmanage", "true")

      ui_label = self.__createNode(document, ui_application, "label")
      if 'name' in self.data_:
        self.__createTextNode(document, ui_label, self.data_['name'])

      if ('name' in self.data_ and
          'icons' in self.data_ and
          '128' in self.data_['icons'] and
          os.path.exists(self.data_path_ + self.data_['icons']['128'])):
        icon = self.__createNode(document, ui_application, "icon")
        self.__createTextNode(
            document,
            icon,
            self.package_id_ + "." + self.data_['name'] + ".png")

      text_re = re.compile('>\n\s+([^<>\s].*?)\n\s+</', re.DOTALL)
      pretty_xml = text_re.sub('>\g<1></', document.toprettyxml(indent="  "))
      output_file.write(pretty_xml)
      print("Converting manifest.json into %s.xml for installation[DONE]"
            % self.package_id_)

      return self.output_path_
    except IOError:
      traceback.print_exc()

  def copyOrLinkResources(self):
    """Prepare required resources for home screen launch.
    icon path    : location of application icon,
                   /opt/share/icons/default/small/|package_id|.|name|.png
    execute path : symbol link to Crosswalk binary,
                   /opt/usr/apps/applications/|package_id_|/bin/|package_id_|
    XML path     : /opt/share/packages/|package_id_|.xml
    """
    try:
      if 'name' in self.data_:
        icon_path = "/opt/share/icons/default/small/" + self.package_id_ +\
                    "." + self.data_['name'] + ".png"
      if ('icons' in self.data_ and
          '128' in self.data_['icons']):
        icon = self.data_path_ + "/" + self.data_['icons']['128']
        if os.path.exists(icon):
          shutil.copy2(icon, icon_path)

      xwalk_path = "/usr/lib/xwalk/xwalk"
      install_dir = "/opt/usr/apps/applications/" + self.package_id_ + "/bin/"
      install_path = install_dir + self.package_id_
      if not os.path.exists(install_dir):
        os.makedirs(install_dir)
      if not os.path.lexists(install_path):
        os.symlink(xwalk_path, install_path)

      print("Copying and linking files into correct locations [DONE]")
    except IOError:
      traceback.print_exc()

  @classmethod
  def installPkgInfoDB(cls, data_path, xml_file):
    """Install pkginfo compatible XML data into database."""
    try:
      (user_id, group_id) = cls.__getNumericID("app", "app")
      # Application is installed under |args.datapath|/applications
      # and |args.datapath|/applications_db
      cls.__changeOwner(data_path + "/applications", user_id, group_id)
      cls.__changeOwner(data_path + "/applications_db", user_id, group_id)
      if os.path.exists(xml_file):
        command = "/usr/bin/pkginfo --imd " + xml_file
        os.system(command)
    except OSError:
      traceback.print_exc()

  @classmethod
  def __createNode(cls, document, parentNode, nodeName):
    node = document.createElement(nodeName)
    if not parentNode is None:
      parentNode.appendChild(node)
      return node

  @classmethod
  def __createTextNode(cls, document, parentNode, nodeName):
    node = document.createTextNode(nodeName)
    if not parentNode is None:
      parentNode.appendChild(node)
      return node

  @classmethod
  def __setAttribute(cls, node, attrName, attrValue):
    if not node is None:
      node.setAttribute(attrName, attrValue)

  @classmethod
  def __getNumericID(cls, user, group):
    user_id = pwd.getpwnam(user).pw_uid
    group_id = pwd.getpwnam(group).pw_gid
    return (user_id, group_id)

  @classmethod
  def __changeOwner(cls, path, uid, gid):
    os.lchown(path, uid, gid)
    if os.path.isdir(path):
      for item in os.listdir(path):
        itempath = os.path.join(path, item)
        cls.__changeOwner(itempath, uid, gid)


def main():
  parser = argparse.ArgumentParser(
      description='pass arguments from schell script to InstallHelper')
  parser.add_argument(
      '-i', '--input',
      help='File path to Crosswalk compatible manifest.json',
      required=True)
  parser.add_argument('-p', '--pkgid', help='Package ID', required=True)
  parser.add_argument(
      '-d', '--datapath',
      help="Directory path to Crosswalk data",
      default="/home/app/xwalk/")
  args = parser.parse_args()

  installer = InstallHelper(args.pkgid, args.input, args.datapath)
  installer.show()
  xml_file = installer.generatePkgInfoXML()
  installer.copyOrLinkResources()
  if os.path.exists(args.datapath):
    installer.installPkgInfoDB(args.datapath, xml_file)

if __name__ == '__main__':
  main()
