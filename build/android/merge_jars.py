#!/usr/bin/env python

# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import fnmatch
import optparse
import os
import sys
import shutil
import subprocess


def GetCommandOutput(command, cwd=None):
  proc = subprocess.Popen(command, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, bufsize=1,
                          cwd=cwd)
  output = proc.communicate()[0]
  result = proc.returncode
  if result:
    raise Exception('%s: %s' % (subprocess.list2cmdline(command), output))
  return output


def UnpackJar(jar, classes_dir):
  jar_cmd = ['jar', 'xvf', os.path.abspath(jar)]
  GetCommandOutput(jar_cmd, classes_dir)


def FindInDirectory(directory, filename_filter):
  files = []
  for root, _dirnames, filenames in os.walk(directory):
    matched_files = fnmatch.filter(filenames, filename_filter)
    files.extend((os.path.join(root, f) for f in matched_files))
  return files


def DoJar(classes_dir, jar_path):
  class_files = FindInDirectory(classes_dir, '*.class')

  jar_path = os.path.abspath(jar_path)

  class_files_rel = [os.path.relpath(f, classes_dir) for f in class_files]
  jar_cmd = ['jar', 'cf0', jar_path] + class_files_rel

  GetCommandOutput(jar_cmd, classes_dir)


def main():
  parser = optparse.OptionParser()
  info = ('The folder to place unzipped classes')
  parser.add_option('--classes-dir', help=info)
  info = ('The jars to merge')
  parser.add_option('--jars', help=info)
  info = ('The output merged jar file')
  parser.add_option('--jar-path', help=info)
  options, _ = parser.parse_args()

  if os.path.isdir(options.classes_dir):
    shutil.rmtree(options.classes_dir)
  os.makedirs(options.classes_dir)

  for jar in options.jars.split(' '):
    UnpackJar(eval(jar), options.classes_dir)

  DoJar(options.classes_dir, options.jar_path)

if __name__ == '__main__':
  sys.exit(main())
