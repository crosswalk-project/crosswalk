{
  'targets': [
    {
      'target_name': 'wifidirect_extension',
      'variables': {
        'output_dir': '<(PRODUCT_DIR)',
        'output_name': 'wifidirect_extension',
        'project_path': '<(DEPTH)/xwalk/experimental/wifidirect/wifidirect_extension.csproj',
        'sources': [
          'XWalkExtension.cs',
          'XWalkExtensionInstance.cs',
        ],
      },
      'includes': [
        '../../build/msbuild.gypi',
      ],
    },
  ],
}
