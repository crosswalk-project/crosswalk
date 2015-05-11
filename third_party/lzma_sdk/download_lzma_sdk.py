#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This script will do:
  1. Download 7zip SDK.
  2. unzip SDK and prepare under xwalk directory.
"""

import os
import shlex
import shutil
import sys
import tarfile
import urllib2

SDK_URL = 'http://downloads.sourceforge.net/sevenzip/lzma920.tar.bz2'
SDK_DIR = os.path.dirname(os.path.abspath(__file__))


def DownloadFromURL(url, dest_file):
  if os.path.exists(dest_file) and os.path.isfile(dest_file):
    return True

  f = urllib2.urlopen(url)
  # TODO(halton): return False when error
  with open(dest_file, 'wb') as code:
    code.write(f.read())
  return True


def ExtractJavaSDK(lzma_file):
  tar = tarfile.open(lzma_file)
  java_dir = [tarinfo for tarinfo in tar.getmembers()
              if tarinfo.name.startswith('Java/')]
  tar.extractall(SDK_DIR, java_dir)
  tar.close()
  target_dir = os.path.join(SDK_DIR, 'src')
  if os.path.exists(target_dir):
    shutil.rmtree(os.path.join(SDK_DIR, 'src'))

  # Need rename Java to src so that java.gypi can recognize.
  os.rename(os.path.join(SDK_DIR, 'Java'), os.path.join(SDK_DIR, 'src'))
  return True


def main():
  if not sys.platform.startswith('linux'):
    return 0

  sdk_file = os.path.join(SDK_DIR, os.path.basename(SDK_URL))
  valid_java_file = os.path.join(SDK_DIR, 'src', 'SevenZip', 'CRC.java')

  if not os.path.exists(SDK_DIR):
    os.mkdir(SDK_DIR)
  elif os.path.exists(sdk_file) and os.path.exists(valid_java_file):
    print('LZMA SDK already exists. Nothing done.')
    return 0

  if not DownloadFromURL(SDK_URL, sdk_file):
    return 1

  return (1, 0)[ExtractJavaSDK(sdk_file)]


if __name__ == '__main__':
  sys.exit(main())
