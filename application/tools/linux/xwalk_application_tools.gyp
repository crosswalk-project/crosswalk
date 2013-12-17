{
  'targets': [
    {
      'target_name': 'gio',
      'type': 'none',
      'variables': {
        'glib_packages': 'glib-2.0 gio-2.0',
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
        '..',
      ],
      'sources': [
        'xwalk_tizen_user.h',
        'xwalk_tizen_user.c',
        'xwalkctl_main.c',
      ],
    },
    {
      'target_name': 'xwalk_launcher',
      'type': 'executable',
      'product_name': 'xwalk-launcher',
      'sources': [
        'xwalk_tizen_user.h',
        'xwalk_tizen_user.c',
        'xwalk_launcher_main.c',
      ],
      'conditions' : [
        ['OS=="linux"', {
          'dependencies': [
            'gio',
          ],
        }],
        ['tizen_mobile==1', {
          'dependencies': [
            'gio',
            '../../../build/system.gyp:tizen_appcore_common'
          ],
          'sources': [
            'xwalk_launcher_tizen.c',
            'xwalk_launcher_tizen.h',
          ],
        }],
      ],
      'include_dirs': [
        '..',
      ],
    },
  ],
}
