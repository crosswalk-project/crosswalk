#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This is a helper script supposed to be invoked via `gclient recurse'.

Its purpose is to remove directories pointed to by gclient entries that are
expected to be git checkouts but are SVN checkouts instead (and vice-versa).

It is useful when we migrate certain Crosswalk dependencies from standard SVN
checkouts from Chromium to git ones hosted elsewhere and do not want to
require everyone to manually remove their old checkouts first.
"""

import os
import shutil
import sys


def main():
  try:
    repo_name = os.environ['GCLIENT_DEP_PATH']
    scm_name = os.environ['GCLIENT_SCM']
  except KeyError, key:
    print '%s is not part of the environment.' % key
    print 'Make sure this command is being run by gclient recurse.'
    return 1

  repo_abspath = os.getcwd()
  git_dir = os.path.join(repo_abspath, '.git')
  svn_dir = os.path.join(repo_abspath, '.svn')
  needs_clobber = False

  if scm_name == 'git' and os.path.isdir(svn_dir):
    print 'Old SVN checkout detected in %s. Removing directory.' % repo_name
    needs_clobber = True
  elif scm_name == 'svn' and os.path.isdir(git_dir):
    print 'Old git checkout detected in %s. Removing directory.' % repo_name
    needs_clobber = True

  if needs_clobber:
    shutil.rmtree(repo_abspath)

  return 0


if __name__ == '__main__':
  sys.exit(main())
