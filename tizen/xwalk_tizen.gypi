{
  'targets': [
  {
    'target_name': 'xwalk_tizen_lib',
    'type': 'static_library',
    'dependencies': [
      '../build/system.gyp:tizen_appcore',
    ],
    'include_dirs': [
      '../..',
    ],
    'sources': [
      'appcore_context.cc',
      'appcore_context.h',
    ],
  }],
}
