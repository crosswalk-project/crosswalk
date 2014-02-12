#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import fnmatch
import os
import stat
import subprocess

def GetJARFilename():
  # Version of YUI Compressor.
  version = "2.4.8"
  # yuicompressor-*.jar was gotten from http://yui.github.io/yuicompressor/.
  file_name = "yuicompressor-%s.jar" % version
  cur_dir = os.path.realpath(os.path.dirname(__file__))
  return os.path.join(cur_dir, "libs", file_name)

def GetFileList(path, ext, sub_dir = True):
  if os.path.exists(path):
    file_list = []
    for name in os.listdir(path):
      full_name = os.path.join(path, name)
      st = os.lstat(full_name)
      if stat.S_ISDIR(st.st_mode) and sub_dir:
        file_list += GetFileList(full_name, ext)
      elif os.path.isfile(full_name):
        if fnmatch.fnmatch(full_name, ext):
          file_list.append(full_name)
    return file_list
  else:
    return []

def ExecuteCmd(path, ext):
  file_list = GetFileList(path, "*." + ext)
  for file_full_path in file_list:
    if os.path.exists(file_full_path):
      cmd_args = ["java", "-jar", GetJARFilename(), "--type=" + ext,
          file_full_path, "-o", file_full_path]
      subprocess.call(cmd_args)

class CompressJsAndCss(object):
  def __init__(self, input_path):
    self.input_path = input_path

  def CompressJavaScript(self):
    ExecuteCmd(self.input_path, "js")

  def CompressCss(self):
    ExecuteCmd(self.input_path, "css")
