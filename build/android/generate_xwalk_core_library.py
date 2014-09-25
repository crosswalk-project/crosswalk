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
import zipfile
from common_function import RemoveUnusedFilesInReleaseMode
from xml.dom.minidom import Document

XWALK_CORE_SHELL_APK = 'xwalk_core_shell_apk'

def AddGeneratorOptions(option_parser):
  option_parser.add_option('-s', dest='source',
                           help='Source directory of project root.',
                           type='string')
  option_parser.add_option('-t', dest='target',
                           help='Product out target directory.',
                           type='string')
  option_parser.add_option('--src-package', action='store_true',
                           default=False,
                           help='Use java sources instead of java libs.')


def CleanLibraryProject(out_project_dir):
  if os.path.exists(out_project_dir):
    for item in os.listdir(out_project_dir):
      sub_path = os.path.join(out_project_dir, item)
      if os.path.isdir(sub_path):
        shutil.rmtree(sub_path)
      elif os.path.isfile(sub_path):
        os.remove(sub_path)


def CopyProjectFiles(project_source, out_project_dir):
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
    target_file = os.path.join(out_project_dir, f)

    shutil.copy2(source_file, target_file)


def CopyJSBindingFiles(project_source, out_project_dir):
  print 'Copying js binding files...'
  jsapi_dir = os.path.join(out_project_dir, 'res', 'raw')
  if not os.path.exists(jsapi_dir):
    os.makedirs(jsapi_dir)

  jsfiles_to_copy = [
      'xwalk/experimental/launch_screen/launch_screen_api.js',
      'xwalk/experimental/presentation/presentation_api.js',
      'xwalk/runtime/android/core_internal/src/org/xwalk/core/'
      + 'internal/extension/api/contacts/contacts_api.js',
      'xwalk/runtime/android/core_internal/src/org/xwalk/core/'
      + 'internal/extension/api/device_capabilities/device_capabilities_api.js',
      'xwalk/runtime/android/core_internal/src/org/xwalk/core/'
      + 'internal/extension/api/messaging/messaging_api.js'
  ]

  # Copy JS binding file to assets/jsapi folder.
  for jsfile in jsfiles_to_copy:
    source_file = os.path.join(project_source, jsfile)
    target_file = os.path.join(jsapi_dir, os.path.basename(source_file))
    shutil.copyfile(source_file, target_file)


def CopyBinaries(out_dir, out_project_dir, src_package):
  """cp out/Release/<pak> out/Release/xwalk_core_library/res/raw/<pak>
     cp out/Release/lib.java/<lib> out/Release/xwalk_core_library/libs/<lib>
     cp out/Release/xwalk_core_shell_apk/libs/*
        out/Release/xwalk_core_library/libs
  """

  print 'Copying binaries...'
  # Copy assets.
  res_raw_dir = os.path.join(out_project_dir, 'res', 'raw')
  res_value_dir = os.path.join(out_project_dir, 'res', 'values')
  if not os.path.exists(res_raw_dir):
    os.mkdir(res_raw_dir)
  if not os.path.exists(res_value_dir):
    os.mkdir(res_value_dir)

  paks_to_copy = [
      'icudtl.dat',
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

  libs_dir = os.path.join(out_project_dir, 'libs')
  if not os.path.exists(libs_dir):
    os.mkdir(libs_dir)

  # Copy jar files to libs.
  if src_package:
    libs_to_copy = [
        'eyesfree_java.jar',
        'jsr_305_javalib.jar',
    ]
  else:
    libs_to_copy = [
        'xwalk_core_library_java_app_part.jar',
        'xwalk_core_library_java_library_part.jar',
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


def MoveImagesToNonMdpiFolders(res_root):
  """Move images from drawable-*-mdpi-* folders to drawable-* folders.

  Why? http://crbug.com/289843

  Copied from build/android/gyp/package_resources.py.
  """
  for src_dir_name in os.listdir(res_root):
    src_components = src_dir_name.split('-')
    if src_components[0] != 'drawable' or 'mdpi' not in src_components:
      continue
    src_dir = os.path.join(res_root, src_dir_name)
    if not os.path.isdir(src_dir):
      continue
    dst_components = [c for c in src_components if c != 'mdpi']
    assert dst_components != src_components
    dst_dir_name = '-'.join(dst_components)
    dst_dir = os.path.join(res_root, dst_dir_name)
    if not os.path.isdir(dst_dir):
      os.makedirs(dst_dir)
    for src_file_name in os.listdir(src_dir):
      if not src_file_name.endswith('.png'):
        continue
      src_file = os.path.join(src_dir, src_file_name)
      dst_file = os.path.join(dst_dir, src_file_name)
      assert not os.path.lexists(dst_file)
      shutil.move(src_file, dst_file)


def ReplaceCrunchedImage(project_source, filename, filepath):
  """Replace crunched images with source images.
  """
  search_dir = [
      'content/public/android/java/res',
      'ui/android/java/res'
  ]

  pathname = os.path.basename(filepath)
  #replace crunched 9-patch image resources.
  for search in search_dir:
    absdir = os.path.join(project_source, search)
    for dirname, _, files in os.walk(absdir):
      if filename in files:
        relativedir = os.path.basename(dirname)
        if (pathname == 'drawable' and relativedir == 'drawable-mdpi') or \
            relativedir == pathname:
          source_file = os.path.abspath(os.path.join(dirname, filename))
          target_file = os.path.join(filepath, filename)
          shutil.copyfile(source_file, target_file)
          return


def CopyResources(project_source, out_dir, out_project_dir):
  print 'Copying resources...'
  res_dir = os.path.join(out_project_dir, 'res')
  temp_dir = os.path.join(out_project_dir, 'temp')
  if os.path.exists(res_dir):
    shutil.rmtree(res_dir)
  if os.path.exists(temp_dir):
    shutil.rmtree(temp_dir)

  # All resources should be in specific folders in res_directory.
  # Since there might be some resource files with same names from
  # different folders like ui_java, content_java and others,
  # it's necessary to rename some files to avoid overridding.
  res_to_copy = [
      # zip file list
      'content_java.zip',
      'content_strings_grd.zip',
      'ui_java.zip',
      'ui_strings_grd.zip',
      'xwalk_core_internal_java.zip',
      'xwalk_core_strings.zip'
  ]

  for res_zip in res_to_copy:
    zip_file = os.path.join(out_dir, 'res.java', res_zip)
    zip_name = os.path.splitext(res_zip)[0]
    if not os.path.isfile(zip_file):
      raise Exception('Resource zip not found: ' + zip_file)
    subdir = os.path.join(temp_dir, zip_name)
    if os.path.isdir(subdir):
      raise Exception('Resource zip name conflict: ' + zip_name)
    os.makedirs(subdir)
    with zipfile.ZipFile(zip_file) as z:
      z.extractall(path=subdir)
    CopyDirAndPrefixDuplicates(subdir, res_dir, zip_name)
    MoveImagesToNonMdpiFolders(res_dir)

  if os.path.isdir(temp_dir):
    shutil.rmtree(temp_dir)

  #search 9-patch, then replace it with uncrunch image.
  for dirname, _, files in os.walk(res_dir):
    for filename in files:
      if filename.endswith('.9.png'):
        ReplaceCrunchedImage(project_source, filename, dirname)


def main(argv):
  print 'Generating XWalkCore Library Project...'
  option_parser = optparse.OptionParser()
  AddGeneratorOptions(option_parser)
  options, _ = option_parser.parse_args(argv)

  if not os.path.exists(options.source):
    print 'Source project does not exist, please provide correct directory.'
    sys.exit(1)
  out_dir = options.target
  if options.src_package:
    out_project_dir = os.path.join(out_dir, 'xwalk_core_library_src')
  else:
    out_project_dir = os.path.join(out_dir, 'xwalk_core_library')

  # Clean directory for project first.
  CleanLibraryProject(out_project_dir)

  if not os.path.exists(out_project_dir):
    os.mkdir(out_project_dir)

  # Copy Eclipse project files of library project.
  CopyProjectFiles(options.source, out_project_dir)
  # Copy binaries and resuorces.
  CopyResources(options.source, out_dir, out_project_dir)
  CopyBinaries(out_dir, out_project_dir, options.src_package)
  # Copy JS API binding files.
  CopyJSBindingFiles(options.source, out_project_dir)
  # Remove unused files.
  mode = os.path.basename(os.path.normpath(out_dir))
  RemoveUnusedFilesInReleaseMode(mode,
      os.path.join(out_project_dir, 'libs'))
  # Create empty src directory
  src_dir = os.path.join(out_project_dir, 'src')
  if not os.path.isdir(src_dir):
    os.mkdir(src_dir)
  readme = os.path.join(src_dir, 'README.md')
  open(readme, 'w').write(
      "# Source folder for xwalk_core_library\n"
      "## Why it's empty\n"
      "xwalk_core_library doesn't contain java sources.\n"
      "## Why put me here\n"
      "To make archives keep the folder, "
      "the src directory is needed to build an apk by ant.")
  print 'Your Android library project has been created at %s' % (
      out_project_dir)

if __name__ == '__main__':
  sys.exit(main(sys.argv))
