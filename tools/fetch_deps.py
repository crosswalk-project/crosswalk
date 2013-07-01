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
import re
import sys

from utils import TryAddDepotToolsToPythonPath

try:
  import gclient_utils
except ImportError:
  TryAddDepotToolsToPythonPath()

try:
  import gclient_utils
  import gclient_scm
  import subprocess2
  from third_party.repo.progress import Progress
except ImportError:
  sys.stderr.write("Can't find gclient_utils, please add your depot_tools "\
                   "to PATH or PYTHONPATH\n")

percent_re = re.compile('.* ([0-9]{1,2})% .*')
def _GitFilter(line):
  # git uses an escape sequence to clear the line; elide it.
  esc = line.find(unichr(033))
  if esc > -1:
    line = line[:esc]
  match = percent_re.match(line)
  if not match or not int(match.group(1)) % 10:
    print '%s' % line

class FetchingError(Exception):
  pass

class FolderExistGitWrapper(gclient_scm.GitWrapper):
  """Handle the case that we need to initial git environment
    when the folder is already there.
    We need to do:
      1. git init
      2. git remote add
      3. git fetch
      4. git checkout
    Then we can let gclient sync to handle the rest of its life.
  """
  def __init__(self, url=None, root_dir=None, relpath=None):
    gclient_scm.GitWrapper.__init__(self, url, root_dir, relpath)
    self._split_url = gclient_utils.SplitUrlRevision(self.url)

  def _Fetch(self, remote, options):
    fetch_cmd = ['fetch', remote, '--progress']
    if options.verbose:
      fetch_cmd.append('--verbose')

    for _ in range(3):
      try:
        # git fetch for chromium will take a looooong time for the
        # first time, so set timeout for 30 minutes
        self._Run(fetch_cmd, options=options, cwd=self.checkout_path,
                  filter_fn=_GitFilter, print_stdout=False,
                  nag_timer=30, nag_max=60)
        break
      except subprocess2.CalledProcessError, e:
        if e.returncode == 128:
          print(str(e))
          print('Retrying...')
          continue
        raise e

  def _DoCheckOut(self, options):
    revision = self._split_url[1]
    if revision:
      if revision.startswith('refs/heads/'):
        revision = revision.replace('refs/heads/', 'origin/')
        rev_type = "branch"
      elif revision.startswith('origin/'):
        rev_type = "branch"
      else:
        rev_type = 'hash'
      if rev_type == 'hash':
        co_args = [revision]
      else:
        branch = revision[len('origin/'):]
        branches = self._Capture(['branch'])
        if branch.strip() in [br.strip() for br in branches.split('\n')]:
          print('branch %s already exist, skip checkout', branch)
          return
        else:
          co_args = ['-b', branch, revision]
    else:
      co_args = ['-b', 'master', 'origin/master']
    self._Run(['checkout'] + co_args, options=options, cwd=self.checkout_path,
              filter_fn=_GitFilter, print_stdout=False)

  def DoInitAndCheckout(self, options):
    # Do git init if necessary
    if not os.path.exists(os.path.join(self.checkout_path, '.git')):
      print('_____ initialize %s to be a git repo' % self.relpath)
      self._Capture(['init'])
    # Find out remote origin exists or not
    remotes = self._Capture(['remote']).strip().splitlines()
    if 'origin' not in [remote.strip() for remote in remotes]:
      print('_____ setting remote for  %s' % self.relpath)
      self._Capture(['remote', 'add', 'origin', self._split_url[0]])
    else:
      current_url = self._Capture(['config', 'remote.origin.url'])
      if current_url != self._split_url[0]:
        print('_____ switching %s to a new upstream' % self.relpath)
        # Switch over to the new upstream
        self._Run(['remote', 'set-url', 'origin', self._split_url[0]], options)

    self._Fetch('origin', options)
    self._DoCheckOut(options)

class DepsFetcher(gclient_utils.WorkItem):
  def __init__(self, name, options):
    gclient_utils.WorkItem.__init__(self, name)
    self._options = options
    self._cameo_dir = os.path.dirname(
        os.path.dirname(os.path.abspath(__file__)))
    if options.deps:
      self._deps_file = options.deps
    else:
      self._deps_file = os.path.join(self._cameo_dir, 'DEPS.cameo')
    self._deps = None
    self._chromium_version = None
    self._ParseDepsFile()
    if not 'src' in self._deps:
      raise FetchingError("'src' not specified in deps file(%s)" % options.deps)
    self._src_dep = self._deps['src']
    # self should be at src/cameo/tools/fetch_deps.py
    # so src is at self/../../../
    self._src_dir = os.path.dirname(self._cameo_dir)
    self._root_dir = os.path.dirname(self._src_dir)
    self._new_gclient_file = os.path.join(self._root_dir,
                                          '.gclient-cameo')
    self._src_git = FolderExistGitWrapper(self._src_dep, self._root_dir, 'src')
    
  def _ParseDepsFile(self):
    if not os.path.exists(self._deps_file):
      raise FetchingError('Deps file does not exist (%s).' % self._deps_file)
    exec_globals = {}

    execfile(self._deps_file, exec_globals)
    self._deps = exec_globals['deps_cameo']
    self._chromium_version = exec_globals['chromium_version']

  @property
  # pylint: disable=R0201
  def requirements(self):
    # No requirements at all
    return set()

  def run(self, work_queue):
    self._src_git.DoInitAndCheckout(self._options)
    self.PrepareGclient()
    return 0

  def AddIgnorePathFromEnv(self):
    """Read paths from environ CAMEO_SYNC_IGNORE.
       Set the path with None value to ignore it when syncing chromium.

       If environ not set, will ignore the ones upstream wiki recommended
       by default.
    """
    ignores_str = os.environ.get("CAMEO_SYNC_IGNORE")
    if not ignores_str:
      ignores = ['src/webkit/data/layout_tests/LayoutTests',
                 'src/third_party/WebKit/LayoutTests',
                 'src/content/test/data/layout_tests/LayoutTests',
                 'src/chrome/tools/test/reference_build/chrome_win',
                 'src/chrome_frame/tools/test/reference_build/chrome_win',
                 'src/chrome/tools/test/reference_build/chrome_linux',
                 'src/chrome/tools/test/reference_build/chrome_mac',
                 'src/third_party/hunspell_dictionaries']
    else:
      ignores_str = ignores_str.replace(':', ';')
      ignores = ignores_str.split(';')
    for ignore in ignores:
      self._deps[ignore] = None
      

  def PrepareGclient(self):
    """It is very important here to know if the based chromium is trunk
       or versioned.

       If it's trunk, we must use .DEPS.git. Because if we use DEPS, gclient
       will try to find all repos under the same url we host chromium-crosswalk.
       And we need to remove 'src' from custom deps, because 'src' will be the
       main subject for the gclient sync.

       Otherwise, we must use DEPS, and we can find the DEPS at
         http://src.chromium.org/svn/releases/<version>
       In this case, we need to keep 'src' in custom deps.
    """
    solution = {}
    if self._chromium_version == 'Trunk':
      solution['name'] = 'src'
      solution['url'] = self._src_dep
      solution['deps_file'] = '.DEPS.git'
      del(self._deps['src'])
    else:
      solution['name'] = self._chromium_version
      solution['url'] = \
          'http://src.chromium.org/svn/releases/%s' % self._chromium_version
    self.AddIgnorePathFromEnv()
    solution['custom_deps'] = self._deps
    solutions = [solution]
    gclient_file = open(self._new_gclient_file, 'w')
    print "Place %s with solutions:\n%s" % (self._new_gclient_file, solutions)
    gclient_file.write('solutions = %s' % solutions)

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

  option_parser.add_option('--deps', default=None,
      help='The deps file contains the dependencies path and url')
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

  pm = Progress('Syncing chromium-crosswalk', 1)
  work_queue = gclient_utils.ExecutionQueue(1, pm, None)
  deps_fetcher = DepsFetcher('fetching', options)
  work_queue.enqueue(deps_fetcher)
  work_queue.flush()
  sys.exit(deps_fetcher.DoGclientSyncForChromium())

if __name__ == '__main__':
  main()
