#!/usr/bin/env python
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Runs all ChromeDriver end to end tests."""

import optparse
import os
import platform
import sys
import tempfile

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
sys.path.insert(0, os.path.join(_THIS_DIR, os.pardir))

import archive
import chrome_paths
import util

sys.path.insert(0, os.path.join(chrome_paths.GetSrc(), 'build', 'android'))
from pylib import constants


def _AppendEnvironmentPath(env_name, path):
  if env_name in os.environ:
    lib_path = os.environ[env_name]
    if path not in lib_path:
      os.environ[env_name] += os.pathsep + path
  else:
    os.environ[env_name] = path


def _AddToolsToSystemPathForWindows():
  path_cfg_file = 'C:\\tools\\bots_path.cfg'
  if not os.path.exists(path_cfg_file):
    print 'Failed to find file', path_cfg_file
  with open(path_cfg_file, 'r') as cfg:
    paths = cfg.read().split('\n')
  os.environ['PATH'] = os.pathsep.join(paths) + os.pathsep + os.environ['PATH']


def _GenerateTestCommand(script,
                         chromedriver,
                         ref_chromedriver=None,
                         chrome=None,
                         chrome_version=None,
                         android_package=None,
                         verbose=False):
  _, log_path = tempfile.mkstemp(prefix='chromedriver_')
  print 'chromedriver server log: %s' % log_path
  cmd = [
      sys.executable,
      os.path.join(_THIS_DIR, script),
      '--chromedriver=%s' % chromedriver,
      '--log-path=%s' % log_path,
  ]
  if ref_chromedriver:
    cmd.append('--reference-chromedriver=' + ref_chromedriver)

  if chrome:
    cmd.append('--chrome=' + chrome)

  if chrome_version:
    cmd.append('--chrome-version=' + chrome_version)

  if verbose:
    cmd.append('--verbose')

  if android_package:
    cmd = ['xvfb-run', '-a'] + cmd
    cmd.append('--android-package=' + android_package)
  return cmd


def RunPythonTests(chromedriver, ref_chromedriver,
                   chrome=None, chrome_version=None,
                   chrome_version_name=None, android_package=None):
  version_info = ''
  if chrome_version_name:
    version_info = '(%s)' % chrome_version_name
  util.MarkBuildStepStart('python_tests%s' % version_info)
  code = util.RunCommand(
      _GenerateTestCommand('run_py_tests.py',
                           chromedriver,
                           ref_chromedriver=ref_chromedriver,
                           chrome=chrome,
                           chrome_version=chrome_version,
                           android_package=android_package))
  if code:
    util.MarkBuildStepError()
  return code


def RunJavaTests(chromedriver, chrome=None, chrome_version=None,
                 chrome_version_name=None, android_package=None,
                 verbose=False):
  version_info = ''
  if chrome_version_name:
    version_info = '(%s)' % chrome_version_name
  util.MarkBuildStepStart('java_tests%s' % version_info)
  code = util.RunCommand(
      _GenerateTestCommand('run_java_tests.py',
                           chromedriver,
                           ref_chromedriver=None,
                           chrome=chrome,
                           chrome_version=chrome_version,
                           android_package=android_package,
                           verbose=verbose))
  if code:
    util.MarkBuildStepError()
  return code


def RunCppTests(cpp_tests):
  util.MarkBuildStepStart('chromedriver_tests')
  code = util.RunCommand([cpp_tests])
  if code:
    util.MarkBuildStepError()
  return code


def DownloadChrome(version_name, revision, download_site):
  util.MarkBuildStepStart('download %s' % version_name)
  return archive.DownloadChrome(revision, util.MakeTempDir(), download_site)


def main():
  parser = optparse.OptionParser()
  parser.add_option(
      '', '--android-packages',
      help='Comma separated list of application package names, '
           'if running tests on Android.')
  # Option 'chrome-version' is for desktop only.
  parser.add_option(
      '', '--chrome-version',
      help='Version of chrome, e.g., \'HEAD\', \'27\', or \'26\'.'
           'Default is to run tests against all of these versions.'
           'Notice: this option only applies to desktop.')
  options, _ = parser.parse_args()

  exe_postfix = ''
  if util.IsWindows():
    exe_postfix = '.exe'
  cpp_tests_name = 'chromedriver_tests' + exe_postfix
  server_name = 'chromedriver' + exe_postfix

  required_build_outputs = [server_name]
  if not options.android_packages:
    required_build_outputs += [cpp_tests_name]
  build_dir = chrome_paths.GetBuildDir(required_build_outputs)
  constants.SetBuildType(os.path.basename(build_dir))
  print 'Using build outputs from', build_dir

  chromedriver = os.path.join(build_dir, server_name)
  platform_name = util.GetPlatformName()
  if util.IsLinux() and platform.architecture()[0] == '64bit':
    platform_name += '64'
  ref_chromedriver = os.path.join(
      chrome_paths.GetSrc(),
      'chrome', 'test', 'chromedriver', 'third_party', 'java_tests',
      'reference_builds',
      'chromedriver_%s%s' % (platform_name, exe_postfix))

  if util.IsLinux():
    # Set LD_LIBRARY_PATH to enable successful loading of shared object files,
    # when chromedriver2.so is not a static build.
    _AppendEnvironmentPath('LD_LIBRARY_PATH', os.path.join(build_dir, 'lib'))
  elif util.IsWindows():
    # For Windows bots: add ant, java(jre) and the like to system path.
    _AddToolsToSystemPathForWindows()

  if options.android_packages:
    os.environ['PATH'] += os.pathsep + os.path.join(
        _THIS_DIR, os.pardir, 'chrome')
    code = 0
    for package in options.android_packages.split(','):
      code1 = RunPythonTests(chromedriver,
                             ref_chromedriver,
                             chrome_version_name=package,
                             android_package=package)
      code2 = RunJavaTests(chromedriver,
                           chrome_version_name=package,
                           android_package=package,
                           verbose=True)
      code = code or code1 or code2
    return code
  else:
    latest_snapshot_revision = archive.GetLatestRevision(archive.Site.SNAPSHOT)
    versions = [
        ['HEAD', latest_snapshot_revision],
        ['31', archive.CHROME_31_REVISION],
        ['30', archive.CHROME_30_REVISION],
        ['29', archive.CHROME_29_REVISION]
    ]
    code = 0
    for version in versions:
      if options.chrome_version and version[0] != options.chrome_version:
        continue
      download_site = archive.Site.CONTINUOUS
      version_name = version[0]
      if version_name == 'HEAD':
        version_name = version[1]
        download_site = archive.Site.SNAPSHOT
      chrome_path = DownloadChrome(version_name, version[1], download_site)
      code1 = RunPythonTests(chromedriver,
                             ref_chromedriver,
                             chrome=chrome_path,
                             chrome_version=version[0],
                             chrome_version_name='v%s' % version_name)
      code2 = RunJavaTests(chromedriver, chrome=chrome_path,
                           chrome_version=version[0],
                           chrome_version_name='v%s' % version_name)
      code = code or code1 or code2
    cpp_tests = os.path.join(build_dir, cpp_tests_name)
    return RunCppTests(cpp_tests) or code


if __name__ == '__main__':
  sys.exit(main())
