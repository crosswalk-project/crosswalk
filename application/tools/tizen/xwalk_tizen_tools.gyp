{
  'targets': [
    {
      'target_name': 'xwalk_backend',
      'type': 'executable',
      'product_name': 'xwalk_backend',
      'dependencies': [
        '../../../application/common/xwalk_application_common.gypi:xwalk_application_common_lib',
        '../../../build/system.gyp:gio',
        '../../../build/system.gyp:tizen',
        '../../../build/system.gyp:tizen_tzplatform_config',
      ],
      'include_dirs': [
        '../../../..',
      ],
      'sources': [
        'xwalk_backend.cc',
        'xwalk_package_installer.cc',
        'xwalk_package_installer.h',
        'xwalk_packageinfo_constants.cc',
        'xwalk_packageinfo_constants.h',
        'xwalk_platform_installer.cc',
        'xwalk_platform_installer.h',
        'xwalk_rds_delta_parser.cc',
        'xwalk_rds_delta_parser.h',
        'xwalk_tizen_user.cc',
        'xwalk_tizen_user.h',
        # TODO(t.iwanek) fix me - this duplicates compilation of those files
        '../../../runtime/common/xwalk_paths.cc',
        '../../../runtime/common/xwalk_paths.h',
        '../../../runtime/common/xwalk_switches.cc',
        '../../../runtime/common/xwalk_switches.h',
        '../../../runtime/common/xwalk_system_locale.cc',
        '../../../runtime/common/xwalk_system_locale.h',
      ],
    },
    {
      'target_name': 'xwalk_backend_lib',
      'type': 'shared_library',
      'product_name': 'xwalk_backend_lib',
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
        '../../../runtime/common/xwalk_switches.cc',
        '../../../runtime/common/xwalk_switches.h',
        '../../../runtime/common/xwalk_system_locale.cc',
        '../../../runtime/common/xwalk_system_locale.h',
      ],
    },
  ],
}
