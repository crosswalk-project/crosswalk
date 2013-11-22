{
  'targets': [
  {
    'target_name': 'xwalk_tizen_lib',
    'type': 'static_library',
    'dependencies': [
      '../../skia/skia.gyp:skia',
      '../../ui/ui.gyp:ui',
      '../build/system.gyp:tizen_appcore',
    ],
    'include_dirs': [
      '../..',
    ],
    'sources': [
      'appcore_context.cc',
      'appcore_context.h',
      'mobile/ui/tizen_plug_message_writer.cc',
      'mobile/ui/tizen_plug_message_writer.h',
      'mobile/ui/tizen_system_indicator.cc',
      'mobile/ui/tizen_system_indicator.h',
      'mobile/ui/tizen_system_indicator_watcher.cc',
      'mobile/ui/tizen_system_indicator_watcher.h',
    ],
  }],
}
