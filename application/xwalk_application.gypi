{
'targets': [
    {
      'target_name': 'xwalk_application_lib',
      'type': 'static_library',
      'dependencies': [
        '../base/base.gyp:base',
        '../crypto/crypto.gyp:crypto',
        '../ipc/ipc.gyp:ipc',
        '../sql/sql.gyp:sql',
        '../ui/ui.gyp:ui',
        '../url/url.gyp:url_lib',
        '../third_party/WebKit/public/blink.gyp:blink',
        '../third_party/zlib/google/zip.gyp:zip',
        'xwalk_application_resources',
      ],
      'sources': [
        'browser/application_event_manager.cc',
        'browser/application_event_manager.h',
        'browser/application_event_router.cc',
        'browser/application_event_router.h',
        'browser/application_process_manager.cc',
        'browser/application_process_manager.h',
        'browser/application_protocols.cc',
        'browser/application_protocols.h',
        'browser/application_service.cc',
        'browser/application_service.h',
        'browser/application_service_provider.cc',
        'browser/application_service_provider.h',
        'browser/application_store.cc',
        'browser/application_store.h',
        'browser/application_system.cc',
        'browser/application_system.h',
        'browser/event_observer.cc',
        'browser/event_observer.h',
        'browser/installer/package.h',
        'browser/installer/package.cc',
        'browser/installer/wgt_package.h',
        'browser/installer/wgt_package.cc',
        'browser/installer/xpk_package.cc',
        'browser/installer/xpk_package.h',

        'common/application.cc',
        'common/application.h',
        'common/application_file_util.cc',
        'common/application_file_util.h',
        'common/application_manifest_constants.cc',
        'common/application_manifest_constants.h',
        'common/application_resource.cc',
        'common/application_resource.h',
        'common/constants.cc',
        'common/constants.h',
        'common/db_store.cc',
        'common/db_store.h',
        'common/db_store_constants.cc',
        'common/db_store_constants.h',
        'common/db_store_sqlite_impl.cc',
        'common/db_store_sqlite_impl.h',
        'common/event_names.cc',
        'common/event_names.h',
        'common/id_util.cc',
        'common/id_util.h',
        'common/install_warning.h',
        'common/manifest.cc',
        'common/manifest.h',
        'common/manifest_handler.cc',
        'common/manifest_handler.h',
        'common/manifest_handlers/permissions_handler.cc',
        'common/manifest_handlers/permissions_handler.h',

        'extension/application_event_extension.cc',
        'extension/application_event_extension.h',
        'extension/application_runtime_extension.cc',
        'extension/application_runtime_extension.h',

        'renderer/application_native_module.cc',
        'renderer/application_native_module.h',
      ],
      'conditions': [
        [ 'OS == "linux"', {
          'dependencies': [
            '../build/linux/system.gyp:dbus',
            '../dbus/dbus.gyp:dbus',
            'dbus/xwalk_dbus.gyp:xwalk_dbus',
          ],
          'sources': [
            'browser/application_service_provider_linux.cc',
            'browser/application_service_provider_linux.h',
            'browser/linux/installed_application_object.cc',
            'browser/linux/installed_application_object.h',
            'browser/linux/installed_applications_root.cc',
            'browser/linux/installed_applications_root.h',
          ],
        }],
        [ 'tizen_mobile == 1', {
          'dependencies': [
            '../third_party/libxml/libxml.gyp:libxml',
            'build/system.gyp:tizen',
            'tizen/xwalk_tizen.gypi:xwalk_tizen_lib',
          ],
          'sources': [
            'browser/installer/tizen/packageinfo_constants.cc',
            'browser/installer/tizen/packageinfo_constants.h',
            'browser/installer/tizen/package_installer.cc',
            'browser/installer/tizen/package_installer.h',
          ],
        }],
      ],
      'include_dirs': [
        '../..',
      ],
    },

    {
      'target_name': 'xwalk_application_resources',
      'type': 'none',
      'dependencies': [
        'generate_xwalk_application_resources',
      ],
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/xwalk',
      },
      'includes': [ '../../build/grit_target.gypi' ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_application_resources.pak'
          ],
        },
      ],
    },

    {
      'target_name': 'generate_xwalk_application_resources',
      'type': 'none',
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/xwalk',
      },
      'actions': [
        {
          'action_name': 'xwalk_application_resources',
          'variables': {
            'grit_resource_ids': 'resources/resource_ids',
            'grit_grd_file': 'application_resources.grd',
          },
          'includes': [ '../../build/grit_action.gypi' ],
        },
      ],
    },
    {
      'target_name': 'xwalk_application_tools',
      'type': 'none',
      'defines': ['XWALK_VERSION="<(xwalk_version)"'],
      'conditions': [
        ['OS=="linux"', {
          'dependencies': [
            'application/tools/linux/xwalk_application_tools.gyp:xwalkctl',
          ],
        }],
      ],
    },
  ],
}
