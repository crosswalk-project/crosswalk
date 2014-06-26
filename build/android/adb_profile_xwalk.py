#!/usr/bin/env python
#
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

chrome_src = os.environ['CHROME_SRC']
chrome_tool_path = os.path.join(chrome_src, 'build', 'android')
sys.path.append(chrome_tool_path)

# Below two modules should be imported at runtime, but pylint can not find it,
# add below pylint attribute to ignore this error.
#
# pylint: disable=F0401
import adb_profile_chrome
from pylib import constants

# Wrapper for package info, the key 'stable' is needed by adb_profile_chrome.
PACKAGE_INFO = {
    'xwalk_embedded_shell': constants.PackageInfo(
        'org.xwalk.runtime.client.embedded.shell',
        'org.xwalk.runtime.client.embedded.shell'
        '.XWalkRuntimeClientEmbeddedShellActivity',
        '/data/local/tmp/xwview-shell-command-line',
        None,
        None),
}


def _GetSupportedBrowsers():
  # Add aliases for backwards compatibility.
  supported_browsers = {
    'stable': PACKAGE_INFO['xwalk_embedded_shell']
  }
  supported_browsers.update(constants.PACKAGE_INFO)
  unsupported_browsers = ['content_browsertests', 'gtest', 'legacy_browser']
  for browser in unsupported_browsers:
    del supported_browsers[browser]
  return supported_browsers


def main():
  adb_profile_chrome._GetSupportedBrowsers = _GetSupportedBrowsers
  adb_profile_chrome.main()


if __name__ == '__main__':
  sys.exit(main())
