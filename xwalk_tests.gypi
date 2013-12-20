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
    ],
    'conditions': [
      ['OS=="linux"', {
        'dependencies': [
          'dbus/xwalk_dbus.gyp:xwalk_dbus_unittests',
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
      '../ui/ui.gyp:ui',
      'test/base/base.gyp:xwalk_test_base',
      'xwalk_application_lib',
      'xwalk_runtime',
    ],
    'includes': [
      'sysapps/sysapps_unittests.gypi',
    ],
    'sources': [
      'application/browser/application_event_router_unittest.cc',
      'application/browser/application_storage_impl_unittest.cc',
      'application/browser/installer/package_unittest.cc',
      'application/common/application_unittest.cc',
      'application/common/application_file_util_unittest.cc',
      'application/common/id_util_unittest.cc',
      'application/common/manifest_handlers/csp_handler_unittest.cc',
      'application/common/manifest_handlers/main_document_handler_unittest.cc',
      'application/common/manifest_handlers/permissions_handler_unittest.cc',
      'application/common/manifest_handler_unittest.cc',
      'application/common/manifest_unittest.cc',
      'runtime/common/xwalk_content_client_unittest.cc',
      'runtime/common/xwalk_runtime_features_unittest.cc',
    ],
    'conditions': [
      ['OS=="win" and win_use_allocator_shim==1', {
        'dependencies': [
          '../base/allocator/allocator.gyp:allocator',
        ],
      }],
      ['toolkit_views == 1', {
        'sources': [
          'runtime/browser/ui/top_view_layout_views_unittest.cc',
        ],
        'dependencies': [
          '../skia/skia.gyp:skia',
        ],
      }],
    ],
  }, # xwalk_unit_tests target

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
      '../ui/ui.gyp:ui',
      'test/base/base.gyp:xwalk_test_base',
      'xwalk_application_lib',
      'xwalk_runtime',
    ],
    'defines': [
      'HAS_OUT_OF_PROC_TEST_RUNNER',
    ],
    'sources': [
      'application/test/application_apitest.cc',
      'application/test/application_apitest.h',
      'application/test/application_browsertest.cc',
      'application/test/application_browsertest.h',
      'application/test/application_event_test.cc',
      'application/test/application_eventapi_test.cc',
      'application/test/application_main_document_browsertest.cc',
      'application/test/application_testapi.cc',
      'application/test/application_testapi.h',
      'application/test/application_testapi_test.cc',
      'runtime/browser/xwalk_download_browsertest.cc',
      'runtime/browser/xwalk_form_input_browsertest.cc',
      'runtime/browser/xwalk_runtime_browsertest.cc',
      'runtime/browser/xwalk_switches_browsertest.cc',
      'runtime/browser/devtools/xwalk_devtools_browsertest.cc',
      'runtime/browser/geolocation/xwalk_geolocation_browsertest.cc',
    ],
    'includes': [
      'sysapps/sysapps_browsertests.gypi',
      'xwalk_jsapi.gypi',
    ],
    'conditions': [
      ['OS=="win" and win_use_allocator_shim==1', {
        'dependencies': [
          '../base/allocator/allocator.gyp:allocator',
        ],
      }],
      ['OS=="win"', {
        'sources': [
          'runtime/browser/ui/taskbar_util_browsertest.cc',
        ],
      }],  # OS=="win"
    ],
  }], # xwalk_browser_tests target
}
