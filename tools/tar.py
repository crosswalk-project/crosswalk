#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import sys
import tarfile

def GenerateTemplate(dir_path):
  src = '%s/app_src/AndroidManifest.xml' % dir_path
  if not os.path.exists(src + '.template'):
    shutil.move(src, src + '.template')
  root_path = '%s/app_src/src/org/xwalk/app/template/' % dir_path
  activity = root_path + 'AppTemplateActivity.java'
  application = root_path + 'AppTemplateApplication.java'
  if (os.path.isfile(activity + '.template') and
      os.path.isfile(application + '.template')):
    return
  shutil.move(activity, activity + '.template')
  shutil.move(application, application + '.template')


def main(args):
  if len(args) != 1:
    print 'You must provide only one argument: folder to pack'
    return 1
  dir_to_tar = args[0]
  if dir_to_tar.endswith(os.path.sep):
    dir_to_tar = dir_to_tar[:-1]
  if not os.path.isdir(dir_to_tar):
    print '%s does not exist or not a directory' % dir_to_tar
    return 1

  GenerateTemplate(dir_to_tar)

  work_dir, dir_name = os.path.split(dir_to_tar)
  tar_filename = dir_name + ".tar.gz"
  cur_cwd = os.getcwd()
  try:
    os.chdir(work_dir)
    tar = tarfile.open(tar_filename, "w:gz")
    for root, _, files in os.walk(dir_name):
      for f in files:
        tar.add(os.path.join(root, f))
    tar.close()
  finally:
    os.chdir(cur_cwd)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
