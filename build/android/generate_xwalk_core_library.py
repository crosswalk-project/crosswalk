#!/usr/bin/env python
#
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import distutils.dir_util
import optparse
import os
import shutil
import sys
from common_function import RemoveUnusedFilesInReleaseMode
from xml.dom.minidom import Document

LIBRARY_PROJECT_NAME = 'xwalk_core_library'
XWALK_CORE_SHELL_APK = 'xwalk_core_shell_apk'

def AddGeneratorOptions(option_parser):
  option_parser.add_option('-s', dest='source',
                           help='Source directory of project root.',
                           type='string')
  option_parser.add_option('-t', dest='target',
                           help='Product out target directory.',
                           type='string')


def CleanLibraryProject(out_dir):
  out_project_path = os.path.join(out_dir, LIBRARY_PROJECT_NAME)
  if os.path.exists(out_project_path):
    for item in os.listdir(out_project_path):
      sub_path = os.path.join(out_project_path, item)
      if os.path.isdir(sub_path):
        shutil.rmtree(sub_path)
      elif os.path.isfile(sub_path):
        os.remove(sub_path)


def CopyProjectFiles(project_source, out_dir):
  """cp xwalk/build/android/xwalkcore_library_template/<file>
        out/Release/xwalk_core_library/<file>
  """

  print 'Copying library project files...'
  template_dir = os.path.join(project_source, 'xwalk', 'build', 'android',
                              'xwalkcore_library_template')
  files_to_copy = [
      # AndroidManifest.xml from template.
      'AndroidManifest.xml',
      # Eclipse project properties from template.
      'project.properties',
      # Ant build file.
      'build.xml',
      # Ant properties file.
      'ant.properties',
  ]
  for f in files_to_copy:
    source_file = os.path.join(template_dir, f)
    target_file = os.path.join(out_dir, LIBRARY_PROJECT_NAME, f)

    shutil.copy2(source_file, target_file)


def CopyJavaSources(project_source, out_dir):
  """cp <path>/java/src/<package>
        out/Release/xwalk_core_library/src/<package>
  """

  print 'Copying Java sources...'
  target_source_dir = os.path.join(
      out_dir, LIBRARY_PROJECT_NAME, 'src')
  if not os.path.exists(target_source_dir):
    os.makedirs(target_source_dir)

  # FIXME(wang16): There is an assumption here the package names listed
  # here are all beginned with "org". If the assumption is broken in
  # future, the logic needs to be adjusted accordingly.
  java_srcs_to_copy = [
      # Chromium java sources.
      'base/android/java/src/org/chromium/base',
      'content/public/android/java/src/org/chromium/content',
      'content/public/android/java/src/org/chromium/content_public',
      'media/base/android/java/src/org/chromium/media',
      'net/android/java/src/org/chromium/net',
      'ui/android/java/src/org/chromium/ui',
      'components/navigation_interception/android/java/'
          'src/org/chromium/components/navigation_interception',
      'components/web_contents_delegate_android/android/java/'
          'src/org/chromium/components/web_contents_delegate_android',

      # R.javas
      'content/public/android/java/resource_map/org/chromium/content/R.java',
      'ui/android/java/resource_map/org/chromium/ui/R.java',

      # XWalk java sources.
      'xwalk/runtime/android/core/src/org/xwalk/core',
      'xwalk/extensions/android/java/src/org/xwalk/core/extensions',
  ]

  for source in java_srcs_to_copy:
    # find the src/org in the path
    slash_org_pos = source.find(r'/org/')
    if slash_org_pos < 0:
      raise Exception('Invalid java source path: %s' % source)
    source_path = os.path.join(project_source, source)
    package_path = source[slash_org_pos+1:]
    target_path = os.path.join(target_source_dir, package_path)
    if os.path.isfile(source_path):
      if not os.path.isdir(os.path.dirname(target_path)):
        os.makedirs(os.path.dirname(target_path))
      shutil.copyfile(source_path, target_path)
    else:
      shutil.copytree(source_path, target_path)


def CopyGeneratedSources(out_dir):
  """cp out/Release/gen/templates/<path>
        out/Release/xwalk_core_library/src/<path>
     cp out/Release/xwalk_core_shell_apk/
            native_libraries_java/NativeLibraries.java
        out/Release/xwalk_core_library/src/org/
            chromium/base/library_loader/NativeLibraries.java
  """

  print 'Copying generated source files...'
  generated_srcs_to_copy = [
      'org/chromium/base/ApplicationState.java',
      'org/chromium/base/MemoryPressureLevelList.java',
      'org/chromium/base/library_loader/NativeLibraries.java',
      'org/chromium/content/browser/GestureEventType.java',
      'org/chromium/content/browser/input/PopupItemType.java',
      'org/chromium/content/browser/PageTransitionTypes.java',
      'org/chromium/content/browser/SpeechRecognitionError.java',
      'org/chromium/content/common/ResultCodes.java',
      'org/chromium/content/common/ScreenOrientationValues.java',
      'org/chromium/content/common/TopControlsState.java',
      'org/chromium/media/ImageFormat.java',
      'org/chromium/net/CertificateMimeType.java',
      'org/chromium/net/CertVerifyStatusAndroid.java',
      'org/chromium/net/NetError.java',
      'org/chromium/net/PrivateKeyType.java',
      'org/chromium/ui/gfx/BitmapFormat.java',
      'org/chromium/ui/WindowOpenDisposition.java'
  ]

  for source in generated_srcs_to_copy:
    source_file = os.path.join(out_dir, 'gen', 'templates', source)
    target_file = os.path.join(
        out_dir, LIBRARY_PROJECT_NAME, 'src', source)
    shutil.copyfile(source_file, target_file)

  source_file = os.path.join(out_dir, XWALK_CORE_SHELL_APK,
                             'native_libraries_java',
                             'NativeLibraries.java')
  target_file = os.path.join(out_dir, LIBRARY_PROJECT_NAME, 'src', 'org',
                             'chromium', 'base', 'library_loader',
                             'NativeLibraries.java')
  shutil.copyfile(source_file, target_file)

def CopyJSBindingFiles(project_source, out_dir):
  print 'Copying js binding files...'
  jsapi_dir = os.path.join(out_dir,
                           LIBRARY_PROJECT_NAME,
                           'res',
                           'raw')
  if not os.path.exists(jsapi_dir):
    os.makedirs(jsapi_dir)

  jsfiles_to_copy = [
      'xwalk/experimental/launch_screen/launch_screen_api.js',
      'xwalk/experimental/presentation/presentation_api.js',
      'xwalk/runtime/extension/screen_orientation_api.js',
      'xwalk/sysapps/device_capabilities/device_capabilities_api.js'
  ]

  # Copy JS binding file to assets/jsapi folder.
  for jsfile in jsfiles_to_copy:
    source_file = os.path.join(project_source, jsfile)
    target_file = os.path.join(jsapi_dir, os.path.basename(source_file))
    shutil.copyfile(source_file, target_file)


def CopyBinaries(out_dir):
  """cp out/Release/<pak> out/Release/xwalk_core_library/res/raw/<pak>
     cp out/Release/lib.java/<lib> out/Release/xwalk_core_library/libs/<lib>
     cp out/Release/xwalk_core_shell_apk/libs/*
        out/Release/xwalk_core_library/libs
  """

  print 'Copying binaries...'
  # Copy assets.
  res_raw_dir = os.path.join(
      out_dir, LIBRARY_PROJECT_NAME, 'res', 'raw')
  res_value_dir = os.path.join(
      out_dir, LIBRARY_PROJECT_NAME, 'res', 'values')
  if not os.path.exists(res_raw_dir):
    os.mkdir(res_raw_dir)
  if not os.path.exists(res_value_dir):
    os.mkdir(res_value_dir)

  paks_to_copy = [
      'xwalk.pak',
  ]

  pak_list_xml = Document()
  resources_node = pak_list_xml.createElement('resources')
  string_array_node = pak_list_xml.createElement('string-array')
  string_array_node.setAttribute('name', 'xwalk_resources_list')
  pak_list_xml.appendChild(resources_node)
  resources_node.appendChild(string_array_node)
  for pak in paks_to_copy:
    source_file = os.path.join(out_dir, pak)
    target_file = os.path.join(res_raw_dir, pak)
    shutil.copyfile(source_file, target_file)
    item_node = pak_list_xml.createElement('item')
    item_node.appendChild(pak_list_xml.createTextNode(pak))
    string_array_node.appendChild(item_node)
  pak_list_file = open(os.path.join(res_value_dir,
                                    'xwalk_resources_list.xml'), 'w')
  pak_list_xml.writexml(pak_list_file, newl='\n', encoding='utf-8')
  pak_list_file.close()

  # Copy jar files to libs.
  libs_dir = os.path.join(out_dir, LIBRARY_PROJECT_NAME, 'libs')
  if not os.path.exists(libs_dir):
    os.mkdir(libs_dir)

  libs_to_copy = [
      'eyesfree_java.jar',
      'guava_javalib.jar',
      'jsr_305_javalib.jar',
  ]

  for lib in libs_to_copy:
    source_file = os.path.join(out_dir, 'lib.java', lib)
    target_file = os.path.join(libs_dir, lib)
    shutil.copyfile(source_file, target_file)

  # Copy native libraries.
  source_dir = os.path.join(out_dir, XWALK_CORE_SHELL_APK, 'libs')
  target_dir = libs_dir
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


def CopyResources(project_source, out_dir):
  print 'Copying resources...'
  res_dir = os.path.join(out_dir, LIBRARY_PROJECT_NAME, 'res')
  if os.path.exists(res_dir):
    shutil.rmtree(res_dir)

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
    res_path_grit = os.path.join(out_dir,
        'gen', '%s_java' % prefix, 'res_grit')
    res_path_v14 = os.path.join(out_dir,
        'gen', '%s_java' % prefix, 'res_v14_compatibility')

    for res_path in [res_path_src, res_path_grit, res_path_v14]:
      CopyDirAndPrefixDuplicates(res_path, res_dir, prefix)


def PostCopyLibraryProject(out_dir):
  print 'Post Copy Library Project...'
  aidls_to_remove = [
      'org/chromium/content/common/common.aidl',
      'org/chromium/net/IRemoteAndroidKeyStoreInterface.aidl',
  ]
  for aidl in aidls_to_remove:
    aidl_file = os.path.join(out_dir, LIBRARY_PROJECT_NAME, 'src', aidl)
    if os.path.exists(aidl_file):
      os.remove(aidl_file)


def main(argv):
  print 'Generating XWalkCore Library Project...'
  option_parser = optparse.OptionParser()
  AddGeneratorOptions(option_parser)
  options, _ = option_parser.parse_args(argv)

  if not os.path.exists(options.source):
    print 'Source project does not exist, please provide correct directory.'
    sys.exit(1)
  out_dir = options.target

  # Clean directory for project first.
  CleanLibraryProject(out_dir)

  out_project_dir = os.path.join(out_dir, LIBRARY_PROJECT_NAME)
  if not os.path.exists(out_project_dir):
    os.mkdir(out_project_dir)

  # Copy Eclipse project files of library project.
  CopyProjectFiles(options.source, out_dir)
  # Copy Java sources of chromium and xwalk.
  CopyJavaSources(options.source, out_dir)
  CopyGeneratedSources(out_dir)
  # Copy binaries and resuorces.
  CopyResources(options.source, out_dir)
  CopyBinaries(out_dir)
  # Copy JS API binding files.
  CopyJSBindingFiles(options.source, out_dir)
  # Post copy library project.
  PostCopyLibraryProject(out_dir)
  # Remove unused files.
  mode = os.path.basename(os.path.normpath(out_dir))
  RemoveUnusedFilesInReleaseMode(mode,
      os.path.join(out_dir, LIBRARY_PROJECT_NAME, 'libs'))
  print 'Your Android library project has been created at %s' % (
      out_project_dir)

if __name__ == '__main__':
  sys.exit(main(sys.argv))
