#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Runs all the buildbot steps for ChromeDriver except for update/compile."""

import bisect
import csv
import datetime
import glob
import json
import optparse
import os
import platform as platform_module
import re
import shutil
import StringIO
import subprocess
import sys
import tempfile
import time
import urllib2

import archive
import chrome_paths
import util

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
GS_CHROMEDRIVER_BUCKET = 'gs://chromedriver'
GS_CHROMEDRIVER_DATA_BUCKET = 'gs://chromedriver-data'
GS_CONTINUOUS_URL = GS_CHROMEDRIVER_DATA_BUCKET + '/continuous'
GS_PREBUILTS_URL = GS_CHROMEDRIVER_DATA_BUCKET + '/prebuilts'
GS_SERVER_LOGS_URL = GS_CHROMEDRIVER_DATA_BUCKET + '/server_logs'
SERVER_LOGS_LINK = (
  'http://chromedriver-data.storage.googleapis.com/server_logs')
TEST_LOG_FORMAT = '%s_log.json'

SCRIPT_DIR = os.path.join(_THIS_DIR, os.pardir, os.pardir, os.pardir, os.pardir,
                          os.pardir, os.pardir, os.pardir, 'scripts')
SITE_CONFIG_DIR = os.path.join(_THIS_DIR, os.pardir, os.pardir, os.pardir,
                               os.pardir, os.pardir, os.pardir, os.pardir,
                               'site_config')
sys.path.append(SCRIPT_DIR)
sys.path.append(SITE_CONFIG_DIR)
from slave import gsutil_download
from slave import slave_utils


def _ArchivePrebuilts(revision):
  """Uploads the prebuilts to google storage."""
  util.MarkBuildStepStart('archive prebuilts')
  zip_path = util.Zip(os.path.join(chrome_paths.GetBuildDir(['chromedriver']),
                                   'chromedriver'))
  if slave_utils.GSUtilCopy(
      zip_path,
      '%s/%s' % (GS_PREBUILTS_URL, 'r%s.zip' % revision)):
    util.MarkBuildStepError()


def _ArchiveServerLogs():
  """Uploads chromedriver server logs to google storage."""
  util.MarkBuildStepStart('archive chromedriver server logs')
  for server_log in glob.glob(os.path.join(tempfile.gettempdir(),
                                           'chromedriver_*')):
    base_name = os.path.basename(server_log)
    util.AddLink(base_name, '%s/%s' % (SERVER_LOGS_LINK, base_name))
    slave_utils.GSUtilCopy(
        server_log,
        '%s/%s' % (GS_SERVER_LOGS_URL, base_name),
        mimetype='text/plain')


def _DownloadPrebuilts():
  """Downloads the most recent prebuilts from google storage."""
  util.MarkBuildStepStart('Download latest chromedriver')

  zip_path = os.path.join(util.MakeTempDir(), 'build.zip')
  if gsutil_download.DownloadLatestFile(GS_PREBUILTS_URL, 'r', zip_path):
    util.MarkBuildStepError()

  util.Unzip(zip_path, chrome_paths.GetBuildDir(['host_forwarder']))


def _GetTestResultsLog(platform):
  """Gets the test results log for the given platform.

  Returns:
    A dictionary where the keys are SVN revisions and the values are booleans
    indicating whether the tests passed.
  """
  temp_log = tempfile.mkstemp()[1]
  log_name = TEST_LOG_FORMAT % platform
  result = slave_utils.GSUtilDownloadFile(
      '%s/%s' % (GS_CHROMEDRIVER_DATA_BUCKET, log_name), temp_log)
  if result:
    return {}
  with open(temp_log, 'rb') as log_file:
    json_dict = json.load(log_file)
  # Workaround for json encoding dictionary keys as strings.
  return dict([(int(v[0]), v[1]) for v in json_dict.items()])


def _PutTestResultsLog(platform, test_results_log):
  """Pushes the given test results log to google storage."""
  temp_dir = util.MakeTempDir()
  log_name = TEST_LOG_FORMAT % platform
  log_path = os.path.join(temp_dir, log_name)
  with open(log_path, 'wb') as log_file:
    json.dump(test_results_log, log_file)
  if slave_utils.GSUtilCopyFile(log_path, GS_CHROMEDRIVER_DATA_BUCKET):
    raise Exception('Failed to upload test results log to google storage')


def _UpdateTestResultsLog(platform, revision, passed):
  """Updates the test results log for the given platform.

  Args:
    platform: The platform name.
    revision: The SVN revision number.
    passed: Boolean indicating whether the tests passed at this revision.
  """
  assert isinstance(revision, int), 'The revision must be an integer'
  log = _GetTestResultsLog(platform)
  if len(log) > 500:
    del log[min(log.keys())]
  assert revision not in log, 'Results already exist for revision %s' % revision
  log[revision] = bool(passed)
  _PutTestResultsLog(platform, log)


def _GetVersion():
  """Get the current chromedriver version."""
  with open(os.path.join(_THIS_DIR, 'VERSION'), 'r') as f:
    return f.read().strip()


def _GetSupportedChromeVersions():
  """Get the minimum and maximum supported Chrome versions.

  Returns:
    A tuple of the form (min_version, max_version).
  """
  # Minimum supported Chrome version is embedded as:
  # const int kMinimumSupportedChromeVersion[] = {27, 0, 1453, 0};
  with open(os.path.join(_THIS_DIR, 'chrome', 'version.cc'), 'r') as f:
    lines = f.readlines()
    chrome_min_version_line = filter(
        lambda x: 'kMinimumSupportedChromeVersion' in x, lines)
  chrome_min_version = chrome_min_version_line[0].split('{')[1].split(',')[0]
  with open(os.path.join(chrome_paths.GetSrc(), 'chrome', 'VERSION'), 'r') as f:
    chrome_max_version = f.readlines()[0].split('=')[1].strip()
  return (chrome_min_version, chrome_max_version)


def _RevisionState(test_results_log, revision):
  """Check the state of tests at a given SVN revision.

  Considers tests as having passed at a revision if they passed at revisons both
  before and after.

  Args:
    test_results_log: A test results log dictionary from _GetTestResultsLog().
    revision: The revision to check at.

  Returns:
    'passed', 'failed', or 'unknown'
  """
  assert isinstance(revision, int), 'The revision must be an integer'
  keys = sorted(test_results_log.keys())
  # Return passed if the exact revision passed on Android.
  if revision in test_results_log:
    return 'passed' if test_results_log[revision] else 'failed'
  # Tests were not run on this exact revision on Android.
  index = bisect.bisect_right(keys, revision)
  # Tests have not yet run on Android at or above this revision.
  if index == len(test_results_log):
    return 'unknown'
  # No log exists for any prior revision, assume it failed.
  if index == 0:
    return 'failed'
  # Return passed if the revisions on both sides passed.
  if test_results_log[keys[index]] and test_results_log[keys[index - 1]]:
    return 'passed'
  return 'failed'


def _ArchiveGoodBuild(platform, revision):
  assert platform != 'android'
  util.MarkBuildStepStart('archive build')

  server_name = 'chromedriver'
  if util.IsWindows():
    server_name += '.exe'
  zip_path = util.Zip(os.path.join(chrome_paths.GetBuildDir([server_name]),
                                   server_name))

  build_url = '%s/chromedriver_%s_%s.%s.zip' % (
      GS_CONTINUOUS_URL, platform, _GetVersion(), revision)
  if slave_utils.GSUtilCopy(zip_path, build_url):
    util.MarkBuildStepError()


def _MaybeRelease(platform):
  """Releases a release candidate if conditions are right."""
  assert platform != 'android'

  # Check if the current version has already been released.
  result, _ = slave_utils.GSUtilListBucket(
      '%s/%s/chromedriver_%s*' % (
          GS_CHROMEDRIVER_BUCKET, _GetVersion(), platform),
      [])
  if result == 0:
    return

  # Fetch Android test results.
  android_test_results = _GetTestResultsLog('android')

  # Fetch release candidates.
  result, output = slave_utils.GSUtilListBucket(
      '%s/chromedriver_%s_%s*' % (
          GS_CONTINUOUS_URL, platform, _GetVersion()),
      [])
  assert result == 0 and output, 'No release candidates found'
  candidates = [b.split('/')[-1] for b in output.strip().split('\n')]

  # Release the first candidate build that passed Android, if any.
  for candidate in candidates:
    if not candidate.startswith('chromedriver_%s' % platform):
      continue
    revision = candidate.split('.')[2]
    android_result = _RevisionState(android_test_results, int(revision))
    if android_result == 'failed':
      print 'Android tests did not pass at revision', revision
    elif android_result == 'passed':
      print 'Android tests passed at revision', revision
      _Release('%s/%s' % (GS_CONTINUOUS_URL, candidate), platform)
      break
    else:
      print 'Android tests have not run at a revision as recent as', revision


def _Release(build, platform):
  """Releases the given candidate build."""
  release_name = 'chromedriver_%s.zip' % platform
  util.MarkBuildStepStart('releasing %s' % release_name)
  slave_utils.GSUtilCopy(
      build, '%s/%s/%s' % (GS_CHROMEDRIVER_BUCKET, _GetVersion(), release_name))

  _MaybeUploadReleaseNotes()


def _MaybeUploadReleaseNotes():
  """Upload release notes if conditions are right."""
  # Check if the current version has already been released.
  version = _GetVersion()
  notes_name = 'notes.txt'
  notes_url = '%s/%s/%s' % (GS_CHROMEDRIVER_BUCKET, version, notes_name)
  prev_version = '.'.join([version.split('.')[0],
                          str(int(version.split('.')[1]) - 1)])
  prev_notes_url = '%s/%s/%s' % (
      GS_CHROMEDRIVER_BUCKET, prev_version, notes_name)

  result, _ = slave_utils.GSUtilListBucket(notes_url, [])
  if result == 0:
    return

  fixed_issues = []
  query = ('https://code.google.com/p/chromedriver/issues/csv?'
           'q=status%3AToBeReleased&colspec=ID%20Summary')
  issues = StringIO.StringIO(urllib2.urlopen(query).read().split('\n', 1)[1])
  for issue in csv.reader(issues):
    if not issue:
      continue
    id = issue[0]
    desc = issue[1]
    labels = issue[2]
    fixed_issues += ['Resolved issue %s: %s [%s]' % (id, desc, labels)]

  old_notes = ''
  temp_notes_fname = tempfile.mkstemp()[1]
  if not slave_utils.GSUtilDownloadFile(prev_notes_url, temp_notes_fname):
    with open(temp_notes_fname, 'rb') as f:
      old_notes = f.read()

  new_notes = '----------ChromeDriver v%s (%s)----------\n%s\n%s\n\n%s' % (
      version, datetime.date.today().isoformat(),
      'Supports Chrome v%s-%s' % _GetSupportedChromeVersions(),
      '\n'.join(fixed_issues),
      old_notes)
  with open(temp_notes_fname, 'w') as f:
    f.write(new_notes)

  if slave_utils.GSUtilCopy(temp_notes_fname, notes_url, mimetype='text/plain'):
    util.MarkBuildStepError()


def _KillChromes():
  chrome_map = {
      'win': 'chrome.exe',
      'mac': 'Chromium',
      'linux': 'chrome',
  }
  if util.IsWindows():
    cmd = ['taskkill', '/F', '/IM']
  else:
    cmd = ['killall', '-9']
  cmd.append(chrome_map[util.GetPlatformName()])
  util.RunCommand(cmd)


def _CleanTmpDir():
  tmp_dir = tempfile.gettempdir()
  print 'cleaning temp directory:', tmp_dir
  for file_name in os.listdir(tmp_dir):
    file_path = os.path.join(tmp_dir, file_name)
    if os.path.isdir(file_path):
      print 'deleting sub-directory', file_path
      shutil.rmtree(file_path, True)
    if file_name.startswith('chromedriver_'):
      print 'deleting file', file_path
      os.remove(file_path)


def _WaitForLatestSnapshot(revision):
  util.MarkBuildStepStart('wait_for_snapshot')
  while True:
    snapshot_revision = archive.GetLatestRevision(archive.Site.SNAPSHOT)
    if int(snapshot_revision) >= int(revision):
      break
    util.PrintAndFlush('Waiting for snapshot >= %s, found %s' %
                       (revision, snapshot_revision))
    time.sleep(60)
  util.PrintAndFlush('Got snapshot revision %s' % snapshot_revision)


def main():
  parser = optparse.OptionParser()
  parser.add_option(
      '', '--android-packages',
      help='Comma separated list of application package names, '
           'if running tests on Android.')
  parser.add_option(
      '-r', '--revision', type='int', help='Chromium revision')
  parser.add_option('', '--update-log', action='store_true',
      help='Update the test results log (only applicable to Android)')
  options, _ = parser.parse_args()

  bitness = '32'
  if util.IsLinux() and platform_module.architecture()[0] == '64bit':
    bitness = '64'
  platform = '%s%s' % (util.GetPlatformName(), bitness)
  if options.android_packages:
    platform = 'android'

  if platform != 'android':
    _KillChromes()
  _CleanTmpDir()

  if platform == 'android':
    if not options.revision and options.update_log:
      parser.error('Must supply a --revision with --update-log')
    _DownloadPrebuilts()
  else:
    if not options.revision:
      parser.error('Must supply a --revision')
    if platform == 'linux64':
      _ArchivePrebuilts(options.revision)
    _WaitForLatestSnapshot(options.revision)

  cmd = [
      sys.executable,
      os.path.join(_THIS_DIR, 'test', 'run_all_tests.py'),
  ]
  if platform == 'android':
    cmd.append('--android-packages=' + options.android_packages)

  passed = (util.RunCommand(cmd) == 0)

  _ArchiveServerLogs()

  if platform == 'android':
    if options.update_log:
      util.MarkBuildStepStart('update test result log')
      _UpdateTestResultsLog(platform, options.revision, passed)
  elif passed:
    _ArchiveGoodBuild(platform, options.revision)
    _MaybeRelease(platform)


if __name__ == '__main__':
  main()
