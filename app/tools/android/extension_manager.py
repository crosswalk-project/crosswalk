#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import fnmatch
import json
import optparse
import os
import sys

from util import CleanDir, RunCommand, GetVersion


"""
* Fodler structure for extension.
1. 1 extension of 1 repo.
.
|-- all.gyp     #optional. If your extension is built from source.
|                          You have to provide this file.
|-- build.json  #required. Tell make_apk.json where to find extension.
|-- <other files and dirs>

2. multiple extesnions of 1 repo
.
|-- all.gyp
|-- <extension>
    |-- build.json
    |-- <other files and dirs>

* Format of build.json
Example:
{
  "binary_path":"../out/Default/gen/iap"
}
The base dir of "binary_path" is the path of extension.
"""


def GetExtensionList(extensions_path):
  if not os.path.isdir(extensions_path):
    return []

  extension_list = []
  for item in os.listdir(extensions_path):
    sub_path = os.path.join(extensions_path, item)
    if not os.path.isdir(sub_path):
      continue
    if os.path.isfile(os.path.join(sub_path, "build.json")):
      extension_list.append(item)
    else:
      for sub_item in os.listdir(sub_path):
        if os.path.isfile(
            os.path.join(sub_path, sub_item, "build.json")):
          extension_list.append(os.path.join(item, sub_item))

  return extension_list


def EnableExtension(extension_name, extensions_path, is_enable):
  extension_list = GetExtensionList(extensions_path)
  filtered_extensions = fnmatch.filter(extension_list, extension_name)
  for item in filtered_extensions:
    build_json_path = os.path.join(extensions_path, item, "build.json")
    with open(build_json_path, "r") as fd:
      data = json.load(fd)
      data["enable"] = is_enable
    with open(build_json_path, "w") as fd:
      fd.write(
          json.dumps(data, sort_keys=True, indent=4, separators=(',', ': ')))


def GetExtensionStatus(extension_name, extensions_path):
  build_json_path = os.path.join(extensions_path, extension_name, "build.json")
  with open(build_json_path, "r") as fd:
    data = json.load(fd)
  return data.get('enable', True)


def BuildExtension(repo_path):
  old_cwd = os.getcwd()
  os.chdir(repo_path)
  os.environ["GYP_GENERATORS"] = "ninja"
  gyp_cmd = ["gyp", "--depth=.", "all.gyp"]
  RunCommand(gyp_cmd, True)
  #Currently, the output path is set to out/Default.
  ninja_cmd = ["ninja", "-C", os.path.join("out", "Default")]
  RunCommand(ninja_cmd, True)
  os.chdir(old_cwd)


def HandleAdd(git_url, extensions_path, name=None):
  if name is None:
    name = git_url.split('/')[-1].split('.')[0]
  if not os.path.isdir(extensions_path):
    if os.path.isfile(extensions_path):
      print("WARNING: Please remove file %s" % (extensions_path))
      sys.exit(1)
    else:
      os.mkdir(extensions_path)
  local_extension_path = os.path.join(extensions_path, name)
  if os.path.exists(local_extension_path):
    print("ERROR: You already have a repo named \"%s\"." % name)
    return 
  os.mkdir(local_extension_path)
  #Only support git.
  git_cmd = ["git", "clone", git_url, local_extension_path]
  RunCommand(git_cmd, True)
  if os.path.isfile(os.path.join(local_extension_path, "all.gyp")):
    BuildExtension(local_extension_path)


def HandleRemove(remove_name, extensions_path):
  extension_path = os.path.join(extensions_path, remove_name)
  if os.path.exists(extension_path):
    CleanDir(extension_path)
  else:
    print("ERROR: Don't have extension \"%s\"" % (remove_name))


def PrintExtensionInfo(extension_name, extensions_path):
  print("{0} {1}".format(
      "+" if GetExtensionStatus(extension_name, extensions_path) else "-",
      extension_name))


def HandleList(extensions_path):
  extension_list = GetExtensionList(extensions_path)
  print("")
  for extension_name in extension_list:
    PrintExtensionInfo(extension_name, extensions_path)
  print("")


def HandleSearch(key, extensions_path):
  extension_list = GetExtensionList(extensions_path)
  filtered_extensions = fnmatch.filter(extension_list, key)
  print("")
  for extension_name in filtered_extensions:
    PrintExtensionInfo(extension_name, extensions_path)
  print("")


def HandleEnable(extension_name, extension_path):
  EnableExtension(extension_name, extension_path, True)


def HandleDisable(extension_name, extension_path):
  EnableExtension(extension_name, extension_path, False)


def HandleVersion():
  version_path = \
      os.path.join(os.path.dirname(os.path.realpath(__file__)), "VERSION")
  if os.path.isfile(version_path):
    print(GetVersion("VERSION"))
  else:
    print ("ERROR: VERSION was not found, so Crosswalk\'s version could not"
           "be determined.")


def main(argv):
  parser = optparse.OptionParser()
  parser.add_option("--add", action="store",
                    type="string", dest="git_url",
                    metavar="URL",
                    help="Add an extension")
  parser.add_option("--enable", action="store",
                    type="string", dest="enable",
                    metavar="NAME",
                    help="Enabled extension list")
  parser.add_option("--disable", action="store",
                    type="string", dest="disable",
                    metavar="NAME",
                    help="Disabled extension list")
  parser.add_option("--name", action="store",
                    type="string", dest="name",
                    metavar="NAME",
                    help="Extension name in local path. "
                         "Work with --add option.")
  parser.add_option("--remove", action="store",
                    type="string", dest="remove_name",
                    metavar="NAME",
                    help="Remove an extension")
  parser.add_option("-l", "--list", action="store_true",
                    dest="list_extensions", default=False,
                    help="List all extensions")
  parser.add_option("--search", action="store",
                    type="string", dest="search_key",
                    metavar="KEYWORD",
                    help="List all extensions")
  parser.add_option("-v", "--version", action="store_true",
                    dest="version", default=False,
                    help="The version of this python tool.")

  options, _ = parser.parse_args()
  if len(argv) == 1:
    parser.print_help()
    return 0
  
  extensions_path = os.path.join(os.getcwd(), "extensions")

  if options.git_url:
    HandleAdd(options.git_url, extensions_path, options.name)
  elif options.enable:
    HandleEnable(options.enable, extensions_path)
  elif options.disable:
    HandleDisable(options.disable, extensions_path)
  elif options.remove_name:
    HandleRemove(options.remove_name, extensions_path)
  elif options.list_extensions:
    HandleList(extensions_path)
  elif options.search_key:
    HandleSearch(options.search_key, extensions_path)
  elif options.version:
    HandleVersion()


if __name__ == '__main__':
  sys.exit(main(sys.argv))
