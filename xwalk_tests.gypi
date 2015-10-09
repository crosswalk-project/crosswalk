{
  'targets': [
    {
      'target_name': 'xwalk_all_tests',
      'type': 'none',
      'dependencies': [
        'xwalk_browsertest',
        'xwalk_unittest',
        'extensions/extensions_tests.gyp:xwalk_extensions_browsertest',
        'extensions/extensions_tests.gyp:xwalk_extensions_unittest',
        'sysapps/sysapps_tests.gyp:xwalk_sysapps_browsertest',
        'sysapps/sysapps_tests.gyp:xwalk_sysapps_unittest',
      ],
      'conditions': [
        ['OS=="win"', {
          'dependencies': [
            'extensions/extensions_tests.gyp:xwalk_dotnet_extensions_browsertest',
            'extensions/extensions_tests.gyp:xwalk_dotnet_extensions_unittest',
          ],
        }],
      ],
    },
    {
      'target_name': 'xwalk_unittest',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../content/content.gyp:content_common',
        '../content/content_shell_and_tests.gyp:test_support_content',
        '../testing/gtest.gyp:gtest',
        '../ui/base/ui_base.gyp:ui_base',
        'test/base/base.gyp:xwalk_test_base',
        'xwalk_application_lib',
        'xwalk_runtime',
      ],
      'sources': [
        'application/common/package/package_unittest.cc',
        'application/common/application_unittest.cc',
        'application/common/application_file_util_unittest.cc',
        'application/common/id_util_unittest.cc',
        'application/common/manifest_handlers/csp_handler_unittest.cc',
        'application/common/manifest_handlers/permissions_handler_unittest.cc',
        'application/common/manifest_handlers/unittest_util.cc',
        'application/common/manifest_handlers/unittest_util.h',
        'application/common/manifest_handlers/warp_handler_unittest.cc',
        'application/common/manifest_handlers/widget_handler_unittest.cc',
        'application/common/manifest_handler_unittest.cc',
        'application/common/manifest_unittest.cc',
        'runtime/common/xwalk_content_client_unittest.cc',
        'runtime/common/xwalk_runtime_features_unittest.cc',
      ],
      'conditions': [
        ['toolkit_views == 1', {
          'sources': [
            'runtime/browser/ui/top_view_layout_views_unittest.cc',
          ],
          'dependencies': [
            '../skia/skia.gyp:skia',
          ],
        }],
        ['tizen==1', {
          'sources': [
            'application/common/manifest_handlers/tizen_appwidget_handler_unittest.cc',
            'application/common/manifest_handlers/tizen_metadata_handler_unittest.cc',
            'application/common/manifest_handlers/tizen_navigation_handler_unittest.cc',
          ],
        }],
      ],
    },
    {
      'target_name': 'xwalk_browsertest',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../content/content.gyp:content_browser',
        '../content/content.gyp:content_common',
        '../content/content_shell_and_tests.gyp:test_support_content',
        '../net/net.gyp:net',
        '../skia/skia.gyp:skia',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../third_party/libxml/libxml.gyp:libxml',
        '../ui/base/ui_base.gyp:ui_base',
        'test/base/base.gyp:xwalk_test_base',
        'xwalk_application_lib',
        'xwalk_resources',
        'xwalk_runtime',
      ],
      'includes': [
        '../build/filename_rules.gypi',
      ],
      'defines': [
        'HAS_OUT_OF_PROC_TEST_RUNNER',
      ],
      'sources': [
        'application/test/application_browsertest.cc',
        'application/test/application_browsertest.h',
        'application/test/application_test.cc',
        'application/test/application_testapi.cc',
        'application/test/application_testapi.h',
        'application/test/application_testapi_test.cc',
        'experimental/native_file_system/native_file_system_api_browsertest.cc',
        'runtime/browser/devtools/xwalk_devtools_browsertest.cc',
        'runtime/browser/ui/taskbar_util_browsertest_win.cc',
        'runtime/browser/xwalk_download_browsertest.cc',
        'runtime/browser/xwalk_form_input_browsertest.cc',
        'runtime/browser/xwalk_runtime_browsertest.cc',
        'runtime/browser/xwalk_switches_browsertest.cc',
      ],
    }
  ],
}
