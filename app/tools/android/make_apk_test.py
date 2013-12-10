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
  @classmethod
  def setUpClass(cls):
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
    cls._mode = ''
    if options.mode == 'shared':
      cls._mode = '--mode=shared'
    elif options.mode == 'embedded':
      cls._mode = '--mode=embedded'
    cls.fakeNativeLibrary()

  @classmethod
  def tearDownClass(cls):
    # Clean the test data.
    test_data_dir = os.path.join(options.build_dir,
                                 options.target,
                                 'xwalk_app_template',
                                 'test_data')
    if os.path.exists(test_data_dir):
      shutil.rmtree(test_data_dir)
    cls.restoreNativeLibrary()

  @staticmethod
  def fakeNativeLibrary():
    # To reduce the time consumption of make_apk test for embedded mode,
    # replace the original native library with an empty library.
    # Because it doesn't affect the result of test.
    if options.mode == 'embedded':
      native_library_dir = os.path.join('native_libs', 'libs')
      native_library_temp_dir = os.path.join('native_libs', 'temp')
      shutil.copytree(native_library_dir, native_library_temp_dir)
      for root, _, files in os.walk(native_library_dir):
        if 'libxwalkcore.so' in files:
          native_library_path = os.path.join(root, 'libxwalkcore.so')
          # Remove the original library
          os.remove(native_library_path)
          # Create an empty library file
          open(native_library_path, 'a').close()

  @staticmethod
  def restoreNativeLibrary():
    # Restore the original native library for embedded mode.
    if options.mode == 'embedded':
      native_library_dir = os.path.join('native_libs', 'libs')
      native_library_temp_dir = os.path.join('native_libs', 'temp')
      shutil.rmtree(native_library_dir)
      shutil.move(native_library_temp_dir, native_library_dir)

  def checkApk(self, apk_path):
    # Check files whether is contained in the apk.
    proc = subprocess.Popen(['jar', 'tvf', apk_path],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    common_files = ['AndroidManifest.xml', 'classes.dex']
    for res_file in common_files:
      self.assertTrue(out.find(res_file) != -1)
    if self._mode.find('embedded') != -1:
      embedded_related_files = ['xwalk.pak',
                                'presentation_api.js',
                                'device_capabilities_api.js',
                                'libxwalkcore.so']
      for res_file in embedded_related_files:
        self.assertTrue(out.find(res_file) != -1)

  def testName(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--app-version=1.0.0',
                             '--package=org.xwalk.example', self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The APK name is required!') != -1)
    Clean('Example')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example', self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The APK name is required!') == -1)
    Clean('Example')
    # The following invalid chars verification is too heavy for embedded mode,
    # and the result of verification should be the same between shared mode
    # and embedded mode. So only do the verification in the shared mode.
    if self._mode.find('shared') != -1:
      invalid_chars = '\/:.*?"<>|- '
      for c in invalid_chars:
        invalid_name = '--name=Example' + c
        proc = subprocess.Popen(['python', 'make_apk.py', invalid_name,
                                 '--app-version=1.0.0',
                                 '--package=org.xwalk.example',
                                 '--app-url=http://www.intel.com',
                                 self._mode],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)
        out, _ = proc.communicate()
        self.assertTrue(out.find('Illegal character') != -1)
        Clean('Example_')

  def testAppDescriptionAndVesion(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--package=org.xwalk.example',
                             '--app-version=1.0.0',
                             '--description=a sample application',
                             '--app-url=http://www.intel.com',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('description') != -1)
    self.assertTrue(content.find('versionName') != -1)
    self.checkApk('Example.apk')
    Clean('Example')

  def testPermissions(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--permissions="geolocation"',
                             '--app-url=http://www.intel.com',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('LOCATION_HARDWARE') != -1)
    self.checkApk('Example.apk')
    Clean('Example')

  def testPackage(self):
    proc = subprocess.Popen(['python', 'make_apk.py',
                             '--name=Example', '--app-version=1.0.0',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The package name is required!') != -1)
    Clean('Example')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The package name is required!') == -1)
    Clean('Example')

  def testEntry(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') == -1)
    self.assertTrue(os.path.exists('Example.apk'))
    self.checkApk('Example.apk')
    Clean('Example')

    test_entry_root = 'test_data/entry'
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-root=%s' % test_entry_root,
                             '--app-local-path=index.html',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') == -1)
    self.assertTrue(os.path.exists('Example.apk'))
    self.checkApk('Example.apk')
    Clean('Example')

  def testEntryWithErrors(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') != -1)
    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example')

    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--app-root=.',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') != -1)
    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example')

    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example', '--app-root=./',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('The entry is required.') != -1)
    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example')

    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-local-path=index.html',
                             self._mode],
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
                             '--icon=%s' % icon_path,
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(content.find('crosswalk') != -1)
    self.assertTrue(os.path.exists('Example/res/drawable'))
    self.checkApk('Example.apk')
    Clean('Example')

  def testFullscreen(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com', '-f',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('Fullscreen') != -1)
    self.checkApk('Example.apk')
    Clean('Example')

  def testEnableRemoteDebugging(self):
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--enable-remote-debugging',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    activity = 'Example/src/org/xwalk/example/ExampleActivity.java'
    with open(activity, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(activity))
    self.assertTrue(content.find('setRemoteDebugging') != -1)
    self.checkApk('Example.apk')
    Clean('Example')

  def testKeystore(self):
    keystore_path = os.path.join('test_data', 'keystore', 'xwalk-test.keystore')
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--keystore-path=%s' % keystore_path,
                             '--keystore-alias=xwalk-test',
                             '--keystore-passcode=xwalk-test',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    self.assertTrue(os.path.exists('Example'))
    self.assertTrue(os.path.exists('Example.apk'))
    proc = subprocess.Popen(['jarsigner', '-verify', '-keystore',
                             keystore_path, '-verbose', 'Example.apk'],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('smk') != -1)
    self.checkApk('Example.apk')
    Clean('Example')

  def testManifest(self):
    manifest_path = os.path.join('test_data', 'manifest', 'manifest.json')
    proc = subprocess.Popen(['python', 'make_apk.py',
                             '--manifest=%s' % manifest_path,
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('Fullscreen') != -1)
    self.assertTrue(os.path.exists('Example'))
    self.assertTrue(os.path.exists('Example.apk'))
    self.checkApk('Example.apk')
    Clean('Example')

  def testManifestWithError(self):
    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_no_app_launch_path.json')
    proc = subprocess.Popen(['python', 'make_apk.py',
                             '--manifest=%s' % manifest_path,
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = proc.communicate()
    self.assertTrue(out.find('no app launch path') != -1)

  def testExtensionsWithOneExtension(self):
    # Test with an existed extension.
    extension_path = 'test_data/extensions/myextension'
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--extensions=%s' % extension_path,
                             self._mode],
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
    self.checkApk('Example.apk')
    Clean('Example')

  def testExtensionsWithNonExtension(self):
    # Test with a non-existed extension.
    extension_path = 'test_data/extensions/myextension'
    proc = subprocess.Popen(['python', 'make_apk.py', '--name=Example',
                             '--app-version=1.0.0',
                             '--package=org.xwalk.example',
                             '--app-url=http://www.intel.com',
                             '--extensions=%s1' % extension_path,
                             self._mode],
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
                             '--extensions=%s' % extension_path,
                             self._mode],
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
    self.checkApk('Example.apk')
    Clean('Example')

  def testXPK(self):
    xpk_file = os.path.join('test_data', 'xpk', 'example.xpk')
    proc = subprocess.Popen(['python', 'make_apk.py', '--xpk=%s' % xpk_file,
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    self.assertTrue(os.path.exists('Example'))
    self.assertTrue(os.path.isfile('Example.apk'))
    self.checkApk('Example.apk')
    Clean('Example')

  def testXPKWithError(self):
    xpk_file = os.path.join('test_data', 'xpk', 'error.xpk')
    proc = subprocess.Popen(['python', 'make_apk.py', '--xpk=%s' % xpk_file,
                             self._mode],
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
                             '--orientation=landscape',
                             self._mode],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    _, _ = proc.communicate()
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('landscape') != -1)
    self.assertTrue(os.path.exists('Example'))
    self.assertTrue(os.path.isfile('Example.apk'))
    self.checkApk('Example.apk')
    Clean('Example')


if __name__ == '__main__':
  parser = optparse.OptionParser()
  info = ('The build directory for xwalk.'
          'Such as: --build-dir=src/out')
  parser.add_option('--build-dir', help=info)
  info = ('The build target for xwalk.'
          'Such as: --target=Release')
  parser.add_option('--target', help=info)
  info = ('The packaging mode for xwalk. Such as: --mode=embedded.'
          'Please refer the detail to the option of make_apk.py.')
  parser.add_option('--mode', help=info)
  options, temp = parser.parse_args()
  if len(sys.argv) == 1:
    parser.print_help()
    sys.exit(1)

  del sys.argv[1:]
  if options.mode:
    unittest.main(verbosity=2)
  else:
    # Run tests in both embedded and shared mode
    # when the mode option isn't specified.
    options.mode = 'embedded'
    print 'Run tests in embedded mode.'
    unittest.main(verbosity=2, exit=False)
    options.mode = 'shared'
    print 'Run tests in shared mode.'
    unittest.main(verbosity=2)
