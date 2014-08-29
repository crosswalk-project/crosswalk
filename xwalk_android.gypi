{
  'variables': {
    'reflection_java_dir': '<(PRODUCT_DIR)/gen/xwalk_core_reflection_layer',
  },
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
      'target_name': 'xwalk_core_strings',
      'type': 'none',
      'variables': {
          'grd_file': '../xwalk/runtime/android/core_internal/strings/android_xwalk_strings.grd',
       },
       'includes': [
          '../build/java_strings_grd.gypi',
        ],
    },
    {
      'target_name': 'xwalk_core_reflection_layer_java_gen',
      'type': 'none',
      'variables': {
        'timestamp': '<(reflection_java_dir)/gen.timestamp',
        'internal_src': 'runtime/android/core_internal/src/org/xwalk/core/internal',
        'internal_java_sources': [
          '>!@(find <(internal_src) -name "*.java")'
        ],
      },
      'all_dependent_settings': {
        'variables': {
          'reflection_layer_gen_timestamp': '<(timestamp)',
          'reflection_gen_dir': '<(reflection_java_dir)',
        },
      },
      'actions': [
        {
          'action_name': 'generate_reflection',
          'message': 'Creating reflection layer',
          'inputs': [
            'tools/reflection_generator/bridge_generator.py',
            'tools/reflection_generator/code_generator.py',
            'tools/reflection_generator/interface_generator.py',
            'tools/reflection_generator/java_class_component.py',
            'tools/reflection_generator/java_class.py',
            'tools/reflection_generator/java_method.py',
            'tools/reflection_generator/reflection_generator.py',
            'tools/reflection_generator/wrapper_generator.py',
            '>@(internal_java_sources)',
          ],
          'outputs': [
            '<(timestamp)',
          ],
          'action': [
            'python', 'tools/reflection_generator/reflection_generator.py',
            '--input_dir', '<(internal_src)',
            '--bridge_output', '<(reflection_java_dir)/bridge',
            '--wrap_output', '<(reflection_java_dir)/wrapper',
            '--helper_class', 'runtime/android/core_internal/src/org/xwalk/core/internal/ReflectionHelper.java',
            '--stamp', '<(timestamp)',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_internal_java',
      'type': 'none',
      'dependencies': [
        '../components/components.gyp:navigation_interception_java',
        '../components/components.gyp:web_contents_delegate_android_java',
        '../content/content.gyp:content_java',
        '../ui/android/ui_android.gyp:ui_java',
        'xwalk_core_extensions_java',
        'xwalk_core_strings',
        'xwalk_core_reflection_layer_java_gen',
      ],
      'variables': {
        'java_in_dir': 'runtime/android/core_internal',
        'has_java_resources': 1,
        'R_package': 'org.xwalk.core.internal',
        'R_package_relpath': 'org/xwalk/core/internal',
        'generated_src_dirs': [
          '<(reflection_java_dir)/bridge',
        ],
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'xwalk_core_java',
      'type': 'none',
      'dependencies': [
        'xwalk_core_reflection_layer_java_gen',
      ],
      'variables': {
        'java_in_dir': 'runtime/android/core',
        'additional_input_paths': [ '>(reflection_layer_gen_timestamp)' ],
        'generated_src_dirs': [
          '<(reflection_java_dir)/wrapper',
        ],
      },
      'includes': ['../build/java.gypi']
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
        'runtime/android/core_internal/src/org/xwalk/core/internal/AndroidProtocolHandler.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/InterceptedRequestData.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkContent.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkContentsClientBridge.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkContentsIoThreadClient.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkCookieManager.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkDevToolsServer.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkHttpAuthHandler.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkPathHelper.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkSettings.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkViewDelegate.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkWebContentsDelegate.java',
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
        'R_package': 'org.xwalk.core.internal.extensions',
        'R_package_relpath': 'org/xwalk/core/internal/extensions',
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
        'extensions/android/java/src/org/xwalk/core/internal/extensions/XWalkExtensionAndroid.java',
      ],
      'includes': ['../build/jni_generator.gypi'],
    },
    {
      'target_name': 'xwalk_runtime_lib_apk',
      'type': 'none',
      'dependencies': [
        'libxwalkcore',
        'xwalk_core_internal_java',
        'xwalk_runtime_lib_apk_extension',
        'xwalk_runtime_lib_apk_pak',
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeLib',
        'java_in_dir': 'runtime/android/runtime_lib',
        'resource_dir': 'runtime/android/runtime_lib/res',
        'native_lib_target': 'libxwalkcore',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/jsapi/contacts_api.js',
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/jsapi/device_capabilities_api.js',
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/jsapi/launch_screen_api.js',
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/jsapi/messaging_api.js',
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/jsapi/presentation_api.js',
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/xwalk.pak',
        ],
        'conditions': [
          ['icu_use_data_file_flag==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/icudtl.dat',
            ],
          }],
        ],
        'asset_location': '<(PRODUCT_DIR)/xwalk_runtime_lib/assets',
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
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
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
            'experimental/launch_screen/launch_screen_api.js',
            'experimental/presentation/presentation_api.js',
            'runtime/android/core_internal/src/org/xwalk/core/internal/extension/api/contacts/contacts_api.js',
            'runtime/android/core_internal/src/org/xwalk/core/internal/extension/api/device_capabilities/device_capabilities_api.js',
            'runtime/android/core_internal/src/org/xwalk/core/internal/extension/api/messaging/messaging_api.js',
          ],
        },
      ],
    },
  ],
}
