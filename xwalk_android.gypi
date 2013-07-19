{
  'targets': [
    {
      'target_name': 'libxwalkcore',
      'type': 'shared_library',
      'android_unmangled_name': 1,
      'dependencies': [
        'xwalk_core_native_jni',
        'xwalk_pak',
        'xwalk_runtime',
      ],
      'include_dirs': [
        '..',
      ],
      'ldflags': [
        '-Wl,--no-fatal-warnings',
      ],
      'sources': [
        'runtime/app/android/xwalk_entry_point.cc',
        'runtime/app/android/xwalk_jni_registrar.cc',
        'runtime/app/android/xwalk_jni_registrar.h',
      ],
    },
    {
      'target_name': 'xwalk_core_java',
      'type': 'none',
      'dependencies': [
        '../content/content.gyp:content_java',
        '../ui/ui.gyp:ui_java',
      ],
      'variables': {
        'java_in_dir': 'runtime/android/java',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'xwalk_core_native_jni',
      'type': 'none',
      'variables': {
        'jni_gen_package': 'xwalk',
      },
      'sources': [
        'runtime/android/java/src/org/xwalk/core/XwViewContent.java',
      ],
      'includes': ['../build/jni_generator.gypi'],
    },
  ],
}
