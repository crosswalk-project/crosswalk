# Copyright (c) 2009 The Chromium Embedded Framework Authors. All rights
# reserved.
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

from optparse import OptionParser
import os
import sys
from patch_util import from_file


# cannot be loaded as a module
if __name__ != "__main__":
  sys.stderr.write('This file cannot be loaded as a module!')
  sys.exit()

# currently only apply patch for android port
if not os.environ.get('XWALK_OS_ANDROID'):
  sys.exit()

# parse command-line options
disc = """
This utility applies patch files.
"""

parser = OptionParser(description=disc)
parser.add_option('--patch-config', dest='patchconfig', metavar='DIR',
                  help='patch configuration file')
(options, args) = parser.parse_args()

# the patchconfig option is required
if options.patchconfig is None:
  parser.print_help(sys.stdout)
  sys.exit()

# normalize the patch directory value
patchdir = os.path.dirname(
    os.path.abspath(options.patchconfig)).replace('\\', '/')
if patchdir[-1] != '/':
  patchdir += '/'

# check if the patching should be skipped
if os.path.isfile(patchdir + 'NOPATCH'):
  nopatch = True
  sys.stdout.write('NOPATCH exists -- files have not been patched.\n')
else:
  nopatch = False
  # locate the patch configuration file
  if not os.path.isfile(options.patchconfig):
    sys.stderr.write('File '+options.patchconfig+' does not exist.\n')
    sys.exit()

  scope = {}
  execfile(options.patchconfig, scope)
  patches = scope["patches"]

  for patch in patches:
    pfile = patchdir + 'patches/' + patch['name'] + '.patch'
    dopatch = True

    if 'condition' in patch:
      # Check that the environment variable is set.
      if patch['condition'] not in os.environ:
        sys.stderr.write('Skipping patch file ' + pfile + '\n')
        dopatch = False

    if dopatch:
      if not os.path.isfile(pfile):
        sys.stderr.write('Patch file ' + pfile + ' does not exist.\n')
      else:
        sys.stderr.write('Reading patch file ' + pfile + '\n')
        pdir = patch['path']
        patchObj = from_file(pfile)
        patchObj.apply(pdir)
        if 'note' in patch:
          separator = '-' * 79 + '\n'
          sys.stderr.write(separator)
          sys.stderr.write('NOTE: '+patch['note']+'\n')
          sys.stderr.write(separator)
