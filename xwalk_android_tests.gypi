{
  'targets': [
    {
      'target_name': 'xwalk_core_shell_apk',
      'type': 'none',
      'dependencies': [
        'libxwalkcore',
        'xwalk_core_java',
        'xwalk_core_shell_apk_pak',
      ],
      'variables': {
        'apk_name': 'XWalkCoreShell',
        'java_in_dir': 'runtime/android/shell',
        'resource_dir': 'runtime/android/shell/res',
        'native_lib_target': 'libxwalkcore',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_xwview/assets/xwalk.pak',
        ],
        'asset_location': '<(ant_build_out)/xwalk_xwview/assets',
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_core_shell_apk_pak',
      'type': 'none',
      'dependencies': [
        'xwalk_pak',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_xwview/assets',
          'files': [
            '<(PRODUCT_DIR)/xwalk.pak',
          ],
        },
      ],
    },
  ],
}
