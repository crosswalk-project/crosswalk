{
'targets': [
    {
      'target_name': 'xwalk_application_lib',
      'type': 'static_library',
      'dependencies': [
        '../base/base.gyp:base',
        '../content/content.gyp:content_browser',
        '../crypto/crypto.gyp:crypto',
        '../ipc/ipc.gyp:ipc',
        '../ui/base/ui_base.gyp:ui_base',
        '../url/url.gyp:url_lib',
        '../third_party/WebKit/public/blink.gyp:blink',
        '../third_party/zlib/google/zip.gyp:zip',
        'xwalk_application_resources',
        'application/common/xwalk_application_common.gypi:xwalk_application_common_lib',
        '../third_party/libxml/libxml.gyp:libxml',
      ],
      'sources': [
        'browser/application.cc',
        'browser/application.h',
        'browser/application_protocols.cc',
        'browser/application_protocols.h',
        'browser/application_service.cc',
        'browser/application_service.h',
        'browser/application_system.cc',
        'browser/application_system.h',

        'common/security_policy.cc',
        'common/security_policy.h',

        'extension/application_runtime_extension.cc',
        'extension/application_runtime_extension.h',
        'extension/application_widget_extension.cc',
        'extension/application_widget_extension.h',
        'extension/application_widget_storage.cc',
        'extension/application_widget_storage.h',

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
            'browser/application_system_linux.cc',
            'browser/application_system_linux.h',
            'browser/linux/running_application_object.cc',
            'browser/linux/running_application_object.h',
            'browser/linux/running_applications_manager.cc',
            'browser/linux/running_applications_manager.h',
          ],
        }],
        ['tizen==1', {
          'dependencies': [
            'build/system.gyp:tizen',
            'tizen/xwalk_tizen.gypi:xwalk_tizen_lib',
            '../third_party/re2/re2.gyp:re2',
          ],
          'sources': [
            'browser/application_tizen.cc',
            'browser/application_tizen.h',
          ],
        }],
      ],
      'include_dirs': [
        '..',
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
            'application/tools/linux/xwalk_application_tools.gyp:xwalk_launcher',
          ],
        }],
        ['tizen == 1', {
          'dependencies': [
            'application/tools/tizen/xwalk_tizen_helper.gyp:xwalk-pkg-helper',
          ],
        }],
      ],
    },
  ],
}
