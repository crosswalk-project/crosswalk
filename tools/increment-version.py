#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
increment-version.py -- Bump Beta or Canary version number across all required
files.

Crosswalk's versioning schema is "MAJOR.MINOR.BUILD.PATCH". Incrementing a beta
version will monotonically increase the PATCH number, while incrementing a
canary version will monotonically increase the BUILD number.
"""

import optparse
import os
import re
import sys


def PathFromRoot(path):
  """
  Returns the absolute path to |path|, which is supposed to be relative to the
  repository's root directory.
  """
  return os.path.join(os.path.abspath(os.path.dirname(__file__)), '..', path)


def IncrementVersions(replacements):
  """
  |replacements| is a dictionary whose keys are files (relative to the root of
  the repository) and values are regular expresions that match a section in the
  file with the version number we want to increase.

  The regular expression is expected to have 2 groups, the first matching
  whatever precedes the version number and needs to stay the same, and the
  second matching the number itself.

  Each of the files specified will be overwritten with new version numbers.
  """
  for path, regexp in replacements.iteritems():
    # The paths are always relative to the repository's root directory.
    path = PathFromRoot(path)

    def _ReplacementFunction(match_obj):
      version_number = int(match_obj.group(2))
      return '%s%s' % (match_obj.group(1), version_number + 1)

    contents = re.sub(regexp, _ReplacementFunction, open(path).read())
    open(path, 'w').write(contents)


def Main():
  option_parser = optparse.OptionParser()
  option_parser.add_option(
    '', '--type', choices=('beta', 'canary'), dest='release_type',
    help='What part of the version number must be increased. \"beta\" '
         'increases the patch version, \"canary\" increases the build '
         'version.')
  options, _ = option_parser.parse_args()

  if options.release_type == 'beta':
    replacements = {
      'VERSION': r'(PATCH=)(\d+)',
      'packaging/crosswalk.spec': r'(Version:\s+\d+\.\d+\.\d+\.)(\d+)',
    }
    IncrementVersions(replacements)
  elif options.release_type == 'canary':
    replacements = {
      'VERSION': r'(BUILD=)(\d+)',
      'packaging/crosswalk.spec': r'(Version:\s+\d+\.\d+\.)(\d+)',
    }
    IncrementVersions(replacements)
  else:
    print '--type is a required argument and has not been specified. Exiting.'
    return 1

  return 0


if __name__ == '__main__':
  sys.exit(Main())
