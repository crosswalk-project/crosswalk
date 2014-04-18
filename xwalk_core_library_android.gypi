# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
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
      'target_name': 'xwalk_core_library_java',
      'type': 'none',
      'dependencies': [
        'xwalk_core_java',
      ],
      'variables': {
        'java_in_dir': 'runtime/android/core',
        'classes_dir': '<(PRODUCT_DIR)/<(_target_name)/classes',
        'jar_name': '<(_target_name).jar',
        'jar_final_path': '<(PRODUCT_DIR)/lib.java/<(jar_name)',
        'jar_excluded_classes': [ '*/R.class', '*/R##*.class' ],
      },
      'all_dependent_settings': {
        'variables': {
          'input_jars_paths': ['<(jar_final_path)'],
        },
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
