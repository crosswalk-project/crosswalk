{
  'targets': [
  {
    'target_name': 'xwalk_tizen_lib',
    'type': 'static_library',
    'dependencies': [
      '../../skia/skia.gyp:skia',
      '../../ui/ui.gyp:ui',
      '../build/system.gyp:tizen_appcore',
      '../build/system.gyp:tizen_vibration',
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
      'mobile/ui/tizen_system_indicator_widget.cc',
      'mobile/ui/tizen_system_indicator_widget.h',
      'mobile/ui/widget_container_view.cc',
      'mobile/ui/widget_container_view.h',
      'mobile/sensor/sensor_provider.cc',
      'mobile/sensor/sensor_provider.h',
      'mobile/sensor/tizen_data_fetcher_shared_memory.cc',
      'mobile/sensor/tizen_data_fetcher_shared_memory.h',
      'mobile/sensor/tizen_platform_sensor.cc',
      'mobile/sensor/tizen_platform_sensor.h',
      'browser/vibration/vibration_provider_tizen.cc',
      'browser/vibration/vibration_provider_tizen.h',
    ],
  }],
}
