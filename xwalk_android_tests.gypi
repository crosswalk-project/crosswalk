{
  'targets': [
    {
      'target_name': 'xwalk_core_shell_apk',
      'type': 'none',
      'dependencies': [
        'libxwalkcore',
        'xwalk_core_extensions_java',
        'xwalk_core_java',
        'xwalk_core_shell_apk_pak',
      ],
      'variables': {
        'apk_name': 'XWalkCoreShell',
        'java_in_dir': 'runtime/android/core_shell',
        'resource_dir': 'runtime/android/core_shell/res',
        'native_lib_target': 'libxwalkcore',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_xwview/assets/index.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/xwalk.pak',
        ],
        'asset_location': '<(ant_build_out)/xwalk_xwview/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_xwview/assets',
          'files': ['<(java_in_dir)/assets/index.html'],
        }
      ],
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
        '../tools/android/md5sum/md5sum.gyp:md5sum',
        '../tools/android/forwarder2/forwarder.gyp:forwarder2',
      ],
      'variables': {
        'apk_name': 'XWalkCoreTest',
        'java_in_dir': 'test/android/core/javatests',
        'is_test_apk': 1,
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/echo.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/echoSync.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/index.html',
        ],
        'asset_location': '<(ant_build_out)/xwalk_xwview_test/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_xwview_test/assets',
          'files': ['<(java_in_dir)/assets/echo.html'],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_xwview_test/assets',
          'files': ['<(java_in_dir)/assets/echoSync.html'],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_xwview_test/assets',
          'files': ['<(java_in_dir)/assets/index.html'],
        },
      ],
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
    {
      'target_name': 'xwalk_runtime_shell_apk',
      'type': 'none',
      'dependencies': [
        'libxwalkcore',
        'xwalk_core_extensions_java',
        # Runtime code is also built by this target.
        'xwalk_core_java',
        'xwalk_runtime_shell_apk_pak',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeShell',
        'java_in_dir': 'runtime/android/runtime_shell',
        'resource_dir': 'runtime/android/runtime_shell/res',
        'native_lib_target': 'libxwalkcore',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_runtime/assets/xwalk.pak',
        ],
        'asset_location': '<(ant_build_out)/xwalk_runtime/assets',
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_runtime_shell_apk_pak',
      'type': 'none',
      'dependencies': [
        'xwalk_pak',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_runtime/assets',
          'files': [
            '<(PRODUCT_DIR)/xwalk.pak',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_runtime_shell_apk_java',
      'type': 'none',
      'dependencies': [
        'xwalk_runtime_shell_apk',
      ],
      'includes': [ '../build/apk_fake_jar.gypi' ],
    },
    {
      'target_name': 'xwalk_runtime_client_shell_apk',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_client_java',
        'xwalk_app_runtime_activity_java',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeClientShell',
        'java_in_dir': 'app/android/runtime_client_shell',
        'resource_dir': 'app/android/runtime_client_shell/res',
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_runtime_client_shell_apk_java',
      'type': 'none',
      'dependencies': [
        'xwalk_runtime_client_shell_apk',
      ],
      'includes': [ '../build/apk_fake_jar.gypi' ],
    },
    {
      'target_name': 'xwalk_runtime_client_embedded_shell_apk',
      'type': 'none',
      'dependencies': [
        'libxwalkcore',
        'xwalk_app_runtime_client_java',
        'xwalk_app_runtime_activity_java',
        'xwalk_core_java',
        'xwalk_runtime_client_embedded_shell_apk_pak',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeClientEmbeddedShell',
        'java_in_dir': 'app/android/runtime_client_embedded_shell',
        'resource_dir': 'app/android/runtime_client_embedded_shell/res',
        'native_lib_target': 'libxwalkcore',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_runtime_client_embedded_shell/assets/xwalk.pak',
        ],
        'asset_location': '<(ant_build_out)/xwalk_runtime_client_embedded_shell/assets',
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_runtime_client_embedded_shell_apk_pak',
      'type': 'none',
      'dependencies': [
        'xwalk_pak',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_runtime_client_embedded_shell/assets',
          'files': [
            '<(PRODUCT_DIR)/xwalk.pak',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_runtime_test_apk',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:base_java_test_support',
        '../content/content.gyp:content_java_test_support',
        '../net/net.gyp:net_java_test_support',
        '../tools/android/forwarder2/forwarder.gyp:forwarder2',
        '../tools/android/md5sum/md5sum.gyp:md5sum',
        'xwalk_runtime_shell_apk_java',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeTest',
        'java_in_dir': 'test/android/runtime/javatests',
        'is_test_apk': 1,
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
  ],
}
