{
  'targets': [
    {
      'target_name': 'generate_xwalk_runtime_client_version',
      'type': 'none',
      'actions': [
        {
          # Generate the version for runtime client.
          'action_name': 'generate_runtime_client_version',
          'variables': {
            'template_file': 'app/android/runtime_client/src/templates/XWalkRuntimeClientVersion.template',
            'output_file': '<(SHARED_INTERMEDIATE_DIR)/version_java/XWalkRuntimeClientVersion.java',
          },
          'inputs': [
            'VERSION',
            '<(template_file)',
            'build/android/generate_runtime_client_version.py',
          ],
          'outputs': [
            '<(output_file)',
          ],
          'action': [
            'python', 'build/android/generate_runtime_client_version.py',
            '--template=<(template_file)',
            '--output=<(output_file)',
            '--xwalk-version=<(xwalk_version)',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_app_runtime_client_java',
      'type': 'none',
      'dependencies': [
        'generate_xwalk_runtime_client_version',
      ],
      'variables': {
        'java_in_dir': 'app/android/runtime_client',
        'generated_src_dirs': [ '<(SHARED_INTERMEDIATE_DIR)/version_java' ],
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'xwalk_app_runtime_activity_java',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_client_java',
      ],
      'variables': {
        'java_in_dir': 'app/android/runtime_activity',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'xwalk_app_hello_world_apk',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_activity_java',
      ],
      'variables': {
        'apk_name': 'XWalkAppHelloWorld',
        'java_in_dir': 'app/android/app_hello_world',
        'resource_dir': 'app/android/app_hello_world/res',
        'asset_location': 'app/android/app_hello_world/assets',
        'additional_input_paths': [
          '<(asset_location)/extensions-config.json',
          '<(asset_location)/index.html',
          '<(asset_location)/sampapp-icon-helloworld.png',
        ],
        'app_manifest_version_code': '<(xwalk_version_code)',
        'app_manifest_version_name': '<(xwalk_version)',
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_app_template_apk',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_activity_java',
      ],
      'variables': {
        'apk_name': 'XWalkAppTemplate',
        'java_in_dir': 'app/android/app_template',
        'resource_dir': 'app/android/app_template/res',
        'asset_location': 'app/android/app_template/assets',
        'additional_input_paths': [
          '<(asset_location)/extensions-config.json',
          '<(asset_location)/index.html',
          '<(asset_location)/sampapp-icon-helloworld.png',
        ],
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'prepare_xwalk_app_template',
      'type': 'none',
      'dependencies': [
        'xwalk_app_template_apk',
        'xwalk_runtime_lib_apk',
        'xwalk_core_embedded',
      ],
      'actions': [
        {
          'action_name': 'prepare_xwalk_app_template',
          'inputs': [
            'tools/prepare.py',
          ],
          'outputs': [
            # put an inexist file here to do this step every time.
            '<(PRODUCT_DIR)/xwalk_app_template_1'
          ],
          'action': [
            'python', 'tools/prepare.py',
            '<(PRODUCT_DIR)/xwalk_app_template'
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_app_template',
      'type': 'none',
      'dependencies': [
        'prepare_xwalk_app_template',
      ],
      'actions': [
        {
          'action_name': 'tar_app_template',
          'inputs': [
            'app/android/app_template/AndroidManifest.xml',
            'tools/tar.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_app_template.tar.gz',
            # put an inexist file here to do this step every time.
            '<(PRODUCT_DIR)/xwalk_app_template.tar.gz1',
          ],
          'action': [
            'python', 'tools/tar.py',
            '<(PRODUCT_DIR)/xwalk_app_template'
          ],
        },
      ],
    },
  ],
}
