# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'core_library_empty_embedder_apk_name': 'XWalkCoreLibraryEmptyEmbedder',
  },
  'targets': [
    {
      'target_name': 'xwalk_core_library_documentation',
      'type': 'none',
      'variables': {
        'api_files': [
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkJavascriptResult.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkNavigationHistory.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkNavigationItem.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkPreferences.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkResourceClient.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkUIClient.java',
          '<(DEPTH)/xwalk/runtime/android/core/src/org/xwalk/core/XWalkView.java',
        ],
        'docs': '<(PRODUCT_DIR)/xwalk_core_library_docs',
      },
      'actions': [
        {
          'action_name': 'javadoc_xwalk_core_library',
          'message': 'Creating documentation for XWalk Core Library',
          'inputs': [
            '<@(api_files)',
          ],
          'outputs': [
            '<(docs)/index.html',
          ],
          'action': [
            'javadoc',
            '-XDignore.symbol.file',
            '-d', '<(docs)',
            '<@(api_files)',
          ],
        },
      ],
    },
    {
      'target_name': 'pack_xwalk_core_library',
      'type': 'none',
      'dependencies': [
        'xwalk_core_library'
      ],
      'actions': [
        {
          'action_name': 'pack_xwalk_core_library',
          'message': 'Packaging XwalkCore Library Project.',
          'inputs': [
            '<(DEPTH)/xwalk/tools/tar.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_core_library.tar.gz',
            '<(PRODUCT_DIR)/pack_xwalk_core_library_intermediate/always_run',
          ],
          'action': [
            'python', 'tools/tar.py',
            '<(PRODUCT_DIR)/xwalk_core_library'
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_library_empty_embedder_apk',
      'type': 'none',
      'dependencies': [
        'libxwalkcore',
        'xwalk_core_java',
      ],
      'variables': {
        'apk_name': '<(core_library_empty_embedder_apk_name)',
        'java_in_dir': 'runtime/android/core_library_empty',
        'native_lib_target': 'libxwalkcore',
        'is_test_apk': 1,
        'additional_src_dirs': [
           '<(DEPTH)/ui/android/java/resource_map',
           '<(DEPTH)/content/public/android/java/resource_map',
           '<(DEPTH)/xwalk/runtime/android/core_internal/resource_map',
        ],
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      # pack classes compiled from the java files chromium generated into a
      # jar file.
      'target_name': 'chromium_generated_java',
      'type': 'none',
      'dependencies': [
        'xwalk_core_library_empty_embedder_apk',
      ],
      'variables': {
        'jar_name': '<(_target_name).jar',
        'jar_final_path': '<(PRODUCT_DIR)/lib.java/<(jar_name)',
        'jar_excluded_classes': [
          '*org/xwalk/core/R*',
        ],
      },
      'all_dependent_settings': {
        'variables': {
          'input_jars_paths': ['<(jar_final_path)'],
        },
      },
      'actions': [
        {
          'action_name': 'jar_<(_target_name)',
          'message': 'Creating <(_target_name) jar',
          'inputs': [
            '<(DEPTH)/build/android/gyp/util/build_utils.py',
            '<(DEPTH)/build/android/gyp/util/md5_check.py',
            '<(DEPTH)/build/android/gyp/jar.py',
            '<(PRODUCT_DIR)/apks/<(core_library_empty_embedder_apk_name).apk',
          ],
          'outputs': [
            '<(jar_final_path)',
          ],
          'action': [
            'python', '<(DEPTH)/build/android/gyp/jar.py',
            '--classes-dir=<(PRODUCT_DIR)/xwalk_core_library_empty_embedder_apk/classes',
            '--jar-path=<(jar_final_path)',
            '--excluded-classes=<(jar_excluded_classes)',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk_core_library_java',
      'type': 'none',
      'dependencies': [
        'xwalk_core_java',
        'chromium_generated_java',
      ],
      'variables': {
        'classes_dir': '<(PRODUCT_DIR)/<(_target_name)/classes',
        'jar_name': '<(_target_name).jar',
        'jar_final_path': '<(PRODUCT_DIR)/lib.java/<(jar_name)',
      },
      'actions': [
        {
          'action_name': 'jars_<(_target_name)',
          'message': 'Creating <(_target_name) jar',
          'inputs': [
            'build/android/merge_jars.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/pack_xwalk_core_library_java_intermediate/always_run',
          ],
          'action': [
            'python', 'build/android/merge_jars.py',
            '--classes-dir=<(classes_dir)',
            '--jars=>(input_jars_paths)',
            '--jar-path=<(jar_final_path)',
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
          'message': 'Generating XwalkCore Library Project.',
          'inputs': [
            '<(DEPTH)/xwalk/build/android/common_function.py',
            '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/xwalk_core_library_intermediate/always_run',
          ],
          'action': [
            'python', '<(DEPTH)/xwalk/build/android/generate_xwalk_core_library.py',
            '-s',  '<(DEPTH)',
            '-t', '<(PRODUCT_DIR)'
          ],
        },
      ],
    },
  ],
}
