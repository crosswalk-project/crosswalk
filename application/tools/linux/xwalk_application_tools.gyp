{
  'targets': [
    {
      'target_name': 'xwalkctl',
      'type': 'executable',
      'product_name': 'xwalkctl',
      'dependencies': [
        '../../../application/common/xwalk_application_common.gypi:xwalk_application_common_lib',
        '../../../build/system.gyp:gio',
        '../../../../build/linux/system.gyp:dbus',
        '../../../../dbus/dbus.gyp:dbus',
      ],
      'include_dirs': [
        '../../../..',
      ],
      'sources': [
        'dbus_connection.cc',
        'dbus_connection.h',
        'xwalkctl_main.cc',
        # TODO(t.iwanek) fix me - this duplicates compilation of those files
        '../../../runtime/common/xwalk_paths.cc',
        '../../../runtime/common/xwalk_paths.h',
        '../../../runtime/common/xwalk_system_locale.cc',
        '../../../runtime/common/xwalk_system_locale.h',
      ],
      'conditions' : [
        ['tizen==1', {
          'dependencies': [
            '../../../build/system.gyp:tizen',
          ],
          'sources': [
            'xwalk_tizen_user.cc',
            'xwalk_tizen_user.h',
          ],
        }],
      ],
    },
    {
      'target_name': 'xwalk_launcher',
      'type': 'executable',
      'product_name': 'xwalk-launcher',
      'include_dirs': [
        '../../../..',
      ],
      'dependencies': [
        '../../../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../../../extensions/extensions.gyp:xwalk_extensions',
        '../../../application/common/xwalk_application_common.gypi:xwalk_application_common_lib',
      ],
      'sources': [
        'dbus_connection.cc',
        'dbus_connection.h',
        'xwalk_extension_process_launcher.cc',
        'xwalk_extension_process_launcher.h',
        'xwalk_launcher_main.cc',
      ],
      'conditions' : [
        ['OS=="linux"', {
          'dependencies': [
            '../../../build/system.gyp:gio',
          ],
        }],
        ['tizen==1', {
          'dependencies': [
            '../../../build/system.gyp:gio',
            '../../../build/system.gyp:tizen',
            '../../../build/system.gyp:tizen_appcore_common'
          ],
          'sources': [
            'xwalk_launcher_tizen.cc',
            'xwalk_launcher_tizen.h',
            'xwalk_tizen_user.cc',
            'xwalk_tizen_user.h',
          ],
        }],
      ],
    },
  ],
}
