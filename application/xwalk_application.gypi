{
'targets': [
    {
      'target_name': 'xwalk_application_lib',
      'type': 'static_library',
      'dependencies': [
        '../base/base.gyp:base',
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '../ipc/ipc.gyp:ipc',
        '../ui/ui.gyp:ui',
        '../webkit/support/webkit_support.gyp:webkit_support',
        '../third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
      ],
      'sources': [
        'browser/application_process_manager.cc',
        'browser/application_process_manager.h',
        'browser/application_service.cc',
        'browser/application_service.h',
        'browser/application_system.cc',
        'browser/application_system.h',

        'common/application.cc',
        'common/application.h',
        'common/application_file_util.cc',
        'common/application_file_util.h',
        'common/application_manifest_constants.cc',
        'common/application_manifest_constants.h',
        'common/constants.cc',
        'common/constants.h',
        'common/id_util.cc',
        'common/id_util.h',
        'common/install_warning.h',
        'common/manifest.cc',
        'common/manifest.h',
      ],
      'include_dirs': [
        '../..',
      ],
    }],
}
