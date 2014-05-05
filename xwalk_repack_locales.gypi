# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# To use this the following variables need to be defined:
#   pak_locales: string: the list of all the locales that need repacking
{
  'variables': {
    'repack_locales_path': 'tools/repack_xwalk_locales.py',
  },
  'inputs': [
    '<(repack_locales_path)',
    '<!@pymod_do_main(repack_xwalk_locales -i -p <(OS)  -s <(SHARED_INTERMEDIATE_DIR) -x <(SHARED_INTERMEDIATE_DIR) <(pak_locales))'
  ],
  'outputs': [
    '<!@pymod_do_main(repack_xwalk_locales -o -p <(OS) -s <(SHARED_INTERMEDIATE_DIR) -x <(SHARED_INTERMEDIATE_DIR) <(pak_locales))'
  ],
  'action': [
    'python',
    '<(repack_locales_path)',
    '-p', '<(OS)',
    '-s', '<(SHARED_INTERMEDIATE_DIR)',
    '-x', '<(SHARED_INTERMEDIATE_DIR)/.',
    '<@(pak_locales)',
  ],
}
