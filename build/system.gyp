# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'conditions': [
    [ 'tizen == 1 or tizen_mobile == 1', {
      'targets': [
        {
          'target_name': 'tizen_geolocation',
          'type': 'none',
          'variables': {
            'packages': [
              'capi-location-manager',
            ],
          },
          'direct_dependent_settings': {
            'cflags': [
              '<!@(pkg-config --cflags <@(packages))',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!@(pkg-config --libs-only-L --libs-only-other <@(packages))',
            ],
            'libraries': [
              '<!@(pkg-config --libs-only-l <@(packages))',
            ],
          },
        },
        {
          'target_name': 'tizen',
          'type': 'none',
          'variables': {
            'packages': [
              'pkgmgr-parser',
              'pkgmgr-info',
            ],
          },
          'direct_dependent_settings': {
            'cflags': [
              '<!@(pkg-config --cflags <@(packages))',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!@(pkg-config --libs-only-L --libs-only-other <@(packages))',
            ],
            'libraries': [
              '<!@(pkg-config --libs-only-l <@(packages))',
            ],
          },
        },
        {
          'target_name': 'tizen_sensor',
          'type': 'none',
          'variables': {
            'packages': [
              'sensor',
              'vconf',
            ],
          },
          'direct_dependent_settings': {
            'cflags': [
              '<!@(pkg-config --cflags <@(packages))',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!@(pkg-config --libs-only-L --libs-only-other <@(packages))',
            ],
            'libraries': [
              '<!@(pkg-config --libs-only-l <@(packages))',
            ],
          },
        },
        {
          'target_name': 'tizen_sysapps',
          'type': 'none',
          'variables': {
            'packages': [
              'vconf',
            ],
          },
          'direct_dependent_settings': {
            'cflags': [
              '<!@(pkg-config --cflags <@(packages))',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!@(pkg-config --libs-only-L --libs-only-other <@(packages))',
            ],
            'libraries': [
              '<!@(pkg-config --libs-only-l <@(packages))',
            ],
          },
        },
        {
          'target_name': 'tizen_appcore_common',
          'type': 'none',
          'variables': {
            'packages': [
              'appcore-common',
            ],
          },
          'direct_dependent_settings': {
            'cflags': [
              '<!@(pkg-config --cflags <@(packages))',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!@(pkg-config --libs-only-L --libs-only-other <@(packages))',
            ],
            'libraries': [
              '<!@(pkg-config --libs-only-l <@(packages))',
            ],
          },
        },
        {
          'target_name': 'tizen_vibration',
          'type': 'none',
          'variables': {
            'packages': [
              'haptic',
            ],
          },
          'direct_dependent_settings': {
            'cflags': [
              '<!@(pkg-config --cflags <@(packages))',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!@(pkg-config --libs-only-L --libs-only-other <@(packages))',
            ],
            'libraries': [
              '<!@(pkg-config --libs-only-l <@(packages))',
            ],
          },
        },
      ],  # targets
    }],
  ],  # conditions
}
