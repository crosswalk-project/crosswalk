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
      # Prepare the test data.
      test_src_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                                  'test_data')
      test_des_dir = os.path.join(target_dir, 'test_data')
      if not os.path.exists(test_des_dir):
        shutil.copytree(test_src_dir, test_des_dir)
      os.chdir(target_dir)
    else:
      unittest.SkipTest('xwalk_app_template folder doesn\'t exist. '
                        'Skipping all tests in make_apk_test.py')

  def tearDown(self):
    # Clean the test data
    test_data_dir = os.path.join(options.build_dir,
                                 options.target,
                                 'xwalk_app_template',
                                 'test_data')
    if os.path.exists(test_data_dir):
      shutil.rmtree(test_data_dir)

  def testName(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--app-version=1.0.0',
                             '--package=org.xwalk.example'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The APK name is required!') != -1)
    Clean('Example')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The APK name is required!') == -1)
    Clean('Example')
    invalid_chars = '\/:.*?"<>|- '
    for c in invalid_chars:
      invalid_name = '--name=Example' + c
      proc = subprocess.Popen(['python', 'make_apk.py', invalid_name,
                               '--app-version=1.0.0',
                               '--package=org.xwalk.example',
                               '--app-url=http://www.intel.com'],
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
      out, _ = proc.communicate()
      self.assertTrue(out.find('Illegal character') != -1)
      Clean('Example_')

  def testAppVersion(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example',
                             '--app-version=1.0.0',
                             '--app-url=http://www.intel.com'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('versionName') != -1)
    Clean('Example')

  def testAppDescription(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example',
                             '--app-version=1.0.0',
                             '--description=a sample application',
                             '--app-url=http://www.intel.com'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('description') != -1)
    Clean('Example')

  def testPermissions(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--permissions="geolocation"',
                             '--app-url=http://www.intel.com'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('LOCATION_HARDWARE') != -1)
    Clean('Example')

  def testPackage(self):
    proc = subprocess.Popen(['python', 'make_apk.py',
                             '--name=Example', '--app-version=1.0.0'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The package name is required!') != -1)
    Clean('Example')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The package name is required!') == -1)
    Clean('Example')

  def testEntry(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') == -1)
    self.assertTrue(os.path.exists('Example.apk'))
    Clean('Example')

    test_entry_root = 'test_data/entry'
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-root=%s' % test_entry_root,
                             '--app-local-path=index.html'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') == -1)
    self.assertTrue(os.path.exists('Example.apk'))
    Clean('Example')

  def testEntryWithErrors(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') != -1)
    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example')

    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--app-root=.'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') != -1)
    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example')

    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example', '--app-root=./'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') != -1)
    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example')

    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-local-path=index.html'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') != -1)
    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example')

  def testIcon(self):
    icon_path = './app_src/res/drawable-xhdpi/crosswalk.png'
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--icon=%s' % icon_path],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(content.find('crosswalk') != -1)
    self.assertTrue(os.path.exists('Example/res/drawable'))
    Clean('Example')

  def testFullscreen(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com', '-f'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('Fullscreen') != -1)
    Clean('Example')

  def testEnableRemoteDebugging(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--enable-remote-debugging'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    activity = 'Example/src/org/xwalk/example/ExampleActivity.java'
    with open(activity, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(activity))
    self.assertTrue(content.find('setRemoteDebugging') != -1)
    Clean('Example')

  def testKeystore(self):
    keystore_path = os.path.join('test_data', 'keystore', 'xwalk-test.keystore')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--keystore-path=%s' % keystore_path,
                             '--keystore-alias=xwalk-test',
                             '--keystore-passcode=xwalk-test'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    self.assertTrue(os.path.exists('Example'))
    self.assertTrue(os.path.exists('Example.apk'))
    proc = subprocess.Popen(['jarsigner', '-verify', '-keystore',
                             keystore_path, '-verbose', 'Example.apk'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('smk') != -1)
    Clean('Example')

  def testManifest(self):
    manifest_path = os.path.join('test_data', 'manifest', 'manifest.json')
    proc = subprocess.Popen(['python', 'make_apk.py',
                             '--manifest=%s' % manifest_path],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('Fullscreen') != -1)
    self.assertTrue(os.path.exists('Example'))
    self.assertTrue(os.path.exists('Example.apk'))
    Clean('Example')

  def testExtensionsWithOneExtension(self):
    # Test with an existed extension.
    extension_path = 'test_data/extensions/myextension'
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--extensions=%s' % extension_path],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    self.assertTrue(os.path.exists('Example'))
    self.assertTrue(os.path.exists('Example.apk'))
    extensions_config_json = 'Example/assets/extensions-config.json'
    self.assertTrue(os.path.exists(extensions_config_json))
    with open(extensions_config_json, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(content.find('xwalk-extensions/myextension/myextension.js'))
    self.assertTrue(content.find('com.example.extension.MyExtension'))
    extension_js = 'Example/assets/xwalk-extensions/myextension/myextension.js'
    self.assertTrue(os.path.exists(extension_js))
    extension_jar = 'Example/xwalk-extensions/myextension/myextension.jar'
    self.assertTrue(os.path.exists(extension_jar))
    Clean('Example')

  def testExtensionsWithNonExtension(self):
    # Test with a non-existed extension.
    extension_path = 'test_data/extensions/myextension'
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--extensions=%s1' % extension_path],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    error_msg = 'Error: can\'t find the extension directory'
    self.assertTrue(out.find(error_msg) != -1)
    self.assertTrue(out.find('Exiting with error code: 9') != -1)

  def testExtensionWithPermissions(self):
    test_entry_root = 'test_data/entry'
    # Add redundant separators for test.
    extension_path = 'test_data//extensions/contactextension/'
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-root=%s' % test_entry_root,
                             '--app-local-path=contactextension.html',
                             '--extensions=%s' % extension_path],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    self.assertTrue(os.path.exists('Example'))
    self.assertTrue(os.path.exists('Example.apk'))
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('android.permission.WRITE_CONTACTS') != -1)
    self.assertTrue(content.find('android.permission.READ_CONTACTS') != -1)
    Clean('Example')

  def testMode(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    self.assertTrue(os.path.exists('Example.apk'))
    shared_mode_size = os.path.getsize('Example.apk')
    Clean('Example')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--mode=embedded'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    self.assertTrue(os.path.exists('Example.apk'))
    embeded_mode_size = os.path.getsize('Example.apk')
    self.assertTrue(shared_mode_size < embeded_mode_size)
    Clean('Example')

  def testXPK(self):
    xpk_file = os.path.join('test_data', 'xpk', 'example.xpk')
    proc = subprocess.Popen(['python', 'make_apk.py', '--xpk=%s' % xpk_file],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    self.assertTrue(os.path.exists('Example'))
    self.assertTrue(os.path.isfile('Example.apk'))
    Clean('Example')

  def testXPKWithError(self):
    xpk_file = os.path.join('test_data', 'xpk', 'error.xpk')
    proc = subprocess.Popen(['python', 'make_apk.py', '--xpk=%s' % xpk_file],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    error_msg = 'XPK doesn\'t contain manifest file'
    self.assertTrue(out.find(error_msg) != -1)
    self.assertFalse(os.path.exists('Example'))
    self.assertFalse(os.path.isfile('Example.apk'))

  def testOrientation(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--orientation=landscape'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('landscape') != -1)
    self.assertTrue(os.path.exists('Example'))
    self.assertTrue(os.path.isfile('Example.apk'))
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
