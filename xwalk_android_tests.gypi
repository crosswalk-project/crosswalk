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
        'xwalk_core_extensions_java',
        'xwalk_core_internal_java',
        'xwalk_core_java',
        'xwalk_core_shell_apk_pak',
        'libxwalkdummy',
      ],
      'variables': {
        'apk_name': 'XWalkCoreShell',
        'java_in_dir': 'runtime/android/core_shell',
        'resource_dir': 'runtime/android/core_shell/res',
        'native_lib_target': 'libxwalkdummy',
        'additional_bundled_libs': [
          '<(PRODUCT_DIR)/lib/libxwalkcore.>(android_product_extension)',
        ],
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/cross_origin.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/index.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/request_focus_left_frame.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/request_focus_main.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/request_focus_right_frame.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/request_focus_right_frame1.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/www/asset_file.html',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/xwalk.pak',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/xwalk_100_percent.pak',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/jsapi/launch_screen_api.js',
          '<(PRODUCT_DIR)/xwalk_xwview/assets/jsapi/wifidirect_api.js',
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
            'test/android/data/asset_file.html',
            'test/android/data/www/cross_origin.html',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/xwalk_xwview/assets/jsapi',
          'files': [
            'experimental/launch_screen/launch_screen_api.js',
            'experimental/wifidirect/wifidirect_api.js',
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
            '<(PRODUCT_DIR)/xwalk_100_percent.pak',
          ],
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
            ['v8_use_external_startup_data==1', {
              'files': [
                '<(PRODUCT_DIR)/natives_blob.bin',
                '<(PRODUCT_DIR)/snapshot_blob.bin',
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
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/create_window_1.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/create_window_2.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/console_message.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/echo_binary_java.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/echo_java.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/echo_sync_java.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/favicon.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/file_chooser.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/framesEcho.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/fullscreen_enter_exit.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/fullscreen_togged.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/icon.png',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/index.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/js_modal_dialog.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/new_window.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/profile.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/read_video_data.html',
          '<(PRODUCT_DIR)/xwalk_xwview_test/assets/save_video_data.html',
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
            'test/android/data/create_window_1.html',
            'test/android/data/create_window_2.html',
            'test/android/data/console_message.html',
            'test/android/data/echo_binary_java.html',
            'test/android/data/echo_java.html',
            'test/android/data/echo_sync_java.html',
            'test/android/data/favicon.html',
            'test/android/data/file_chooser.html',
            'test/android/data/framesEcho.html',
            'test/android/data/fullscreen_enter_exit.html',
            'test/android/data/fullscreen_togged.html',
            'test/android/data/icon.png',
            'test/android/data/index.html',
            'test/android/data/js_modal_dialog.html',
            'test/android/data/new_window.html',
            'test/android/data/profile.html',
            'test/android/data/read_video_data.html',
            'test/android/data/save_video_data.html',
            'test/android/data/scale_changed.html',
            'test/android/data/window.close.html',
          ],
        },
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_runtime_client_shell_apk',
      'type': 'none',
      'dependencies': [
        'extensions/external_extension_sample.gyp:echo_extension',
        'xwalk_app_runtime_java',
        'xwalk_runtime_client_test_utils_java',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeClientShell',
        'java_in_dir': 'app/android/runtime_client_shell',
        'resource_dir': 'app/android/runtime_client_shell/res',
        'is_test_apk': 1,
        'native_lib_target': 'libxwalkdummy',
        'additional_bundled_libs': [
          '<(PRODUCT_DIR)/lib/libecho_extension.>(android_product_extension)',
        ],
        'additional_input_paths': [
          '<(PRODUCT_DIR)/runtime_client_shell/assets/index.html',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/manifest.json',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/xwalk-extensions/myextension/myextension.js',
          '<(PRODUCT_DIR)/runtime_client_shell/assets/xwalk-extensions/myextension/myextension.json',
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
            'test/android/data/index.html',
            'test/android/data/sampapp-icon-helloworld.png',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_shell/assets/xwalk-extensions/myextension',
          'files': [
            'test/android/data/myextension/myextension.js',
            'test/android/data/myextension/myextension.json',
          ],
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
        'extensions/external_extension_sample.gyp:echo_extension',
        'libxwalkdummy',
        'xwalk_app_runtime_java',
        'xwalk_core_internal_java',
        'xwalk_runtime_client_embedded_shell_apk_pak',
        'xwalk_runtime_client_test_utils_java',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeClientEmbeddedShell',
        'java_in_dir': 'app/android/runtime_client_embedded_shell',
        'resource_dir': 'app/android/runtime_client_embedded_shell/res',
        'native_lib_target': 'libxwalkdummy',
        'additional_bundled_libs': [
          '<(PRODUCT_DIR)/lib/libecho_extension.>(android_product_extension)',
          '<(PRODUCT_DIR)/lib/libxwalkcore.>(android_product_extension)',
        ],
        'additional_input_paths': [
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/index.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/jsapi/launch_screen_api.js',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/jsapi/wifidirect_api.js',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/manifest.json',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/xwalk-extensions/myextension/myextension.js',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/xwalk-extensions/myextension/myextension.json',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/sampapp-icon-helloworld.png',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/xwalk.pak',
          '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/xwalk_100_percent.pak',
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
          'destination': '<(PRODUCT_DIR)/lib',
          'files': ['<(PRODUCT_DIR)/tests/extension/echo_extension/libecho_extension.>(android_product_extension)'],
        },
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets',
          'files': [
            'test/android/data/manifest.json',
            'test/android/data/index.html',
            'test/android/data/sampapp-icon-helloworld.png',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/xwalk-extensions/myextension',
          'files': [
            'test/android/data/myextension/myextension.js',
            'test/android/data/myextension/myextension.json',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/runtime_client_embedded_shell/assets/jsapi',
          'files': [
            'experimental/launch_screen/launch_screen_api.js',
            'experimental/wifidirect/wifidirect_api.js',
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
            '<(PRODUCT_DIR)/xwalk_100_percent.pak',
          ],
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
            ['v8_use_external_startup_data==1', {
              'files': [
                '<(PRODUCT_DIR)/natives_blob.bin',
                '<(PRODUCT_DIR)/snapshot_blob.bin',
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
          '<(PRODUCT_DIR)/runtime_client_test/assets/displayAvailableTest.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/echo.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/echo_binary_java.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/echo_java.html',
          '<(PRODUCT_DIR)/runtime_client_test/assets/echo_sync_java.html',
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
            'extensions/test/data/echo.html',
            'test/android/data/displayAvailableTest.html',
            'test/android/data/echo_binary_java.html',
            'test/android/data/echo_java.html',
            'test/android/data/echo_sync_java.html',
            'test/android/data/native_file_system.html',
            'test/android/data/screen_orientation.html',
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
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/displayAvailableTest.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/echo.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/echo_binary_java.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/echo_java.html',
          '<(PRODUCT_DIR)/runtime_client_embedded_test/assets/echo_sync_java.html',
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
            'extensions/test/data/echo.html',
            'test/android/data/displayAvailableTest.html',
            'test/android/data/echo_binary_java.html',
            'test/android/data/echo_java.html',
            'test/android/data/echo_sync_java.html',
            'test/android/data/native_file_system.html',
            'test/android/data/screen_orientation.html',
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
          '<(PRODUCT_DIR)/sample/assets/builtinzoom.html',
          '<(PRODUCT_DIR)/sample/assets/create_window_1.html',
          '<(PRODUCT_DIR)/sample/assets/create_window_2.html',
          '<(PRODUCT_DIR)/sample/assets/doubletapzoom.html',
          '<(PRODUCT_DIR)/sample/assets/echo_java.html',
          '<(PRODUCT_DIR)/sample/assets/favicon.html',
          '<(PRODUCT_DIR)/sample/assets/icon.png',
          '<(PRODUCT_DIR)/sample/assets/index.html',
          '<(PRODUCT_DIR)/sample/assets/manifest.json',
          '<(PRODUCT_DIR)/sample/assets/new_window.html',
          '<(PRODUCT_DIR)/sample/assets/pause_timers.html',
          '<(PRODUCT_DIR)/sample/assets/xwalk.pak',
          '<(PRODUCT_DIR)/sample/assets/xwalk_100_percent.pak',
        ],
        'conditions': [
          ['icu_use_data_file_flag==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/sample/assets/icudtl.dat',
            ],
          }],
          ['v8_use_external_startup_data==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/natives_blob.bin',
              '<(PRODUCT_DIR)/snapshot_blob.bin',
            ],
          }],
        ],
        'asset_location': '<(PRODUCT_DIR)/sample/assets',
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/sample/assets',
          'files': [
            'runtime/android/sample/assets/builtinzoom.html',
            'runtime/android/sample/assets/create_window_1.html',
            'runtime/android/sample/assets/create_window_2.html',
            'runtime/android/sample/assets/doubletapzoom.html',
            'runtime/android/sample/assets/favicon.html',
            'runtime/android/sample/assets/icon.png',
            'runtime/android/sample/assets/index.html',
            'runtime/android/sample/assets/manifest.json',
            'runtime/android/sample/assets/new_window.html',
            'runtime/android/sample/assets/pause_timers.html',
            'test/android/data/echo_java.html',
            '<(PRODUCT_DIR)/xwalk.pak',
            '<(PRODUCT_DIR)/xwalk_100_percent.pak',
          ],
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
            ['v8_use_external_startup_data==1', {
              'files': [
                '<(PRODUCT_DIR)/natives_blob.bin',
                '<(PRODUCT_DIR)/snapshot_blob.bin',
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
          '<(PRODUCT_DIR)/xwalk_internal_xwview/assets/xwalk_100_percent.pak',
        ],
        'conditions': [
          ['icu_use_data_file_flag==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/xwalk_internal_xwview/assets/icudtl.dat',
            ],
          }],
          ['v8_use_external_startup_data==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/natives_blob.bin',
              '<(PRODUCT_DIR)/snapshot_blob.bin',
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
            '<(PRODUCT_DIR)/xwalk_100_percent.pak',
          ],
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
            ['v8_use_external_startup_data==1', {
              'files': [
                '<(PRODUCT_DIR)/natives_blob.bin',
                '<(PRODUCT_DIR)/snapshot_blob.bin',
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
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/echo_binary_java.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/echo_java.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/echo_sync_java.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/framesEcho.html',
          '<(PRODUCT_DIR)/xwalk_internal_xwview_test/assets/full_screen_video_test.html',
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
            'test/android/data/echo_binary_java.html',
            'test/android/data/echo_java.html',
            'test/android/data/echo_sync_java.html',
            'test/android/data/framesEcho.html',
            'test/android/data/full_screen_video_test.html',
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
    {
      'target_name': 'libxwalkdummy',
      'type': 'shared_library',
      'sources': [
        'runtime/android/dummy_lib/dummy_lib.cc',
      ],
    },
  ],
}
