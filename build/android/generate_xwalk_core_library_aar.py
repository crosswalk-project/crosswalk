#!/usr/bin/env python
#
# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import sys
import zipfile


def main():
  option_parser = optparse.OptionParser()
  option_parser.add_option('-t', dest='target',
                           help='Product out target directory.')
  option_parser.add_option('--shared', action='store_true',
                           default=False,
                           help='Generate shared library', )
  options, _ = option_parser.parse_args()

  # The first entry of each tuple is the source file/directory that will be
  # copied (and must exist), the second entry is its relative path inside the
  # AAR file.
  if options.shared:
    dirs = (
      (os.path.join(options.target, 'xwalk_shared_library', 'libs'),
       'jni'),
      (os.path.join(options.target, 'xwalk_shared_library', 'res'),
       'res'),
    )
    files = (
      (os.path.join(options.target, 'xwalk_shared_library',
        'AndroidManifest.xml'), 'AndroidManifest.xml'),
      (os.path.join(options.target, 'xwalk_shared_library', 'libs',
        'xwalk_core_library_java_app_part.jar'), 'classes.jar'),
      (os.path.join(options.target, 'xwalk_core_empty_embedder_apk',
        'gen', 'R.txt'), 'R.txt'),
    )
    exclude_files = (
      os.path.join(options.target, 'xwalk_shared_library', 'libs',
        'xwalk_core_library_java_app_part.jar'),
    )

    aar_path = os.path.join(options.target, 'xwalk_shared_library.aar')
  else:
    dirs = (
      (os.path.join(options.target, 'xwalk_core_library', 'libs'),
       'jni'),
      (os.path.join(options.target, 'xwalk_core_library', 'res'),
       'res'),
    )
    files = (
      (os.path.join(options.target, 'xwalk_core_library', 'AndroidManifest.xml'),
       'AndroidManifest.xml'),
      (os.path.join(options.target, 'xwalk_core_library', 'libs',
        'xwalk_core_library_java.jar'),
      'classes.jar'),
      (os.path.join(options.target, 'xwalk_core_empty_embedder_apk',
        'gen', 'R.txt'), 'R.txt'),
    )
    # This is a list of files that will not be packaged: mostly a blacklist of
    # files within |dirs|.
    exclude_files = (
      os.path.join(options.target, 'xwalk_core_library', 'libs',
      'xwalk_core_library_java.jar'),
    )

    aar_path = os.path.join(options.target, 'xwalk_core_library.aar')

  with zipfile.ZipFile(aar_path, 'w', zipfile.ZIP_DEFLATED) as aar_file:
    for src, dest in files:
      aar_file.write(src, dest)
    for src, dest in dirs:
      for root, _, files in os.walk(src):
        for f in files:
          real_path = os.path.join(root, f)
          zip_path = os.path.join(dest, os.path.relpath(root, src), f)
          if real_path in exclude_files:
            continue
          aar_file.write(real_path, zip_path)

  return 0


if __name__ == '__main__':
  sys.exit(main())
