{
'targets': [
    {
      'target_name': 'xwalk_application_common_lib',
      'type': 'static_library',
      'dependencies': [
        '../../../base/base.gyp:base',
        '../../../base/base.gyp:base_i18n',
        '../../../content/content.gyp:content_common',
        '../../../crypto/crypto.gyp:crypto',
        '../../../net/net.gyp:net',
        '../../../sql/sql.gyp:sql',
        '../../../url/url.gyp:url_lib',
        '../../../third_party/libxml/libxml.gyp:libxml',
        '../../../third_party/zlib/google/zip.gyp:zip',
      ],
      'sources': [
        'application_storage.cc',
        'application_storage.h',
        'application_storage_impl.cc',
        'application_storage_impl.h',

        'application_data.cc',
        'application_data.h',
        'application_file_util.cc',
        'application_file_util.h',
        'application_manifest_constants.cc',
        'application_manifest_constants.h',
        'application_resource.cc',
        'application_resource.h',
        'application_storage_constants.cc',
        'application_storage_constants.h',
        'constants.cc',
        'constants.h',
        'id_util.cc',
        'id_util.h',
        'install_warning.h',
        'manifest.cc',
        'manifest.h',
        'manifest_handler.cc',
        'manifest_handler.h',
        'manifest_handlers/csp_handler.cc',
        'manifest_handlers/csp_handler.h',
        'manifest_handlers/permissions_handler.cc',
        'manifest_handlers/permissions_handler.h',
        'manifest_handlers/warp_handler.cc',
        'manifest_handlers/warp_handler.h',
        'manifest_handlers/widget_handler.cc',
        'manifest_handlers/widget_handler.h',
        'permission_policy_manager.cc',
        'permission_policy_manager.h',
        'permission_types.h',

        'installer/package.h',
        'installer/package.cc',
        'installer/package_installer.cc',
        'installer/package_installer.h',
        'installer/wgt_package.h',
        'installer/wgt_package.cc',
        'installer/xpk_package.cc',
        'installer/xpk_package.h',
      ],
      'conditions': [
        ['tizen==1', {
          'dependencies': [
            '../../build/system.gyp:tizen',
            '../../tizen/xwalk_tizen.gypi:xwalk_tizen_lib',
            '../../../third_party/re2/re2.gyp:re2',
          ],
          'sources': [
            'manifest_handlers/navigation_handler.cc',
            'manifest_handlers/navigation_handler.h',
            'manifest_handlers/tizen_application_handler.cc',
            'manifest_handlers/tizen_application_handler.h',
            'manifest_handlers/tizen_metadata_handler.cc',
            'manifest_handlers/tizen_metadata_handler.h',
            'manifest_handlers/tizen_setting_handler.cc',
            'manifest_handlers/tizen_setting_handler.h',
            'manifest_handlers/tizen_splash_screen_handler.cc',
            'manifest_handlers/tizen_splash_screen_handler.h',
            'installer/package_installer_tizen.cc',
            'installer/package_installer_tizen.h',
            'installer/tizen/packageinfo_constants.cc',
            'installer/tizen/packageinfo_constants.h',
          ],
        }],
      ],
      'include_dirs': [
        '..',
        '../..',
        '../../..',
      ],
    },
  ],
}
