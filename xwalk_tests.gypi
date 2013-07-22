{
  'targets': [
  {
    'target_name': 'xwalk_test_common',
    'type': 'static_library',
    'dependencies': [
      'xwalk_runtime',
      'xwalk_resources',
      '../base/base.gyp:test_support_base',
      '../base/base.gyp:base_prefs_test_support',
      '../content/content.gyp:content_app',
      '../content/content.gyp:test_support_content',
      '../net/net.gyp:net',
      '../net/net.gyp:net_test_support',
      '../skia/skia.gyp:skia',
      '../testing/gmock.gyp:gmock',
      '../testing/gtest.gyp:gtest',
      '../third_party/zlib/zlib.gyp:zlib',
    ],
    'export_dependent_settings': [
      '../base/base.gyp:test_support_base',
    ],
    'include_dirs': [
      '..',
    ],
    'sources': [
      'test/base/xwalk_test_suite.cc',
      'test/base/xwalk_test_suite.h',
      'test/base/xwalk_test_utils.cc',
      'test/base/xwalk_test_utils.h',
    ],
    'conditions': [
      ['toolkit_uses_gtk == 1', {
        'dependencies' : [
          '../build/linux/system.gyp:gtk',
          '../build/linux/system.gyp:ssl',
        ],
      }],
      ['OS=="win"', {
        'include_dirs': [
          '<DEPTH>/third_party/wtl/include',
        ],
      }],
      ['OS=="win" and win_use_allocator_shim==1', {
        'dependencies': [
          '../base/allocator/allocator.gyp:allocator',
        ],
      }],
    ],
  },  # xwalk_test_common target

  {
    'target_name': 'xwalk_unittest',
    'type': 'executable',
    'dependencies': [
      'xwalk_test_common',
      '../testing/gtest.gyp:gtest',
    ],
    'include_dirs' : [
      '..',
    ],
    'includes': [
      'extensions/extensions_unittests.gypi',
    ],
    'sources': [
      'runtime/common/xwalk_content_client_unittest.cc',
      'test/base/run_all_unittests.cc',
    ],
    'conditions': [
      ['OS=="win" and win_use_allocator_shim==1', {
        'dependencies': [
          '../base/allocator/allocator.gyp:allocator',
        ],
      }],
    ],
  }, # xwalk_unit_tests target

  {
    'target_name': 'xwalk_browsertest',
    'type': 'executable',
    'dependencies': [
      'xwalk',
      'xwalk_test_common',
      '../skia/skia.gyp:skia',
      '../testing/gtest.gyp:gtest',
      '../testing/gmock.gyp:gmock',
    ],
    'include_dirs': [
      '..',
    ],
    'defines': [
      'HAS_OUT_OF_PROC_TEST_RUNNER',
    ],
    'sources': [
      'runtime/browser/xwalk_download_browsertest.cc',
      'runtime/browser/xwalk_form_input_browsertest.cc',
      'runtime/browser/xwalk_runtime_browsertest.cc',
      'runtime/browser/xwalk_switches_browsertest.cc',
      'runtime/browser/devtools/xwalk_devtools_browsertest.cc',
      'runtime/browser/geolocation/xwalk_geolocation_browsertest.cc',
      'test/base/in_process_browser_test.cc',
      'test/base/in_process_browser_test.h',
      'test/base/xwalk_test_launcher.cc',
    ],
    'includes': [
      'extensions/extensions_browsertests.gypi',
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
