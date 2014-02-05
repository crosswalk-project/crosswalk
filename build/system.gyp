# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'conditions': [
    [ 'tizen_mobile == 1', {
      'targets': [
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
          'target_name': 'tizen_appcore',
          'type': 'none',
          'variables': {
            'packages': [
              'appcore-efl',
              'aul',
              'capi-appfw-application',
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
        {
          'target_name': 'tizen_dialog_launcher',
          'type': 'none',
          'variables': {
            'packages': [
              'ecore',
              'edbus',
              'dbus-1',
              'elementary',
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
