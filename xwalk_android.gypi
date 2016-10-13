{
  'variables': {
    'reflection_java_dir': '<(PRODUCT_DIR)/gen/xwalk_core_reflection_layer',
    'internal_dir': 'runtime/android/core_internal/src/org/xwalk/core/internal',
  },
  'targets': [
    {
      'target_name': 'libxwalkcore',
      'type': 'shared_library',
      'android_unmangled_name': 1,
      'dependencies': [
        '../components/components.gyp:auto_login_parser',
        '../components/components.gyp:cdm_browser',
        '../components/components.gyp:cdm_renderer',
        '../components/components.gyp:navigation_interception',
        '../components/components.gyp:visitedlink_browser',
        '../components/components.gyp:visitedlink_renderer',
        '../components/components.gyp:web_contents_delegate_android',
        '../skia/skia.gyp:skia',
        '../mojo/mojo_public.gyp:mojo_cpp_bindings',
        'xwalk_core_extensions_native_jni',
        'xwalk_core_jar_jni',
        'xwalk_core_native_jni',
        'xwalk_runtime',
      ],
      'include_dirs': [
        '..',
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
      'target_name': 'xwalk_app_strings',
      'type': 'none',
      'variables': {
          'grd_file': '../xwalk/runtime/android/core/strings/xwalk_app_strings.grd',
       },
      'includes': [
          '../build/java_strings_grd.gypi',
      ],
    },
    {
      'target_name': 'xwalk_core_reflection_layer_java_gen',
      'type': 'none',
      'variables': {
        'script_dir': 'tools/reflection_generator',
        'internal_dir': 'runtime/android/core_internal/src/org/xwalk/core/internal',
        'scripts': [
          '>!@(find <(script_dir) -name "*.py")'
        ],
        'internal_sources': [
          '>!@(find <(internal_dir) -name "*Internal.java")'
        ],
        'reflect_sources': [
          '>!@(find <(internal_dir) -name "Reflect*.java")'
        ],
        'xwalk_app_version_template': 'runtime/android/templates/XWalkAppVersion.template',
        'xwalk_core_version_template': 'runtime/android/templates/XWalkCoreVersion.template',
        'timestamp': '<(reflection_java_dir)/gen.timestamp',
        'extra_reflection_args': [],
      },
      'all_dependent_settings': {
        'variables': {
          'reflection_layer_gen_timestamp': '<(timestamp)',
          'reflection_gen_dir': '<(reflection_java_dir)',
        },
      },
      'conditions': [
        ['verify_xwalk_apk==1', {
          'variables': {
            'extra_reflection_args': ['--verify-xwalk-apk'],
          },
        }],
      ],
      'actions': [
        {
          'action_name': 'generate_reflection',
          'message': 'Creating reflection layer',
          'inputs': [
            '>@(scripts)',
            '>@(internal_sources)',
            '>@(reflect_sources)',
            '<(xwalk_app_version_template)',
            '<(xwalk_core_version_template)',
            'API_VERSION',
            'VERSION',
          ],
          'outputs': [
            '<(timestamp)',
          ],
          'action': [
            'python', '<(script_dir)/reflection_generator.py',
            '--input-dir', '<(internal_dir)',
            '--xwalk-app-version-template-path', '<(xwalk_app_version_template)',
            '--xwalk-core-version-template-path', '<(xwalk_core_version_template)',
            '--bridge-output', '<(reflection_java_dir)/bridge',
            '--wrapper-output', '<(reflection_java_dir)/wrapper',
            '--stamp', '<(timestamp)',
            '--api-version=<(api_version)',
            '--min-api-version=<(min_api_version)',
            '--xwalk-build-version=<(xwalk_version)',
            '<@(extra_reflection_args)',
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
        'additional_input_paths': [ '>(reflection_layer_gen_timestamp)' ],
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
        'xwalk_app_strings',
        'third_party/lzma_sdk/lzma_sdk_android.gyp:lzma_sdk_java',
      ],
      'variables': {
        'java_in_dir': 'runtime/android/core',
        'has_java_resources': 1,
        'R_package': 'org.xwalk.core',
        'R_package_relpath': 'org/xwalk/core',
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
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkAutofillClientAndroid.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkContent.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkContentLifecycleNotifier.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkContentsClientBridge.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkContentsIoThreadClient.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkCookieManagerInternal.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkDevToolsServer.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkHttpAuthHandlerInternal.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkPathHelper.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkPresentationHost.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkSettingsInternal.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkViewDelegate.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkWebContentsDelegate.java',
        'runtime/android/core_internal/src/org/xwalk/core/internal/XWalkWebResourceResponseInternal.java',
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
        'extensions/android/java/src/org/xwalk/core/internal/extensions/XWalkNativeExtensionLoaderAndroid.java',
      ],
      'includes': ['../build/jni_generator.gypi'],
    },
    {
      'target_name': 'xwalk_runtime_lib_lzma_apk',
      'type': 'none',
      'dependencies': [
        'xwalk_runtime_lib_apk'
      ],
      'variables': {
        'apk_name': 'XWalkRuntimeLibLzma',
        'java_in_dir': 'runtime/android/runtime_lib',
        'resource_dir': 'runtime/android/runtime_lib/res',
        'asset_location': '<(PRODUCT_DIR)/xwalk_runtime_lib_lzma/assets',
        'app_manifest_version_name': '<(xwalk_version)',
        'app_manifest_version_code': '<(xwalk_version_code)',
        'is_test_apk': 1,
      },
      'includes': ['../build/java_apk.gypi'],
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
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/jsapi/launch_screen_api.js',
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/jsapi/wifidirect_api.js',
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/xwalk.pak',
          '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/xwalk_100_percent.pak',
        ],
        'conditions': [
          ['icu_use_data_file_flag==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/icudtl.dat',
            ],
          }],
          ['v8_use_external_startup_data==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/natives_blob.bin',
              '<(PRODUCT_DIR)/snapshot_blob.bin',
            ],
          }],
        ],
        'asset_location': '<(PRODUCT_DIR)/xwalk_runtime_lib/assets',
        'app_manifest_version_name': '<(xwalk_version)',
        'app_manifest_version_code': '<(xwalk_version_code)',
      },
      'includes': ['../build/java_apk.gypi'],
      'actions': [
        {
          'action_name': 'runtime_lib_lzma',
          'message': 'Compressing runtime APK assets with LZMA',
          'variables': {
            # We have to use three separate variables because libxwalkcore.so
            # in the <(stripped_libraries_dir) is not registered as an output
            # in the build system, so we cannot use it as an input for the
            # action. Instead, we use the stamp file produced by
            # strip_native_libraries.gypi and pass the library as an input only
            # to the lzma_compress.py script.
            'base_inputs': [
              '<(dex_path)',
              '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/xwalk.pak',
              '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/xwalk_100_percent.pak',
              '<(PRODUCT_DIR)/xwalk_runtime_lib/assets/icudtl.dat',
            ],
            'build_system_inputs': [
              '<@(base_inputs)',
              '<(strip_stamp)',
            ],
            'lzma_compress_inputs': [
              '<@(base_inputs)',
              '<(stripped_libraries_dir)/libxwalkcore.so',
            ],
            'assets_dir': '<(PRODUCT_DIR)/xwalk_runtime_lib_lzma/assets',
          },
          'inputs': [
            '<@(build_system_inputs)',
          ],
          'outputs': [
            '<(assets_dir)/classes.dex',
            '<(assets_dir)/icudtl.dat.lzma',
            '<(assets_dir)/libxwalkcore.so.lzma',
            '<(assets_dir)/xwalk.pak.lzma',
            '<(assets_dir)/xwalk_100_percent.pak.lzma',
          ],
          'action': [
            'python', 'build/android/lzma_compress.py',
            '--dest-path=<(assets_dir)',
            '--sources=<(lzma_compress_inputs)',
          ],
        },
      ],
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
            '<(PRODUCT_DIR)/xwalk_100_percent.pak',
          ],
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
            ['v8_use_external_startup_data==1', {
              'files': [
                '<(PRODUCT_DIR)/natives_blob.bin',
                '<(PRODUCT_DIR)/snapshot_blob.bin',
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
            'experimental/wifidirect/wifidirect_api.js',
          ],
        },
      ],
    },
  ],
}
