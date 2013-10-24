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
      ],
      'sources': [
        'browser/application_daemon.cc',
        'browser/application_daemon.h',
        'browser/application_store.cc',
        'browser/application_store.h',
        'browser/application_process_manager.cc',
        'browser/application_process_manager.h',
        'browser/application_protocols.cc',
        'browser/application_protocols.h',
        'browser/application_service.cc',
        'browser/application_service.h',
        'browser/application_system.cc',
        'browser/application_system.h',
        'browser/installer/xpk_extractor.cc',
        'browser/installer/xpk_extractor.h',
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
        'common/id_util.cc',
        'common/id_util.h',
        'common/install_warning.h',
        'common/manifest.cc',
        'common/manifest.h',
        'common/db_store.cc',
        'common/db_store.h',
        'common/db_store_sqlite_impl.cc',
        'common/db_store_sqlite_impl.h',
      ],
      'conditions': [
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
        [ 'OS=="linux"', {
          'dependencies': [
            'build/system.gyp:dbus_daemon',
          ],
          'sources': [
            'browser/application_daemon_linux.cc',
          ],
          'sources!': [
            'browser/application_daemon.cc',
          ],
        }],
      ],
      'include_dirs': [
        '../..',
      ],
    }],
}
