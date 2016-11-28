#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import re
import shutil
from string import Template
import sys
import zipfile

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
    'XWalkHitTestResultInternal',
    'XWalkViewInternal',
    'XWalkViewDatabaseInternal',
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


class BaseJavaWriter(object):
  """Abstract base class for writing .java files."""
  def WriteFile(self, file_path, file_name, contents):
    raise NotImplementedError


# TODO(rakuco): We will only need SrcJarJavaWriter once we stop supporting gyp.
class FileSystemJavaWriter(BaseJavaWriter):
  """Writes .java files as actual files in the file system."""
  def WriteFile(self, file_path, file_name, contents):
    build_utils.MakeDirectory(file_path)
    with open(os.path.join(file_path, file_name), 'w') as f:
      f.write(contents)


class SrcJarJavaWriter(BaseJavaWriter):
  """Writes .java files into a .srcjar zip file."""
  def __init__(self, srcjar_path):
    self.srcjar_path = srcjar_path

  def WriteFile(self, file_path, file_name, contents):
    path = file_path.replace('.', os.path.sep) + '/' + file_name
    with zipfile.ZipFile(self.srcjar_path, 'a', zipfile.ZIP_STORED) as f:
      build_utils.AddToZipHermetic(f, path, data=contents)


def GenerateJavaBindingClass(input_dir, bridge_path, bridge_writer,
                             wrapper_path, wrapper_writer):
  class_loader = JavaClassLoader(input_dir, CLASSES_TO_PROCESS)
  for input_class in CLASSES_TO_PROCESS:
    print('Generating bridge and wrapper code for %s.' % input_class)
    java_data = class_loader.GetJavaData(input_class)
    if java_data.class_type == 'interface':
      # Generate Interface code.
      interface_generator = InterfaceGenerator(java_data, class_loader)
      interface_generator.RunTask()
      wrapper_writer.WriteFile(wrapper_path,
                               interface_generator.GetGeneratedClassFileName(),
                               interface_generator.GetGeneratedCode())
    else:
      # Generate Bridge code.
      bridge_generator = BridgeGenerator(java_data, class_loader)
      bridge_generator.RunTask()
      bridge_writer.WriteFile(bridge_path,
                              bridge_generator.GetGeneratedClassFileName(),
                              bridge_generator.GetGeneratedCode())
      # Generate Wrapper code.
      wrapper_generator = WrapperGenerator(java_data, class_loader)
      wrapper_generator.RunTask()
      wrapper_writer.WriteFile(wrapper_path,
                               wrapper_generator.GetGeneratedClassFileName(),
                               wrapper_generator.GetGeneratedCode())


def GenerateJavaReflectClass(input_dir, wrapper_path, wrapper_writer):
  for helper in REFLECTION_HELPERS:
    with open(os.path.join(input_dir, helper)) as helper_file:
      wrapped_contents = re.sub(r'\npackage .+;\n',
                                '\npackage %s;\n' % WRAPPER_PACKAGE,
                                helper_file.read())
      wrapper_writer.WriteFile(wrapper_path, helper, wrapped_contents)


def main(argv):
  parser = argparse.ArgumentParser()
  parser.add_argument('--input-dir', required=True,
                      help=('Input source file directory which contains input '
                            'files'))
  parser.add_argument('--xwalk-app-version-template-path', required=True,
                      help='Path to the XWalkAppVersion.template template.')
  parser.add_argument('--xwalk-core-version-template-path', required=True,
                      help='Path to the XWalkCoreVersion.template template.')
  parser.add_argument('--bridge-output', required=True,
                      help='Output directory where the bridge code is placed.')
  parser.add_argument('--wrapper-output', required=True,
                      help='Output directory where the wrap code is placed.')
  parser.add_argument('--use-srcjars', action='store_true', default=False,
                      help='Write the .java files into .srcjar files.')
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

  # TODO(rakuco): once we stop supporting gyp we shouldn't need those.
  build_utils.MakeDirectory(options.bridge_output)
  build_utils.MakeDirectory(options.wrapper_output)

  if options.use_srcjars:
    bridge_path = BRIDGE_PACKAGE.replace('.', os.path.sep)
    bridge_writer = SrcJarJavaWriter(os.path.join(options.bridge_output,
                                                  'bridge.srcjar'))
    wrapper_path = WRAPPER_PACKAGE.replace('.', os.path.sep)
    wrapper_writer = SrcJarJavaWriter(os.path.join(options.wrapper_output,
                                                   'wrapper.srcjar'))
  else:
    # TODO(rakuco): remove this code path once we stop supporting gyp.
    bridge_path = os.path.join(options.bridge_output,
                               BRIDGE_PACKAGE.replace('.', os.path.sep))
    bridge_writer = FileSystemJavaWriter()
    wrapper_path = os.path.join(options.wrapper_output,
                                WRAPPER_PACKAGE.replace('.', os.path.sep))
    wrapper_writer = FileSystemJavaWriter()

  if options.input_dir:
    GenerateJavaBindingClass(options.input_dir, bridge_path, bridge_writer,
                             wrapper_path, wrapper_writer)
    GenerateJavaReflectClass(options.input_dir, wrapper_path, wrapper_writer)

  # TODO(rakuco): template handling should not be done in this script.
  # Once we stop supporting gyp, we should do this as part of the build system.
  mapping = {
    'API_VERSION': options.api_version,
    'MIN_API_VERSION': options.min_api_version,
    'XWALK_BUILD_VERSION': options.xwalk_build_version,
    'VERIFY_XWALK_APK': str(options.verify_xwalk_apk).lower(),
  }
  with open(os.path.join(options.xwalk_app_version_template_path)) as f:
    contents = f.read()
    wrapper_writer.WriteFile(wrapper_path, 'XWalkAppVersion.java',
                             Template(contents).substitute(mapping))
  with open(os.path.join(options.xwalk_core_version_template_path)) as f:
    contents = f.read()
    bridge_writer.WriteFile(bridge_path, 'XWalkCoreVersion.java',
                            Template(contents).substitute(mapping))

  if options.stamp:
    build_utils.Touch(options.stamp)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
