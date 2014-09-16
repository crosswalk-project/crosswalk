{
  'targets': [
    {
      'target_name': 'xwalk_test_util_java',
      'type': 'none',
      'dependencies': [
        '../content/content_shell_and_tests.gyp:content_java_test_support',
        '../net/net.gyp:net_java_test_support',
      ],
      'variables': {
        'java_in_dir': 'test/android/util',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      # Java utils for runtime client related tests.
      'target_name': 'xwalk_runtime_client_test_utils_java',
      'type': 'none',
      'dependencies': [
        'xwalk_test_util_java',
        'xwalk_app_runtime_java',
      ],
      'variables': {
        'java_in_dir': 'test/android/util/runtime_client',
      },
      'includes': [ '../build/java.gypi' ],
    },
    {
      'target_name': 'xwalk_core_shell_apk',
      'type': 'none',
      'dependencies': [
        '../third_party/android_tools/android_tools.gyp:android_support_v13_javalib',
        'libxwalkcore',
        'xwalk_core_extensions_java',
        'xwalk_core_internal_java',
        'xwalk_core_java',
        'xwalk_core_shell_apk_pak',
      ],
      'variables': {
        'apk_name': 'XWalkCoreShell',
        'java_in_dir': 'runtime/android/core_shell',
        'resource_dir': 'runtime/android/core_shell/res',
        'native_lib_target': 'libxwalkcore',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/index.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/request_focus_left_frame.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/request_focus_main.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/request_focus_right_frame.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/request_focus_right_frame1.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/xwalk.pak',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/jsapi/contacts_api.js',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/jsapi/device_capabilities_api.js',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/jsapi/launch_screen_api.js',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/jsapi/messaging_api.js',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/jsapi/presentation_api.js',
        ],
        'conditions': [
          ['icu_use_data_file_flag==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/xwalk_xwview/assets/icudtl.dat',
            ],
          }],
        ],
        'asset_location': '<(PRODUCT_DIR)/xwalk_xwview/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_xwview/assets/www',
          'files': [
            'test/android/data/index.html',
            'test/android/data/request_focus_left_frame.html',
            'test/android/data/request_focus_main.html',
            'test/android/data/request_focus_right_frame.html',
            'test/android/data/request_focus_right_frame1.html',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_xwview/assets/jsapi',
          'files': [
            'experimental/launch_screen/launch_screen_api.js',
            'experimental/presentation/presentation_api.js',
            'runtime/android/core_internal/src/org/xwalk/core/internal/extension/api/contacts/contacts_api.js',
            'runtime/android/core_internal/src/org/xwalk/core/internal/extension/api/device_capabilities/device_capabilities_api.js',
            'runtime/android/core_internal/src/org/xwalk/core/internal/extension/api/messaging/messaging_api.js',
          ],
        },
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
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
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
        '../content/content_shell_and_tests.gyp:content_java_test_support',
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
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/add_js_interface.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/echo.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/echoSync.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/framesEcho.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/fullscreen_enter_exit.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/index.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/profile.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/scale_changed.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/window.close.html',
        ],
        'asset_location': '<(PRODUCT_DIR)/xwalk_xwview_test/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_xwview_test/assets',
          'files': [
            'test/android/data/add_js_interface.html',
            'test/android/data/echo.html',
            'test/android/data/echoSync.html',
            'test/android/data/framesEcho.html',
            'test/android/data/fullscreen_enter_exit.html',
            'test/android/data/index.html',
            'test/android/data/profile.html',
            'test/android/data/scale_changed.html',
            'test/android/data/window.close.html',
          ],
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
      'target_name': 'xwalk_runtime_client_shell_apk',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_java',
        'xwalk_runtime_client_test_utils_java',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeClientShell',
        'java_in_dir': 'app/android/runtime_client_shell',
        'resource_dir': 'app/android/runtime_client_shell/res',
        'is_test_apk': 1,
        'additional_input_paths': [
          '<(PRODUCT_DIR)/runtime_client_shell/assets/extensions-config.json',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/index.html',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/manifest.json',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/myextension/myextension.js',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/sampapp-icon-helloworld.png',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/www/manifest_self.json',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/www/manifest_inline_script.json',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/www/cross_origin.html',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/www/csp.html',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/www/manifest_without_xwalk_hosts.json',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/www/manifest_xwalk_hosts.json',
        ],
        'asset_location': '<(PRODUCT_DIR)/runtime_client_shell/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_shell/assets',
          'files': [
            'test/android/data/manifest.json',
            'test/android/data/extensions-config.json',
            'test/android/data/index.html',
            'test/android/data/sampapp-icon-helloworld.png',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_shell/assets/myextension',
          'files': ['test/android/data/myextension/myextension.js'],
        },
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_shell/assets/www',
          'files': [
            'test/android/data/www/manifest_self.json',
            'test/android/data/www/manifest_inline_script.json',
            'test/android/data/www/cross_origin.html',
            'test/android/data/www/csp.html',
            'test/android/data/www/manifest_without_xwalk_hosts.json',
            'test/android/data/www/manifest_xwalk_hosts.json',
          ],
        },
      ],
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
        'xwalk_app_runtime_java',
        'xwalk_core_internal_java',
        'xwalk_runtime_client_embedded_shell_apk_pak',
        'xwalk_runtime_client_test_utils_java',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeClientEmbeddedShell',
        'java_in_dir': 'app/android/runtime_client_embedded_shell',
        'resource_dir': 'app/android/runtime_client_embedded_shell/res',
        'native_lib_target': 'libxwalkcore',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/extensions-config.json',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/index.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/jsapi/contacts_api.js',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/jsapi/device_capabilities_api.js',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/jsapi/launch_screen_api.js',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/jsapi/messaging_api.js',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/jsapi/presentation_api.js',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/manifest.json',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/myextension/myextension.js',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/sampapp-icon-helloworld.png',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/xwalk.pak',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/www/manifest_self.json',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/www/manifest_inline_script.json',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/www/cross_origin.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/www/csp.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/www/manifest_without_xwalk_hosts.json',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/www/manifest_xwalk_hosts.json',
        ],
        'conditions': [
          ['icu_use_data_file_flag==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/icudtl.dat',
            ],
          }],
        ],
        'asset_location': '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets',
          'files': [
            'test/android/data/manifest.json',
            'test/android/data/extensions-config.json',
            'test/android/data/index.html',
            'test/android/data/sampapp-icon-helloworld.png',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/myextension',
          'files': ['test/android/data/myextension/myextension.js'],
        },
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/jsapi',
          'files': [
            'experimental/launch_screen/launch_screen_api.js',
            'experimental/presentation/presentation_api.js',
            'runtime/android/core_internal/src/org/xwalk/core/internal/extension/api/contacts/contacts_api.js',
            'runtime/android/core_internal/src/org/xwalk/core/internal/extension/api/device_capabilities/device_capabilities_api.js',
            'runtime/android/core_internal/src/org/xwalk/core/internal/extension/api/messaging/messaging_api.js',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/www',
          'files': [
            'test/android/data/www/manifest_self.json',
            'test/android/data/www/manifest_inline_script.json',
            'test/android/data/www/cross_origin.html',
            'test/android/data/www/csp.html',
            'test/android/data/www/manifest_without_xwalk_hosts.json',
            'test/android/data/www/manifest_xwalk_hosts.json',
          ],
        },
      ],
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
          'destination': '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets',
          'files': [
            '<(PRODUCT_DIR)/xwalk.pak',
          ],
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_runtime_client_test_apk',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:base_java_test_support',
        '../content/content_shell_and_tests.gyp:content_java_test_support',
        '../net/net.gyp:net_java_test_support',
        '../tools/android/forwarder2/forwarder.gyp:forwarder2',
        '../tools/android/md5sum/md5sum.gyp:md5sum',
        'xwalk_runtime_client_shell_apk_java',
        'xwalk_test_util_java',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeClientTest',
        'java_in_dir': 'test/android/runtime_client/javatests',
        'is_test_apk': 1,
        'additional_input_paths': [
          '<(PRODUCT_DIR)/runtime_client_test/assets/contacts.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/device_capabilities.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/displayAvailableTest.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/echo.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/echoSync.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/messaging_mini.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/native_file_system.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/screen_orientation.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/timer.html',
        ],
        'asset_location': '<(PRODUCT_DIR)/runtime_client_test/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_test/assets',
          'files': [
            'test/android/data/contacts.html',
            'test/android/data/device_capabilities.html',
            'test/android/data/displayAvailableTest.html',
            'test/android/data/echo.html',
            'test/android/data/echoSync.html',
            'test/android/data/native_file_system.html',
            'test/android/data/screen_orientation.html',
            'test/android/data/sysapps/messaging/messaging_mini.html',
            'test/android/data/timer.html',
          ],
        },
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_runtime_client_embedded_shell_apk_java',
      'type': 'none',
      'dependencies': [
        'xwalk_runtime_client_embedded_shell_apk',
      ],
      'includes': [ '../build/apk_fake_jar.gypi' ],
    },
    {
      'target_name': 'xwalk_runtime_client_embedded_test_apk',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:base_java_test_support',
        '../content/content_shell_and_tests.gyp:content_java_test_support',
        '../net/net.gyp:net_java_test_support',
        '../tools/android/forwarder2/forwarder.gyp:forwarder2',
        '../tools/android/md5sum/md5sum.gyp:md5sum',
        'xwalk_runtime_client_embedded_shell_apk_java',
        'xwalk_test_util_java',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeClientEmbeddedTest',
        'java_in_dir': 'test/android/runtime_client_embedded/javatests',
        'is_test_apk': 1,
        'additional_input_paths': [
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/contacts.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/device_capabilities.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/displayAvailableTest.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/echo.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/echoSync.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/messaging_mini.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/native_file_system.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/screen_orientation.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/timer.html',
        ],
        'asset_location': '<(PRODUCT_DIR)/runtime_client_embedded_test/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_embedded_test/assets',
          'files': [
            'test/android/data/contacts.html',
            'test/android/data/device_capabilities.html',
            'test/android/data/displayAvailableTest.html',
            'test/android/data/echo.html',
            'test/android/data/echoSync.html',
            'test/android/data/native_file_system.html',
            'test/android/data/screen_orientation.html',
            'test/android/data/sysapps/messaging/messaging_mini.html',
            'test/android/data/timer.html',
          ],
        },
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_core_sample_apk',
      'type': 'none',
      'dependencies': [
        'libxwalkcore',
        'xwalk_core_extensions_java',
        'xwalk_core_internal_java',
        'xwalk_core_java',
        'xwalk_core_shell_apk_pak',
      ],
      'variables': {
        'apk_name': 'CrosswalkSample',
        'java_in_dir': 'runtime/android/sample',
        'resource_dir': 'runtime/android/sample/res',
        'native_lib_target': 'libxwalkcore',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/sample/assets/echo.html',
          '<(PRODUCT_DIR)/sample/assets/index.html',
          '<(PRODUCT_DIR)/sample/assets/manifest.json',
          '<(PRODUCT_DIR)/sample/assets/pause_timers.html',
          '<(PRODUCT_DIR)/sample/assets/xwalk.pak',
        ],
        'conditions': [
          ['icu_use_data_file_flag==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/sample/assets/icudtl.dat',
            ],
          }],
        ],
        'asset_location': '<(PRODUCT_DIR)/sample/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/sample/assets',
          'files': [
            'runtime/android/sample/assets/index.html',
            'runtime/android/sample/assets/manifest.json',
            'runtime/android/sample/assets/pause_timers.html',
            'test/android/data/echo.html',
            '<(PRODUCT_DIR)/xwalk.pak',
          ],
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
          ],
        },
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_core_internal_shell_apk',
      'type': 'none',
      'dependencies': [
        '../third_party/android_tools/android_tools.gyp:android_support_v13_javalib',
        'libxwalkcore',
        'xwalk_core_extensions_java',
        'xwalk_core_internal_java',
        'xwalk_core_internal_shell_apk_pak',
      ],
      'variables': {
        'apk_name': 'XWalkCoreInternalShell',
        'java_in_dir': 'runtime/android/core_internal_shell',
        'resource_dir': 'runtime/android/core_internal_shell/res',
        'native_lib_target': 'libxwalkcore',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_internal_xwview/assets/www/index.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview/assets/xwalk.pak',
        ],
        'conditions': [
          ['icu_use_data_file_flag==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/xwalk_internal_xwview/assets/icudtl.dat',
            ],
          }],
        ],
        'asset_location': '<(PRODUCT_DIR)/xwalk_internal_xwview/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_internal_xwview/assets/www',
          'files': [
            'test/android/data/index.html',
          ],
        }
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_core_internal_shell_apk_pak',
      'type': 'none',
      'dependencies': [
        'xwalk_pak',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_internal_xwview/assets',
          'files': [
            '<(PRODUCT_DIR)/xwalk.pak',
          ],
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_internal_shell_apk_java',
      'type': 'none',
      'dependencies': [
        'xwalk_core_internal_shell_apk',
      ],
      'includes': [ '../build/apk_fake_jar.gypi' ],
    },
    {
      'target_name': 'xwalk_core_internal_test_apk',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:base_java_test_support',
        '../content/content_shell_and_tests.gyp:content_java_test_support',
        '../net/net.gyp:net_java_test_support',
        '../tools/android/md5sum/md5sum.gyp:md5sum',
        '../tools/android/forwarder2/forwarder.gyp:forwarder2',
        'xwalk_core_internal_shell_apk_java',
      ],
      'variables': {
        'apk_name': 'XWalkCoreInternalTest',
        'java_in_dir': 'test/android/core_internal/javatests',
        'is_test_apk': 1,
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/broadcast.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/echo.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/echoSync.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/framesEcho.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/geolocation.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/index.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/navigator.online.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/notification.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/renderHung.html',
        ],
        'asset_location': '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets',
          'files': [
            'test/android/data/broadcast.html',
            'test/android/data/echo.html',
            'test/android/data/echoSync.html',
            'test/android/data/framesEcho.html',
            'test/android/data/geolocation.html',
            'test/android/data/index.html',
            'test/android/data/navigator.online.html',
            'test/android/data/notification.html',
            'test/android/data/renderHung.html',
          ],
        },
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },
  ],
}
