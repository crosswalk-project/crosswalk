# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets' : [
    {
      'target_name': 'libnotify',
      'type': 'none',
      'direct_dependent_settings': {
        'defines': ['USE_LIBNOTIFY=1'],
        'cflags': [
          '<!@(<(pkg-config) --cflags libnotify)',
        ],
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg-config) --libs-only-L --libs-only-other libnotify)',
        ],
        'libraries': [
          '<!@(<(pkg-config) --libs-only-l libnotify)',
        ],
      },
    },
  ], # targets
}
