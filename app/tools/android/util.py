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
import tempfile

build_dir = None

def GetBuildDir(name):
  global build_dir
  if not build_dir:
    build_dir = tempfile.mkdtemp(prefix="%s-" % name)
  return build_dir


def CleanDir(path):
  if os.path.exists(path):
    shutil.rmtree(path)


def AllArchitectures():
  return ("x86", "arm")


def RunCommand(command, verbose=False, shell=False):
  """Runs the command list, print the output, and propagate its result."""
  proc = subprocess.Popen(command, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, shell=shell)
  if not shell:
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


def CreateAndCopyDir(src_dir, dest_dir, delete_if_exists=False):
  if not os.path.isdir(src_dir):
    return False
  # create path, except last directory (handled by copytree)
  pre_dest_dir = os.path.dirname(dest_dir)
  if not os.path.isdir(pre_dest_dir):
    try:
      os.makedirs(pre_dest_dir)  # throws exception on error
    except OSError:
      return False
  if os.path.exists(dest_dir):
    if delete_if_exists:
      shutil.rmtree(dest_dir)
    else:
      return False
  shutil.copytree(src_dir, dest_dir)
  return True

