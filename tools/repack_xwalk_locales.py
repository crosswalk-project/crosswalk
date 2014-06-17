#!/usr/bin/env python
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import optparse
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..',
                             'tools', 'grit'))
from grit.format import data_pack

# Some build paths defined by gyp.
GRIT_DIR = None
SHARE_INT_DIR = None
INT_DIR = None

# The target platform. If it is not defined, sys.platform will be used.
OS = None

def calc_output(locale):
  """Determine the file that will be generated for the given locale."""
  #e.g. '<(INTERMEDIATE_DIR)/repack/da.pak',
  # For Fake Bidi, generate it at a fixed path so that tests can safely
  # reference it.
  if locale == 'fake-bidi':
    return '%s/%s.pak' % (INT_DIR, locale)
  if OS == 'mac' or OS == 'ios':
    # For Cocoa to find the locale at runtime, it needs to use '_' instead
    # of '-' (http://crbug.com/20441).  Also, 'en-US' should be represented
    # simply as 'en' (http://crbug.com/19165, http://crbug.com/25578).
    if locale == 'en-US':
      locale = 'en'
    return '%s/repack/%s.lproj/locale.pak' % (INT_DIR, locale.replace('-', '_'))
  else:
    return os.path.join(INT_DIR, 'repack', locale + '.pak')


def calc_inputs(locale):
  """Determine the files that need processing for the given locale."""
  inputs = []

  #e.g. '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_strings_da.pak'
  inputs.append(os.path.join(SHARE_INT_DIR, 'webkit',
                'webkit_strings_%s.pak' % locale))

  #e.g. '<(SHARED_INTERMEDIATE_DIR)/ui/ui_strings_da.pak',
  inputs.append(os.path.join(SHARE_INT_DIR, 'ui', 'ui_strings',
                'ui_strings_%s.pak' % locale))

  #e.g. '<(SHARED_INTERMEDIATE_DIR)/ui/app_locale_settings_da.pak',
  inputs.append(os.path.join(SHARE_INT_DIR, 'ui', 'app_locale_settings',
                'app_locale_settings_%s.pak' % locale))

  return inputs


def list_outputs(locales):
  """Returns the names of files that will be generated for the given locales.

  This is to provide gyp the list of output files, so build targets can
  properly track what needs to be built.
  """
  outputs = []
  for locale in locales:
    outputs.append(calc_output(locale))
  # Quote each element so filename spaces don't mess up gyp's attempt to parse
  # it into a list.
  return " ".join(['"%s"' % x for x in outputs])

def list_inputs(locales):
  """Returns the names of files that will be processed for the given locales.

  This is to provide gyp the list of input files, so build targets can properly
  track their prerequisites.
  """
  inputs = []
  for locale in locales:
    inputs += calc_inputs(locale)
  # Quote each element so filename spaces don't mess up gyp's attempt to parse
  # it into a list.
  return " ".join(['"%s"' % x for x in inputs])


def repack_locales(locales):
  """ Loop over and repack the given locales."""
  for locale in locales:
    inputs = []
    inputs += calc_inputs(locale)
    output = calc_output(locale)
    data_pack.DataPack.RePack(output, inputs)


def DoMain(argv):
  global SHARE_INT_DIR
  global INT_DIR
  global OS

  parser = optparse.OptionParser("usage: %prog [options] locales")
  parser.add_option("-i", action="store_true", dest="inputs", default=False,
                    help="Print the expected input file list, then exit.")
  parser.add_option("-o", action="store_true", dest="outputs", default=False,
                    help="Print the expected output file list, then exit.")
  parser.add_option("-x", action="store", dest="int_dir",
                    help="Intermediate build files output directory.")
  parser.add_option("-s", action="store", dest="share_int_dir",
                    help="Shared intermediate build files output directory.")
  parser.add_option("-p", action="store", dest="os",
                    help="The target OS. (e.g. mac, linux, win, etc.)")
  options, locales = parser.parse_args(argv)
  if not locales:
    parser.error('Please specificy at least one locale to process.\n')

  print_inputs = options.inputs
  print_outputs = options.outputs
  INT_DIR = options.int_dir
  SHARE_INT_DIR = options.share_int_dir
  OS = options.os

  if not OS:
    if sys.platform == 'darwin':
      OS = 'mac'
    elif sys.platform.startswith('linux'):
      OS = 'linux'
    elif sys.platform in ('cygwin', 'win32'):
      OS = 'win'
    else:
      OS = sys.platform

  if not (INT_DIR and SHARE_INT_DIR):
    parser.error('Please specify all of and "-x" and "-s".\n')
  if print_inputs and print_outputs:
    parser.error('Please specify only one of "-i" or "-o".\n')

  if print_inputs:
    return list_inputs(locales)

  if print_outputs:
    return list_outputs(locales)

  return repack_locales(locales)

if __name__ == '__main__':
  results = DoMain(sys.argv[1:])
  if results:
    print results
