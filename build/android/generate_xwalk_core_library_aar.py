#!/usr/bin/env python
#
# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Creates an AAR file.

An AAR is a zip file whose format is described here:
https://sites.google.com/a/android.com/tools/tech-docs/new-build-system/aar-format
"""

import argparse
import os
import sys
import zipfile


def AddDirectoryToAAR(aar_file, src_dir, dest_dir):
  """Adds |src_dir| to |aar_file| with the name |dest_dir|."""
  for root, _, files in os.walk(src_dir):
    for f in files:
      aar_file.write(
          os.path.join(root, f),
          os.path.join(dest_dir, os.path.relpath(root, src_dir), f))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--aar-path', required=True,
                      help='Path to the AAR file that will be created.')
  parser.add_argument('--android-manifest', required=True,
                      help='Path to AndroidManifest.xml to copy to the AAR.')
  parser.add_argument('--classes-jar', required=True,
                      help='Path to the JAR that will become classes.jar.')
  parser.add_argument('--jni-abi',
                      help='Android ABI name.')
  parser.add_argument('--jni-dir',
                      help='/path/to/<ABI> that will be copied to libs/.')
  parser.add_argument('--res-dir', required=True,
                      help='/path/to/res to copy to res/ in the AAR file.')
  parser.add_argument('--r-txt', required=True,
                      help='Path to the R.txt file to copy to the AAR file.')

  options = parser.parse_args()
  if bool(options.jni_abi) ^ bool(options.jni_dir):
    print '--jni-abi and --jni-dir must be specified together.'
    return 1

  with zipfile.ZipFile(options.aar_path, 'w', zipfile.ZIP_DEFLATED) as aar:
    if options.jni_dir:
      AddDirectoryToAAR(aar, options.jni_dir,
                        os.path.join('jni', options.jni_abi))
    AddDirectoryToAAR(aar, options.res_dir, 'res')

    aar.write(options.android_manifest, 'AndroidManifest.xml')
    aar.write(options.classes_jar, 'classes.jar')
    aar.write(options.r_txt, 'R.txt')

  return 0


if __name__ == '__main__':
  sys.exit(main())
