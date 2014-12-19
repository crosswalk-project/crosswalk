#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Copyright (c) 2014 Intel Corp. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
upstream_revision.py -- Upstream revision fetching utility.
"""

import optparse
import os
import sys


def WriteIfChanged(file_name, contents):
  """
  Writes the specified contents to the specified file_name
  iff the contents are different than the current contents.
  """
  try:
    old_contents = open(file_name, 'r').read()
  except EnvironmentError:
    pass
  else:
    if contents == old_contents:
      return
    os.unlink(file_name)
  open(file_name, 'w').write(contents)


def main(argv=None):
  if argv is None:
    argv = sys.argv

  parser = optparse.OptionParser(usage="upstream_revision.py [options]")
  parser.add_option("-r", "--revision",
                    help="The revision number.")
  parser.add_option("-o", "--output", metavar="FILE",
                    help="Write revision to FILE. ")
  opts, _ = parser.parse_args(argv[1:])

  contents = 'UPSTREAM_REVISION=%s' % opts.revision
  WriteIfChanged(opts.output, contents)

  return 0


if __name__ == '__main__':
  sys.exit(main())
