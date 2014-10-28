#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
import sys
import shutil


def DoCopy(path, target_path):
  if os.path.isfile(path):
    package = ''
    package_re = re.compile(
        '^package (?P<package>([a-zA-Z0-9_]+.)*[a-zA-Z0-9_]+);$')
    for line in open(path).readlines():
      match = package_re.match(line)
      if match:
        package = match.group('package')
        break
    sub_path = os.path.sep.join(package.split('.'))
    shutil.copy(path, os.path.join(target_path, sub_path))
    return

  for dirpath, _, files in os.walk(path):
    if not files:
      continue
    sub_path = os.path.relpath(dirpath, path)
    target_dirpath = os.path.join(target_path, sub_path)
    if not os.path.isdir(target_dirpath):
      os.makedirs(target_dirpath)
    for f in files:
      fpath = os.path.join(dirpath, f)
      # "interface type;" is invalid for normal android project,
      # It's only for chromium's build system, ignore these aidl files.
      if f.endswith('.aidl'):
        invalid_lines = []
        for line in open(fpath).readlines():
          if re.match('^interface .*;$', line):
            invalid_lines.append(line)
        if invalid_lines:
          continue
      elif not f.endswith('.java'):
        continue
      shutil.copy(fpath, target_dirpath)


def main():
  parser = optparse.OptionParser()
  info = ('The java source dirs to merge.')
  parser.add_option('--dirs', help=info)
  info = ('The target to place all the sources.')
  parser.add_option('--target-path', help=info)
  options, _ = parser.parse_args()

  if os.path.isdir(options.target_path):
    shutil.rmtree(options.target_path)
  os.makedirs(options.target_path)

  for path in options.dirs.split(' '):
    if path.startswith('"') and path.endswith('"'):
      path = eval(path)
    DoCopy(path, options.target_path)


if __name__ == '__main__':
  sys.exit(main())
