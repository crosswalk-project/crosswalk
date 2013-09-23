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
      'target_name': 'prepare_xwalk_app_template_from_chromium',
      'type': 'none',
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_app_template/scripts/gyp/',
          'files': [
            '../build/android/gyp/ant.py',
            '../build/android/gyp/util/',
          ],
        },
      ],
    },
    {
      'target_name': 'prepare_xwalk_app_template_from_xwalk',
      'type': 'none',
      'dependencies': [
        'prepare_xwalk_app_template_from_chromium',
        'xwalk_app_template_apk',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_app_template/libs/',
          'files': [
            '<(PRODUCT_DIR)/lib.java/xwalk_app_runtime_activity_java.dex.jar',
            '<(PRODUCT_DIR)/lib.java/xwalk_app_runtime_activity_java.jar',
            '<(PRODUCT_DIR)/lib.java/xwalk_app_runtime_client_java.dex.jar',
            '<(PRODUCT_DIR)/lib.java/xwalk_app_runtime_client_java.jar',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_app_template/scripts/ant',
          'files': [
            './app/tools/android/ant/apk-codegen.xml',
            './app/tools/android/ant/apk-package.xml',
            './app/tools/android/ant/apk-package-resources.xml',
            './app/tools/android/ant/xwalk-debug.keystore',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_app_template/scripts/gyp/',
          'files': [
            './app/tools/android/gyp/dex.py',
            './app/tools/android/gyp/finalize_apk.py',
            './app/tools/android/gyp/jar.py',
            './app/tools/android/gyp/javac.py',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_app_template/scripts/gyp/util/',
          'files': [
            './app/tools/android/gyp/util/build_utils.py',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_app_template/app_src/',
          'files': [
            './app/android/app_template/AndroidManifest.xml',
            './app/android/app_template/assets',
            './app/android/app_template/res',
            './app/android/app_template/src',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_app_template/',
          'files': [
            './app/tools/android/customize.py',
            './app/tools/android/make_apk.py',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_app_template',
      'type': 'none',
      'dependencies': [
        'prepare_xwalk_app_template_from_xwalk',
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
