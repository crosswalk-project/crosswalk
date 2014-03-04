# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""TestEnvironment classes.

These classes abstract away the various setups needed to run the WebDriver java
tests in various environments.
"""

import os
import sys

import chrome_paths
import util

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))

if util.IsLinux():
  sys.path.insert(0, os.path.join(chrome_paths.GetSrc(), 'build', 'android'))
  from pylib import android_commands
  from pylib import forwarder
  from pylib import valgrind_tools

ANDROID_TEST_HTTP_PORT = 2311
ANDROID_TEST_HTTPS_PORT = 2411

_EXPECTATIONS = {}
execfile(os.path.join(_THIS_DIR, 'test_expectations'), _EXPECTATIONS)


class BaseTestEnvironment(object):
  """Manages the environment java tests require to run."""

  def __init__(self, chrome_version='HEAD'):
    """Initializes a desktop test environment.

    Args:
      chrome_version: Optionally a chrome version to run the tests against.
    """
    self._chrome_version = chrome_version

  def GetOS(self):
    """Name of the OS."""
    raise NotImplementedError

  def GlobalSetUp(self):
    """Sets up the global test environment state."""
    pass

  def GlobalTearDown(self):
    """Tears down the global test environment state."""
    pass

  def GetDisabledJavaTestMatchers(self):
    """Get the list of disabled java test matchers.

    Returns:
      List of disabled test matchers, which may contain '*' wildcards.
    """
    return _EXPECTATIONS['GetDisabledTestMatchers'](
        self.GetOS(), self._chrome_version)

  def GetPassedJavaTests(self):
    """Get the list of passed java tests.

    Returns:
      List of passed test names.
    """
    with open(os.path.join(_THIS_DIR, 'java_tests.txt'), 'r') as f:
      return _EXPECTATIONS['ApplyJavaTestFilter'](
          self.GetOS(), self._chrome_version,
          [t.strip('\n') for t in f.readlines()])


class DesktopTestEnvironment(BaseTestEnvironment):
  """Manages the environment java tests require to run on Desktop."""

  # override
  def GetOS(self):
    return util.GetPlatformName()


class AndroidTestEnvironment(DesktopTestEnvironment):
  """Manages the environment java tests require to run on Android."""

  def __init__(self, chrome_version='HEAD'):
    super(AndroidTestEnvironment, self).__init__(chrome_version)
    self._adb = None
    self._forwarder = None

  # override
  def GlobalSetUp(self):
    os.putenv('TEST_HTTP_PORT', str(ANDROID_TEST_HTTP_PORT))
    os.putenv('TEST_HTTPS_PORT', str(ANDROID_TEST_HTTPS_PORT))
    self._adb = android_commands.AndroidCommands()
    forwarder.Forwarder.Map(
        [(ANDROID_TEST_HTTP_PORT, ANDROID_TEST_HTTP_PORT),
         (ANDROID_TEST_HTTPS_PORT, ANDROID_TEST_HTTPS_PORT)],
        self._adb)

  # override
  def GlobalTearDown(self):
    forwarder.Forwarder.UnmapAllDevicePorts(self._adb)

  # override
  def GetOS(self):
    return 'android'
