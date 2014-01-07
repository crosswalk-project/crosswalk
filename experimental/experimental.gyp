{
  'targets': [
    {
      'target_name': 'xwalk_dialog',
      'type': 'static_library',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../content/content.gyp:content',
        '../../skia/skia.gyp:skia',
        '../../ui/ui.gyp:ui',
        '../extensions/extensions.gyp:xwalk_extensions',
        'experimental_resources.gyp:xwalk_experimental_resources',
      ],
      'variables': {
        'jsapi_component': 'experimental',
      },
      'includes': [
        '../xwalk_jsapi.gypi',
      ],
      'sources': [
        'dialog/dialog.idl',
        'dialog/dialog_extension.cc',
        'dialog/dialog_extension.h',
      ],
    },
  ],
}
