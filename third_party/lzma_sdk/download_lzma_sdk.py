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
import subprocess
import sys
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
  # Determine the file type
  cmd = shlex.split('file --mime-type {0}'.format(lzma_file))
  result = subprocess.check_output(cmd)
  mime_type = result.split()[-1]

  if mime_type == 'application/x-bzip2':
    cmd = ['tar', 'xjf', lzma_file, '-C', SDK_DIR, 'Java']
  elif mime_type == 'application/x-7z-compressed':
    cmd = ['7zr', 'x', '-o' + SDK_DIR, lzma_file, 'Java']
  else:
    print('Error: mime type of %s is not supported.' % lzma_file)
    return False

  target_dir = os.path.join(SDK_DIR, 'src')
  if os.path.exists(target_dir):
    shutil.rmtree(os.path.join(SDK_DIR, 'src'))

  subprocess.call(cmd)
  # Need rename Java to src so that java.gypi can recognize.
  os.rename(os.path.join(SDK_DIR, 'Java'), os.path.join(SDK_DIR, 'src'))
  return True


def main():
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
