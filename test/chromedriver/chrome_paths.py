# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Paths to common resources in the Chrome repository."""

import os


_THIS_DIR = os.path.abspath(os.path.dirname(__file__))


def GetSrc():
  """Returns the path to the root src directory."""
  return os.path.abspath(os.path.join(_THIS_DIR, os.pardir, os.pardir,
                                      os.pardir))


def GetTestData():
  """Returns the path to the src/chrome/test/data directory."""
  return os.path.join(GetSrc(), 'chrome', 'test', 'data')


def GetBuildDir(required_paths):
  """Returns the preferred build directory that contains given paths."""
  dirs = ['out', 'build', 'xcodebuild']
  rel_dirs = [os.path.join(x, 'Release') for x in dirs]
  debug_dirs = [os.path.join(x, 'Debug') for x in dirs]
  full_dirs = [os.path.join(GetSrc(), x) for x in rel_dirs + debug_dirs]
  for build_dir in full_dirs:
    for required_path in required_paths:
      if not os.path.exists(os.path.join(build_dir, required_path)):
        break
    else:
      return build_dir
  raise RuntimeError('Cannot find build directory containing ' +
                     ', '.join(required_paths))
