{
  'targets': [
    {
      'target_name': 'wifidirect_extension',
      'type': 'none',
      'actions': [
        {
          'action_name': 'build_wifidirect_extension',
          'inputs': [
            'XWalkExtension.cs',
            'XWalkExtensionInstance.cs',
          ],
          'outputs': [
            'wifidirect_extension.dll',
          ],
          'action': ['python',
                     '../../tools/msbuild_dotnet.py',
                     '--output-dir', '<(PRODUCT_DIR)',
                     '--project', '../../../experimental/wifidirect/wifidirect_extension.csproj',
                     '--build-type', 'Debug',
          ],
        },
      ],
    },
    {
      'target_name': 'copy_wifidirect_extension_bridge',
      'type': 'none',
      'dependencies': [
        'wifidirect_extension',
      ],
      'actions': [
        {
          'action_name': 'copy_and_rename_bridge_wifidirect_extension',
          'inputs': [
            '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
          ],
          'outputs': [
            'wifidirect_extension_bridge.dll',
          ],
          'action': ['python',
                     '../../tools/copy_rename.py',
                     '--source-dir', '<(PRODUCT_DIR)',
                     '--input-file', 'xwalk_dotnet_bridge.dll',
                     '--output-file', 'wifidirect_extension_bridge.dll',
                     '--destination-dir', '<(PRODUCT_DIR)/',
          ],
        },
      ],
    },
  ],
}
