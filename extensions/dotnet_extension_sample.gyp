{
  'targets': [
    {
      'target_name': 'dotnet_echo_extension',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_echo_extension',
          'inputs': [
            'test/win/echo_extension/XWalkExtensionInstance.cs',
            'test/win/echo_extension/XWalkExtension.cs'
          ],
          'outputs': [
            'echo_extension.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/echo_extension',
                     '--project', 'echo_extension/echo_extension.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_1',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_1',
          'inputs': [
            'test/win/invalid_extension_1/Bla.cs'
          ],
          'outputs': [
            'invalid_extension_1.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_1',
                     '--project', 'invalid_extension_1/invalid_extension_1.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_2',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_2',
          'inputs': [
            'test/win/invalid_extension_2/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_2.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_2',
                     '--project', 'invalid_extension_2/invalid_extension_2.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_3',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_3',
          'inputs': [
            'test/win/invalid_extension_3/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_3.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_3',
                     '--project', 'invalid_extension_3/invalid_extension_3.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_4',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_4',
          'inputs': [
            'test/win/invalid_extension_4/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_4.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_4',
                     '--project', 'invalid_extension_4/invalid_extension_4.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_5',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_5',
          'inputs': [
            'test/win/invalid_extension_5/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_5.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_5',
                     '--project', 'invalid_extension_5/invalid_extension_5.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_6',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_6',
          'inputs': [
            'test/win/invalid_extension_6/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_6.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_6',
                     '--project', 'invalid_extension_6/invalid_extension_6.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_7',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_7',
          'inputs': [
            'test/win/invalid_extension_7/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_7.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_7',
                     '--project', 'invalid_extension_7/invalid_extension_7.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_8',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_8',
          'inputs': [
            'test/win/invalid_extension_8/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_8.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_8',
                     '--project', 'invalid_extension_8/invalid_extension_8.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_9',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_9',
          'inputs': [
            'test/win/invalid_extension_9/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_9.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_9',
                     '--project', 'invalid_extension_9/invalid_extension_9.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_10',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_10',
          'inputs': [
            'test/win/invalid_extension_10/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_10.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_10',
                     '--project', 'invalid_extension_10/invalid_extension_10.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_11',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_11',
          'inputs': [
            'test/win/invalid_extension_11/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_11.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_11',
                     '--project', 'invalid_extension_11/invalid_extension_11.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_invalid_extension_12',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_invalid_extension_12',
          'inputs': [
            'test/win/invalid_extension_12/XWalkExtension.cs'
          ],
          'outputs': [
            'invalid_extension_12.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_12',
                     '--project', 'invalid_extension_12/invalid_extension_12.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
  ],
}
