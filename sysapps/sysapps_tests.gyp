{
  'targets': [
    {
      'target_name': 'xwalk_sysapps_unittest',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:run_all_unittests',
        '../../content/content_shell_and_tests.gyp:test_support_content',
        '../../testing/gtest.gyp:gtest',
        '../extensions/extensions.gyp:xwalk_extensions',
        'sysapps.gyp:sysapps',
      ],
      'sources': [
        'common/binding_object_store_unittest.cc',
        'common/event_target_unittest.cc',
        'common/sysapps_manager_unittest.cc',
        'device_capabilities/av_codecs_provider_unittest.cc',
        'device_capabilities/cpu_info_provider_unittest.cc',
        'device_capabilities/display_info_provider_unittest.cc',
        'device_capabilities/memory_info_provider_unittest.cc',
        'device_capabilities/storage_info_provider_unittest.cc',
      ],
      'conditions': [
        ['OS=="linux"', {
          'dependencies': [
            '../../dbus/dbus.gyp:dbus',
          ],
        }],
        ['os_posix==1 and OS!="mac" and linux_use_tcmalloc==1', {
          'dependencies': [
            '../../base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['use_aura==1', {
          'dependencies': [
          '../../ui/views/views.gyp:views',
          ],
        }],
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
        'device_capabilities/device_capabilities_api_browsertest.cc',
        'raw_socket/raw_socket_api_browsertest.cc'
      ],
    },
  ],
}
