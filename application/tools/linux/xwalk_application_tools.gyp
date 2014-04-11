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
      ],
      'include_dirs': [
        '../../../..',
      ],
      'sources': [
        'dbus_connection.h',
        'dbus_connection.cc',
        'xwalk_tizen_user.h',
        'xwalk_tizen_user.cc',
        'xwalkctl_main.cc',
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
        '../../../extensions/extensions.gyp:xwalk_extensions',
      ],
      'sources': [
        'dbus_connection.h',
        'dbus_connection.cc',
        'xwalk_extension_process_launcher.h',
        'xwalk_extension_process_launcher.cc',
        'xwalk_tizen_user.h',
        'xwalk_tizen_user.cc',
        'xwalk_launcher_main.cc',
      ],
      'conditions' : [
        ['OS=="linux"', {
          'dependencies': [
            'gio',
          ],
        }],
        ['tizen==1 or tizen_mobile==1', {
          'dependencies': [
            'gio',
            '../../../build/system.gyp:tizen_appcore_common'
          ],
          'sources': [
            'xwalk_launcher_tizen.cc',
            'xwalk_launcher_tizen.h',
          ],
        }],
      ],
    },
  ],
}
