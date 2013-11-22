#!/usr/bin/env python
#
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import distutils.dir_util
import optparse
import os
import shutil
import sys

LIBRARY_PROJECT_NAME = 'xwalk_core_library'
XWALK_CORE_SHELL_APK = 'xwalk_core_shell_apk'

def AddGeneratorOptions(option_parser):
  option_parser.add_option('-s', dest='source',
                           help='Source directory of project root.',
                           type='string')
  option_parser.add_option('-t', dest='target',
                           help='Product out target directory.',
                           type='string')


def CleanLibraryProject(out_directory):
  out_project_path = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src')
  if os.path.exists(out_project_path):
    shutil.rmtree(out_project_path)


def CopyProjectFiles(project_source, out_directory):
  """cp xwalk/build/android/xwalkcore_library_template/<file>
        out/Release/xwalk_core_library/<file>
  """

  print 'Copying library project files...'
  template_folder = os.path.join(project_source, 'xwalk', 'build', 'android',
                                 'xwalkcore_library_template')
  files_to_copy = [
      # AndroidManifest.xml from template.
      'AndroidManifest.xml',
      # Eclipse project properties from template.
      'project.properties',
      # Ant build file.
      'build.xml',
      # Customized Ant build file.
      'precompile.xml',
      # Python script to copy R.java.
      'prepare_r_java.py',
      # Ant properties file.
      'ant.properties',
      # Eclipse project file and builders.
      '.project',
      '.classpath',
      '.externalToolBuilders/prepare_r_java.launch'
  ]
  for f in files_to_copy:
    source_file = os.path.join(template_folder, f)
    target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, f)

    target_dir = os.path.dirname(target_file)

    if not os.path.isdir(target_dir):
      os.makedirs(target_dir)

    shutil.copy2(source_file, target_file)


def CopyJavaSources(project_source, out_directory):
  """cp <path>/java/src/<package>
        out/Release/xwalk_core_library/src/<package>
  """

  print 'Copying Java sources...'
  target_source_directory = os.path.join(
      out_directory, LIBRARY_PROJECT_NAME, 'src')
  if not os.path.exists(target_source_directory):
    os.makedirs(target_source_directory)

  # FIXME(wang16): There is an assumption here the package names listed
  # here are all beginned with "org". If the assumption is broken in
  # future, the logic needs to be adjusted accordingly.
  java_srcs_to_copy = [
      # Chromium java sources.
      'base/android/java/src/org/chromium/base',
      'content/public/android/java/src/org/chromium/content',
      'media/base/android/java/src/org/chromium/media',
      'net/android/java/src/org/chromium/net',
      'ui/android/java/src/org/chromium/ui',
      'components/navigation_interception/android/java/'
          'src/org/chromium/components/navigation_interception',
      'components/web_contents_delegate_android/android/java/'
          'src/org/chromium/components/web_contents_delegate_android',

      # XWalk java sources.
      'xwalk/runtime/android/java/src/org/xwalk/core',
      'xwalk/extensions/android/java/src/org/xwalk/core/extensions',
      'xwalk/runtime/android/java/src/org/xwalk/runtime/extension',
  ]

  for source in java_srcs_to_copy:
    # find the src/org in the path
    src_slash_org_pos = source.find(r'src/org')
    if src_slash_org_pos < 0:
      raise Exception('Invalid java source path: %s' % source)
    source_path = os.path.join(project_source, source)
    package_path = source[src_slash_org_pos+4:]
    target_path = os.path.join(target_source_directory, package_path)
    if os.path.isfile(source_path):
      if not os.path.isdir(os.path.dirname(target_path)):
        os.makedirs(os.path.dirname(target_path))
      shutil.copyfile(source_path, target_path)
    else:
      shutil.copytree(source_path, target_path)


def CopyGeneratedSources(out_directory):
  """cp out/Release/gen/templates/<path>
        out/Release/xwalk_core_library/src/<path>
     cp out/Release/xwalk_core_shell_apk/
            native_libraries_java/NativeLibraries.java
        out/Release/xwalk_core_library/src/org/
            chromium/content/app/NativeLibraries.java
  """

  print 'Copying generated source files...'
  generated_srcs_to_copy = [
      'org/chromium/content/common/ResultCodes.java',
      'org/chromium/net/NetError.java',
      'org/chromium/content/browser/PageTransitionTypes.java',
      'org/chromium/content/browser/SpeechRecognitionError.java',
      'org/chromium/net/PrivateKeyType.java',
      'org/chromium/net/CertVerifyResultAndroid.java',
      'org/chromium/net/CertificateMimeType.java',
      'org/chromium/base/ActivityState.java',
      'org/chromium/base/MemoryPressureLevelList.java',
      'org/chromium/media/ImageFormat.java',
  ]

  for source in generated_srcs_to_copy:
    source_file = os.path.join(out_directory, 'gen', 'templates', source)
    target_file = os.path.join(
        out_directory, LIBRARY_PROJECT_NAME, 'src', source)
    shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, XWALK_CORE_SHELL_APK,
                             'native_libraries_java',
                             'NativeLibraries.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'content',
                             'app', 'NativeLibraries.java')
  shutil.copyfile(source_file, target_file)

def CopyJSBindingFiles(project_source, out_directory):
  print 'Copying js binding files...'
  jsapi_directory = os.path.join(out_directory,
                                 LIBRARY_PROJECT_NAME,
                                 'assets',
                                 'jsapi')
  if not os.path.exists(jsapi_directory):
    os.makedirs(jsapi_directory)

  jsfiles_to_copy = [
      'xwalk/experimental/presentation/presentation_api.js',
      'xwalk/sysapps/device_capabilities/device_capabilities_api.js'
  ]

  # Copy JS binding file to assets/jsapi folder.
  for jsfile in jsfiles_to_copy:
    source_file = os.path.join(project_source, jsfile)
    target_file = os.path.join(jsapi_directory, os.path.basename(source_file))
    shutil.copyfile(source_file, target_file)


def CopyBinaries(out_directory):
  """cp out/Release/<asset> out/Release/xwalk_core_library/assets/<asset>
     cp out/Release/lib.java/<lib> out/Release/xwalk_core_library/libs/<lib>
     cp out/Release/xwalk_core_shell_apk/libs/*
        out/Release/xwalk_core_library/libs
  """

  print 'Copying binaries...'
  # Copy assets.
  asset_directory = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'assets')
  if not os.path.exists(asset_directory):
    os.mkdir(asset_directory)

  assets_to_copy = [
      'xwalk.pak',
  ]

  for asset in assets_to_copy:
    source_file = os.path.join(out_directory, asset)
    target_file = os.path.join(asset_directory, asset)
    shutil.copyfile(source_file, target_file)

  # Copy jar files to libs.
  libs_directory = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'libs')
  if not os.path.exists(libs_directory):
    os.mkdir(libs_directory)

  libs_to_copy = [
      'eyesfree_java.jar',
      'guava_javalib.jar',
      'jsr_305_javalib.jar',
  ]

  for lib in libs_to_copy:
    source_file = os.path.join(out_directory, 'lib.java', lib)
    target_file = os.path.join(libs_directory, lib)
    shutil.copyfile(source_file, target_file)

  # Copy native libraries.
  source_dir = os.path.join(out_directory, XWALK_CORE_SHELL_APK, 'libs')
  target_dir = libs_directory
  distutils.dir_util.copy_tree(source_dir, target_dir)


def CopyDirAndPrefixDuplicates(input_dir, output_dir, prefix):
  """ Copy the files into the output directory. If one file in input_dir folder
  doesn't exist, copy it directly. If a file exists, copy it and rename the
  file so that the resources won't be overrided. So all of them could be
  packaged into the xwalk core library.
  """
  for root, _, files in os.walk(input_dir):
    for f in files:
      src_file = os.path.join(root, f)
      relative_path = os.path.relpath(src_file, input_dir)
      target_file = os.path.join(output_dir, relative_path)
      target_dir_name = os.path.dirname(target_file)
      if not os.path.exists(target_dir_name):
        os.makedirs(target_dir_name)
      # If the file exists, copy it and rename it with another name to
      # avoid overwriting the existing one.
      if os.path.exists(target_file):
        target_base_name = os.path.basename(target_file)
        target_base_name = prefix + '_' + target_base_name
        target_file = os.path.join(target_dir_name, target_base_name)
      shutil.copyfile(src_file, target_file)


def CopyResources(project_source, out_directory):
  print 'Copying resources...'
  res_directory = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'res')
  if os.path.exists(res_directory):
    shutil.rmtree(res_directory)

  # All resources should be in specific folders in res_directory.
  # Since there might be some resource files with same names from
  # different folders like ui_java, content_java and others,
  # it's necessary to rename some files to avoid overridding.
  res_to_copy = [
      # (package, prefix) turple.
      ('ui/android/java/res', 'ui'),
      ('content/public/android/java/res', 'content'),
      ('xwalk/runtime/android/java/res', 'xwalk_core'),
  ]

  # For each res, there are two generated res folder in out directory,
  # they are res_grit and res_v14_compatibility.
  for res, prefix in res_to_copy:
    res_path_src = os.path.join(project_source, res)
    res_path_grit = os.path.join(out_directory,
        'gen', '%s_java' % prefix, 'res_grit')
    res_path_v14 = os.path.join(out_directory,
        'gen', '%s_java' % prefix, 'res_v14_compatibility')

    for res_path in [res_path_src, res_path_grit, res_path_v14]:
      CopyDirAndPrefixDuplicates(res_path, res_directory, prefix)


def PostCopyLibraryProject(out_directory):
  print 'Post Copy Library Project...'
  common_aidl_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src',
                                  'org', 'chromium', 'content', 'common',
                                  'common.aidl')
  if os.path.exists(common_aidl_file):
    os.remove(common_aidl_file)


def main(argv):
  print 'Generating XWalkCore Library Project...'
  option_parser = optparse.OptionParser()
  AddGeneratorOptions(option_parser)
  options, _ = option_parser.parse_args(argv)

  if not os.path.exists(options.source):
    print 'Source project does not exist, please provide correct directory.'
    sys.exit(1)
  out_directory = options.target

  # Clean directory for project first.
  CleanLibraryProject(out_directory)

  out_project_directory = os.path.join(out_directory, LIBRARY_PROJECT_NAME)
  if not os.path.exists(out_project_directory):
    os.mkdir(out_project_directory)

  # Copy Eclipse project files of library project.
  CopyProjectFiles(options.source, out_directory)
  # Copy Java sources of chromium and xwalk.
  CopyJavaSources(options.source, out_directory)
  CopyGeneratedSources(out_directory)
  # Copy binaries and resuorces.
  CopyBinaries(out_directory)
  CopyResources(options.source, out_directory)
  # Copy JS API binding files.
  CopyJSBindingFiles(options.source, out_directory)
  # Post copy library project.
  PostCopyLibraryProject(out_directory)
  print 'Your Android library project has been created at %s' % (
      out_project_directory)

if __name__ == '__main__':
  sys.exit(main(sys.argv))
