{
  'variables': {
    'extension_prefix': '<(DEPTH)/xwalk/extensions/test/win'
  },
  'targets': [
    {
      'target_name': 'dotnet_echo_extension',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/echo_extension',
        'output_name': 'echo_extension',
        'project_path': '<(extension_prefix)/echo_extension/echo_extension.csproj',
        'sources': [
          'test/win/echo_extension/XWalkExtension.cs',
          'test/win/echo_extension/XWalkExtensionInstance.cs',
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_1',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_1',
        'output_name': 'invalid_extension_1',
        'project_path': '<(extension_prefix)/invalid_extension_1/invalid_extension_1.csproj',
        'sources': [
          'test/win/invalid_extension_1/Bla.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_2',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_2',
        'output_name': 'invalid_extension_2',
        'project_path': '<(extension_prefix)/invalid_extension_2/invalid_extension_2.csproj',
        'sources': [
          'test/win/invalid_extension_2/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_3',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_3',
        'output_name': 'invalid_extension_3',
        'project_path': '<(extension_prefix)/invalid_extension_3/invalid_extension_3.csproj',
        'sources': [
          'test/win/invalid_extension_3/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_4',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_4',
        'output_name': 'invalid_extension_4',
        'project_path': '<(extension_prefix)/invalid_extension_4/invalid_extension_4.csproj',
        'sources': [
          'test/win/invalid_extension_4/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_5',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_5',
        'output_name': 'invalid_extension_5',
        'project_path': '<(extension_prefix)/invalid_extension_5/invalid_extension_5.csproj',
        'sources': [
          'test/win/invalid_extension_5/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_6',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_6',
        'output_name': 'invalid_extension_6',
        'project_path': '<(extension_prefix)/invalid_extension_6/invalid_extension_6.csproj',
        'sources': [
          'test/win/invalid_extension_6/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_7',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_7',
        'output_name': 'invalid_extension_7',
        'project_path': '<(extension_prefix)/invalid_extension_7/invalid_extension_7.csproj',
        'sources': [
          'test/win/invalid_extension_7/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_8',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_8',
        'output_name': 'invalid_extension_8',
        'project_path': '<(extension_prefix)/invalid_extension_8/invalid_extension_8.csproj',
        'sources': [
          'test/win/invalid_extension_8/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_9',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_9',
        'output_name': 'invalid_extension_9',
        'project_path': '<(extension_prefix)/invalid_extension_9/invalid_extension_9.csproj',
        'sources': [
          'test/win/invalid_extension_9/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_10',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_10',
        'output_name': 'invalid_extension_10',
        'project_path': '<(extension_prefix)/invalid_extension_10/invalid_extension_10.csproj',
        'sources': [
          'test/win/invalid_extension_10/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_11',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_11',
        'output_name': 'invalid_extension_11',
        'project_path': '<(extension_prefix)/invalid_extension_11/invalid_extension_11.csproj',
        'sources': [
          'test/win/invalid_extension_11/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_invalid_extension_12',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/invalid_extension_12',
        'output_name': 'invalid_extension_12',
        'project_path': '<(extension_prefix)/invalid_extension_12/invalid_extension_12.csproj',
        'sources': [
          'test/win/invalid_extension_12/XWalkExtension.cs'
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_echo_extension1',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/multiple_extension',
        'output_name': 'echo_extension1',
        'project_path': '<(extension_prefix)/echo_extension1/echo_extension1.csproj',
        'sources': [
          'test/win/echo_extension1/XWalkExtension.cs',
          'test/win/echo_extension1/XWalkExtensionInstance.cs',
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
    {
      'target_name': 'dotnet_echo_extension2',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)/tests/dotnet_extension/multiple_extension',
        'output_name': 'echo_extension2',
        'project_path': '<(extension_prefix)/echo_extension2/echo_extension2.csproj',
        'sources': [
          'test/win/echo_extension2/XWalkExtension.cs',
          'test/win/echo_extension2/XWalkExtensionInstance.cs',
        ],
      },
      'includes': ['../build/msbuild.gypi'],
    },
  ],
}
