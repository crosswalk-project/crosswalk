#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess


def GetJARFilename():
  # Version of YUI Compressor.
  version = "2.4.8"
  # yuicompressor-*.jar was gotten from http://yui.github.io/yuicompressor/.
  file_name = "yuicompressor-%s.jar" % version
  cur_dir = os.path.realpath(os.path.dirname(__file__))
  return os.path.join(cur_dir, file_name)


def ExecuteCmd(file_list, ext):
  for file_full_path in file_list:
    if os.path.exists(file_full_path):
      cmd_args = ["java", "-jar", GetJARFilename(), "--type=" + ext,
                  file_full_path, "-o", file_full_path]
      subprocess.call(cmd_args)


def CompressJavaScript(file_list):
  ExecuteCmd(file_list, "js")


def CompressCss(file_list):
  ExecuteCmd(file_list, "css")
