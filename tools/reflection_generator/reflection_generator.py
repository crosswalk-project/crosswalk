#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import shutil
from string import Template
import sys

from bridge_generator import BridgeGenerator
from interface_generator import InterfaceGenerator
from java_class import JavaClassLoader
from wrapper_generator import WrapperGenerator

GYP_ANDROID_DIR = os.path.join(os.path.dirname(__file__),
                               os.pardir, os.pardir, os.pardir,
                               'build',
                               'android',
                               'gyp')
sys.path.append(GYP_ANDROID_DIR)

from util import build_utils


# Classes list that have to generate bridge and wrap code.
CLASSES_TO_PROCESS = [
    'ClientCertRequestHandlerInternal',
    'ClientCertRequestInternal',
    'CustomViewCallbackHandlerInternal',
    'CustomViewCallbackInternal',
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
    "XWalkHitTestResultInternal",
    'XWalkViewInternal',
    'XWalkWebResourceRequestHandlerInternal',
    'XWalkWebResourceRequestInternal',
    'XWalkWebResourceResponseInternal',
]

REFLECTION_HELPERS = [
    'ReflectMethod.java',
    'ReflectField.java',
    'ReflectConstructor.java',
]

WRAPPER_PACKAGE = 'org.xwalk.core'
BRIDGE_PACKAGE = 'org.xwalk.core.internal'


def PerformSerialize(output_path, generator):
  file_name = generator.GetGeneratedClassFileName()
  with open(os.path.join(output_path, file_name), 'w') as f:
    f.write(generator.GetGeneratedCode())
  print('Generated %s.' % file_name)


def GenerateJavaBindingClass(input_dir, bridge_path, wrapper_path):
  class_loader = JavaClassLoader(input_dir, CLASSES_TO_PROCESS)
  for input_class in CLASSES_TO_PROCESS:
    print('Generating bridge and wrapper code for %s.' % input_class)
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


def GenerateJavaReflectClass(input_dir, wrapper_path):
  for helper in REFLECTION_HELPERS:
    with open(os.path.join(wrapper_path, helper), 'w') as f:
      for line in open(os.path.join(input_dir, helper), 'r'):
        if line.startswith('package '):
          f.write('package ' + WRAPPER_PACKAGE + ';\n')
        else:
          f.write(line)


def GenerateJavaTemplateClass(template_dir, bridge_path, wrapper_path,
                              xwalk_build_version, api_version,
                              min_api_version, verify_xwalk_apk):
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
           'VERIFY_XWALK_APK': str(verify_xwalk_apk).lower(),
           'XWALK_BUILD_VERSION': xwalk_build_version}
  output_file = os.path.join(wrapper_path, "XWalkAppVersion.java")
  with open(output_file, 'w') as f:
    f.write(template.substitute(value))


def main(argv):
  parser = argparse.ArgumentParser()
  parser.add_argument('--input-dir', required=True,
                      help=('Input source file directory which contains input '
                            'files'))
  parser.add_argument('--template-dir',
                      help='Templates directory to generate java source file')
  parser.add_argument('--bridge-output', required=True,
                      help='Output directory where the bridge code is placed.')
  parser.add_argument('--wrapper-output', required=True,
                      help='Output directory where the wrap code is placed.')
  parser.add_argument('--stamp', help='the file to touch on success.')
  parser.add_argument('--api-version', help='API Version')
  parser.add_argument('--min-api-version', help='Min API Version')
  parser.add_argument('--verify-xwalk-apk', action='store_true',
                      default=False,
                      help='Verify Crosswalk library APK before loading.')
  parser.add_argument('--xwalk-build-version', help='XWalk Build Version')

  options = parser.parse_args()

  if os.path.isdir(options.bridge_output):
    shutil.rmtree(options.bridge_output)
  if os.path.isdir(options.wrapper_output):
    shutil.rmtree(options.wrapper_output)

  bridge_path = os.path.join(options.bridge_output,
                             BRIDGE_PACKAGE.replace('.', os.path.sep))
  os.makedirs(bridge_path)

  wrapper_path = os.path.join(options.wrapper_output,
                              WRAPPER_PACKAGE.replace('.', os.path.sep))
  os.makedirs(wrapper_path)

  if options.input_dir:
    GenerateJavaBindingClass(options.input_dir, bridge_path, wrapper_path)
    GenerateJavaReflectClass(options.input_dir, wrapper_path)

  if options.template_dir:
    GenerateJavaTemplateClass(options.template_dir,
                              bridge_path, wrapper_path,
                              options.xwalk_build_version,
                              options.api_version, options.min_api_version,
                              options.verify_xwalk_apk)

  if options.stamp:
    build_utils.Touch(options.stamp)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
