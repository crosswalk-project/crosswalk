# Copyright (c) 2015 Intel Corp. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
msbuild_dotnet.py -- Invoke MSBuild for .NET project.
"""

import logging
import optparse
import os
import subprocess
import sys


def LaunchMSBuild(options):
  if not options.output_dir:
    print('You must specify the output-dir')
    return 1
  if not options.project:
    print('You have not specified the project you want to build')
    return 1
  if not options.build_type:
    print('You have not specified the build-type')
    return 1

  output_dir = os.path.abspath(options.output_dir)
  tools_dir = os.path.dirname(os.path.abspath(__file__))
  xwalk_dir = os.path.dirname(tools_dir)
  extensions_dir = os.path.join(xwalk_dir, 'extensions')
  extensions_test_dir = os.path.join(extensions_dir, 'test')
  extensions_win_test_dir = os.path.join(extensions_test_dir,
                                         'win' + os.path.sep)

  output = subprocess.call(['msbuild', extensions_win_test_dir
      + options.project, '/p:Platform=AnyCPU,Configuration='
      + options.build_type + ',OutDir=' + output_dir
      + ',BaseIntermediateOutputPath=' + output_dir + os.path.sep],
      cwd=extensions_win_test_dir)
  if output != 0:
    return output

def main():
  option_parser = optparse.OptionParser()
  option_parser.add_option('--output-dir',
                           help='Set the output dir for MSBuild.')
  option_parser.add_option('--project',
                           help='The project to build with MSBuild.')
  option_parser.add_option('--build-type',
                           help='The type of build : Release/Debug.')
  options, _ = option_parser.parse_args()
  LaunchMSBuild(options)


if __name__ == '__main__':
  sys.exit(main())
