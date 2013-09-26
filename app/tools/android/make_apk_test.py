#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import shutil
import subprocess
import sys
import unittest

def Clean(name):
  if os.path.exists(name):
    shutil.rmtree(name)
  if os.path.isfile(name + '.apk'):
    os.remove(name + '.apk')


class TestMakeApk(unittest.TestCase):
  def setUp(self):
    target_dir = os.path.join(options.build_dir,
                              options.target,
                              'xwalk_app_template')
    if os.path.exists(target_dir):
      os.chdir(target_dir)
    else:
      unittest.SkipTest('xwalk_app_template folder doesn\'t exist. '
                        'Skipping all tests in make_apk_test.py')

  def testName(self):
    proc = subprocess.Popen(['python', 'make_apk.py',
                             '--package=org.xwalk.example'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The APK name is required!') != -1)
    Clean('Example')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example'],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The APK name is required!') == -1)
    Clean('Example')

  def testPackage(self):
    proc = subprocess.Popen(['python', 'make_apk.py',
                             '--name=Example'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The package name is required!') != -1)
    Clean('Example')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example'],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The package name is required!') == -1)
    Clean('Example')

  def testEntry(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') != -1)
    Clean('Example')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com'],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') == -1)
    Clean('Example')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example', '--app-root=./'],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') == -1)
    Clean('Example')

  def testIcon(self):
    icon_path = './app_src/res/drawable-xhdpi/crosswalk.png'
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--icon=%s' % icon_path],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(content.find('crosswalk'))
    self.assertTrue(os.path.exists('Example/res/drawable'))
    Clean('Example')

  def testFullscreen(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com', '-f'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('Fullscreen'))
    Clean('Example')

  def testEnableRemoteDebugging(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--enable-remote-debugging'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    activity = 'Example/src/org/xwalk/example/ExampleActivity.java'
    with open(activity, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(activity))
    self.assertTrue(content.find('setRemoteDebugging'))
    Clean('Example')


if __name__ == '__main__':
  parser = optparse.OptionParser()
  info = ('The build directory for xwalk.'
          'Such as: --build-dir=src/out')
  parser.add_option('--build-dir', help=info)
  info = ('The build target for xwalk.'
          'Such as: --target=Release')
  parser.add_option('--target', help=info)
  options, temp = parser.parse_args()
  if len(sys.argv) == 1:
    parser.print_help()
    sys.exit(1)

  del sys.argv[1:]
  unittest.main(verbosity=2)
