{
  'targets': [
    {
      'target_name': 'libxwalkcore',
      'type': 'shared_library',
      'android_unmangled_name': 1,
      'dependencies': [
        'xwalk_runtime',
      ],
      'ldflags': [
        '-Wl,--no-fatal-warnings',
      ],
      'sources': [
        'runtime/app/android/xwalk_entry_point.cc',
      ],
    }
  ],
}
