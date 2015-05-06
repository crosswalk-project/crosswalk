# Copyright (c) 2015 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'stripped_library': '<(intermediate_dir)/libs/<(android_app_abi)/libxwalkcore.>(android_product_extension)',
    'compressed_library': '<(resource_dir)/raw/libxwalkcore.>(android_product_extension).<(android_app_abi)',
    'native_lib_target': 'libxwalkdummy',
    'additional_bundled_libs': [
      '<(PRODUCT_DIR)/lib/libxwalkcore.>(android_product_extension)',
    ],
    'additional_input_paths': [
      '<(resource_dir)/raw/libxwalkcore.>(android_product_extension).<(android_app_abi)',
    ],
  },
  'actions': [
    {
      'action_name': 'lzma_compression',
      'message': 'compress library',
      'inputs': [
        '<(strip_additional_stamp)',
      ],
      'outputs': [
        '<(stripped_library).lzma',
      ],
      'action': ['lzma', '-f', '<(stripped_library)'],
    },
    {
      'action_name': 'copy_compressed_library',
      'message': 'copy compressed library',
      'inputs': [
        '<(stripped_library).lzma',
      ],
      'outputs': [
        '<(compressed_library)',
      ],
      'action': ['cp', '<(stripped_library).lzma', '<(compressed_library)'],
    },
  ],
  'includes': [ '../build/java_apk.gypi' ],
}
