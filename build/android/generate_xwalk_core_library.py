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
  # Copy AndroidManifest.xml from template.
  source_file = os.path.join(project_source, 'xwalk', 'build', 'android',
                             'xwalkcore_library_template',
                             'AndroidManifest.xml')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME,
                             'AndroidManifest.xml')
  shutil.copyfile(source_file, target_file)
  # Copy Eclipse project properties from template.
  source_file = os.path.join(project_source, 'xwalk', 'build', 'android',
                             'xwalkcore_library_template',
                             'project.properties')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME,
                             'project.properties')
  shutil.copyfile(source_file, target_file)
  # Copy Eclipse .project file from template.
  source_file = os.path.join(project_source, 'xwalk', 'build', 'android',
                             'xwalkcore_library_template',
                             '.project')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, '.project')
  shutil.copyfile(source_file, target_file)
  # Copy Eclipse .classpath file from template.
  source_file = os.path.join(project_source, 'xwalk', 'build', 'android',
                             'xwalkcore_library_template',
                             '.classpath')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, '.classpath')
  shutil.copyfile(source_file, target_file)
  # Copy Ant build file.
  source_file = os.path.join(project_source, 'xwalk', 'build', 'android',
                             'xwalkcore_library_template',
                             'build.xml')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'build.xml')
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

  source_file = os.path.join(project_source, 'content', 'public', 'android',
                             'java', 'resource_map', 'org', 'chromium',
                             'content', 'R.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'content', 'R.java')
  shutil.copyfile(source_file, target_file)

  source_file = os.path.join(project_source, 'ui', 'android', 'java',
                             'resource_map', 'org', 'chromium', 'ui', 'R.java')
  target_file = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'ui', 'R.java')
  shutil.copyfile(source_file, target_file)


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



def CopyResources(out_directory):
  print 'Copying resources...'
  res_directory = os.path.join(out_directory, LIBRARY_PROJECT_NAME, 'res')
  if os.path.exists(res_directory):
    shutil.rmtree(res_directory)
  source_path = os.path.join(out_directory, 'gen', 'content_java', 'res_grit')
  distutils.dir_util.copy_tree(source_path, res_directory)

  source_path = os.path.join(out_directory, 'gen', 'xwalk_core_java',
                             'res_grit')
  distutils.dir_util.copy_tree(source_path, res_directory)

  source_path = os.path.join(out_directory, 'gen', 'ui_java', 'res_grit')
  distutils.dir_util.copy_tree(source_path, res_directory)


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
  CopyResources(out_directory)
  # Post copy library project.
  PostCopyLibraryProject(out_directory)
  print 'Your Android library project has been created at %s' % (
      out_project_directory)

if __name__ == '__main__':
  sys.exit(main(sys.argv))
