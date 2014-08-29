#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import shutil
import sys

from bridge_generator import BridgeGenerator
from interface_generator import InterfaceGenerator
from java_class import JavaClassLoader
from wrapper_generator import WrapperGenerator

# Classes list that have to generate bridge and wrap code.
CLASSES_TO_BE_PROCESS = [
  'XWalkExtensionInternal',
  'XWalkViewInternal',
  'XWalkUIClientInternal',
  'XWalkResourceClientInternal',
  'XWalkPreferencesInternal',
  'XWalkNavigationItemInternal',
  'XWalkNavigationHistoryInternal',
  'XWalkJavascriptResultHandlerInternal',
  'XWalkJavascriptResultInternal',
]


WRAPPER_PACKAGE = 'org.xwalk.core'
BRIDGE_PACKAGE = 'org.xwalk.core.internal'


def FormatPackagePath(folder, package):
  return os.path.join(folder, os.path.sep.join(package.split('.')))


def PerformSerialize(output_path, generator, package):
  # Serialize the code.
  file_name = os.path.join(FormatPackagePath(output_path, package),
                           generator.GetGeneratedClassFileName())
  if not os.path.isdir(os.path.dirname(file_name)):
    os.makedirs(os.path.dirname(file_name))
  file_handle = open(file_name, 'w')
  file_handle.write(generator.GetGeneratedCode())
  file_handle.close()
  print '%s has been generated!' % (file_name)


def GenerateBindingForJavaClass(
    java_data, bridge_output, wrap_output, class_loader):
  if java_data.class_type == 'interface':
    interface_generator = InterfaceGenerator(java_data, class_loader)
    interface_generator.RunTask()
    PerformSerialize(wrap_output, interface_generator, WRAPPER_PACKAGE)
  else:
    # Generate Bridge code.
    bridge_generator = BridgeGenerator(java_data, class_loader)
    bridge_generator.RunTask()
    # Serialize.
    PerformSerialize(bridge_output, bridge_generator, BRIDGE_PACKAGE)
    # Generate Wrapper code.
    wrapper_generator = WrapperGenerator(java_data, class_loader)
    wrapper_generator.RunTask()
    PerformSerialize(wrap_output, wrapper_generator, WRAPPER_PACKAGE)


def GenerateBindingForJavaDirectory(input_dir, bridge_output, wrap_output):
  java_class_loader = JavaClassLoader(input_dir, CLASSES_TO_BE_PROCESS)
  for input_file in os.listdir(input_dir):
    input_class_name = input_file.replace('.java', '')
    if java_class_loader.IsInternalClass(input_class_name):
      # Load all java classes in first.
      java_data = java_class_loader.LoadJavaClass(input_class_name)
      print 'Generate bridge and wrapper code for %s' % input_class_name
      GenerateBindingForJavaClass(
          java_data, bridge_output, wrap_output, java_class_loader)


def CopyReflectionHelperJava(helper_class, wrap_output):
  if helper_class is None:
    return
  f = open(helper_class, 'r')
  output = os.path.join(FormatPackagePath(wrap_output, WRAPPER_PACKAGE),
                        os.path.basename(helper_class))
  if not os.path.isdir(os.path.dirname(output)):
    os.makedirs(os.path.dirname(output))
  fo = open(output, 'w')
  for line in f.read().split('\n'):
    if line.startswith('package '):
      fo.write('package org.xwalk.core;\n')
    else:
      if 'Wrapper Only' in line:
        pass
      else:
        fo.write(line + '\n')
  fo.close()
  f.close()


def Touch(path):
  if not os.path.isdir(os.path.dirname(path)):
    os.makedirs(os.path.dirname(path))
  with open(path, 'a'):
    os.utime(path, None)


def main(argv):
  usage = """Usage: %prog [OPTIONS]
This script can generate bridge and wrap source files for given directory. 
\'input_dir\' is provided as directory containing source files.
  """
  option_parser = optparse.OptionParser(usage=usage)
  option_parser.add_option('--input_dir',
                           help= ('Input source file directory which contains'
                                  'input files'))
  option_parser.add_option('--bridge_output',
                           help=('Output directory where the bridge code'
                                 'is placed.'))
  option_parser.add_option('--wrap_output',
                           help=('Output directory where the wrap code'
                                'is placed.'))
  option_parser.add_option('--helper_class',
                           help=('the path of ReflectionHelper java source, '
                                'will copy it to output folder'))
  option_parser.add_option('--stamp', help='the file to touch on success.')
  options, _ = option_parser.parse_args(argv)
  if not options.input_dir:
    print('Error: Must specify input.')
    return 1
  if os.path.isdir(options.bridge_output):
    shutil.rmtree(options.bridge_output)
  if os.path.isdir(options.wrap_output):
    shutil.rmtree(options.wrap_output)

  if options.input_dir:
    GenerateBindingForJavaDirectory(options.input_dir,
        options.bridge_output, options.wrap_output)
    CopyReflectionHelperJava(options.helper_class,
        options.wrap_output)

  if options.stamp:
    Touch(options.stamp)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
