{
  'targets': [
    {
      'target_name': 'xwalk_app_hello_world_apk',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
      ],
      'variables': {
        'apk_name': 'XWalkAppHelloWorld',
        'java_in_dir': 'app/android/app_hello_world',
        'resource_dir': 'app/android/app_hello_world/res',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/app_hello_world/assets/www/index.html',
          '<(PRODUCT_DIR)/app_hello_world/assets/www/sampapp-icon-helloworld.png',
        ],
        'asset_location': '<(PRODUCT_DIR)/app_hello_world/assets',
        'app_manifest_version_code': '<(xwalk_version_code)',
        'app_manifest_version_name': '<(xwalk_version)',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/app_hello_world/assets/www',
          'files': [
            'test/android/data/index.html',
            'test/android/data/sampapp-icon-helloworld.png',
          ],
        },
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_app_template_apk',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
      ],
      'variables': {
        'apk_name': 'XWalkAppTemplate',
        'java_in_dir': 'app/android/app_template',
        'resource_dir': 'app/android/app_template/res',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/app_template/assets/www/index.html',
          '<(PRODUCT_DIR)/app_template/assets/www/sampapp-icon-helloworld.png',
        ],
        'asset_location': '<(PRODUCT_DIR)/app_template/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/app_template/assets/www',
          'files': [
            'test/android/data/index.html',
            'test/android/data/sampapp-icon-helloworld.png',
          ],
        },
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      # Combine runtime client and activity into one jar.
      'target_name': 'xwalk_app_runtime_java',
      'type': 'none',
      'dependencies': [
        'xwalk_core_java',
      ],
      'variables': {
        'java_in_dir': 'app/android/runtime_activity',
        'additional_src_dirs': [
          'app/android/runtime_client',
        ],
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'xwalk_app_template',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
        'xwalk_core_library',
        'xwalk_shared_library',
      ],
      'actions': [
        {
          'action_name': 'prepare_xwalk_app_template',
          'message': 'Generating XWalk App Template.',
          'inputs': [
            'build/android/generate_app_packaging_tool.py',
            '<@(version_files)',
          ],
          'outputs': [
            '<(stamp)',
          ],
          'variables': {
            'stamp': '<(INTERMEDIATE_DIR)/stamp',
            'version_files': [
              '<(DEPTH)/xwalk/API_VERSION',
              '<(DEPTH)/xwalk/VERSION',
            ],
          },
          'action': [
            'python', 'build/android/generate_app_packaging_tool.py',
            '--android-template', '<(DEPTH)/xwalk/app/android/app_template',
            '--core-library-dir', '<(PRODUCT_DIR)/xwalk_core_library',
            '--extra-files', '<(version_files)',
            '--output-dir', '<(PRODUCT_DIR)/xwalk_app_template',
            '--shared-library-dir', '<(PRODUCT_DIR)/xwalk_shared_library',
            '--stamp', '<(stamp)',
            '--xwalk-runtime-jar',
            '<(PRODUCT_DIR)/lib.java/xwalk_app_runtime_java.jar',
          ],
        },
      ],
    },
  ],
}
