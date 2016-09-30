# Copyright (c) 2016 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is meant to be included into a target in order to invoke MSBuild,
# generate an extension .dll and its respective accompanying bridge DLL from a
# number of C# source files.
# The actions below generate two files in |output_dir|: "<output_name>.dll" and
# "<output_name>_bridge.dll".
#
# Usage:
# {
#   'target_name': 'my_dotnet_extension',
#   'variables': {
#     'output_dir': '<(PRODUCT_DIR)/my_dotnet_extension',
#     'output_name': 'dotnet_extension',
#     'project_path': '<(DEPTH)/xwalk/path/to/my_dotnet_extension/project.csproj',
#     'sources': ['foo.cs', 'bar.cs'],
#   },
#   'includes': ['<(DEPTH)/xwalk/msbuild.gypi'],
# }
#
# Required variables:
#  output_dir - Path where the DLLs will be generated.
#  output_name - Name of the generated DLL (without ".dll" at the end).
#  project_path - Full path to the .csproj that MSBuild will use.
#  sources - List of C# source files.

{
  'type': 'none',
  'dependencies': [
    '<(DEPTH)/xwalk/dotnet/dotnet_bridge.gyp:xwalk_dotnet_bridge',
  ],
  'actions': [
    {
      'action_name': 'msbuild_<(_target_name)',
      'message': 'Building <(_target_name)',
      'inputs': [
        '<(DEPTH)/xwalk/tools/msbuild_dotnet.py',
        '<@(sources)',
      ],
      'outputs': [
        '<(output_name).dll',
      ],
      'variables': {
        'conditions': [
          ['CONFIGURATION_NAME=="Debug" or CONFIGURATION_NAME=="Debug_x64"', {
            'build_type': 'Debug',
          }, {
            'build_type': 'Release',
          }],
        ],
      },
      'action': [
        'python', '<(DEPTH)/xwalk/tools/msbuild_dotnet.py',
        '--build-type', '<(build_type)',
        '--project', '<(project_path)',
        '--output-dir', '<(output_dir)',
      ],
    },
    {
      'action_name': 'rename_bridge_<(_target_name)',
      'message': 'Renaming bridge for <(_target_name)',
      'inputs': [
        '<(DEPTH)/xwalk/tools/copy_rename.py',
        '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
        '<(output_name).dll',
      ],
      'outputs': [
        '<(output_name)_bridge.dll',
      ],
      'action': [
        'python', '<(DEPTH)/xwalk/tools/copy_rename.py',
        '--source-dir', '<(PRODUCT_DIR)',
        '--input-file', 'xwalk_dotnet_bridge.dll',
        '--output-file', '<(output_name)_bridge.dll',
        '--destination-dir', '<(output_dir)/',
      ],
    },
  ],
}
