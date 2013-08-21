{
  'targets': [
    {
      'target_name': 'xwalk_app_runtime_client_java',
      'type': 'none',
      'variables': {
        'java_in_dir': 'app/android/runtime_client',
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
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'prepare_xwalk_app_template',
      'type': 'none',
      'dependencies': [
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
            './build/android/ant/apk-codegen.xml',
            './build/android/ant/apk-package.xml',
            './build/android/ant/apk-package-resources.xml',
            '../build/android/ant/chromium-debug.keystore',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_app_template/scripts/gyp/',
          'files': [
            './build/android/gyp/dex.py',
            '../build/android/gyp/ant.py',
            '../build/android/gyp/jar.py',
            '../build/android/gyp/javac.py',
            '../build/android/gyp/finalize_apk.py',
            '../build/android/gyp/util/',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_app_template/app_src/',
          'files': [
            'app/android/app_template/AndroidManifest.xml',
            'app/android/app_template/assets',
            'app/android/app_template/res',
            'app/android/app_template/src',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_app_template/',
          'files': [
            'build/android/customize.py',
            'build/android/make_apk.py',
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
