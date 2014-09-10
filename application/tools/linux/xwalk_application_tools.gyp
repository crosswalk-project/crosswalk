{
  'targets': [  
    {
      'target_name': 'xwalk_launcher',
      'type': 'executable',
      'product_name': 'xwalk-launcher',
      'include_dirs': [
        '../../../..',
      ],
      'dependencies': [
        '../../../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../../../extensions/extensions.gyp:xwalk_extensions',
        '../../../application/common/xwalk_application_common.gypi:xwalk_application_common_lib',
      ],
      'sources': [
        'dbus_connection.cc',
        'dbus_connection.h',
        'xwalk_extension_process_launcher.cc',
        'xwalk_extension_process_launcher.h',
        'xwalk_launcher_main.cc',
      ],
      'conditions' : [
        ['OS=="linux"', {
          'dependencies': [
            '../../../build/system.gyp:gio',
          ],
        }],
        ['tizen==1', {
          'dependencies': [
            '../../../build/system.gyp:gio',
            '../../../build/system.gyp:tizen',
            '../../../build/system.gyp:tizen_appcore_common'
          ],
          'sources': [
            'xwalk_launcher_tizen.cc',
            'xwalk_launcher_tizen.h',
            '../tizen/xwalk_tizen_user.cc',
            '../tizen/xwalk_tizen_user.h',
          ],
        }],
      ],
    },
  ],
}
