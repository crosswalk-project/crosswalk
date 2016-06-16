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
from string import Template
from wrapper_generator import WrapperGenerator

GYP_ANDROID_DIR = os.path.join(os.path.dirname(__file__),
                               os.pardir, os.pardir, os.pardir,
                               'build',
                               'android',
                               'gyp')
sys.path.append(GYP_ANDROID_DIR)

from util import build_utils


# Classes list that have to generate bridge and wrap code.
CLASSES_TO_BE_PROCESS = [
    'ClientCertRequestHandlerInternal',
    'ClientCertRequestInternal',
    'XWalkCookieManagerInternal',
    'XWalkDownloadListenerInternal',
    'XWalkExtensionInternal',
    'XWalkExternalExtensionManagerInternal',
    'XWalkFindListenerInternal',
    'XWalkGetBitmapCallbackInternal',
    'XWalkHttpAuthHandlerInternal',
    'XWalkHttpAuthInternal',
    'XWalkJavascriptResultHandlerInternal',
    'XWalkJavascriptResultInternal',
    'XWalkNativeExtensionLoaderInternal',
    'XWalkNavigationHistoryInternal',
    'XWalkNavigationItemInternal',
    'XWalkPreferencesInternal',
    'XWalkResourceClientInternal',
    'XWalkSettingsInternal',
    'XWalkUIClientInternal',
    'XWalkViewInternal',
    'XWalkWebResourceRequestHandlerInternal',
    'XWalkWebResourceRequestInternal',
    'XWalkWebResourceResponseInternal',
    "XWalkHitTestResultInternal",
]

REFLECTION_HERLPER = [
    'ReflectMethod.java',
    'ReflectField.java',
    'ReflectConstructor.java',
]

WRAPPER_PACKAGE = 'org.xwalk.core'
BRIDGE_PACKAGE = 'org.xwalk.core.internal'

bridge_path = ''
wrapper_path = ''


def PerformSerialize(output_path, generator):
  file_name = generator.GetGeneratedClassFileName()
  with open(os.path.join(output_path, file_name), 'w') as f:
    f.write(generator.GetGeneratedCode())
  print('%s has been generated!' % file_name)


def GenerateJavaBindingClass(input_dir):
  class_loader = JavaClassLoader(input_dir, CLASSES_TO_BE_PROCESS)
  for input_class in CLASSES_TO_BE_PROCESS:
    print('Generate bridge and wrapper code for %s' % input_class)
    java_data = class_loader.GetJavaData(input_class)
    if java_data.class_type == 'interface':
      # Generate Interface code.
      interface_generator = InterfaceGenerator(java_data, class_loader)
      interface_generator.RunTask()
      PerformSerialize(wrapper_path, interface_generator)
    else:
      # Generate Bridge code.
      bridge_generator = BridgeGenerator(java_data, class_loader)
      bridge_generator.RunTask()
      PerformSerialize(bridge_path, bridge_generator)
      # Generate Wrapper code.
      wrapper_generator = WrapperGenerator(java_data, class_loader)
      wrapper_generator.RunTask()
      PerformSerialize(wrapper_path, wrapper_generator)


def GenerateJavaReflectClass(input_dir):
  for helper in REFLECTION_HERLPER:
    with open(os.path.join(wrapper_path, helper), 'w') as f:
      for line in open(os.path.join(input_dir, helper), 'r'):
        if line.startswith('package '):
          f.write('package ' + WRAPPER_PACKAGE + ';\n')
        else:
          f.write(line)


def GenerateJavaTemplateClass(template_dir, xwalk_build_version,
    api_version, min_api_version, verify_xwalk_apk):
  template_file = os.path.join(template_dir, 'XWalkCoreVersion.template')
  template = Template(open(template_file, 'r').read())
  value = {'API_VERSION': api_version,
           'MIN_API_VERSION': min_api_version,
           'XWALK_BUILD_VERSION': xwalk_build_version}
  output_file = os.path.join(bridge_path, "XWalkCoreVersion.java")
  with open(output_file, 'w') as f:
    f.write(template.substitute(value))

  template_file = os.path.join(template_dir, 'XWalkAppVersion.template')
  template = Template(open(template_file, 'r').read())
  value = {'API_VERSION': api_version,
           'VERIFY_XWALK_APK': 'true' if verify_xwalk_apk == 1 else 'false',
           'XWALK_BUILD_VERSION': xwalk_build_version}
  output_file = os.path.join(wrapper_path, "XWalkAppVersion.java")
  with open(output_file, 'w') as f:
    f.write(template.substitute(value))


def main(argv):
  usage = """Usage: %prog [OPTIONS]
This script can generate bridge and wrap source files for given directory.
\'input_dir\' is provided as directory containing source files.
  """
  option_parser = optparse.OptionParser(usage=usage)
  option_parser.add_option('--input-dir',
                           help=('Input source file directory which contains '
                                 'input files'))
  option_parser.add_option('--template-dir',
                           help=('Templates directory to generate java source '
                                 'file'))
  option_parser.add_option('--bridge-output',
                           help=('Output directory where the bridge code is '
                                 'placed.'))
  option_parser.add_option('--wrapper-output',
                           help=('Output directory where the wrap code is '
                                 'placed.'))
  option_parser.add_option('--stamp', help='the file to touch on success.')
  option_parser.add_option('--api-version', help='API Version')
  option_parser.add_option('--min-api-version', help='Min API Version')
  option_parser.add_option('--verify-xwalk-apk', default=0, type='int',
      help='Verify Crosswalk library APK before loading')
  option_parser.add_option('--xwalk-build-version', help='XWalk Build Version')

  options, _ = option_parser.parse_args(argv)
  if (not options.input_dir or
      not options.bridge_output or
      not options.wrapper_output):
    print('Error: Must specify input and output.')
    return 1

  if os.path.isdir(options.bridge_output):
    shutil.rmtree(options.bridge_output)
  if os.path.isdir(options.wrapper_output):
    shutil.rmtree(options.wrapper_output)

  global bridge_path
  bridge_path = os.path.join(options.bridge_output,
                             os.path.sep.join(BRIDGE_PACKAGE.split('.')))
  os.makedirs(bridge_path)

  global wrapper_path
  wrapper_path = os.path.join(options.wrapper_output,
                              os.path.sep.join(WRAPPER_PACKAGE.split('.')))
  os.makedirs(wrapper_path)

  if options.input_dir:
    GenerateJavaBindingClass(options.input_dir)
    GenerateJavaReflectClass(options.input_dir)

  if options.template_dir:
    GenerateJavaTemplateClass(options.template_dir, options.xwalk_build_version,
        options.api_version, options.min_api_version, options.verify_xwalk_apk)

  if options.stamp:
    build_utils.Touch(options.stamp)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
