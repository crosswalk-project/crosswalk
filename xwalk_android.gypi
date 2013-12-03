{
  'targets': [
    {
      'target_name': 'libxwalkcore',
      'type': 'shared_library',
      'android_unmangled_name': 1,
      'dependencies': [
        '../components/components.gyp:auto_login_parser',
        '../components/components.gyp:navigation_interception',
        '../components/components.gyp:visitedlink_browser',
        '../components/components.gyp:visitedlink_renderer',
        '../components/components.gyp:web_contents_delegate_android',
        '../skia/skia.gyp:skia',
        'xwalk_core_extensions_native_jni',
        'xwalk_core_jar_jni',
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
        '../components/components.gyp:navigation_interception_java',
        '../components/components.gyp:web_contents_delegate_android_java',
        '../content/content.gyp:content_java',
        '../ui/ui.gyp:ui_java',
        'xwalk_core_extensions_java',
      ],
      'variables': {
        'java_in_dir': 'runtime/android/java',
        'has_java_resources': 1,
        'R_package': 'org.xwalk.core',
        'R_package_relpath': 'org/xwalk/core',
        'java_strings_grd': 'android_xwalk_strings.grd',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'xwalk_core_embedded',
      'type': 'none',
      'dependencies': [
        'xwalk_core_java',
      ],
      'actions': [
        {
          'action_name': 'xwalk_core_embedded',
          'variables': {
            'dex_input_paths': [
              '<(PRODUCT_DIR)/lib.java/base_java.dex.jar',
              '<(PRODUCT_DIR)/lib.java/content_java.dex.jar',
              '<(PRODUCT_DIR)/lib.java/eyesfree_java.dex.jar',
              '<(PRODUCT_DIR)/lib.java/guava_javalib.dex.jar',
              '<(PRODUCT_DIR)/lib.java/jsr_305_javalib.dex.jar',
              '<(PRODUCT_DIR)/lib.java/media_java.dex.jar',
              '<(PRODUCT_DIR)/lib.java/navigation_interception_java.dex.jar',
              '<(PRODUCT_DIR)/lib.java/net_java.dex.jar',
              '<(PRODUCT_DIR)/lib.java/ui_java.dex.jar',
              '<(PRODUCT_DIR)/lib.java/web_contents_delegate_android_java.dex.jar',
              '<(PRODUCT_DIR)/lib.java/xwalk_core_extensions_java.dex.jar',
              '<(PRODUCT_DIR)/lib.java/xwalk_core_java.dex.jar' ],
            'output_path': '<(PRODUCT_DIR)/lib.java/xwalk_core_embedded.dex.jar',
          },
          'includes': [ '../build/android/dex_action.gypi' ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_jar_jni',
      'type': 'none',
      'variables': {
        'jni_gen_package': 'xwalk',
        'input_java_class': 'java/io/InputStream.class',
      },
      'includes': [ '../build/jar_file_jni_generator.gypi' ],
    },
    {
      'target_name': 'xwalk_core_native_jni',
      'type': 'none',
      'variables': {
        'jni_gen_package': 'xwalk',
      },
      'sources': [
        'runtime/android/java/src/org/xwalk/core/AndroidProtocolHandler.java',
        'runtime/android/java/src/org/xwalk/core/XWalkContentsClientBridge.java',
        'runtime/android/java/src/org/xwalk/core/XWalkContent.java',
        'runtime/android/java/src/org/xwalk/core/XWalkDevToolsServer.java',
        'runtime/android/java/src/org/xwalk/core/XWalkSettings.java',
        'runtime/android/java/src/org/xwalk/core/XWalkWebContentsDelegate.java',
      ],
      'includes': ['../build/jni_generator.gypi'],
    },
    {
      'target_name': 'xwalk_core_extensions_java',
      'type': 'none',
      'dependencies': [
        '../content/content.gyp:content_java',
      ],
      'variables': {
        'java_in_dir': 'extensions/android/java',
        'has_java_resources': 0,
        'R_package': 'org.xwalk.core.extensions',
        'R_package_relpath': 'org/xwalk/core/extensions',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'xwalk_core_extensions_native_jni',
      'type': 'none',
      'variables': {
        'jni_gen_package': 'xwalk',
      },
      'sources': [
        'extensions/android/java/src/org/xwalk/core/extensions/XWalkExtensionAndroid.java',
      ],
      'includes': ['../build/jni_generator.gypi'],
    },
    {
      'target_name': 'xwalk_runtime_lib_apk',
      'type': 'none',
      'dependencies': [
        'libxwalkcore',
        'xwalk_core_extensions_java',
        # Runtime code is also built by this target.
        'xwalk_core_java',
        'xwalk_runtime_lib_apk_extension',
        'xwalk_runtime_lib_apk_pak',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeLib',
        'java_in_dir': 'runtime/android/runtimelib',
        'resource_dir': 'runtime/android/runtimelib/res',
        'native_lib_target': 'libxwalkcore',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/jsapi/device_capabilities_api.js',
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/jsapi/presentation_api.js',
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/xwalk.pak',
        ],
        'asset_location': '<(ant_build_out)/xwalk_runtime_lib/assets',
        'app_manifest_version_name': '<(xwalk_version)',
        'app_manifest_version_code': '<(xwalk_version_code)',
      },
      'includes': ['../build/java_apk.gypi'],
    },
    {
      'target_name': 'xwalk_runtime_lib_apk_pak',
      'type': 'none',
      'dependencies': [
        'xwalk_pak',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_runtime_lib/assets',
          'files': [
            '<(PRODUCT_DIR)/xwalk.pak',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_runtime_lib_apk_extension',
      'type': 'none',
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/jsapi',
          'files': [
            'experimental/presentation/presentation_api.js',
            'runtime/android/java/src/org/xwalk/runtime/extension/api/device_capabilities/device_capabilities_api.js',
          ],
        },
      ],
    },
  ],
}
