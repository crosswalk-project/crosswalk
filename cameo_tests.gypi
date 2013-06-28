{
  'targets': [
  {
    'target_name': 'cameo_test_common',
    'type': 'static_library',
    'dependencies': [
      'cameo_runtime',
      'cameo_resources',
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
      'test/base/cameo_test_suite.cc',
      'test/base/cameo_test_suite.h',
      'test/base/cameo_test_utils.cc',
      'test/base/cameo_test_utils.h',
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
  },  # cameo_test_common target

  {
    'target_name': 'cameo_unittest',
    'type': 'executable',
    'dependencies': [
      'cameo_test_common',
      '../testing/gtest.gyp:gtest',
    ],
    'include_dirs' : [
      '..',
    ],
    'sources': [
      'runtime/common/cameo_content_client_unittest.cc',
      'test/base/run_all_unittests.cc',
    ],
    'conditions': [
      ['OS=="win" and win_use_allocator_shim==1', {
        'dependencies': [
          '../base/allocator/allocator.gyp:allocator',
        ],
      }],
    ],
  }, # cameo_unit_tests target

  {
    'target_name': 'cameo_browsertest',
    'type': 'executable',
    'dependencies': [
      'cameo',
      'cameo_test_common',
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
      'runtime/browser/cameo_form_input_browsertest.cc',
      'runtime/browser/cameo_runtime_browsertest.cc',
      'runtime/browser/cameo_switches_browsertest.cc',
      'runtime/browser/devtools/cameo_devtools_browsertest.cc',
      'runtime/browser/geolocation/cameo_geolocation_browsertest.cc',
      'test/base/cameo_test_launcher.cc',
      'test/base/in_process_browser_test.cc',
      'test/base/in_process_browser_test.h',
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
  }], # cameo_browser_tests target
}
