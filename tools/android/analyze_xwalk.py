#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This script is responsible for analyze the tombstone or ANR files
from Android deivces.
"""

import argparse
import os
import platform
import re
import subprocess
import sys


ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '..', '..', '..')


def GetPrebuiltPath(target_arch='x86'):
  # GCC verion for Android is hard coded as other chroium tool does,
  # for eg: src/build/android/adb_gdb
  gcc_version = '4.6'
  if target_arch == 'x86':
    gcc_arch_dir = 'x86-' + gcc_version
  elif target_arch == 'arm':
    gcc_arch_dir = 'arm-linux-androideabi-' + gcc_version
  elif target_arch == 'mips':
    gcc_arch_dir = 'mipsel-linux-android-' + gcc_version

  host_arch = platform.system().lower() + '-' + platform.machine()
  return os.path.join(ROOT_DIR, 'third_party', 'android_tools',
                      'ndk', 'toolchains',
                      gcc_arch_dir, 'prebuilt',
                      host_arch, 'bin')


def get_addr2line_bin(target_arch='x86'):
  if target_arch == 'x86':
    addr2line_bin_name = 'i686-linux-android-addr2line'
  elif target_arch == 'arm':
    addr2line_bin_name = 'arm-linux-androideabi-addr2line'
  elif target_arch == 'mips':
    addr2line_bin_name = 'mipsel-linux-android-addr2line'

  addr2line_bin = os.path.join(GetPrebuiltPath(target_arch),
                               addr2line_bin_name)
  if not os.path.exists(addr2line_bin):
    print '[ERROR] %s could not be found.' % addr2line_bin
    sys.exit(1)

  return addr2line_bin


def DoAnalysis(args):
  if args.build_dir is None:
    lib_dir = os.path.join(ROOT_DIR, 'out', 'Debug', 'lib')
  else:
    lib_dir = os.path.join(args.build_dir, 'lib')

  if not os.path.exists(lib_dir):
    print '[ERROR] %s could not be found.' % lib_dir
    sys.exit(1)

  if args.type == 'tombstone':
    tombstone_dir = '/data/tombstones'
    result = subprocess.check_output(['adb', 'shell', 'ls %s' % tombstone_dir])
    files = result.strip('\n').split('\n')
    # TODO(halton): Get latest tombstone files instead.
    file_name = tombstone_dir + '/' + files[len(files) - 1].strip('\r')
  elif args.type == 'anr':
    file_name = '/data/anr/traces.txt'

  print '[INFO] Stack trace of %s.\n' % file_name
  output = subprocess.check_output(['adb', 'shell', 'cat ' + file_name])
  lines = output.strip('\n').split('\n')
  for line in lines:
    pattern = re.compile('pc (.*)  .*lib(.*)\.so')
    match = pattern.search(line)
    if match is None:
      continue

    name = match.group(2)
    path = os.path.join(lib_dir, 'lib%s.so' % name)
    if not os.path.exists(path):
      continue

    print subprocess.check_output([get_addr2line_bin(args.target_arch),
                                   '-C', '-e', path, '-f', match.group(1)])


def main():
  parser = argparse.ArgumentParser(description='Analyze the tombstone'
                                               ' or ANR files')

  parser.add_argument('--build-dir',
                      help='directory of Crosswalk debugging libraries. '
                           'Default is out/Debug',
                      default=None)
  parser.add_argument('--type',
                      help='tombstone or ANR. Default is tombstone',
                      default='tombstone',
                      choices=['tombstone', 'anr'])
  parser.add_argument('--target-arch',
                      help='target architecture. Default is x86',
                      default='x86',
                      choices=['x86', 'arm', 'mips'])

  args = parser.parse_args()
  DoAnalysis(args)


if __name__ == '__main__':
  main()
