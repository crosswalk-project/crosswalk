#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This script will do:
  1. Setup src's git initialization.
  2. Place .gclient file outside of src.
  3. Call gclient sync outside of src.
"""

import optparse
import os
import sys

from utils import TryAddDepotToolsToPythonPath

try:
  import gclient_utils
except ImportError:
  TryAddDepotToolsToPythonPath()

try:
  import gclient_utils
  import subprocess2
except ImportError:
  sys.stderr.write("Can't find gclient_utils, please add your depot_tools "\
                   "to PATH or PYTHONPATH\n")

class FetchingError(Exception):
  pass

class DepsFetcher(object):
  def __init__(self, options):
    self._options = options
    self._xwalk_dir = os.path.dirname(
        os.path.dirname(os.path.abspath(__file__)))
    # self should be at src/xwalk/tools/fetch_deps.py
    # so src is at self/../../../
    self._src_dir = os.path.dirname(self._xwalk_dir)
    self._root_dir = os.path.dirname(self._src_dir)
    self._new_gclient_file = os.path.join(self._root_dir,
                                          '.gclient-xwalk')
    if not os.path.isfile(self._new_gclient_file):
      raise IOError('%s was not found. Run generate_gclient-xwalk.py.' %
                    self._new_gclient_file)

  @property
  # pylint: disable=R0201
  def requirements(self):
    # No requirements at all
    return set()

  def DoGclientSyncForChromium(self):
    gclient_cmd = ['gclient', 'sync', '--verbose', '--reset',
                   '--force', '--with_branch_heads']
    gclient_cmd.append('--gclientfile=%s' % self._new_gclient_file)
    gclient_utils.CheckCallAndFilterAndHeader(gclient_cmd,
        always=self._options.verbose, cwd=self._root_dir)
    # CheckCallAndFilterAndHeader will raise exception if return
    # value is not 0. So we can easily return 0 here.
    return 0

def main():
  option_parser = optparse.OptionParser()

  option_parser.add_option('-v', '--verbose', action='count', default=0,
      help='Produces additional output for diagnostics. Can be '
           'used up to three times for more logging info.')
  # pylint: disable=W0612
  options, args = option_parser.parse_args()

  # Following code copied from gclient_utils.py
  try:
    # Make stdout auto-flush so buildbot doesn't kill us during lengthy
    # operations. Python as a strong tendency to buffer sys.stdout.
    sys.stdout = gclient_utils.MakeFileAutoFlush(sys.stdout)
    # Make stdout annotated with the thread ids.
    sys.stdout = gclient_utils.MakeFileAnnotated(sys.stdout)
  except (gclient_utils.Error, subprocess2.CalledProcessError), e:
    print >> sys.stderr, 'Error: %s' % str(e)
    return 1

  deps_fetcher = DepsFetcher(options)
  sys.exit(deps_fetcher.DoGclientSyncForChromium())

if __name__ == '__main__':
  main()
