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
  ]
  for f in files_to_copy:
    source_file = os.path.join(template_folder, f)
    target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, f)
    shutil.copyfile(source_file, target_file)


def CopyChromiumJavaSources(project_source, out_directory):
  print 'Copying Java sources...'
  target_package_directory = os.path.join(out_directory, LIBRARY_PROJECT_NAME,
                                          'src', 'org', 'chromium')
  if not os.path.exists(target_package_directory):
    os.makedirs(target_package_directory)

  source_path = os.path.join(project_source, 'base', 'android', 'java', 'src',
                             'org', 'chromium', 'base')
  target_path = os.path.join(target_package_directory, 'base')
  shutil.copytree(source_path, target_path)

  source_path = os.path.join(project_source, 'content', 'public', 'android',
                             'java', 'src', 'org', 'chromium', 'content')
  target_path = os.path.join(target_package_directory, 'content')
  shutil.copytree(source_path, target_path)

  source_path = os.path.join(project_source, 'media', 'base', 'android', 'java',
                             'src', 'org', 'chromium', 'media')
  target_path = os.path.join(target_package_directory, 'media')
  shutil.copytree(source_path, target_path)

  source_path = os.path.join(project_source, 'net', 'android', 'java', 'src',
                             'org', 'chromium', 'net')
  target_path = os.path.join(target_package_directory, 'net')
  shutil.copytree(source_path, target_path)

  source_path = os.path.join(project_source, 'ui', 'android', 'java', 'src',
                             'org', 'chromium', 'ui')
  target_path = os.path.join(target_package_directory, 'ui')
  shutil.copytree(source_path, target_path)

  source_path = os.path.join(project_source, 'components',
                             'navigation_interception', 'android', 'java',
                             'src', 'org', 'chromium', 'components',
                             'navigation_interception',)
  target_path = os.path.join(target_package_directory, 'components',
                             'navigation_interception')
  shutil.copytree(source_path, target_path)

  source_path = os.path.join(project_source, 'components',
                             'web_contents_delegate_android', 'android', 'java',
                             'src', 'org', 'chromium', 'components',
                             'web_contents_delegate_android')
  target_path = os.path.join(target_package_directory, 'components',
                             'web_contents_delegate_android')
  shutil.copytree(source_path, target_path)


def CopyGeneratedSources(out_directory):
  print 'Copying generated source files...'
  source_file = os.path.join(out_directory, XWALK_CORE_SHELL_APK,
                             'native_libraries_java',
                             'NativeLibraries.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'content',
                             'app', 'NativeLibraries.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'gen', 'templates', 'org',
                             'chromium', 'content', 'common',
                             'ResultCodes.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'content', 'common',
                             'ResultCodes.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'gen', 'templates',
                             'org', 'chromium', 'net', 'NetError.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'net', 'NetError.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'gen', 'templates',
                             'org', 'chromium', 'content', 'browser',
                             'PageTransitionTypes.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'content', 'browser',
                             'PageTransitionTypes.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'gen', 'templates',
                             'org', 'chromium', 'content', 'browser',
                             'SpeechRecognitionError.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'content', 'browser',
                             'SpeechRecognitionError.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'gen', 'templates',
                             'org', 'chromium', 'net', 'PrivateKeyType.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'net', 'PrivateKeyType.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'gen', 'templates',
                             'org', 'chromium', 'net',
                             'CertVerifyResultAndroid.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'net', 'CertVerifyResultAndroid.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'gen', 'templates',
                             'org', 'chromium', 'net',
                             'CertificateMimeType.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'net', 'CertificateMimeType.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'gen', 'templates',
                             'org', 'chromium', 'base',
                             'ActivityState.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'base', 'ActivityState.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'gen', 'templates',
                             'org', 'chromium', 'base',
                             'MemoryPressureLevelList.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'base', 'MemoryPressureLevelList.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'gen', 'templates',
                             'org', 'chromium', 'media', 'ImageFormat.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'media', 'ImageFormat.java')
  shutil.copyfile(source_file, target_file)


def CopyXwalkJavaSource(project_source, out_directory):
  print 'Copying XWalk Java sources...'
  target_package_directory = os.path.join(out_directory, LIBRARY_PROJECT_NAME,
                                          'src', 'org', 'xwalk')
  if not os.path.exists(target_package_directory):
    os.mkdir(target_package_directory)
  source_path = os.path.join(project_source, 'xwalk', 'runtime', 'android',
                             'java', 'src', 'org', 'xwalk', 'core')
  target_path = os.path.join(target_package_directory, 'core')
  shutil.copytree(source_path, target_path)

  source_path = os.path.join(project_source, 'xwalk', 'extensions', 'android',
                             'java', 'src', 'org', 'xwalk', 'core', 'extensions')
  target_path = os.path.join(target_package_directory, 'core', 'extensions')
  shutil.copytree(source_path, target_path)

  if not os.path.exists(os.path.join(target_package_directory, 'runtime')):
    os.mkdir(os.path.join(target_package_directory, 'runtime'))
  source_path = os.path.join(project_source, 'xwalk', 'runtime', 'android',
                             'java', 'src', 'org', 'xwalk', 'runtime',
                             'extension')
  target_path = os.path.join(target_package_directory, 'runtime', 'extension')
  shutil.copytree(source_path, target_path)

  source_file = os.path.join(project_source, 'xwalk', 'runtime', 'android',
                             'java', 'src', 'org', 'xwalk', 'runtime',
                             'XWalkCoreExtensionBridge.java')
  target_file = os.path.join(target_package_directory, 'runtime',
                             'XWalkCoreExtensionBridge.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(project_source, 'xwalk', 'runtime', 'android',
                             'java', 'src', 'org', 'xwalk', 'runtime',
                             'XWalkManifestReader.java')
  target_file = os.path.join(target_package_directory, 'runtime',
                             'XWalkManifestReader.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(project_source, 'xwalk', 'runtime', 'android',
                             'java', 'src', 'org', 'xwalk', 'runtime',
                             'XWalkRuntimeViewProvider.java')
  target_file = os.path.join(target_package_directory, 'runtime',
                             'XWalkRuntimeViewProvider.java')
  shutil.copyfile(source_file, target_file)

def CopyBinaries(out_directory):
  print 'Copying binaries...'
  asset_directory = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'assets')
  if not os.path.exists(asset_directory):
    os.mkdir(asset_directory)
  source_file = os.path.join(out_directory,
                             'xwalk.pak')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'assets',
                             'xwalk.pak')
  shutil.copyfile(source_file, target_file)

  # Copy jar files to libs.
  libs_directory = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'libs')
  if not os.path.exists(libs_directory):
    os.mkdir(libs_directory)
  source_file = os.path.join(out_directory, 'lib.java', 'eyesfree_java.jar')
  target_file = os.path.join(libs_directory, 'eyesfree_java.jar')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'lib.java', 'guava_javalib.jar')
  target_file = os.path.join(libs_directory, 'guava_javalib.jar')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_directory, 'lib.java', 'jsr_305_javalib.jar')
  target_file = os.path.join(libs_directory, 'jsr_305_javalib.jar')
  shutil.copyfile(source_file, target_file)

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
  source_path = os.path.join(project_source, 'ui', 'android', 'java', 'res')
  CopyDirAndPrefixDuplicates(source_path, res_directory, 'ui')
  source_path = os.path.join(out_directory, 'gen', 'ui_java',
                             'res_grit')
  CopyDirAndPrefixDuplicates(source_path, res_directory, 'ui')

  source_path = os.path.join(out_directory, 'gen', 'ui_java',
                             'res_v14_compatibility')
  CopyDirAndPrefixDuplicates(source_path, res_directory, 'ui')

  source_path = os.path.join(project_source, 'content', 'public', 'android',
                             'java', 'res')
  CopyDirAndPrefixDuplicates(source_path, res_directory, 'content')
  source_path = os.path.join(out_directory, 'gen', 'content_java',
                             'res_grit')
  CopyDirAndPrefixDuplicates(source_path, res_directory, 'content')
  source_path = os.path.join(out_directory, 'gen', 'content_java',
                             'res_v14_compatibility')
  CopyDirAndPrefixDuplicates(source_path, res_directory, 'content')

  source_path = os.path.join(project_source, 'xwalk', 'runtime', 'android',
                             'java', 'res')
  CopyDirAndPrefixDuplicates(source_path, res_directory, 'xwalk')
  source_path = os.path.join(out_directory, 'gen', 'xwalk_core_java',
                             'res_grit')
  CopyDirAndPrefixDuplicates(source_path, res_directory, 'xwalk')
  source_path = os.path.join(out_directory, 'gen', 'xwalk_core_java',
                             'res_v14_compatibility')
  CopyDirAndPrefixDuplicates(source_path, res_directory, 'xwalk')


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
  # Copy Java sources of chromium.
  CopyChromiumJavaSources(options.source, out_directory)
  CopyXwalkJavaSource(options.source, out_directory)
  CopyGeneratedSources(out_directory)
  # Copy binaries and resuorces.
  CopyBinaries(out_directory)
  CopyResources(options.source, out_directory)
  # Post copy library project.
  PostCopyLibraryProject(out_directory)
  print 'Your Android library project has been created at %s' % (
      out_project_directory)

if __name__ == '__main__':
  sys.exit(main(sys.argv))
