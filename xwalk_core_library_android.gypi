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
        'xwalk_core_library_java_app_part'
      ],
      'variables': {
        'api_files': [
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/JavascriptInterface.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkActivity.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkDialogManager.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkInitializer.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkUpdater.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/ClientCertRequest.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkCookieManager.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkDownloadListener.java',
          '>(reflection_gen_dir)/wrapper/org/xwalk/core/XWalkExtension.java',
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
        'xwalk_core_jar': '<(PRODUCT_DIR)/lib.java/xwalk_core_library_java_app_part.jar',
        'docs': '<(PRODUCT_DIR)/xwalk_core_library_docs',
      },
      'actions': [
        {
          'action_name': 'javadoc_xwalk_core_library',
          'message': 'Creating documentation for XWalk Core Library',
          'inputs': [
            '>(xwalk_core_jar)',
          ],
          'outputs': [
            '<(docs)',
          ],
          'action': [
            'javadoc',
            '-quiet',
            '-XDignore.symbol.file',
            '-d', '<(docs)',
            '-classpath', '<(android_sdk)/android.jar',
            '-bootclasspath', '<(xwalk_core_jar)',
            '<@(api_files)',
          ],
        },
      ],
    },
    {
      'target_name': 'pack_xwalk_core_library',
      'type': 'none',
      'dependencies': [
        'xwalk_core_library',
        'xwalk_core_library_src',
      ],
      'actions': [
        {
          'action_name': 'pack_xwalk_core_library',
          'message': 'Packaging XwalkCore Library Project.',
          'inputs': [
            '<(DEPTH)/xwalk/tools/tar.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/pack_xwalk_core_library_intermediate/always_run',
          ],
          'action': [
            'python', 'tools/tar.py',
            '<(PRODUCT_DIR)/xwalk_core_library'
          ],
        },
        {
          'action_name': 'pack_xwalk_core_library_src',
          'message': 'Packaging XwalkCore Library Project Source.',
          'inputs': [
            '<(DEPTH)/xwalk/tools/tar.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/pack_xwalk_core_library_src_intermediate/always_run',
          ],
          'action': [
            'python', 'tools/tar.py',
            '<(PRODUCT_DIR)/xwalk_core_library_src'
          ],
        },
      ],
    },
    {
      'target_name': 'pack_xwalk_shared_library',
      'type': 'none',
      'dependencies': [
        'xwalk_shared_library',
      ],
      'actions': [
        {
          'action_name': 'pack_xwalk_shared_library',
          'message': 'Packaging XwalkCore Shared Library Project.',
          'inputs': [
            '<(DEPTH)/xwalk/tools/tar.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/pack_xwalk_shared_library_intermediate/always_run',
          ],
          'action': [
            'python', 'tools/tar.py',
            '<(PRODUCT_DIR)/xwalk_shared_library'
          ],
        },
      ],
    },
    {
      'target_name': 'generate_resource_maps',
      'type': 'none',
      'dependencies': [
        'xwalk_core_internal_java',
      ],
      'variables': {
        'resource_map_dir': '<(PRODUCT_DIR)/resource_map',
        'timestamp': '<(resource_map_dir)/gen.timestamp',
      },
      'all_dependent_settings': {
        'variables': {
          'resource_map_gen_timestamp': '<(timestamp)',
        },
      },
      'actions': [
        {
          'action_name': 'generate_resource_maps',
          'message': 'Generating Resource Maps.',
          'inputs': [
            'build/android/generate_resource_map.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/generate_resource_maps_intermediate/always_run',
            '<(timestamp)',
          ],
          'action': [
            'python', 'build/android/generate_resource_map.py',
            '--gen-dir', '<(PRODUCT_DIR)/gen',
            '--resource-map-dir', '<(resource_map_dir)',
            '--stamp', '<(timestamp)',
          ],
        },
      ]
    },
    {
      'target_name': 'xwalk_core_internal_empty_embedder_apk',
      'type': 'none',
      'dependencies': [
        'generate_resource_maps',
        'libxwalkdummy',
      ],
      'variables': {
        'apk_name': '<(core_internal_empty_embedder_apk_name)',
        'java_in_dir': 'runtime/android/core_internal_empty',
        'is_test_apk': 1,
        'additional_input_paths': [ '>(resource_map_gen_timestamp)' ],
        'generated_src_dirs': [
           '<(PRODUCT_DIR)/resource_map',
        ],
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
        'generate_resource_maps',
      ],
      'variables': {
        'apk_name': '<(core_empty_embedder_apk_name)',
        'java_in_dir': 'runtime/android/core_internal_empty',
        'additional_input_paths': [ '>(resource_map_gen_timestamp)' ],
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
            '--jars=>(input_jars_paths)',
            '--output-jar=<(jar_final_path)',
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
            '--jars=>(input_jars_paths)',
            '--output-jar=<(jar_final_path)',
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
            '--jars=>(input_jars_paths)',
            '--output-jar=<(jar_final_path)',
            # This argument is important for this final JAR we are creating, as
            # it validates that we are filtering out the right JARs when doing
            # the merge.
            '--validate-skipped-jars-list',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_library',
      'type': 'none',
      'dependencies': [
        'xwalk_core_shell_apk',
        'xwalk_core_library_java',
      ],
      'actions': [
        {
          'action_name': 'generate_xwalk_core_library',
          'message': 'Generating XWalk Core Library',
          'inputs': [
            '<(DEPTH)/xwalk/build/android/common_function.py',
            '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_core_library_intermediate/always_run',
          ],
          'action': [
            'python', '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
            '-s', '<(DEPTH)',
            '-t', '<(PRODUCT_DIR)'
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_shared_library',
      'type': 'none',
      'dependencies': [
        'xwalk_core_library_java',
      ],
      'actions': [
        {
          'action_name': 'generate_xwalk_shared_library',
          'message': 'Generating XWalk Shared Library',
          'inputs': [
            '<(DEPTH)/xwalk/build/android/common_function.py',
            '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_shared_library_intermediate/always_run',
          ],
          'action': [
            'python', '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
            '-s',  '<(DEPTH)',
            '-t', '<(PRODUCT_DIR)',
            '--shared',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_library_src',
      'type': 'none',
      'dependencies': [
        'xwalk_core_library',
      ],
      'variables': {
         # TODO(wang16): This list is hard coded for now. It might be broken by rebase to
         #               chromium new base. Need to check manually after each rebase.
        'java_source_dirs': [
          '<(DEPTH)/base/android/java/src',
          '<(DEPTH)/components/web_contents_delegate_android/android/java/src',
          '<(DEPTH)/components/navigation_interception/android/java/src',
          '<(DEPTH)/content/public/android/java/src',
          '<(DEPTH)/media/base/android/java/src',
          '<(DEPTH)/net/android/java/src',
          '<(DEPTH)/ui/android/java/src',
          '<(DEPTH)/xwalk/extensions/android/java/src',
          '<(DEPTH)/xwalk/runtime/android/core/src',
          '<(DEPTH)/xwalk/runtime/android/core_internal/src',
          '<(PRODUCT_DIR)/gen/enums/bitmap_format_java',
          '<(PRODUCT_DIR)/gen/enums/window_open_disposition_java',
          '<(PRODUCT_DIR)/gen/templates',
          '<(PRODUCT_DIR)/resource_map',
          # NativeLibraries.java must be copied later than gen/templates to override the empty
          # NativeLibraries.java in gen/templates.
          '<(PRODUCT_DIR)/xwalk_core_internal_empty_embedder_apk/native_libraries_java/NativeLibraries.java',
          '>(reflection_gen_dir)/bridge',
          '>(reflection_gen_dir)/wrapper',
        ],
      },
      'actions': [
        {
          'action_name': 'generate_xwalk_core_library_src_package',
          'message': 'Generating Source Package of XWalk Core Library',
          'inputs': [
            '<(DEPTH)/xwalk/build/android/common_function.py',
            '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_core_library_src/always_run',
            '<(PRODUCT_DIR)/xwalk_core_library_src/src/README.md',
          ],
          'action': [
            'python', '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
            '-s',  '<(DEPTH)',
            '-t', '<(PRODUCT_DIR)',
            '--src-package',
          ],
        },
        {
          'action_name': 'copy_xwalk_core_library_src',
          'message': 'Copy java sources of xwalk core library',
          'inputs': [
            'build/android/merge_java_srcs.py',
            '<(PRODUCT_DIR)/xwalk_core_library_src/src/README.md',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_core_library_src/copy_src_always_run',
          ],
          'action': [
            'python', 'build/android/merge_java_srcs.py',
            '--dirs=>(java_source_dirs)',
            '--target-path=<(PRODUCT_DIR)/xwalk_core_library_src/src',
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
            '<(DEPTH)/xwalk/build/android/common_function.py',
            '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library_aar.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_core_library_aar_intermediate/always_run',
          ],
          'action': [
            'python', '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library_aar.py',
            '-t', '<(PRODUCT_DIR)',
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
            '<(DEPTH)/xwalk/build/android/common_function.py',
            '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library_aar.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_shared_library_aar_intermediate/always_run',
          ],
          'action': [
            'python', '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library_aar.py',
            '-t', '<(PRODUCT_DIR)',
            '--shared',
          ],
        },
      ],
    },
  ],
}
