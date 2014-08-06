#!/usr/bin/env python
#
# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import optparse
import os
import shutil
import sys
import zipfile

LIBRARY_PROJECT_NAME = 'xwalk_core_library'
AAR_LIBRARY_NAME = 'xwalk_core_library_aar'

def AddGeneratorOptions(option_parser):
  option_parser.add_option('-s', dest='source',
                           help='Source directory of project root.',
                           type='string')
  option_parser.add_option('-t', dest='target',
                           help='Product out target directory.',
                           type='string')


def CopyProjectFiles(out_dir):
  """cp out/Release/xwalk_core_library/AndroidManifest<file>
        out/Release/xwalk_core_library_aar/<file>
  """

  print 'Copying library project files...'
  files_to_copy = [
      # AndroidManifest.xml from template.
      'AndroidManifest.xml',
  ]
  for f in files_to_copy:
    source_file = os.path.join(out_dir, LIBRARY_PROJECT_NAME, f)
    target_file = os.path.join(out_dir, AAR_LIBRARY_NAME, f)
    shutil.copy2(source_file, target_file)


def CopyBinaries(out_dir):
  """cp out/Release/xwalk_core_library/libs/*
        out/Release/xwalk_core_library_aar/jni/
  """

  print 'Copying binaries...'
  # Copy jar files to classes.jar.
  libs_dir = os.path.join(out_dir, LIBRARY_PROJECT_NAME, 'libs')

  source_file = os.path.join(libs_dir, 'xwalk_core_library_java.jar')
  target_file = os.path.join(out_dir, AAR_LIBRARY_NAME, 'classes.jar')
  shutil.copyfile(source_file, target_file)

  # Copy native libraries.
  source_dir = os.path.join(out_dir, LIBRARY_PROJECT_NAME, 'libs')
  target_dir = os.path.join(out_dir, AAR_LIBRARY_NAME, 'jni')
  if not os.path.exists(target_dir):
    os.makedirs(target_dir)

  if os.path.exists(source_dir):
    for item in os.listdir(source_dir):
      sub_path = os.path.join(source_dir, item)
      target_dir = os.path.join(target_dir, item)
      if os.path.isdir(sub_path):
        shutil.copytree(sub_path, target_dir)

  # Copy R.txt.
  r_source_dir = os.path.join(out_dir, 'gen', 'xwalk_core_internal_java') 
  r_source_file = os.path.join(r_source_dir, 'java_R', 'R.txt')
  r_target_file = os.path.join(out_dir, AAR_LIBRARY_NAME, 'R.txt')
  shutil.copyfile(r_source_file, r_target_file) 


def CopyResources(out_dir):
  print 'Copying resources...'
  source_dir = os.path.join(out_dir, LIBRARY_PROJECT_NAME, 'res')
  target_dir = os.path.join(out_dir, AAR_LIBRARY_NAME, 'res')
  shutil.copytree(source_dir, target_dir)


def GenerateAAR(aar_path, aar_dir):
  zfile = zipfile.ZipFile(aar_path, 'w')
  abs_src = os.path.abspath(aar_dir)
  for dirname, _, files in os.walk(aar_dir):
    for filename in files:
      absname = os.path.abspath(os.path.join(dirname, filename))
      relativename = absname[len(abs_src) + 1:]
      zfile.write(absname, relativename)
  zfile.close()
  #delete the AAR dir.
  shutil.rmtree(aar_dir)


def main(argv):
  print 'Generating XWalkCore AAR Library...'
  option_parser = optparse.OptionParser()
  AddGeneratorOptions(option_parser)
  options, _ = option_parser.parse_args(argv)

  if not os.path.exists(options.source):
    print 'Source project does not exist, please provide correct directory.'
    sys.exit(1)
  out_dir = options.target

  # Clean the aar library.
  aar_path = os.path.join(out_dir, 'xwalk_core_library.aar')
  if os.path.exists(aar_path):
    os.remove(aar_path)

  aar_dir = os.path.join(out_dir, AAR_LIBRARY_NAME)
  if os.path.exists(aar_dir):
    shutil.rmtree(aar_dir)
  os.mkdir(aar_dir)

  # Copy Eclipse project files of library project.
  CopyProjectFiles(out_dir)
  # Copy binaries and resuorces.
  CopyResources(out_dir)
  CopyBinaries(out_dir)
  GenerateAAR(aar_path, aar_dir)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
