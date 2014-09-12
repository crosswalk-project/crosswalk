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
        '../../../build/system.gyp:tizen',
      ],
      'include_dirs': [
        '../../../..',
      ],
      'sources': [
        'xwalkctl_main.cc',
        'xwalk_package_installer.cc',
        'xwalk_package_installer.h',
        'xwalk_packageinfo_constants.cc',
        'xwalk_packageinfo_constants.h',
        'xwalk_tizen_user.cc',
        'xwalk_tizen_user.h',
        # TODO(t.iwanek) fix me - this duplicates compilation of those files
        '../../../runtime/common/xwalk_paths.cc',
        '../../../runtime/common/xwalk_paths.h',
        '../../../runtime/common/xwalk_system_locale.cc',
        '../../../runtime/common/xwalk_system_locale.h',
      ],
    },  
    {
      'target_name': 'xwalk-pkg-helper',
      'type': 'executable',
      'product_name': 'xwalk-pkg-helper',
      'dependencies': [
        '../../../build/system.gyp:tizen',
        '../../../build/system.gyp:gio',
        '../../../../base/base.gyp:base',
        '../../common/xwalk_application_common.gypi:xwalk_application_common_lib',
      ],
      'include_dirs': [
        '../../../..',
      ],
      'cflags': [
        '<!@(pkg-config --cflags libtzplatform-config)',
      ],
      'link_settings': {
        'libraries': [
          '<!@(pkg-config --libs libtzplatform-config)',
        ],
      },
      'sources': [
        'xwalk_package_helper.cc',
        'xwalk_package_installer_helper.cc',
        'xwalk_package_installer_helper.h',
      ],
    },
    {
      'target_name': 'xwalk-backendlib',
      'type': 'shared_library',
      'product_name': 'xwalk-backendlib',
      'dependencies': [
        '../../../build/system.gyp:tizen',
        '../../../../base/base.gyp:base',
        '../../common/xwalk_application_common.gypi:xwalk_application_common_lib',
      ],
      'include_dirs': [
        '../../../..',
      ],
      'sources': [
        'xwalk_backendlib.cc',
        'xwalk_backend_plugin.cc',
        'xwalk_backend_plugin.h',
        # TODO(t.iwanek) fix me - this duplicates compilation of those files
        '../../../runtime/common/xwalk_paths.cc',
        '../../../runtime/common/xwalk_paths.h',
        '../../../runtime/common/xwalk_system_locale.cc',
        '../../../runtime/common/xwalk_system_locale.h',
      ],
    },
  ],
}
