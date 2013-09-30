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
      'target_name': 'xwalk_core_library',
      'type': 'none',
      'dependencies': [
        'xwalk_core_shell_apk'
      ],
      'actions': [
        {
          'action_name': 'generate_xwalk_core_library',
          'message': 'Generating XwalkCore Library Project.',
          'inputs': [
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
