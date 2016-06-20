# Copyright (c) 2015 Intel Corp. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
copy_rename.py -- Copy and rename a file to a given destination
"""

import logging
import optparse
import os
import shutil
import sys


def CopyAndRename(options):
  if not options.source_dir:
    print('You must specify the source directory')
    return 1
  if not options.destination_dir:
    print('You must specify the destination directory')
    return 1
  if not options.input_file:
    print('You must specify the file to copy')
    return 1
  if not options.output_file:
    print('You must specify the destination file name')
    return 1

  dest_dir = os.path.abspath(options.destination_dir)
  src_dir = os.path.abspath(options.source_dir)
  source_file = os.path.join(src_dir, options.input_file)
  new_dest_file_name = os.path.join(dest_dir, options.output_file)
  shutil.copy(source_file, new_dest_file_name)

def main():
  option_parser = optparse.OptionParser()
  option_parser.add_option('--source-dir',
                           help='Source path')
  option_parser.add_option('--destination-dir',
                           help='Path to copy into')
  option_parser.add_option('--input-file',
                           help='Source file to copy')
  option_parser.add_option('--output-file',
                           help='Destination file name')
  options, _ = option_parser.parse_args()
  CopyAndRename(options)


if __name__ == '__main__':
  sys.exit(main())
