{
  'targets': [
    {
      'target_name': 'dotnet_echo_extension',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_echo_extension',
          'inputs': [
            'test/win/echo_extension/XWalkExtension.cs',
            'test/win/echo_extension/XWalkExtensionInstance.cs',
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
      'target_name': 'copy_echo_extension_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_echo_extension',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_echo_extension',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'echo_extension_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/echo_extension',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'echo_extension_bridge.dll',
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
      'target_name': 'copy_invalid_extension_1_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_1',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_1',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_1_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_1',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_1_bridge.dll',
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
      'target_name': 'copy_invalid_extension_2_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_2',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_2',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_2_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_2',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_2_bridge.dll',
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
      'target_name': 'copy_invalid_extension_3_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_3',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_3',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_3_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_3',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_3_bridge.dll',
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
      'target_name': 'copy_invalid_extension_4_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_4',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_4',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_4_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_4',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_4_bridge.dll',
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
      'target_name': 'copy_invalid_extension_5_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_5',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_5',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_5_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_5',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_5_bridge.dll',
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
      'target_name': 'copy_invalid_extension_6_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_6',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_6',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_6_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_6',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_6_bridge.dll',
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
      'target_name': 'copy_invalid_extension_7_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_7',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_7',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_7_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_7',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_7_bridge.dll',
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
      'target_name': 'copy_invalid_extension_8_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_8',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_8',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_8_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_8',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_8_bridge.dll',
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
      'target_name': 'copy_invalid_extension_9_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_9',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_9',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_9_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_9',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_9_bridge.dll',
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
      'target_name': 'copy_invalid_extension_10_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_10',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_10',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_10_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_10',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_10_bridge.dll',
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
      'target_name': 'copy_invalid_extension_11_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_11',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_11',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_11_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_11',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_11_bridge.dll',
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
    {
      'target_name': 'copy_invalid_extension_12_bridge',
      'type': 'none',
      'dependencies': [
        'dotnet_invalid_extension_12',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_invalid_extension_12',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'invalid_extension_12_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_12',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'invalid_extension_12_bridge.dll',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_echo_extension2',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_echo_extension2',
          'inputs': [
            'test/win/echo_extension2/XWalkExtension.cs',
            'test/win/echo_extension2/XWalkExtensionInstance.cs',
          ],
          'outputs': [
            'echo_extension2.dll',
          ],
          'action': ['python',
                     '../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/multiple_extension',
                     '--project', 'echo_extension2/echo_extension2.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'dotnet_echo_extension1',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_dotnet_test_echo_extension1',
          'inputs': [
            'test/win/echo_extension1/XWalkExtension.cs',
            'test/win/echo_extension1/XWalkExtensionInstance.cs',
          ],
          'outputs': [
            'echo_extension1.dll',
          ],
          'action': ['python',
                      '../tools/msbuild_dotnet.py',
                      '--output-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/multiple_extension',
                      '--project', 'echo_extension1/echo_extension1.csproj',
                      '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'create_dir_with_multiple_extensions',
      'type': 'none',
      'dependencies': [
        'dotnet_echo_extension1',
        'dotnet_echo_extension2',
      ],
      'actions': [
        {
          'action_name': 'copy_echo_extension2_bridge',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'echo_extension2_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/multiple_extension',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'echo_extension2_bridge.dll',
          ],
        },
        {
          'action_name': 'copy_echo_extension1_bridge',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'echo_extension1_bridge.dll',
          ],
          'action': ['python',
                      '../tools/copy_rename.py',
                      '--source-dir', '<(PRODUCT_DIR)',
                      '--destination-dir', '<(PRODUCT_DIR)/tests/dotnet_extension/multiple_extension',
                      '--input-file', 'xwalk_dotnet_bridge.dll',
                      '--output-file', 'echo_extension1_bridge.dll',
          ],
        },
      ],
    },
  ],
}
