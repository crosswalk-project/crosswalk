{
  'targets': [
    {
      'target_name': 'gio',
      'type': 'none',
      'variables': {
        'glib_packages': 'glib-2.0 gio-unix-2.0',
      },
      'direct_dependent_settings': {
        'cflags': [
          '<!@(pkg-config --cflags <(glib_packages))',
        ],
      },
      'link_settings': {
        'ldflags': [
          '<!@(pkg-config --libs-only-L --libs-only-other <(glib_packages))',
        ],
        'libraries': [
          '<!@(pkg-config --libs-only-l <(glib_packages))',
        ],
      },
    },
    {
      'target_name': 'xwalkctl',
      'type': 'executable',
      'product_name': 'xwalkctl',
      'dependencies': [
        'gio',
        '../../../application/common/xwalk_application_common.gypi:xwalk_application_common_lib',
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
            'gio',
          ],
        }],
        ['tizen==1', {
          'dependencies': [
            'gio',
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
