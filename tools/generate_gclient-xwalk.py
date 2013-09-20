#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This script is responsible for generating .gclient-xwalk in the top-level
source directory by parsing .DEPS.xwalk (or any other file passed to the --deps
option).
"""

import optparse
import os
import pprint
import sys


class GClientFileGenerator(object):
  def __init__(self, options):
    self._options = options
    self._xwalk_dir = os.path.dirname(
        os.path.dirname(os.path.abspath(__file__)))
    if options.deps:
      self._deps_file = options.deps
    else:
      self._deps_file = os.path.join(self._xwalk_dir, 'DEPS.xwalk')
    self._deps = None
    self._vars = None
    self._chromium_version = None
    self._ParseDepsFile()
    if not 'src' in self._deps:
      raise RuntimeError("'src' not specified in deps file(%s)" % options.deps)
    self._src_dep = self._deps['src']
    # self should be at src/xwalk/tools/fetch_deps.py
    # so src is at self/../../../
    self._src_dir = os.path.dirname(self._xwalk_dir)
    self._root_dir = os.path.dirname(self._src_dir)
    self._new_gclient_file = os.path.join(self._root_dir,
                                          '.gclient-xwalk')

  def _ParseDepsFile(self):
    if not os.path.exists(self._deps_file):
      raise IOError('Deps file does not exist (%s).' % self._deps_file)
    exec_globals = {}

    execfile(self._deps_file, exec_globals)
    self._deps = exec_globals['deps_xwalk']
    self._vars = exec_globals['vars_xwalk']
    self._chromium_version = exec_globals['chromium_version']

  def _AddIgnorePathFromEnv(self):
    """Read paths from environ XWALK_SYNC_IGNORE.
       Set the path with None value to ignore it when syncing chromium.

       If environ not set, will ignore the ones upstream wiki recommended
       by default.
    """
    ignores_str = os.environ.get("XWALK_SYNC_IGNORE")
    if not ignores_str:
      ignores = ['build',
                 'build/scripts/command_wrapper/bin',
                 'build/scripts/gsd_generate_index',
                 'build/scripts/private/data/reliability',
                 'build/third_party/cbuildbot_chromite',
                 'build/third_party/gsutil',
                 'build/third_party/lighttpd',
                 'build/third_party/swarm_client',
                 'build/third_party/xvfb',
                 'build/xvfb',
                 'build/third_party/cbuildbot_chromite',
                 'commit-queue',
                 'depot_tools',
                 'src/webkit/data/layout_tests/LayoutTests',
                 'src/third_party/WebKit/LayoutTests',
                 'src/content/test/data/layout_tests/LayoutTests',
                 'src/chrome/tools/test/reference_build/chrome_win',
                 'src/chrome_frame/tools/test/reference_build/chrome_win',
                 'src/chrome/tools/test/reference_build/chrome_linux',
                 'src/chrome/tools/test/reference_build/chrome_mac',
                 'src/third_party/chromite',
                 'src/third_party/cros_system_api',
                 'src/third_party/hunspell_dictionaries',
                 'src/third_party/pyelftools']
    else:
      ignores_str = ignores_str.replace(':', ';')
      ignores = ignores_str.split(';')
    for ignore in ignores:
      self._deps[ignore] = None

  def Generate(self):
    self._AddIgnorePathFromEnv()
    solution = {
      'name': self._chromium_version,
      'url': 'http://src.chromium.org/svn/releases/%s' %
              self._chromium_version,
      'custom_deps': self._deps,
    }
    if self._vars:
      solution['custom_vars'] = self._vars
    solutions = [solution]
    gclient_file = open(self._new_gclient_file, 'w')
    print "Place %s with solutions:\n%s" % (self._new_gclient_file, solutions)
    gclient_file.write('solutions = %s\n' % pprint.pformat(solutions))
    # Check whether the target OS is Android.
    if os.environ.get('XWALK_OS_ANDROID'):
      target_os = ['android']
      gclient_file.write('target_os = %s\n' % target_os)
    if self._options.cache_dir:
      gclient_file.write('cache_dir = %s\n' %
                         pprint.pformat(self._options.cache_dir))


def main():
  option_parser = optparse.OptionParser()

  option_parser.add_option('--deps', default=None,
                           help='The deps file contains the dependencies path '
                                'and url')
  option_parser.add_option('--cache-dir',
                           help='Set "cache_dir" in the .gclient file to this '
                                'directory, so that all git repositories are '
                                'cached there and shared across multiple '
                                'clones.')

  # pylint: disable=W0612
  options, args = option_parser.parse_args()

  sys.exit(GClientFileGenerator(options).Generate())


if __name__ == '__main__':
  main()
