#!/usr/bin/env python

# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Given a list of JAR files passed via --jars, produced one single JAR file with
all their contents merged. JAR files with classes outside org.xwalk,
org.chromium and a few others are ignored to avoid clashing with other
packages.
"""

import argparse
import fnmatch
import os
import sys
import zipfile

GYP_ANDROID_DIR = os.path.join(os.path.dirname(__file__),
                               os.pardir, os.pardir, os.pardir,
                               'build',
                               'android',
                               'gyp')
sys.path.append(GYP_ANDROID_DIR)

import jar
from util import build_utils



# The list below contains all entries allowed in JAR files (it can contain
# wildcards).
# If a JAR contains an entry that does not match the white list below, it will
# be rejected and not merged.
JAR_ENTRY_WHITELIST = (
  'META-INF/MANIFEST.MF',
  'SevenZip/*.class',
  'org/chromium/*.class',
  'org/xwalk/*.class',
)

# These are the JAR files we are known to skip because they contain classes in
# namespaces we do not allow -- they are either not necessary or users must
# embed them on their own together with Crosswalk.
KNOWN_SKIPPED_JARS = (
  # android.support.multidex is used by BaseChromiumApplication via
  # ChromiumMultiDexInstaller, but not in release mode. We have not needed it
  # so far (as of Crosswalk 21).
  'android-support-multidex.jar',

  # Chromium used to depend on support-v4 code for its Background Sync
  # implementation in the content layer, but this has not been the case since
  # crrev.com/1324173002. The dependency remains in the build system though.
  # See also: XWALK-5092.
  'android-support-v13.jar',

  # Annotations are not used at runtime (XWALK-6544).
  'jsr_305_javalib.jar',

  # WebVR code uses the Cardboard classes that in turn depend on protobuf.
  # Users are expected to include it in their projects themselves. See
  # XWALK-6597.
  'cardboard.jar',
  'protobuf_nano_javalib.jar',
)


def IsMergeableJar(jar_path):
  """
  Returns True if a certain JAR does not have any classes outside the
  allowed namespaces.
  """
  with zipfile.ZipFile(jar_path) as zip_file:
    for entry_name in zip_file.namelist():
      if entry_name.endswith('/'):  # Directories are irrelevant.
        continue
      if any(fnmatch.fnmatchcase(entry_name, f) for f in JAR_ENTRY_WHITELIST):
        continue
      return False
  return True


def ValidateKnownSkippedJars(jar_list):
  """
  Returns a 2-element tuple (extra, missing), where |extra| is a set with all
  elements in KNOWN_SKIPPED_JARS that were not detected while merging the jars
  in |jar_list|, and |missing| has all the JARs that could not be merged but
  were not in KNOWN_SKIPPED_JARS.
  """
  skipped_jars = set(os.path.basename(jar_file) for jar_file in jar_list \
                     if not IsMergeableJar(jar_file))
  return (set(KNOWN_SKIPPED_JARS) - skipped_jars,
          skipped_jars - set(KNOWN_SKIPPED_JARS))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('jars', nargs=argparse.REMAINDER,
                      help='The jars to merge.')
  parser.add_argument('--output-jar', help='Name of the merged JAR file.')
  parser.add_argument('--validate-skipped-jars-list', action='store_true',
                      help='Whether to validate KNOWN_SKIPPED_JARS by making '
                      'sure it matches all the jars passed in --jars that are '
                      'being skipped.')

  options = parser.parse_args(build_utils.ExpandFileArgs(sys.argv[1:]))
  jars = []
  for j in options.jars:
    jars.extend(build_utils.ParseGypList(j))
  options.jars = jars

  if options.validate_skipped_jars_list:
    extra, missing = ValidateKnownSkippedJars(options.jars)
    # It is fine for |extra| not to be empty: different build options may
    # include fewer JARs. |missing| being non-empty is fatal, though, as it
    # means there will be problems for users since we are not skipping files
    # that we should.
    if extra:
      print
      print 'merge_jars.py: The following JARs in KNOWN_SKIPPED_JARS were ' \
            'not used:'
      print '  %s' % ', '.join(sorted(extra))
      print
    if missing:
      print
      print 'merge_jars.py: The following JARs are not mergeable but are ' \
            'not part of KNOWN_SKIPPED_JARS:'
      print '  %s' % ', '.join(sorted(missing))
      print
      return 1

  with build_utils.TempDir() as temp_dir:
    for jar_file in options.jars:
      # If a JAR has classes outside our allowed namespaces (mostly
      # org.chromium and org.xwalk), we need to skip it otherwise there can be
      # build issues when a user builds an app with Crosswalk as well as
      # another package with one of these non-allowed namespaces (see
      # XWALK-5092, XWALK-6597).
      if not IsMergeableJar(jar_file):
        continue
      build_utils.ExtractAll(jar_file, path=temp_dir, pattern='*.class')
    jar.JarDirectory(temp_dir, options.output_jar)


if __name__ == '__main__':
  sys.exit(main())
