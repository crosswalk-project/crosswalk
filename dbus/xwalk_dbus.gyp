{
  'targets': [
    {
      'target_name': 'xwalk_dbus',
      'type': 'static_library',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../build/linux/system.gyp:dbus',
        '../../dbus/dbus.gyp:dbus',
      ],
      'sources': [
        'dbus_manager.h',
        'object_manager_adaptor.cc',
        'object_manager_adaptor.h',
        'property_exporter.cc',
        'property_exporter.h',
        'xwalk_service_name.cc',
        'xwalk_service_name.h',
      ],
      'conditions': [
        [ 'tizen == 1 or tizen_mobile == 1', {
          'sources': [
            'dbus_manager_tizen.cc',
          ],
        }, { # tizen == 0
          'sources': [
            'dbus_manager.cc',
          ],
        }],
      ]
    },
    {
      'target_name': 'xwalk_dbus_unittests',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:run_all_unittests',
        '../../base/base.gyp:test_support_base',
        '../../build/linux/system.gyp:dbus',
        '../../dbus/dbus.gyp:dbus',
        '../../testing/gtest.gyp:gtest',
        'xwalk_dbus',
      ],
      'sources': [
        'test_client.cc',
        'test_client.h',
        'dbus_manager_unittest.cc',
        'property_exporter_unittest.cc',
      ],
    },
  ],
}
