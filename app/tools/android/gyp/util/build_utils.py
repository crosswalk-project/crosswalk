# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import fnmatch
import os
import pipes
import shutil
import subprocess
import sys
import traceback


def MakeDirectory(dir_path):
  try:
    os.makedirs(dir_path)
  except OSError:
    pass


def DeleteDirectory(dir_path):
  if os.path.exists(dir_path):
    shutil.rmtree(dir_path)


def Touch(path):
  MakeDirectory(os.path.dirname(path))
  with open(path, 'a'):
    os.utime(path, None)


def FindInDirectory(directory, filter_string):
  files = []
  for root, _, filenames in os.walk(directory):
    matched_files = fnmatch.filter(filenames, filter_string)
    files.extend((os.path.join(root, f) for f in matched_files))
  return files


def FindInDirectories(directories, filter_string):
  all_files = []
  for directory in directories:
    all_files.extend(FindInDirectory(directory, filter_string))
  return all_files


# This can be used in most cases like subprocess.check_call. The output,
# particularly when the command fails, better highlights the command's failure.
# This call will directly exit on a failure in the subprocess so that no python
# stacktrace is printed after the output of the failed command (and will
# instead print a python stack trace before the output of the failed command)
def CheckCallDie(args, suppress_output=False, cwd=None):
  if not cwd:
    cwd = os.getcwd()

  if os.name == 'posix':
    child = subprocess.Popen(args,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=cwd)
  else:
    child = subprocess.Popen(args,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=cwd, shell=True)

  stdout, _ = child.communicate()

  if child.returncode:
    stacktrace = traceback.extract_stack()
    print >> sys.stderr, ''.join(traceback.format_list(stacktrace))
    # A user should be able to simply copy and paste the command that failed
    # into their shell.
    copyable_command = ' '.join(map(pipes.quote, args))
    copyable_command = ('( cd ' + os.path.abspath(cwd) + '; '
        + copyable_command + ' )')
    print >> sys.stderr, 'Command failed:', copyable_command, '\n'

    if stdout:
      print stdout

    # Directly exit to avoid printing stacktrace.
    sys.exit(child.returncode)

  else:
    if stdout and not suppress_output:
      print stdout
    return stdout


def GetModifiedTime(path):
  # For a symlink, the modified time should be the greater of the link's
  # modified time and the modified time of the target.
  return max(os.lstat(path).st_mtime, os.stat(path).st_mtime)
