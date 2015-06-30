#!/usr/bin/env python
# Copyright (c) 2015 Intel Corp. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
patch_skia.py -- Apply patches to skia when running gclient.
"""

import os
import subprocess
import sys


def main():
  tools_dir = os.path.dirname(os.path.abspath(__file__))
  xwalk_dir = os.path.dirname(tools_dir)
  # tools_dir should be at src/xwalk/tools/patch_skia.py
  # so src is at tools_dir/../../../
  src_dir = os.path.dirname(xwalk_dir)
  root_dir = os.path.dirname(src_dir)
  third_party_dir = os.path.join(src_dir, 'third_party')
  skia_dir = os.path.join(third_party_dir, 'skia')
  patch_dir = os.path.join(root_dir, sys.argv[1])

  git_exe = 'git.bat' if sys.platform.startswith('win') else 'git'

  output = subprocess.call([git_exe, "reset", "--hard"], cwd=skia_dir)
  if output != 0:
    return output

  return subprocess.call([git_exe, "apply", patch_dir], cwd=skia_dir)

if __name__ == '__main__':
  sys.exit(main())
