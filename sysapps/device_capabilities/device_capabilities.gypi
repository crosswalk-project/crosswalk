{
  'dependencies': [
    '../third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
    'build/system.gyp:tizen_sysapps',
  ],
  'sources': [
    'device_capabilities_cpu.h',
    'device_capabilities_cpu_posix.cc',
    'device_capabilities_display.h',
    'device_capabilities_display.cc',
    'device_capabilities_extension.cc',
    'device_capabilities_extension.h',
    'device_capabilities_instance.cc',
    'device_capabilities_instance.h',
    'device_capabilities_memory.h',
    'device_capabilities_memory.cc',
    'device_capabilities_storage.h',
    'device_capabilities_storage_tizen.cc',
    'device_capabilities_utils.h',
  ],
}
