# Copyright (c) 2015 Intel Corp. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
msbuild_dotnet.py -- Invoke MSBuild for .NET project.
"""

import argparse
import os
import subprocess
import sys


def LaunchMSBuild(options):
  params = [
    'Platform=AnyCPU',
    'Configuration=%s' % options.build_type,
    'OutDir=%s' % options.output_dir,
    'BaseIntermediateOutputPath=%s\\' % options.output_dir,
  ]
  cmd = [
    'msbuild',
    '/nologo',
    '/p:%s' % ','.join(params),
    '/verbosity:quiet',
    options.project,
  ]
  subprocess.check_call(cmd)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--output-dir', required=True,
                      help='Set the output dir for MSBuild.')
  parser.add_argument('--project', required=True,
                      help='The project to build with MSBuild.')
  parser.add_argument('--build-type', required=True,
                      choices=('Debug', 'Release'),
                      help='The type of build : Release/Debug.')
  options = parser.parse_args()
  # We need to call os.path.abspath() because MSBuild seems consider the
  # project directory the root of a relative path, not the build directory.
  options.output_dir = os.path.abspath(options.output_dir)
  LaunchMSBuild(options)


if __name__ == '__main__':
  sys.exit(main())
