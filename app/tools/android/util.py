#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401


import os
import re
import shutil
import subprocess
import sys


def CleanDir(path):
  if os.path.exists(path):
    shutil.rmtree(path)


def AllArchitectures():
  return ("x86", "arm")


def RunCommand(command, verbose=False, need_output=True, shell=False):
  """Runs the command list, print the output, and propagate its result."""
  proc = subprocess.Popen(command, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, shell=shell)
  if need_output:
    output = proc.communicate()[0]
    result = proc.returncode
    if verbose:
      print(output.decode("utf-8").strip())
    if result != 0:
      print ('Command "%s" exited with non-zero exit code %d'
             % (' '.join(command), result))
      sys.exit(result)
    return output.decode("utf-8")
  else:
    return None


def GetVersion(path):
  """Get the version of this python tool."""
  version_str = 'Crosswalk app packaging tool version is '
  file_handle = open(path, 'r')
  src_content = file_handle.read()
  version_nums = re.findall(r'\d+', src_content)
  version_str += ('.').join(version_nums)
  file_handle.close()
  return version_str

def IsWindows():
  return sys.platform == 'cygwin' or sys.platform.startswith('win')
