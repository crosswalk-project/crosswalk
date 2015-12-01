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
        'generate_xwalk_runtime_client_version',
        'xwalk_core_java',
      ],
      'variables': {
        'java_in_dir': 'app/android/runtime_activity',
        'additional_src_dirs': [
          'app/android/runtime_client',
        ],
        'generated_src_dirs': [ '<(SHARED_INTERMEDIATE_DIR)/version_java' ],
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'prepare_xwalk_app_template',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
        'xwalk_app_template_apk',
        'xwalk_core_library',
        'xwalk_shared_library',
      ],
      'actions': [
        {
          'action_name': 'prepare_xwalk_app_template',
          'message': 'Generating XWalk App Template.',
          'inputs': [
            'build/android/common_function.py',
            'build/android/generate_app_packaging_tool.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/prepare_xwalk_app_template_intermediate/always_run',
          ],
          'action': [
            'python', 'build/android/generate_app_packaging_tool.py',
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
            '<(PRODUCT_DIR)/xwalk_app_template_intermediate/always_run',
          ],
          'action': [
            'python', 'tools/tar.py',
            '<(PRODUCT_DIR)/xwalk_app_template'
          ],
        },
      ],
    },
    {
      'target_name': 'adextension',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
      ],
      'variables': {
        'java_in_dir': 'app/tools/android/test_data/extensions/adextension/',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'contactextension',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
      ],
      'variables': {
        'java_in_dir': 'app/tools/android/test_data/extensions/contactextension/',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'myextension',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
      ],
      'variables': {
        'java_in_dir': 'app/tools/android/test_data/extensions/myextension/',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'echoJsStubGenExtension',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
      ],
      'variables': {
        'java_in_dir': 'app/tools/android/test_data/extensions/echoJsStubGenExtension/',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'constructorJsStubGen',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
      ],
      'variables': {
        'java_in_dir': 'app/tools/android/test_data/extensions/constructorJsStubGen/',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'jsStubExtensionByBMI',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
      ],
      'variables': {
        'java_in_dir': 'app/tools/android/test_data/extensions/jsStubExtensionByBMI/',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'xwalk_packaging_tool_test',
      'type': 'none',
      'dependencies': [
        'adextension',
        'contactextension',
        'myextension',
      ],
    },
  ],
}
