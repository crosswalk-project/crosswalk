{
  'targets': [
    {
      'target_name': 'xwalk_sysapps_unittest',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:run_all_unittests',
        '../../content/content.gyp:content_common',
        '../../testing/gtest.gyp:gtest',
        '../../ui/ui.gyp:ui',
        '../extensions/extensions.gyp:xwalk_extensions',
        'sysapps.gyp:sysapps',
      ],
      'sources': [
        'common/binding_object_store_unittest.cc',
        'common/event_target_unittest.cc',
        'common/sysapps_manager_unittest.cc',
        'device_capabilities_new/av_codecs_provider_unittest.cc',
        'device_capabilities_new/cpu_info_provider_unittest.cc',
        'device_capabilities_new/memory_info_provider_unittest.cc',
      ],
    },
    {
      'target_name': 'xwalk_sysapps_browsertest',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../content/content_shell_and_tests.gyp:test_support_content',
        '../../net/net.gyp:net',
        '../../skia/skia.gyp:skia',
        '../../testing/gtest.gyp:gtest',
        '../extensions/extensions.gyp:xwalk_extensions',
        '../test/base/base.gyp:xwalk_test_base',
        '../xwalk.gyp:xwalk_runtime',
        'sysapps.gyp:sysapps',
      ],
      'defines': [
        'HAS_OUT_OF_PROC_TEST_RUNNER',
      ],
      'sources': [
        'common/common_api_browsertest.cc',
        'common/common_api_browsertest.h',
        'device_capabilities_new/device_capabilities_api_browsertest.cc',
        'raw_socket/raw_socket_api_browsertest.cc'
      ],
    },
  ],
}
