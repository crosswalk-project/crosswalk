#!/usr/bin/env python

import os
import sys
import tarfile

def main(args):
  if len(args) != 1:
    print 'You must provide only one argument: folder to pack'
    return 1
  dir_to_tar = args[0]
  if dir_to_tar.endswith(os.path.sep):
    dir_to_tar = dir_to_tar[:-1]
  if not os.path.isdir(dir_to_tar):
    print '%s is not exist or not a directory' % dir_to_tar
    return 1
  work_dir, dir_name = os.path.split(dir_to_tar)
  tar_filename = dir_name + ".tar.gz"
  cur_cwd = os.getcwd()
  try:
    os.chdir(work_dir)
    tar = tarfile.open(tar_filename, "w:gz")
    # pylint: disable=W0612
    for root, dirs, files in os.walk(dir_name):
      for f in files:
        tar.add(os.path.join(root, f))
    tar.close()
  finally:
    os.chdir(cur_cwd)

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
