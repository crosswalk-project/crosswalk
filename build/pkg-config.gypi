# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'packages%': [],
  },

  'cflags': [
    '>!@(pkg-config --cflags >@(packages))'
  ],

  'link_settings': {
    'ldflags': [
      '>!@(pkg-config --libs-only-L --libs-only-other >@(packages))',
    ],
    'libraries': [
      '>!@(pkg-config --libs-only-l >@(packages))',
    ],
  },
}
