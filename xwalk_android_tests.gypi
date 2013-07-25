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
    {
      'target_name': 'xwalk_core_shell_apk_java',
      'type': 'none',
      'dependencies': [
        'xwalk_core_shell_apk',
      ],
      'includes': [ '../build/apk_fake_jar.gypi' ],
    },
    {
      'target_name': 'xwalk_core_test_apk',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:base_java_test_support',
        '../content/content.gyp:content_java_test_support',
        '../net/net.gyp:net_java_test_support',
        'xwalk_core_shell_apk_java',
      ],
      'variables': {
        'apk_name': 'XWalkCoreTest',
        'java_in_dir': 'test/android/javatests',
        'is_test_apk': 1,
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_core_unittests',
      'type': '<(gtest_target_type)',
      'dependencies': [
        '../base/base.gyp:test_support_base',
        '../net/net.gyp:net_test_support',
        '../testing/android/native_test.gyp:native_test_native_code',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'runtime/common/android/xwalk_core_tests.cc',
      ],
    },
    {
      'target_name': 'xwalk_core_unittests_java',
      'type': 'none',
      'dependencies': [
      ],
      'variables': {
        'java_in_dir': 'test/android/unittestjava',
      },
      # TODO: supress gyp error: "'find ../cameo_webview/unittestjava  -name "*.java"' returned exit status 1"
      # 'includes': [ '../build/java.gypi' ],
    },
    {
      'target_name': 'xwalk_core_unittests_jni',
      'type': 'none',
      'sources': [
      ],
      'variables': {
        'jni_gen_package': 'xwalk_core_unittests',
      },
      'includes': [ '../build/jni_generator.gypi' ],
    },
    {
      'target_name': 'xwalk_core_unittests_apk',
      'type': 'none',
      'dependencies': [
        'xwalk_core_unittests',
        'xwalk_core_unittests_java',
        'xwalk_core_unittests_jni',
      ],
      'variables': {
        'test_suite_name': 'xwalk_core_unittests',
        'input_shlib_path': '<(SHARED_LIB_DIR)/<(SHARED_LIB_PREFIX)xwalk_core_unittests<(SHARED_LIB_SUFFIX)',
      },
      'includes': [ '../build/apk_test.gypi' ],
    },
  ],
}
