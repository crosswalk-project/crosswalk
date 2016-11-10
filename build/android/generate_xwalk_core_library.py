#!/usr/bin/env python
#
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Prepares an Eclipse project directory with Crosswalk added as a library.

It creates a new directory with a base, empty template, fills it with the
appropriate resources from Chromium and Crosswalk and optionally adds native
libraries such as libxwalkcore.so.

The output is used by other tools such as generate_app_packaging_tool.py and
generate_xwalk_core_library_aar.py.
"""

import argparse
import collections
import os
import shutil
import sys

GYP_ANDROID_DIR = os.path.join(os.path.dirname(__file__),
                               os.pardir, os.pardir, os.pardir,
                               'build',
                               'android',
                               'gyp')
sys.path.append(GYP_ANDROID_DIR)

SRC_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                        '..', '..', '..'))

from util import build_utils
import package_resources


Resource = collections.namedtuple('Resource', ['filename', 'src'])


def CopyJSBindingFiles(js_files, output_dir):
  res_raw_dir = os.path.join(output_dir, 'res', 'raw')
  build_utils.MakeDirectory(res_raw_dir)
  for js_file in js_files:
    shutil.copy2(js_file, res_raw_dir)


def CopyMainJar(output_dir, jar_path):
  libs_dir = os.path.join(output_dir, 'libs')
  build_utils.MakeDirectory(libs_dir)
  shutil.copy2(jar_path, libs_dir)


def CopyBinaryData(output_dir, binary_files):
  res_raw_dir = os.path.join(output_dir, 'res', 'raw')
  res_value_dir = os.path.join(output_dir, 'res', 'values')
  build_utils.MakeDirectory(res_raw_dir)
  build_utils.MakeDirectory(res_value_dir)

  # Poor man's XML writer. It is safe to assume there are only ASCII characters
  # in the entries we are going to write.
  resource_file = os.path.join(res_value_dir, 'xwalk_resources_list.xml')
  resource_top = """<?xml version="1.0" encoding="utf-8"?>
<resources>
<string-array name="xwalk_resources_list">
"""
  resource_bottom = """</string-array>
</resources>
"""

  with open(resource_file, 'w') as out_f:
    out_f.write(resource_top)
    for binary_file in binary_files:
      shutil.copy2(binary_file, res_raw_dir)
      out_f.write("<item>%s</item>\n" % os.path.basename(binary_file))
    out_f.write(resource_bottom)


def CopyNativeLibraries(output_dir, abi_name, native_libraries):
  destination_path = os.path.join(output_dir, 'libs', abi_name)
  build_utils.MakeDirectory(destination_path)
  for native_lib in native_libraries:
    shutil.copy2(native_lib, destination_path)


def CopyResources(output_dir, resources, resource_strings):
  res_dir = os.path.join(output_dir, 'res')
  build_utils.MakeDirectory(res_dir)

  def _resource_predicate(name):
    """Predicate for the ExtractAll() call below. Makes sure only the files we
    want are extracted."""
    if name == 'OWNERS':
      return False
    _, ext = os.path.splitext(name)
    if ext not in ('.png', '.wav', '.xml'):
      # We raise an exception here because if there is a new file type being
      # packaged we need to check what changed compared to what was going on
      # before.
      raise ValueError("Unexpected file type: %s" % name)
    return True

  # Part 1: extract the partly-processed resource zip files (which do not
  # include the .grd string zips), making sure we replace crunched 9-patch
  # images with the original ones and avoiding file name colisions.
  for index, resource in enumerate(resources):
    with build_utils.TempDir() as temp_dir:
      temp_res_dir = os.path.join(temp_dir, 'res')
      build_utils.ExtractAll(resource.filename, path=temp_res_dir,
                             predicate=_resource_predicate)
      for dirpath, _, filenames in os.walk(temp_res_dir):
        if dirpath == temp_res_dir:  # Do not create res/res/.
          continue
        res_dir_subpath = os.path.join(res_dir, os.path.basename(dirpath))
        build_utils.MakeDirectory(res_dir_subpath)
        for filename in filenames:
          if filename.endswith('.9.png'):
            # 9-patch files need to be handled specially. We need the original,
            # uncrunched versions to avoid crunching them twice and failing
            # (once when building the resources, and then when the user is
            # building their project with Crosswalk).
            original_9p = os.path.join(resource.src,
                                       os.path.basename(dirpath),
                                       filename)
            if not os.path.isfile(original_9p):
              raise IOError("Expected to find %s." % original_9p)
            shutil.copy2(original_9p, os.path.join(dirpath, filename))
          # Avoid ovewriting existing files.
          root, ext = os.path.splitext(filename)
          if os.path.isfile(os.path.join(res_dir_subpath, filename)):
            destname = '%s_%02d%s' % (root, index, ext)
          else:
            destname = filename
          shutil.copy2(os.path.join(dirpath, filename),
                       os.path.join(res_dir_subpath, destname))
  package_resources.MoveImagesToNonMdpiFolders(res_dir)

  # Part 2: extract .xml strings files (made from .grd files).
  for zip_file in resource_strings:
    # Exclude anything that doesn't end in .xml (such as .stamp files generated
    # with GN).
    build_utils.ExtractAll(zip_file, path=res_dir, pattern='*.xml')


def MakeResourceTuple(resource_zip, resource_src):
  """Helper that returns a Resource tuple with |resource_zip| and the
  correspoding directory in the source tree that generated this file.
  """
  # With gyp, we can just use |resource_src|.
  if resource_src is not None:
    return Resource(filename=resource_zip, src=resource_src)

  # With GN, |resource_src| is None and we derive the source path from
  # |resource_zip|.
  if not resource_zip.startswith('gen/'):
    raise ValueError('%s is expected to be in gen/.' % resource_zip)
  src_subpath = os.path.dirname(resource_zip)
  for res_subpath in ('android/java/res', 'java/res', 'res'):
    path = os.path.join(SRC_ROOT,
                        src_subpath[4:],  # Drop the gen/ part.
                        res_subpath)
    if os.path.isdir(path):
      return Resource(filename=resource_zip, src=path)
  raise ValueError('Cannot find the sources for %s' % resource_zip)


def main(argv):
  parser = argparse.ArgumentParser()
  build_utils.AddDepfileOption(parser)
  parser.add_argument('--abi',
                      help='Android ABI being used in the build.')
  parser.add_argument('--binary-files', default='',
                      help='Binary files to store in res/raw.')
  parser.add_argument('--js-bindings', required=True,
                      help='.js files to copy to res/raw.')
  parser.add_argument('--main-jar', required=True,
                      help='Path to the main JAR to copy to libs/.')
  parser.add_argument('--native-libraries', default='',
                      help='List of libraries to copy to libs/<abi>.')
  parser.add_argument('--output-dir', required=True,
                      help='Directory where the project will be created.')
  parser.add_argument('--resource-strings', default='',
                      help='List of zipped .grd files.')
  parser.add_argument('--resource-zip-sources', default='',
                      help='Source directories corresponding to each zipped '
                      'resource file from --resource-zips.')
  parser.add_argument('--resource-zips', default='',
                      help='Zipped, processed resource files.')
  parser.add_argument('--stamp', required=True,
                      help='Path to touch on success.')
  parser.add_argument('--template-dir', required=True,
                      help='Directory with an empty app template.')

  options = parser.parse_args(build_utils.ExpandFileArgs(sys.argv[1:]))

  options.resource_strings = build_utils.ParseGypList(options.resource_strings)
  options.resource_zips = build_utils.ParseGypList(options.resource_zips)
  options.resource_zip_sources = build_utils.ParseGypList(
      options.resource_zip_sources)

  # With GN, just create a list with None as elements, we derive the source
  # directories based on the zipped resources' paths.
  if len(options.resource_zip_sources) == 0:
    options.resource_zip_sources = [None] * len(options.resource_zips)
  if len(options.resource_zips) != len(options.resource_zip_sources):
    print('--resource-zips and --resource-zip-sources must have the same '
          'number of arguments.')
    return 1

  resources = []
  for resource_zip, resource_src in zip(options.resource_zips,
                                        options.resource_zip_sources):
    if resource_zip.endswith('_grd.resources.zip'):
      # In GN, we just use --resource-zips, and the string files are
      # part of the list.
      # We need to put them into options.resource_strings for separate
      # processing, and do so by filtering the files by their names.
      options.resource_strings.append(resource_zip)
    else:
      resources.append(MakeResourceTuple(resource_zip, resource_src))

  options.binary_files = build_utils.ParseGypList(options.binary_files)
  options.js_bindings = build_utils.ParseGypList(options.js_bindings)
  options.native_libraries = build_utils.ParseGypList(options.native_libraries)

  # Copy Eclipse project files of library project.
  build_utils.DeleteDirectory(options.output_dir)
  shutil.copytree(options.template_dir, options.output_dir)

  # Copy binaries and resources.
  CopyResources(options.output_dir, resources, options.resource_strings)
  CopyMainJar(options.output_dir, options.main_jar)

  if options.binary_files:
    CopyBinaryData(options.output_dir, options.binary_files)

  if options.native_libraries:
    CopyNativeLibraries(options.output_dir, options.abi,
                        options.native_libraries)

  # Copy JS API binding files.
  CopyJSBindingFiles(options.js_bindings, options.output_dir)

  # Create an empty src/.
  build_utils.MakeDirectory(os.path.join(options.output_dir, 'src'))
  build_utils.Touch(os.path.join(options.output_dir, 'src', '.empty'))

  # Write a depfile so that these files, which are not tracked directly by GN,
  # also trigger a re-run of this script when modified.
  build_utils.WriteDepfile(
    options.depfile,
    options.binary_files + options.native_libraries + options.js_bindings)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
