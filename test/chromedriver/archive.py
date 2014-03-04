# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Downloads items from the Chromium continuous archive."""

import os
import platform
import urllib

import util

CHROME_29_REVISION = '208261'
CHROME_30_REVISION = '217281'
CHROME_31_REVISION = '225096'

_SITE = 'http://commondatastorage.googleapis.com'


class Site(object):
  CONTINUOUS = _SITE + '/chromium-browser-continuous'
  SNAPSHOT = _SITE + '/chromium-browser-snapshots'


def GetLatestRevision(site=Site.CONTINUOUS):
  """Returns the latest revision (as a string) available for this platform.

  Args:
    site: the archive site to check against, default to the continuous one.
  """
  url = site + '/%s/LAST_CHANGE'
  return urllib.urlopen(url % _GetDownloadPlatform()).read()


def DownloadChrome(revision, dest_dir, site=Site.CONTINUOUS):
  """Downloads the packaged Chrome from the archive to the given directory.

  Args:
    revision: the revision of Chrome to download.
    dest_dir: the directory to download Chrome to.
    site: the archive site to download from, default to the continuous one.

  Returns:
    The path to the unzipped Chrome binary.
  """
  def GetZipName():
    if util.IsWindows():
      return 'chrome-win32'
    elif util.IsMac():
      return 'chrome-mac'
    elif util.IsLinux():
      return 'chrome-linux'
  def GetChromePathFromPackage():
    if util.IsWindows():
      return 'chrome.exe'
    elif util.IsMac():
      return 'Chromium.app/Contents/MacOS/Chromium'
    elif util.IsLinux():
      return 'chrome'
  zip_path = os.path.join(dest_dir, 'chrome-%s.zip' % revision)
  if not os.path.exists(zip_path):
    url = site + '/%s/%s/%s.zip' % (_GetDownloadPlatform(), revision,
                                    GetZipName())
    print 'Downloading', url, '...'
    urllib.urlretrieve(url, zip_path)
  util.Unzip(zip_path, dest_dir)
  return os.path.join(dest_dir, GetZipName(), GetChromePathFromPackage())


def _GetDownloadPlatform():
  """Returns the name for this platform on the archive site."""
  if util.IsWindows():
    return 'Win'
  elif util.IsMac():
    return 'Mac'
  elif util.IsLinux():
    if platform.architecture()[0] == '64bit':
      return 'Linux_x64'
    else:
      return 'Linux'
