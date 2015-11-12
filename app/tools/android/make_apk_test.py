#!/usr/bin/env python
# coding: UTF-8

# Copyright (c) 2013, 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import shutil
import subprocess
import sys
import unittest
import warnings


def Clean(name, app_version):
  if os.path.exists(name):
    shutil.rmtree(name)
  if options.mode == 'shared':
    if os.path.isfile(name + '_' + app_version + '.apk'):
      os.remove(name + '_' + app_version + '.apk')
  else:
    if os.path.isfile(name + '_' + app_version + '_x86.apk'):
      os.remove(name + '_' + app_version + '_x86.apk')
    if os.path.isfile(name + '_' + app_version + '_x86_64.apk'):
      os.remove(name + '_' + app_version + '_x86_64.apk')
    if os.path.isfile(name + '_' + app_version + '_arm.apk'):
      os.remove(name + '_' + app_version + '_arm.apk')
    if os.path.isfile(name + '_' + app_version + '_arm64.apk'):
      os.remove(name + '_' + app_version + '_arm64.apk')


def CompareSizeForCompressor(mode, original, ext, name, fun):
  size = 0
  compressed_size = 0
  mode_list = ['all', 'js', 'css']

  www_dir = os.path.join(name, 'assets', 'www')
  if os.path.exists(www_dir):
    size = GetFileSize(original)
    compressed_file = os.path.join(www_dir, ext, 'test.' + ext)
    compressed_size = GetFileSize(compressed_file)

    if mode in mode_list:
      fun(compressed_size < size)
    else:
      fun(size == compressed_size)
  else:
    print('Error: %s is not exist.' % www_dir)


def GetFileSize(file_path):
  size = 0
  if os.path.exists(file_path):
    size = os.path.getsize(file_path)
  return size


def RunCommand(command):
  """Runs the command list, return the output."""
  proc = subprocess.Popen(command, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, shell=False)
  return proc.communicate()[0]


def GetResultWithOption(mode=None, manifest=None, name=None, package=None):
  app_url = None
  if manifest is not None:
    manifest = '--manifest=' + manifest
  else:
    app_url = '--app-url=http://www.intel.com'
  if name is not None:
    name = '--name=' + name
  if package is not None:
    package = '--package=' + package

  cmd = ['python', 'make_apk.py',
         '--app-version=1.0.0',
         '%s' % manifest,
         '%s' % name,
         '%s' % package,
         '%s' % app_url,
         '%s' % mode]
  return RunCommand(cmd)


class TestMakeApk(unittest.TestCase):
  @classmethod
  def setUpClass(cls):
    cls._original_dir = os.getcwd()
    if options.tool_path:
      target_dir = os.path.expanduser(options.tool_path)
    elif options.build_dir and options.target:
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
        # get jar files of external extensions
        if not options.external_extensions:
          options.external_extensions = os.path.join(target_dir,
                                                     os.pardir, "lib.java")

        extensions_dir = os.path.join(test_des_dir, "extensions")
        for e_name in os.listdir(extensions_dir):
          e_dir = os.path.join(extensions_dir, e_name)
          src_file = os.path.join(options.external_extensions,
                                  "%s.jar" % e_name)
          if os.path.isfile(src_file):
            dst_file = os.path.join(e_dir, "%s.jar" % e_name)
            shutil.copyfile(src_file, dst_file)
          else:
            print('Failed to get jar file of %s,'
                   'related cases may fail.' % e_name)

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
    if options.tool_path:
      test_data_dir = os.path.join(os.path.expanduser(options.tool_path),
                                   'test_data')
    elif options.build_dir and options.target:
      test_data_dir = os.path.join(options.build_dir,
                                   options.target,
                                   'xwalk_app_template',
                                   'test_data')
    if os.path.exists(test_data_dir):
      shutil.rmtree(test_data_dir)
    cls.restoreNativeLibrary()
    os.chdir(cls._original_dir)

  @staticmethod
  def fakeNativeLibrary():
    # To reduce the time consumption of make_apk test for embedded mode,
    # replace the original native library with an empty library.
    # Because it doesn't affect the result of test.
    if options.mode == 'embedded':
      native_library_dir = os.path.join('xwalk_core_library', 'libs')
      native_library_temp_dir = 'temp'
      Clean('temp', '0') # May be left over from aborted tests
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
      native_library_dir = os.path.join('xwalk_core_library', 'libs')
      native_library_temp_dir = 'temp'
      if os.path.isdir(native_library_dir):
        shutil.rmtree(native_library_dir)
      shutil.move(native_library_temp_dir, native_library_dir)

  @staticmethod
  def archs():
    x86_native_lib_path = os.path.join('xwalk_core_library', 'libs',
                                       'x86', 'libxwalkcore.so')
    x86_64_native_lib_path = os.path.join('xwalk_core_library', 'libs',
                                       'x86_64', 'libxwalkcore.so')
    arm_native_lib_path = os.path.join('xwalk_core_library', 'libs',
                                       'armeabi-v7a', 'libxwalkcore.so')
    arm64_native_lib_path = os.path.join('xwalk_core_library', 'libs',
                                         'arm64-v8a', 'libxwalkcore.so')
    arch_list = []
    if os.path.isfile(x86_native_lib_path):
      arch_list.append('x86')
    if os.path.isfile(x86_64_native_lib_path):
      arch_list.append('x86_64')
    if os.path.isfile(arm_native_lib_path):
      arch_list.append('arm')
    if os.path.isfile(arm64_native_lib_path):
      arch_list.append('arm64')
    return arch_list

  def checkApks(self, apk_name, app_version, keystore_path=None):
    apks = []
    if self._mode.find('shared') != -1:
      apk_path = '%s_%s.apk' % (apk_name, app_version)
      apks.append((apk_path, ''))
    elif self._mode.find('embedded') != -1:
      x86_apk_path = '%s_%s_x86.apk' % (apk_name, app_version)
      if os.path.exists(x86_apk_path):
        apks.append((x86_apk_path, 'x86'))
      x86_64_apk_path = '%s_%s_x86_64.apk' % (apk_name, app_version)
      if os.path.exists(x86_64_apk_path):
        apks.append((x86_64_apk_path, 'x86_64'))
      arm_apk_path = '%s_%s_arm.apk' % (apk_name, app_version)
      if os.path.exists(arm_apk_path):
        apks.append((arm_apk_path, 'arm'))
      arm64_apk_path = '%s_%s_arm64.apk' % (apk_name, app_version)
      if os.path.exists(arm64_apk_path):
        apks.append((arm64_apk_path, 'arm64'))

    for apk, apk_arch in apks:
      self.checkApk(apk, apk_arch, keystore_path)

  def checkApk(self, apk_path, arch, keystore_path=None):
    """Checks whether some files are contained in the given APK,
    and optionally verifies its signature."""
    cmd = ['jar', 'tvf', apk_path]
    out = RunCommand(cmd)
    common_files = ['AndroidManifest.xml', 'classes.dex']
    for res_file in common_files:
      self.assertTrue(out.find(res_file) != -1)
    if self._mode.find('embedded') != -1:
      embedded_related_files = ['icudtl.dat',
                                'xwalk.pak',
                                # Please refer to XWALK-3516, disable v8 use
                                # external startup data, reopen it if needed
                                # later.
                                # 'natives_blob.bin',
                                # 'snapshot_blob.bin',
                                'device_capabilities_api.js',
                                'launch_screen_api.js',
                                'presentation_api.js',
                                'wifidirect_api.js']
      for res_file in embedded_related_files:
        self.assertTrue(out.find(res_file) != -1)
    if arch == 'x86':
      self.assertTrue(out.find('x86/libxwalkcore.so') != -1)
    elif arch == 'x86_64':
      self.assertTrue(out.find('x86_64/libxwalkcore.so') != -1)
    elif arch == 'arm':
      self.assertTrue(out.find('armeabi-v7a/libxwalkcore.so') != -1)
    elif arch == 'arm64':
      self.assertTrue(out.find('arm64-v8a/libxwalkcore.so') != -1)

    if keystore_path:
      cmd = ['jarsigner', '-verify', '-keystore', keystore_path,
             '-verbose', apk_path]
      out = RunCommand(cmd)
      self.assertIn('smk', out)

  def testName(self):
    cmd = ['python', 'make_apk.py', '--app-version=1.0.0',
           '--app-url=http://www.intel.com',
           '--package=org.xwalk.example', self._mode]
    out = RunCommand(cmd)
    Clean('Example', '1.0.0')
    self.assertIn('An APK name is required', out)

    cmd = ['python', 'make_apk.py', '--name=Test_Example',
           '--app-version=1.0.0', '--app-url=http://www.intel.com',
           '--package=org.xwalk.example', self._mode]
    out = RunCommand(cmd)
    self.assertNotIn('An APK name is required', out)
    Clean('Example', '1.0.0')

  def testToolVersion(self):
    cmd = ['python', 'make_apk.py', '--version']
    out = RunCommand(cmd)
    self.assertTrue(out.find('Crosswalk app packaging tool version') != -1)

  def testAppDescriptionAndVersion(self):
    cmd = ['python', 'make_apk.py', '--name=Example',
           '--package=org.xwalk.example', '--app-version=1.0.0',
           '--description=a sample application',
           '--app-url=http://www.intel.com',
           '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('description') != -1)
    self.assertTrue(content.find('versionName') != -1)
    self.checkApks('Example', '1.0.0')

  def testAppVersionCodeFromVersionName(self):
    if self._mode.find('embedded') == -1:
      return
    if 'x86' in self.archs():
      arch = '--arch=x86'
      versionCode = 'versionCode="60100000"'
    elif 'x86_64' in self.archs():
      arch = '--arch=x86_64'
      versionCode = 'versionCode="70100000"'
    elif 'arm64' in self.archs():
      arch = '--arch=arm64'
      versionCode = 'versionCode="30100000"'
    else:
      arch = '--arch=arm'
      versionCode = 'versionCode="20100000"'
    cmd = ['python', 'make_apk.py', '--name=Example',
           '--package=org.xwalk.example', '--app-version=1.0.0',
           '--description=a sample application',
           arch,
           '--app-url=http://www.intel.com',
           '--project-dir=.',
           self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find(versionCode) != -1)
    self.checkApks('Example', '1.0.0')

  def testAppVersionCode(self):
    cmd = ['python', 'make_apk.py', '--name=Example',
           '--package=org.xwalk.example', '--app-version=1.0.0',
           '--description=a sample application',
           '--app-versionCode=3',
           '--app-url=http://www.intel.com',
           '--project-dir=.',
           self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('versionCode="3"') != -1)
    self.checkApks('Example', '1.0.0')

  def testAppVersionCodeBase(self):
    # Arch option only works for embedded mode,
    # so only test it for embedded mode.
    if self._mode.find('embedded') == -1:
      return
    if 'x86' in self.archs():
      arch = '--arch=x86'
      versionCode = 'versionCode="60000003"'
    elif 'x86_64' in self.archs():
      arch = '--arch=x86_64'
      versionCode = 'versionCode="70000003"'
    elif 'arm64' in self.archs():
      arch = '--arch=arm64'
      versionCode = 'versionCode="30000003"'
    else:
      arch = '--arch=arm'
      versionCode = 'versionCode="20000003"'
    cmd = ['python', 'make_apk.py', '--name=Example',
           '--package=org.xwalk.example', '--app-version=1.0.0',
           '--description=a sample application',
           '--app-versionCodeBase=3',
           arch,
           '--app-url=http://www.intel.com',
           '--project-dir=.',
           self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find(versionCode) != -1)
    self.checkApks('Example', '1.0.0')

  def testAppBigVersionCodeBase(self):
    # Arch option only works for embedded mode,
    # so only test it for embedded mode.
    if self._mode.find('embedded') == -1:
      return
    if 'x86' in self.archs():
      arch = '--arch=x86'
    elif 'x86_64' in self.archs():
      arch = '--arch=x86_64'
    elif 'arm64' in self.archs():
      arch = '--arch=arm64'
    else:
      arch = '--arch=arm'
    cmd = ['python', 'make_apk.py', '--name=Example',
           '--package=org.xwalk.example', '--app-version=1.0.0',
           '--description=a sample application',
           '--app-versionCodeBase=3000000',
           arch,
           '--app-url=http://www.intel.com',
           '--project-dir=.',
           self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    self.assertTrue(os.path.exists(manifest))

  def testPermissions(self):
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--permissions=geolocation',
           '--app-url=http://www.intel.com',
           '--project-dir=.',
           self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('ACCESS_FINE_LOCATION') != -1)
    self.checkApks('Example', '1.0.0')

  def testPermissionsWithError(self):
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--permissions=UndefinedPermission',
           '--app-url=http://www.intel.com', self._mode]
    out = RunCommand(cmd)
    self.assertTrue(out.find('related API is not supported.') != -1)
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example',
           '--permissions=Contacts.Geolocation.Messaging',
           '--app-url=http://www.intel.com', self._mode]
    out = RunCommand(cmd)
    self.assertTrue(out.find('related API is not supported.') != -1)

  def testPackage(self):
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--app-url=http://www.intel.com', self._mode]
    out = RunCommand(cmd)
    self.assertIn('A package name is required', out)

    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example',
           '--app-url=http://www.intel.com', self._mode]
    out = RunCommand(cmd)
    self.assertNotIn('A package name is required', out)
    Clean('Example', '1.0.0')

  def testPackageWithInvalidCharacter(self):
    package_name_error = 'package name should be started with letters'

    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk._example',
           '--app-url=http://www.intel.com', self._mode]
    out = RunCommand(cmd)
    self.assertIn(package_name_error, out)

    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.123example',
           '--app-url=http://www.intel.com', self._mode]
    out = RunCommand(cmd)
    self.assertIn(package_name_error, out)

    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example_',
           '--app-url=http://www.intel.com', self._mode]
    out = RunCommand(cmd)
    self.assertNotIn(package_name_error, out)
    Clean('Example', '1.0.0')

  def testEntry(self):
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           self._mode]
    out = RunCommand(cmd)
    self.assertNotIn('You must pass either "--app-url" or',
                     out)
    self.assertNotIn('You must specify both "--app-local-path" and',
                     out)
    self.addCleanup(Clean, 'Example', '1.0.0')
    self.checkApks('Example', '1.0.0')
    Clean('Example', '1.0.0')

    test_entry_root = 'test_data/entry'
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-root=%s' % test_entry_root,
           '--app-local-path=index.html', self._mode]
    out = RunCommand(cmd)
    self.assertNotIn('You must pass either "--app-url" or',
                     out)
    self.assertNotIn('You must specify both "--app-local-path" and',
                     out)
    self.checkApks('Example', '1.0.0')

  def testEntryWithErrors(self):
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', self._mode]
    out = RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    self.assertIn('You must pass either "--app-url" or',
                  out)

    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example', '1.0.0')

    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--app-root=.', self._mode]
    out = RunCommand(cmd)
    self.assertIn('You must pass either "--app-url" or',
                  out)
    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example', '1.0.0')

    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-root=./', self._mode]
    out = RunCommand(cmd)
    self.assertIn('You must specify both "--app-local-path" and',
                  out)
    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example', '1.0.0')

    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-local-path=index.html',
           self._mode]
    out = RunCommand(cmd)
    self.assertIn('You must specify both "--app-local-path" and',
                  out)
    self.assertFalse(os.path.exists('Example.apk'))
    Clean('Example', '1.0.0')

    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_app_launch_local_path.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, self._mode]
    out = RunCommand(cmd)
    self.assertTrue(
        out.find('Please make sure that the local path file') != -1)
    self.assertFalse(os.path.exists('Example.apk'))

  def testIconByOption(self):
    icon = os.path.join('test_data', 'manifest', 'icons', 'icon_96.png')
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--icon=%s' % icon, '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(content.find('drawable/icon_96') != -1)
    self.checkApks('Example', '1.0.0')

  def testIconByManifest(self):
    manifest_path = os.path.join('test_data', 'manifest', 'manifest_icon.json')
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--manifest=%s' % manifest_path, '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(content.find('drawable/icon') != -1)
    self.checkApks('Example', '1.0.0')

  def testFullscreen(self):
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '-f', '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    theme = 'Example/res/values-v14/theme.xml'
    with open(theme, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(theme))
    self.assertTrue(
        content.find(
            '<item name="android:windowFullscreen">true</item>') != -1)
    self.checkApks('Example', '1.0.0')

  def testEnableRemoteDebugging(self):
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--enable-remote-debugging', '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    activity = 'Example/src/org/xwalk/example/ExampleActivity.java'
    with open(activity, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(activity))
    self.assertTrue(content.find('setRemoteDebugging') != -1)
    self.checkApks('Example', '1.0.0')
    Clean('Example', '1.0.0')
    manifest_path = os.path.join('test_data', 'manifest', 'manifest.json')
    cmd = ['python', 'make_apk.py', '--enable-remote-debugging',
           '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path,
           '--project-dir=.',
           self._mode]
    RunCommand(cmd)
    activity = 'Example/src/org/xwalk/example/ExampleActivity.java'
    with open(activity, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(activity))
    self.assertTrue(content.find('setRemoteDebugging') != -1)
    self.checkApks('Example', '1.0.0')

  def testUseAnimatableView(self):
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--use-animatable-view', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    activity = 'Example/src/org/xwalk/example/ExampleActivity.java'
    with open(activity, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(activity))
    self.assertTrue(content.find('setUseAnimatableView') != -1)
    self.checkApks('Example', '1.0.0')
    Clean('Example', '1.0.0')
    manifest_path = os.path.join('test_data', 'manifest', 'manifest.json')
    cmd = ['python', 'make_apk.py', '--use-animatable-view',
           '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, self._mode]
    RunCommand(cmd)
    activity = 'Example/src/org/xwalk/example/ExampleActivity.java'
    with open(activity, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(activity))
    self.assertTrue(content.find('setUseAnimatableView') != -1)
    self.checkApks('Example', '1.0.0')

  def testKeystore(self):
    keystore_path = os.path.join('test_data', 'keystore',
                                 'xwalk-test.keystore')
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--keystore-path=%s' % keystore_path, '--keystore-alias=xwalk-test',
           '--keystore-passcode=xwalk-test',
           '--keystore-alias-passcode=xwalk-test',
           '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.assertTrue(os.path.isdir('Example'))
    self.checkApks('Example', '1.0.0', keystore_path)
    Clean('Example', '1.0.0')

    keystore_path_with_space = os.path.join('test_data', 'keystore',
                                            'test keystore')
    shutil.copy2(keystore_path, keystore_path_with_space)
    keystore_path = os.path.join('test_data', 'keystore',
                                 'xwalk-test.keystore')
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--keystore-path=%s' % keystore_path_with_space,
           '--keystore-alias=xwalk test',
           '--keystore-passcode=xwalk-test',
           '--keystore-alias-passcode=xwalk test',
           '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.assertTrue(os.path.isdir('Example'))
    self.checkApks('Example', '1.0.0', keystore_path_with_space)
    Clean('Example', '1.0.0')

  def testManifest(self):
    manifest_path = os.path.join('test_data', 'manifest', 'manifest.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('android.permission.READ_CONTACTS') != -1)
    self.assertTrue(content.find('android.permission.WRITE_CONTACTS') != -1)
    self.assertTrue(
        content.find('android.permission.ACCESS_FINE_LOCATION') != -1)
    self.assertTrue(content.find('android.permission.READ_SMS') != -1)
    self.assertTrue(content.find('android.permission.RECEIVE_SMS') != -1)
    self.assertTrue(content.find('android.permission.SEND_SMS') != -1)
    self.assertTrue(content.find('android.permission.WRITE_SMS') != -1)
    self.assertTrue(content.find('landscape') != -1)
    theme = 'Example/res/values-v14/theme.xml'
    with open(theme, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(theme))
    self.assertTrue(
        content.find(
            '<item name="android:windowFullscreen">true</item>') != -1)
    self.assertTrue(os.path.exists('Example'))
    self.checkApks('Example', '1.0.0')

  def testManifestWithSpecificValue(self):
    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_app_launch_local_path.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--project-dir=.', self._mode]
    out = RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    self.assertTrue(out.find('no app launch path') == -1)
    self.checkApks('Example', '1.0.0')

  def testManifestWithDeprecatedField(self):
    manifest_path = os.path.join('test_data', 'manifest', 'deprecated',
                                 'manifest_app_local_path.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, self._mode]
    out = RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    self.assertIn('Warning: The following fields have been deprecated', out)
    self.assertIn('app.launch.local_path', out)
    Clean('Example', '1.0.0')

    manifest_path = os.path.join('test_data', 'manifest', 'deprecated',
                                 'manifest_launch_path.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, self._mode]
    out = RunCommand(cmd)
    self.assertIn('Warning: The following fields have been deprecated', out)
    self.assertIn('launch_path', out)
    Clean('Example', '1.0.0')

    manifest_path = os.path.join('test_data', 'manifest', 'deprecated',
                                 'manifest_permissions.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, self._mode]
    out = RunCommand(cmd)
    self.assertIn('Warning: The following fields have been deprecated', out)
    self.assertIn('permissions', out)
    Clean('Example', '1.0.0')

    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_deprecated_icon.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, self._mode]
    out = RunCommand(cmd)
    self.assertIn('Warning: The following fields have been deprecated', out)
    self.assertIn('icons', out)
    Clean('Example', '1.0.0')

    manifest_path = os.path.join('test_data', 'manifest', 'deprecated',
                                 'manifest_description.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, self._mode]
    out = RunCommand(cmd)
    self.assertIn('Warning: The following fields have been deprecated', out)
    self.assertIn('description', out)
    Clean('Example', '1.0.0')

    manifest_path = os.path.join('test_data', 'manifest', 'deprecated',
                                 'manifest_deprecated_version.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, self._mode]
    out = RunCommand(cmd)
    self.assertIn('Warning: The following fields have been deprecated', out)
    self.assertIn('version', out)

  def testManifestWithError(self):
    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_no_app_launch_path.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--verbose', self._mode]
    out = RunCommand(cmd)
    self.assertTrue(out.find('no app launch path') != -1)
    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_no_name.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--verbose', self._mode]
    out = RunCommand(cmd)
    self.assertTrue(out.find('no \'name\' field') != -1)
    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_permissions_format_error.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--verbose', self._mode]
    out = RunCommand(cmd)
    self.assertTrue(out.find('\'Permissions\' field error') != -1)
    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_permissions_field_error.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--verbose', self._mode]
    out = RunCommand(cmd)
    self.assertTrue(out.find('\'Permissions\' field error') != -1)
    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_not_supported_permission.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--verbose', self._mode]
    out = RunCommand(cmd)
    self.assertTrue(
        out.find('\'Telephony\' related API is not supported') != -1)

  def testManifestWithAppRoot(self):
    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_and_app_root', 'manifest.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.1')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('landscape') != -1)
    theme = 'Example/res/values-v14/theme.xml'
    with open(theme, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(theme))
    self.assertTrue(
        content.find(
            '<item name="android:windowFullscreen">true</item>') != -1)
    self.assertTrue(os.path.exists('Example'))
    self.checkApks('Example', '1.0.1')

  def testAppRootWithManifest(self):
    html_path = os.path.join('test_data', 'manifest', 'manifest_and_app_root')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--name=example', '--app-version=1.0.0', '--app-root=%s' % html_path,
           '--app-local-path=%s' % 'index.html', '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('landscape') == -1)
    theme = 'Example/res/values-v14/theme.xml'
    with open(theme, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(theme))
    self.assertTrue(
        content.find(
            '<item name="android:windowFullscreen">true</item>') == -1)
    self.assertTrue(os.path.exists('Example'))
    self.checkApks('Example', '1.0.0')

  def testExtensionsWithOneExtension(self):
    # Test with an existed extension.
    extension_path = 'test_data/extensions/myextension'
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--extensions=%s' % extension_path, '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    self.assertTrue(os.path.exists('Example'))
    extensions_config_json = 'Example/assets/extensions-config.json'
    self.assertTrue(os.path.exists(extensions_config_json))
    with open(extensions_config_json, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(
        content.find('xwalk-extensions/myextension/myextension.js'))
    self.assertTrue(content.find('com.example.extension.MyExtension'))
    extension_js = 'Example/assets/xwalk-extensions/myextension/myextension.js'
    self.assertTrue(os.path.exists(extension_js))
    extension_jar = 'Example/xwalk-extensions/myextension/myextension.jar'
    self.assertTrue(os.path.exists(extension_jar))
    self.checkApks('Example', '1.0.0')

  def testExtensionsWithNonExtension(self):
    # Test with a non-existing extension.
    extension_path = 'test_data/extensions/myextension'
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--extensions=%s1' % extension_path, self._mode, '--verbose']
    out = RunCommand(cmd)
    self.assertTrue(out.find('Exiting with error code: 9') != -1)

  def testExtensionWithPermissions(self):
    test_entry_root = 'test_data/entry'
    # Add redundant separators for test.
    extension_path = 'test_data//extensions/contactextension/'
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-root=%s' % test_entry_root,
           '--app-local-path=contactextension.html',
           '--extensions=%s' % extension_path,
           '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    self.assertTrue(os.path.exists('Example'))
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('android.permission.WRITE_CONTACTS') != -1)
    self.assertTrue(content.find('android.permission.READ_CONTACTS') != -1)
    self.checkApks('Example', '1.0.0')

  def testExtensionWithAndroidManifest(self):
    test_entry_root = 'test_data/entry'
    # Add redundant separators for test.
    extension_path = 'test_data//extensions/adextension/'
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-root=%s' % test_entry_root,
           '--app-local-path=contactextension.html',
           '--extensions=%s' % extension_path,
           '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    self.assertTrue(os.path.exists('Example'))
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('_GOOGLE_PLAY_SERVICES_LIB_VERSION_') != -1)
    self.checkApks('Example', '1.0.0')


  def testXPK(self):
    xpk_file = os.path.join('test_data', 'xpk', 'example.xpk')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--xpk=%s' % xpk_file, '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    self.assertTrue(os.path.exists('Example'))
    self.checkApks('Example', '1.0.0')

  def testXPKWithError(self):
    xpk_file = os.path.join('test_data', 'xpk', 'error.xpk')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--xpk=%s' % xpk_file, '--project-dir=.', self._mode]
    out = RunCommand(cmd)
    error_msg = 'XPK doesn\'t contain manifest file'
    self.assertTrue(out.find(error_msg) != -1)
    self.assertFalse(os.path.exists('Example'))

  def testOrientation(self):
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--orientation=landscape', '--project-dir=.', self._mode]
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    self.assertTrue(content.find('landscape') != -1)
    self.assertTrue(os.path.exists('Example'))
    self.checkApks('Example', '1.0.0')

  def testArch(self):
    # Arch option only works for embedded mode,
    # so only test it for embedded mode.
    if self._mode.find('embedded') != -1:
      cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
             '--package=org.xwalk.example', '--app-url=http://www.intel.com',
             '--arch=x86', self._mode]
      RunCommand(cmd)
      self.addCleanup(Clean, 'Example', '1.0.0')
      if 'x86' in self.archs():
        self.assertTrue(os.path.isfile('Example_1.0.0_x86.apk'))
        self.checkApk('Example_1.0.0_x86.apk', 'x86')
      else:
        self.assertFalse(os.path.isfile('Example_1.0.0_x86.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_x86_64.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_arm.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_arm64.apk'))
      Clean('Example', '1.0.0')
      cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
             '--package=org.xwalk.example', '--app-url=http://www.intel.com',
             '--arch=x86_64', self._mode]
      RunCommand(cmd)
      self.addCleanup(Clean, 'Example', '1.0.0')
      if 'x86_64' in self.archs():
        self.assertTrue(os.path.isfile('Example_1.0.0_x86_64.apk'))
        self.checkApk('Example_1.0.0_x86_64.apk', 'x86_64')
      else:
        self.assertFalse(os.path.isfile('Example_1.0.0_x86_64.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_x86.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_arm.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_arm64.apk'))
      Clean('Example', '1.0.0')
      cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
             '--package=org.xwalk.example', '--app-url=http://www.intel.com',
             '--arch=arm', self._mode]
      RunCommand(cmd)
      self.addCleanup(Clean, 'Example', '1.0.0')
      if 'arm' in self.archs():
        self.assertTrue(os.path.isfile('Example_1.0.0_arm.apk'))
        self.checkApk('Example_1.0.0_arm.apk', 'arm')
      else:
        self.assertFalse(os.path.isfile('Example_1.0.0._arm.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_x86.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_x86_64.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_arm64.apk'))
      Clean('Example', '1.0.0')

      cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
             '--package=org.xwalk.example', '--app-url=http://www.intel.com',
             '--arch=arm64', self._mode]
      RunCommand(cmd)
      self.addCleanup(Clean, 'Example', '1.0.0')
      if 'arm64' in self.archs():
        self.assertTrue(os.path.isfile('Example_1.0.0_arm64.apk'))
        self.checkApk('Example_1.0.0_arm64.apk', 'arm64')
      else:
        self.assertFalse(os.path.isfile('Example_1.0.0._arm64.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_x86.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_x86_64.apk'))
      self.assertFalse(os.path.isfile('Example_1.0.0_arm.apk'))
      Clean('Example', '1.0.0')

      cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
             '--package=org.xwalk.example', '--app-url=http://www.crosswalk-project.org',
             self._mode]
      RunCommand(cmd)
      if 'arm' in self.archs():
        self.assertTrue(os.path.isfile('Example_1.0.0_arm.apk'))
        self.checkApk('Example_1.0.0_arm.apk', 'arm')
      else:
        self.assertFalse(os.path.isfile('Example_1.0.0._arm.apk'))
      if 'arm64' in self.archs():
        self.assertTrue(os.path.isfile('Example_1.0.0_arm64.apk'))
        self.checkApk('Example_1.0.0_arm64.apk', 'arm64')
      else:
        self.assertFalse(os.path.isfile('Example_1.0.0._arm64.apk'))
      if 'x86' in self.archs():
        self.assertTrue(os.path.isfile('Example_1.0.0_x86.apk'))
        self.checkApk('Example_1.0.0_x86.apk', 'x86')
      else:
        self.assertFalse(os.path.isfile('Example_1.0.0._x86.apk'))
      if 'x86_64' in self.archs():
        self.assertTrue(os.path.isfile('Example_1.0.0_x86_64.apk'))
        self.checkApk('Example_1.0.0_x86_64.apk', 'x86_64')
      else:
        self.assertFalse(os.path.isfile('Example_1.0.0._x86_64.apk'))
      Clean('Example', '1.0.0')
      cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
             '--package=org.xwalk.example', '--app-url=http://www.intel.com',
             '--arch=undefined', self._mode]
      out = RunCommand(cmd)
      error_msg = 'invalid choice: \'undefined\''
      self.assertTrue(out.find(error_msg) != -1)

  def testVerbose(self):
    cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--verbose', '--project-dir=.', self._mode]
    result = RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    self.assertTrue(result.find('aapt') != -1)
    self.assertTrue(result.find('crunch') != -1)
    self.assertTrue(result.find('apkbuilder') != -1)
    self.assertTrue(os.path.exists('Example'))
    self.checkApks('Example', '1.0.0')

  def executeCommandAndVerifyResult(self, exec_file):
    # Test all of supported options with empty 'mode' option.
    icon_path = './template/res/drawable-xhdpi/crosswalk.png'
    extension_path = 'test_data/extensions/myextension'
    arch = ''
    icon = ''
    if exec_file.find("make_apk.py") != -1:
      arch_list = self.archs()
      if 'x86' in arch_list:
        arch = '--arch=x86'
      elif 'x86_64' in arch_list:
        arch = '--arch=x86_64'
      elif 'arm64' in arch_list:
        arch = '--arch=arm64'
      else:
        arch = '--arch=arm'
      icon = '--icon=%s' % icon_path
    cmd = ['python', '%s' % exec_file,
           '--app-version=1.0.0',
           '--app-url=http://www.intel.com',
           '%s' % arch,
           '--description=a sample application',
           '--enable-remote-debugging',
           '--extensions=%s' % extension_path,
           '--fullscreen',
           '--keep-screen-on',
           '%s' % icon,
           '--name=Example',
           '--orientation=landscape',
           '--package=org.xwalk.example',
           '--permissions=geolocation',
           '--project-dir=.']
    RunCommand(cmd)
    self.addCleanup(Clean, 'Example', '1.0.0')
    activity = 'Example/src/org/xwalk/example/ExampleActivity.java'
    with open(activity, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(activity))
    # Test remote debugging option.
    self.assertTrue(content.find('setRemoteDebugging') != -1)
    # Test keep screen on option
    self.assertTrue(content.find('FLAG_KEEP_SCREEN_ON') != -1)

    manifest = 'Example/AndroidManifest.xml'
    with open(manifest, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(manifest))
    # Test permission option.
    self.assertTrue(content.find('ACCESS_FINE_LOCATION') != -1)
    # Test description option.
    self.assertTrue(content.find('description') != -1)
    # Test app version option.
    self.assertTrue(content.find('versionName') != -1)
    # Test orientation option.
    self.assertTrue(content.find('landscape') != -1)
    # Test fullscreen option
    theme = 'Example/res/values-v14/theme.xml'
    with open(theme, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(os.path.exists(theme))
    self.assertTrue(
        content.find(
            '<item name="android:windowFullscreen">true</item>') != -1)
    # Test extensions option.
    extensions_config_json = 'Example/assets/extensions-config.json'
    self.assertTrue(os.path.exists(extensions_config_json))
    with open(extensions_config_json, 'r') as content_file:
      content = content_file.read()
      js_file_name = 'xwalk-extensions/myextension/myextension.js'
      self.assertTrue(content.find(js_file_name))
      self.assertTrue(content.find('com.example.extension.MyExtension'))
    extension_js = 'Example/assets/xwalk-extensions/myextension/myextension.js'
    self.assertTrue(os.path.exists(extension_js))
    extension_jar = 'Example/xwalk-extensions/myextension/myextension.jar'
    self.assertTrue(os.path.exists(extension_jar))

  def testEmptyMode(self):
    self.executeCommandAndVerifyResult('make_apk.py')

  def testCustomizeFile(self):
    cmd = ['python', 'make_apk.py',
           '--app-url=http://www.intel.com',
           '--app-version=1.0.0',
           '--name=Example',
           '--package=org.xwalk.example',
           '--verbose',
           '--project-dir=.']
    RunCommand(cmd)
    manifest = 'Example/AndroidManifest.xml'
    if not os.path.exists(manifest):
      print 'The \'%s\' was not generated, please check it.' % manifest
      sys.exit(1)

    self.executeCommandAndVerifyResult('customize.py')

  def testLaunchScreen(self):
    # Prepare launch screen resources.
    launch_screen_path = os.path.join('test_data', 'launchScreen')
    orientations = ['default', 'portrait', 'landscape']
    dimensions = ['0_75', '1', '1_5', '2']
    img_types = ['img', 'bg']
    for orientation in orientations:
      for dimension in dimensions:
        for img_type in img_types:
          name = orientation + '_' + img_type + '_' + dimension
          path_tmp = os.path.join(launch_screen_path, name)
          _file = open(path_tmp, 'w+')
          _file.write(name)
          _file.close()
    # Run Test.
    manifest_path = os.path.join('test_data', 'launchScreen',
                                 'manifest_deprecated_launch_screen.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, self._mode]
    out = RunCommand(cmd)
    self.assertIn('Warning: The following fields have been deprecated', out)
    self.assertTrue(out.find('launch_screen') != -1)
    Clean('Example', '1.0.0')
    manifest_path = os.path.join('test_data', 'launchScreen', 'manifest.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--project-dir=.', self._mode]
    RunCommand(cmd)
    # Check theme.xml.
    theme_path = os.path.join('Example', 'res', 'values-v14', 'theme.xml')
    self.assertTrue(os.path.exists(theme_path))
    with open(theme_path, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(content.find('@drawable/launchscreen_bg') != -1)
    # Check launchscreen_bg.xml
    launch_screen_bg_path = os.path.join(
        "Example", 'res', 'drawable', 'launchscreen_bg.xml')
    self.assertTrue(os.path.exists(launch_screen_bg_path))
    with open(launch_screen_bg_path, 'r') as content_file:
      content = content_file.read()
    self.assertTrue(content.find('@drawable/launchscreen_bg_img') != -1)
    # Check resource images
    for orientation in orientations:
      for dimension in dimensions:
        drawable = 'drawable'
        if orientation == 'portrait':
          drawable = drawable + '-port'
        elif orientation == 'landscape':
          drawable = drawable + '-land'
        if dimension == '0_75':
          drawable = drawable + '-ldpi'
        elif dimension == '1':
          drawable = drawable + '-mdpi'
        elif dimension == '1_5':
          drawable = drawable + '-hdpi'
        elif dimension == '2':
          drawable = drawable + '-xhdpi'
        # Check background image
        bg_drawable = os.path.join(
            "Example", 'res', drawable, 'launchscreen_bg_img')
        self.assertTrue(os.path.exists(bg_drawable))
        with open(bg_drawable, 'r') as content_file:
          content = content_file.read()
        name = orientation + '_' + 'bg' + '_' + dimension
        self.assertTrue(content == name)
        # Check foreground image
        fg_drawable = os.path.join(
            "Example", 'res', drawable, 'launchscreen_img')
        self.assertTrue(os.path.exists(fg_drawable))
        with open(fg_drawable, 'r') as content_file:
          content = content_file.read()
        name = orientation + '_' + 'img' + '_' + dimension
        self.assertTrue(content == name)
    self.checkApks('Example', '1.0.0')
    Clean('Example', '1.0.0')

  def testTargetDir(self):
    test_option = ['./', '../', '~/']
    for option in test_option:
      cmd = ['python', 'make_apk.py', '--name=Example', '--app-version=1.0.0',
             '--package=org.xwalk.example', '--app-url=http://www.intel.com',
             '--target-dir=%s' % option, self._mode]
      RunCommand(cmd)
      self.addCleanup(Clean, os.path.expanduser('%sExample' % option), '1.0.0')
      if self._mode.find('shared') != -1:
        apk_path = os.path.expanduser('%sExample_1.0.0.apk' % option)
        self.assertTrue(os.path.exists(apk_path))
        self.checkApk(apk_path, '')
      elif self._mode.find('embedded') != -1:
        for arch in self.archs():
          apk_path = os.path.expanduser('%sExample_1.0.0_%s.apk'
                                        % (option, arch))
          self.assertTrue(os.path.exists(apk_path))
          self.checkApk(apk_path, arch)

  def testCompressor(self):
    app_root = os.path.join('test_data', 'compressor')
    css_folder = os.path.join('test_data', 'compressor', 'css')
    css_file = os.path.join(css_folder, 'test.css')
    js_folder = os.path.join('test_data', 'compressor', 'js')
    js_file = os.path.join(js_folder, 'test.js')
    fun = self.assertTrue
    name = 'Example'

    cmd = ['python', 'customize.py',
           '--name=%s' % name,
           '--package=org.xwalk.example',
           '--compressor',
           '--app-root=%s' % app_root,
           '--project-dir=.']
    RunCommand(cmd)
    CompareSizeForCompressor('all', css_file, 'css', name, fun)
    CompareSizeForCompressor('all', js_file, 'js', name, fun)

    cmd = ['python', 'customize.py',
           '--name=%s' % name,
           '--package=org.xwalk.example',
           '--app-root=%s' % app_root,
           '--compressor',
           '--project-dir=.']
    RunCommand(cmd)
    CompareSizeForCompressor('all', css_file, 'css', name, fun)
    CompareSizeForCompressor('all', js_file, 'js', name, fun)

    cmd = ['python', 'customize.py',
           '--name=%s' % name,
           '--package=org.xwalk.example',
           '--compressor=js',
           '--app-root=%s' % app_root,
           '--project-dir=.']
    RunCommand(cmd)
    CompareSizeForCompressor('js', js_file, 'js', name, fun)

    cmd = ['python', 'customize.py',
           '--name=%s' % name,
           '--package=org.xwalk.example',
           '--compressor=css',
           '--app-root=%s' % app_root,
           '--project-dir=.']
    RunCommand(cmd)
    CompareSizeForCompressor('css', css_file, 'css', name, fun)

    cmd = ['python', 'customize.py',
           '--name=%s' % name,
           '--package=org.xwalk.example',
           '--app-root=%s' % app_root,
           '--project-dir=.']
    RunCommand(cmd)
    CompareSizeForCompressor(None, css_file, 'css', name, fun)
    CompareSizeForCompressor(None, js_file, 'js', name, fun)

    cmd = ['python', 'customize.py',
           '--name=%s' % name,
           '--package=org.xwalk.example',
           '--app-root=%s' % app_root,
           '--compressor=other',
           '--project-dir=.']
    RunCommand(cmd)
    CompareSizeForCompressor(None, css_file, 'css', name, fun)
    CompareSizeForCompressor(None, js_file, 'js', name, fun)

    Clean(name, '1.0.0')


  def VerifyResultInXMLFile(self, xml_path, piece_content):
    self.assertTrue(os.path.exists(xml_path))
    with open(xml_path, 'r') as content_file:
      content = content_file.read()

    self.assertIn(piece_content, content)
    Clean('Example', '1.0.0')


  def testAppNameWithNonASCII(self):
    xml_path = 'Example/AndroidManifest.xml'
    piece_content = 'android:label="%s"' % '你好'
    cmd = ['python', 'make_apk.py', '--name=你好', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--project-dir=.']
    RunCommand(cmd)
    self.VerifyResultInXMLFile(xml_path, piece_content)

    manifest_path = os.path.join('test_data', 'manifest', 'invalidchars',
                                 'manifest_with_chinese_name.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--project-dir=.']
    RunCommand(cmd)
    self.VerifyResultInXMLFile(xml_path, piece_content)


  def testDescriptionWithDBCS(self):
    xml_path = 'Example/res/values/strings.xml'
    piece_content = '<string name="description">%s</string>' % '你好'
    cmd = ['python', 'make_apk.py', '--name=hello', '--app-version=1.0.0',
           '--package=org.xwalk.example', '--app-url=http://www.intel.com',
           '--description=你好', '--project-dir=.']
    RunCommand(cmd)
    self.VerifyResultInXMLFile(xml_path, piece_content)

    manifest_path = os.path.join('test_data', 'manifest',
                                 'manifest_description_dbcs.json')
    cmd = ['python', 'make_apk.py', '--package=org.xwalk.example',
           '--manifest=%s' % manifest_path, '--project-dir=.']
    RunCommand(cmd)
    piece_content = '"description">%s</string>' % '你好 a sample description'
    self.VerifyResultInXMLFile(xml_path, piece_content)


def SuiteWithModeOption():
  # Gather all the tests for the specified mode option.
  test_suite = unittest.TestSuite()
  test_suite.addTest(TestMakeApk('testAppBigVersionCodeBase'))
  test_suite.addTest(TestMakeApk('testAppVersionCode'))
  test_suite.addTest(TestMakeApk('testAppVersionCodeBase'))
  test_suite.addTest(TestMakeApk('testAppVersionCodeFromVersionName'))
  test_suite.addTest(TestMakeApk('testAppDescriptionAndVersion'))
  test_suite.addTest(TestMakeApk('testAppRootWithManifest'))
  test_suite.addTest(TestMakeApk('testArch'))
  test_suite.addTest(TestMakeApk('testEnableRemoteDebugging'))
  test_suite.addTest(TestMakeApk('testEntry'))
  test_suite.addTest(TestMakeApk('testEntryWithErrors'))
  test_suite.addTest(TestMakeApk('testExtensionsWithOneExtension'))
  test_suite.addTest(TestMakeApk('testExtensionsWithNonExtension'))
  test_suite.addTest(TestMakeApk('testExtensionWithAndroidManifest'))
  test_suite.addTest(TestMakeApk('testExtensionWithPermissions'))
  test_suite.addTest(TestMakeApk('testFullscreen'))
  test_suite.addTest(TestMakeApk('testIconByOption'))
  test_suite.addTest(TestMakeApk('testIconByManifest'))
  test_suite.addTest(TestMakeApk('testKeystore'))
  test_suite.addTest(TestMakeApk('testManifest'))
  test_suite.addTest(TestMakeApk('testManifestWithDeprecatedField'))
  test_suite.addTest(TestMakeApk('testManifestWithError'))
  test_suite.addTest(TestMakeApk('testManifestWithAppRoot'))
  test_suite.addTest(TestMakeApk('testName'))
  test_suite.addTest(TestMakeApk('testOrientation'))
  test_suite.addTest(TestMakeApk('testPackage'))
  test_suite.addTest(TestMakeApk('testPackageWithInvalidCharacter'))
  test_suite.addTest(TestMakeApk('testPermissions'))
  test_suite.addTest(TestMakeApk('testPermissionsWithError'))
  test_suite.addTest(TestMakeApk('testXPK'))
  test_suite.addTest(TestMakeApk('testXPKWithError'))
  test_suite.addTest(TestMakeApk('testTargetDir'))
  test_suite.addTest(TestMakeApk('testLaunchScreen'))
  return test_suite


def SuiteWithEmptyModeOption():
  # Gather all the tests for empty mode option.
  test_suite = unittest.TestSuite()
  test_suite.addTest(TestMakeApk('testAppNameWithNonASCII'))
  test_suite.addTest(TestMakeApk('testCompressor'))
  test_suite.addTest(TestMakeApk('testCustomizeFile'))
  test_suite.addTest(TestMakeApk('testDescriptionWithDBCS'))
  test_suite.addTest(TestMakeApk('testEmptyMode'))
  test_suite.addTest(TestMakeApk('testToolVersion'))
  test_suite.addTest(TestMakeApk('testVerbose'))
  return test_suite


def TestSuiteRun(test_runner, suite):
  results = test_runner.run(suite)
  return results.wasSuccessful()


if __name__ == '__main__':
  parser = optparse.OptionParser()
  info = ('The build directory for xwalk.'
          'Such as: --build-dir=src/out')
  parser.add_option('--build-dir', help=info)
  info = ('The build target for xwalk.'
          'Such as: --target=Release')
  parser.add_option('--target', help=info)
  info = ('The path of package tool.')
  parser.add_option('--tool-path', help=info)
  info = ('The packaging mode for xwalk. Such as: --mode=embedded.'
          'Please refer the detail to the option of make_apk.py.')
  parser.add_option('--mode', help=info)
  info = ('The directory of jar files of external extensions.'
          'Such as: --external-extensions=src/out/Release/lib.java')
  parser.add_option('--external-extensions', help=info)

  options, dummy = parser.parse_args()
  if len(sys.argv) == 1:
    parser.print_help()
    sys.exit(1)

  del sys.argv[1:]
  mode_suite = SuiteWithModeOption()
  empty_mode_suite = SuiteWithEmptyModeOption()
  runner = unittest.TextTestRunner(verbosity=2)
  if options.build_dir or options.target:
    warnings.warn(('"--build-dir" and "--target" will be deprecated soon, '
                   'please leverage "--tool-path" instead.'),
                  Warning)
  test_result = True
  if options.mode:
    test_result = TestSuiteRun(runner, mode_suite)
  else:
    # Run tests in both embedded and shared mode
    # when the mode option isn't specified.
    options.mode = 'embedded'
    print 'Run tests in embedded mode.'
    test_result = TestSuiteRun(runner, mode_suite)
    options.mode = 'shared'
    print 'Run tests in shared mode.'
    test_result = TestSuiteRun(runner, mode_suite) and test_result
    options.mode = ''
    print 'Run test without \'--mode\' option.'
    test_result = TestSuiteRun(runner, empty_mode_suite) and test_result
  if not test_result:
    sys.exit(1)
