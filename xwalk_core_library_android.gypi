# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'core_internal_empty_embedder_apk_name': 'XWalkCoreInternalEmptyEmbedder',
    'core_empty_embedder_apk_name': 'XWalkCoreEmptyEmbedder',
  },
  'targets': [
    {
      'target_name': 'xwalk_core_library_documentation',
      'type': 'none',
      'dependencies': [
        'xwalk_core_java'
      ],
      'variables': {
        'api_files': [
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/JavascriptInterface.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkActivity.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkDialogManager.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkInitializer.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkFileChooser.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkUpdater.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/extension/XWalkExtensionContextClient.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/extension/XWalkExternalExtension.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/ClientCertRequest.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkCookieManager.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkDownloadListener.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkExternalExtensionManager.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkFindListener.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkGetBitmapCallback.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkHttpAuthHandler.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkJavascriptResult.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkNativeExtensionLoader.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkNavigationHistory.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkNavigationItem.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkPreferences.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkResourceClient.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkSettings.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkUIClient.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkView.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkWebResourceRequest.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkWebResourceResponse.java',
        ],
        'javadoc_wrapper': '<(DEPTH)/xwalk/runtime/android/core/tools/generate_javadoc.py',
        'output_dir': '<(PRODUCT_DIR)/xwalk_core_library_docs',
        'stamp': '<(INTERMEDIATE_DIR)/stamp',
        'xwalk_core_jar': '<(PRODUCT_DIR)/lib.java/xwalk_core_java.jar',
      },
      'actions': [
        {
          'action_name': 'javadoc_xwalk_core_library',
          'message': 'Creating documentation for XWalk Core Library',
          'inputs': [
            '<(javadoc_wrapper)',
            '<(xwalk_core_jar)',
          ],
          'outputs': [
            '<(stamp)',
          ],
          'action': [
            'python', '<(javadoc_wrapper)',
            '--classpath', '<(android_sdk_jar)',
            '--classpath', '<(xwalk_core_jar)',
            '--java-files', '<(api_files)',
            '--output-dir', '<(output_dir)',
            '--stamp', '<(stamp)',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_internal_empty_embedder_apk',
      'type': 'none',
      'dependencies': [
        'libxwalkdummy',
        'xwalk_core_internal_java',
      ],
      'variables': {
        'apk_name': '<(core_internal_empty_embedder_apk_name)',
        'java_in_dir': 'runtime/android/core_internal_empty',
        'native_lib_target': 'libxwalkdummy',
        'additional_bundled_libs': [
          '<(PRODUCT_DIR)/lib/libxwalkcore.>(android_product_extension)',
        ],
      },
      'includes': [ '../build/java_apk.gypi' ],
      'all_dependent_settings': {
        'variables': {
          'input_jars_paths': ['<(javac_jar_path)'],
        },
      },
    },
    {
      'target_name': 'xwalk_core_empty_embedder_apk',
      'type': 'none',
      'dependencies': [
        'xwalk_core_library',
      ],
      'variables': {
        'apk_name': '<(core_empty_embedder_apk_name)',
        'java_in_dir': 'runtime/android/core_internal_empty',
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'xwalk_core_library_java_app_part',
      'type': 'none',
      'dependencies': [
        'xwalk_core_java',
      ],
      'variables': {
        'jar_name': '<(_target_name).jar',
        'jar_final_path': '<(PRODUCT_DIR)/lib.java/<(jar_name)',
      },
      'actions': [
        {
          'action_name': 'jars_<(_target_name)',
          'message': 'Creating <(_target_name) jar',
          'inputs': [
            'build/android/merge_jars.py',
            '>@(input_jars_paths)',
          ],
          'outputs': [
            '<(jar_final_path)',
          ],
          'action': [
            'python', 'build/android/merge_jars.py',
            '--output-jar=<(jar_final_path)',
            '>(input_jars_paths)',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_library_java_library_part',
      'type': 'none',
      'dependencies': [
        'xwalk_core_internal_empty_embedder_apk',
      ],
      'variables': {
        'jar_name': '<(_target_name).jar',
        'jar_final_path': '<(PRODUCT_DIR)/lib.java/<(jar_name)',
      },
      'actions': [
        {
          'action_name': 'jars_<(_target_name)',
          'message': 'Creating <(_target_name) jar',
          'inputs': [
            'build/android/merge_jars.py',
            '>@(input_jars_paths)',
          ],
          'outputs': [
            '<(jar_final_path)',
          ],
          'action': [
            'python', 'build/android/merge_jars.py',
            '--output-jar=<(jar_final_path)',
            '>(input_jars_paths)',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_library_java',
      'type': 'none',
      'dependencies': [
        'xwalk_core_library_java_app_part',
        'xwalk_core_library_java_library_part',
      ],
      'variables': {
        'jar_name': '<(_target_name).jar',
        'jar_final_path': '<(PRODUCT_DIR)/lib.java/<(jar_name)',
      },
      'actions': [
        {
          'action_name': 'jars_<(_target_name)',
          'message': 'Creating <(_target_name) jar',
          'inputs': [
            'build/android/merge_jars.py',
            '>@(input_jars_paths)',
          ],
          'outputs': [
            '<(jar_final_path)',
          ],
          'action': [
            'python', 'build/android/merge_jars.py',
            '--output-jar=<(jar_final_path)',
            # This argument is important for this final JAR we are creating, as
            # it validates that we are filtering out the right JARs when doing
            # the merge.
            '--validate-skipped-jars-list',
            '>(input_jars_paths)',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_library_strip_native_libs',
      'type': 'none',
      'dependencies': [
        'libxwalkcore',
        'libxwalkdummy',
      ],
      'variables': {
        'intermediate_dir': '<(PRODUCT_DIR)/<(_target_name)',
        'ordered_libraries_file': '<(intermediate_dir)/native_libraries.json',
      },
      'direct_dependent_settings': {
        'variables': {
          'stripped_native_libraries': [
            '<(intermediate_dir)/libxwalkcore.so',
            '<(intermediate_dir)/libxwalkdummy.so',
          ],
        },
      },
      'actions': [
        {
          'variables': {
            'input_libraries': [
              '<(SHARED_LIB_DIR)/libxwalkcore.so',
              '<(SHARED_LIB_DIR)/libxwalkdummy.so',
            ],
          },
          'includes': ['../build/android/write_ordered_libraries.gypi'],
        },
        {
          'action_name': 'strip_native_libraries',
          'variables': {
            'stripped_libraries_dir': '<(intermediate_dir)',
            'stamp': '<(intermediate_dir)/stamp',
            'input_paths': [
              '<(SHARED_LIB_DIR)/libxwalkcore.so',
              '<(SHARED_LIB_DIR)/libxwalkdummy.so',
            ],
          },
          'includes': ['../build/android/strip_native_libraries.gypi'],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_library',
      'type': 'none',
      'dependencies': [
        # The dependency list includes the targets that generate the files
        # listed below in the action's variables.
        '../components/components.gyp:web_contents_delegate_android_java',
        '../content/content.gyp:content_java',
        '../content/content.gyp:content_strings_grd',
        '../ui/android/ui_android.gyp:ui_java',
        '../ui/android/ui_android.gyp:ui_strings_grd',
        'xwalk_app_strings',
        'xwalk_core_internal_java',
        'xwalk_core_java',
        'xwalk_core_library_java',
        'xwalk_core_library_strip_native_libs',
        'xwalk_core_strings',
        'xwalk_pak',
      ],
      'actions': [
        {
          'action_name': 'generate_xwalk_core_library',
          'message': 'Generating XWalk Core Library',
          'variables': {
            'binary_files': [
              '<(PRODUCT_DIR)/xwalk.pak',
              '<(PRODUCT_DIR)/xwalk_100_percent.pak',
            ],
            'js_bindings': [
              '<(DEPTH)/xwalk/experimental/launch_screen/launch_screen_api.js',
              '<(DEPTH)/xwalk/experimental/wifidirect/wifidirect_api.js',
            ],
            'main_jar': '<(PRODUCT_DIR)/lib.java/xwalk_core_library_java.jar',
            'resource_strings': [
              '<(PRODUCT_DIR)/res.java/content_strings_grd.zip',
              '<(PRODUCT_DIR)/res.java/ui_strings_grd.zip',
              '<(PRODUCT_DIR)/res.java/xwalk_app_strings.zip',
              '<(PRODUCT_DIR)/res.java/xwalk_core_strings.zip',
            ],
            'resource_zip_sources': [
              '<(DEPTH)/components/web_contents_delegate_android/android/java/res',
              '<(DEPTH)/content/public/android/java/res',
              '<(DEPTH)/ui/android/java/res',
              '<(DEPTH)/xwalk/runtime/android/core/res',
              '<(DEPTH)/xwalk/runtime/android/core_internal/res',
            ],
            'resource_zips': [
              # Order matters here: each zip must correspond to the entry in
              # |resource_zip_sources| in the same position.
              '<(PRODUCT_DIR)/res.java/web_contents_delegate_android_java.zip',
              '<(PRODUCT_DIR)/res.java/content_java.zip',
              '<(PRODUCT_DIR)/res.java/ui_java.zip',
              '<(PRODUCT_DIR)/res.java/xwalk_core_java.zip',
              '<(PRODUCT_DIR)/res.java/xwalk_core_internal_java.zip',
            ],
            'stamp': '<(SHARED_INTERMEDIATE_DIR)/<(_target_name)/generate.stamp',
            'template_dir': '<(DEPTH)/xwalk/build/android/xwalkcore_library_template',
            'conditions': [
              ['icu_use_data_file_flag==1', {
                'binary_files': [
                  '<(PRODUCT_DIR)/icudtl.dat',
                ],
              }],
              ['v8_use_external_startup_data==1', {
                'binary_files': [
                  '<(PRODUCT_DIR)/natives_blob.bin',
                  '<(PRODUCT_DIR)/snapshot_blob.bin',
                ],
              }],
            ],
          },
          'inputs': [
            '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
          ],
          'outputs': [
            '<(stamp)',
          ],
          'action': [
            'python', '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
            '--abi', '<(android_app_abi)',
            '--native-libraries', '>(stripped_native_libraries)',
            '--binary-files', '<(binary_files)',
            '--js-bindings', '<(js_bindings)',
            '--main-jar', '<(main_jar)',
            '--output-dir', '<(PRODUCT_DIR)/xwalk_core_library',
            '--resource-strings', '<(resource_strings)',
            '--resource-zip-sources', '<(resource_zip_sources)',
            '--resource-zips', '<(resource_zips)',
            '--stamp', '<(stamp)',
            '--template-dir', '<(template_dir)',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_shared_library',
      'type': 'none',
      'dependencies': [
        # The dependency list includes the targets that generate the files
        # listed below in the action's variables.
        '../components/components.gyp:web_contents_delegate_android_java',
        '../content/content.gyp:content_java',
        '../content/content.gyp:content_strings_grd',
        '../ui/android/ui_android.gyp:ui_java',
        '../ui/android/ui_android.gyp:ui_strings_grd',
        'xwalk_app_strings',
        'xwalk_core_internal_java',
        'xwalk_core_java',
        'xwalk_core_library_java_app_part',
        'xwalk_core_strings',
      ],
      'actions': [
        {
          'action_name': 'generate_xwalk_shared_library',
          'message': 'Generating XWalk Shared Library',
          'variables': {
            'js_bindings': [
              '<(DEPTH)/xwalk/experimental/launch_screen/launch_screen_api.js',
              '<(DEPTH)/xwalk/experimental/wifidirect/wifidirect_api.js',
            ],
            'main_jar': '<(PRODUCT_DIR)/lib.java/xwalk_core_library_java_app_part.jar',
            'resource_strings': [
              '<(PRODUCT_DIR)/res.java/content_strings_grd.zip',
              '<(PRODUCT_DIR)/res.java/ui_strings_grd.zip',
              '<(PRODUCT_DIR)/res.java/xwalk_app_strings.zip',
              '<(PRODUCT_DIR)/res.java/xwalk_core_strings.zip',
            ],
            'resource_zip_sources': [
              '<(DEPTH)/components/web_contents_delegate_android/android/java/res',
              '<(DEPTH)/content/public/android/java/res',
              '<(DEPTH)/ui/android/java/res',
              '<(DEPTH)/xwalk/runtime/android/core/res',
              '<(DEPTH)/xwalk/runtime/android/core_internal/res',
            ],
            'resource_zips': [
              # Order matters here: each zip must correspond to the entry in
              # |resource_zip_sources| in the same position.
              '<(PRODUCT_DIR)/res.java/web_contents_delegate_android_java.zip',
              '<(PRODUCT_DIR)/res.java/content_java.zip',
              '<(PRODUCT_DIR)/res.java/ui_java.zip',
              '<(PRODUCT_DIR)/res.java/xwalk_core_java.zip',
              '<(PRODUCT_DIR)/res.java/xwalk_core_internal_java.zip',
            ],
            'stamp': '<(SHARED_INTERMEDIATE_DIR)/<(_target_name)/generate.stamp',
            'template_dir': '<(DEPTH)/xwalk/build/android/xwalk_shared_library_template',
          },
          'inputs': [
            '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
          ],
          'outputs': [
            '<(stamp)',
          ],
          'action': [
            'python', '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
            '--js-bindings', '<(js_bindings)',
            '--main-jar', '<(main_jar)',
            '--output-dir', '<(PRODUCT_DIR)/xwalk_shared_library',
            '--resource-strings', '<(resource_strings)',
            '--resource-zip-sources', '<(resource_zip_sources)',
            '--resource-zips', '<(resource_zips)',
            '--stamp', '<(stamp)',
            '--template-dir', '<(template_dir)',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_library_pom_gen',
      'type': 'none',
      'variables': {
        'pom_input': '<(DEPTH)/xwalk/runtime/android/maven/xwalk_core_library.pom.xml.in',
        'pom_output': '<(PRODUCT_DIR)/xwalk_core_library.pom.xml',
        'artifact_id': '<(xwalk_core_library_artifact_id)',
        'artifact_version': '<(xwalk_version)',
      },
      'includes': ['build/android/maven_pom.gypi'],
    },
    {
      'target_name': 'xwalk_shared_library_pom_gen',
      'type': 'none',
      'variables': {
        'pom_input': '<(DEPTH)/xwalk/runtime/android/maven/xwalk_shared_library.pom.xml.in',
        'pom_output': '<(PRODUCT_DIR)/xwalk_shared_library.pom.xml',
        'artifact_id': '<(xwalk_shared_library_artifact_id)',
        'artifact_version': '<(xwalk_version)',
      },
      'includes': ['build/android/maven_pom.gypi'],
    },
    {
      'target_name': 'xwalk_core_library_aar',
      'type': 'none',
      'dependencies': [
        'xwalk_core_library',
        'xwalk_core_empty_embedder_apk',
        'xwalk_core_library_pom_gen',
      ],
      'actions': [
        {
          'action_name': 'generate_xwalk_core_library_aar',
          'message': 'Generating AAR of XWalk Core Library',
          'inputs': [
            '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library_aar.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_core_library_aar_intermediate/always_run',
          ],
          'action': [
            'python', '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library_aar.py',
            '--aar-path', '<(PRODUCT_DIR)/xwalk_core_library.aar',
            '--android-manifest', '<(DEPTH)/xwalk/build/android/xwalkcore_library_template/AndroidManifest.xml',
            '--classes-jar', '<(PRODUCT_DIR)/lib.java/xwalk_core_library_java.jar',
            '--jni-abi', '<(android_app_abi)',
            '--jni-dir', '<(PRODUCT_DIR)/xwalk_core_library/libs/<(android_app_abi)',
            '--res-dir', '<(PRODUCT_DIR)/xwalk_core_library/res',
            '--r-txt', '<(PRODUCT_DIR)/xwalk_core_empty_embedder_apk/gen/R.txt',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_shared_library_aar',
      'type': 'none',
      'dependencies': [
        'xwalk_core_empty_embedder_apk',
        'xwalk_shared_library',
        'xwalk_shared_library_pom_gen',
      ],
      'actions': [
        {
          'action_name': 'generate_xwalk_shared_library_aar',
          'message': 'Generating AAR of XWalk Shared Library',
          'inputs': [
            '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library_aar.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_shared_library_aar_intermediate/always_run',
          ],
          'action': [
            'python', '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library_aar.py',
            '--aar-path', '<(PRODUCT_DIR)/xwalk_shared_library.aar',
            '--android-manifest', '<(DEPTH)/xwalk/build/android/xwalk_shared_library_template/AndroidManifest.xml',
            '--classes-jar', '<(PRODUCT_DIR)/lib.java/xwalk_core_library_java_app_part.jar',
            '--res-dir', '<(PRODUCT_DIR)/xwalk_shared_library/res',
            '--r-txt', '<(PRODUCT_DIR)/xwalk_core_empty_embedder_apk/gen/R.txt',
          ],
        },
      ],
    },
  ],
}
