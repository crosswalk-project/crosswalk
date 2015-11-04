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
        'browser/application_security_policy.cc',
        'browser/application_security_policy.h',
        'browser/application_service.cc',
        'browser/application_service.h',
        'browser/application_system.cc',
        'browser/application_system.h',

        'extension/application_runtime_extension.cc',
        'extension/application_runtime_extension.h',
        'extension/application_widget_extension.cc',
        'extension/application_widget_extension.h',
        'extension/application_widget_storage.cc',
        'extension/application_widget_storage.h',

        'renderer/application_native_module.cc',
        'renderer/application_native_module.h',
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
  ],
}
