# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    # If we are not building the crosswalk-bin package (ie. we are only
    # building crosswalk-libs), we do not need to really add the dependencies
    # and call pkg-config.
    'building_crosswalk_bin%': 1,
  },
  'targets' : [
    {
      'target_name': 'gio',
      'type': 'none',
      'variables': {
        'glib_packages': 'glib-2.0 gio-unix-2.0',
      },
      'conditions': [
        ['building_crosswalk_bin==1', {
          'direct_dependent_settings': {
            'cflags': [
              '<!@(pkg-config --cflags <(glib_packages))',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!@(pkg-config --libs-only-L --libs-only-other <(glib_packages))',
            ],
            'libraries': [
              '<!@(pkg-config --libs-only-l <(glib_packages))',
            ],
          },
        },
       ],
      ],
    },
  ],  # targets
  'conditions': [
    ['tizen==1', {
      'targets': [
        {
          'target_name': 'tizen_tzplatform_config',
          'type': 'none',
          'variables': {
            'packages': [
              'libtzplatform-config',
            ],
          },
          'conditions': [
            ['building_crosswalk_bin==1', {
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
           ],
          ],
        },
        {
          'target_name': 'tizen_geolocation',
          'type': 'none',
          'variables': {
            'packages': [
              'capi-location-manager',
              'vconf',
            ],
          },
          'conditions': [
            ['building_crosswalk_bin==1', {
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
           ],
          ],
        },
        {
          'target_name': 'tizen',
          'type': 'none',
          'variables': {
            'packages': [
              'ail',
              'aul',
              'bundle',
              'capi-appfw-application',
              'dlog',
              'nss',
              'nspr',
              'pkgmgr-parser',
              'pkgmgr-info',
              'pkgmgr-installer',
              'pkgmgr',
              'secure-storage',
              'vconf',
            ],
          },
          'conditions': [
            ['building_crosswalk_bin==1', {
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
           ],
          ],
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
          'conditions': [
            ['building_crosswalk_bin==1', {
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
           ],
          ],
        },
        {
          'target_name': 'tizen_sysapps',
          'type': 'none',
          'variables': {
            'packages': [
              'vconf',
            ],
          },
          'conditions': [
            ['building_crosswalk_bin==1', {
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
           ],
          ],
        },
        {
          'target_name': 'tizen_appcore_common',
          'type': 'none',
          'variables': {
            'packages': [
              'appcore-common',
            ],
          },
          'conditions': [
            ['building_crosswalk_bin==1', {
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
           ],
          ],
        },
        {
          'target_name': 'tizen_vibration',
          'type': 'none',
          'variables': {
            'packages': [
              'haptic',
            ],
          },
          'conditions': [
            ['building_crosswalk_bin==1', {
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
           ],
          ],
        },
        {
          'target_name': 'xmlsec',
          'type': 'none',
          'conditions': [
            ['building_crosswalk_bin==1', {
              'direct_dependent_settings': {
                'cflags': [
                  '<!@(pkg-config --cflags xmlsec1)',
                ],
              },
              'link_settings': {
                'ldflags': [
                  '<!@(pkg-config --libs-only-L --libs-only-other xmlsec1)',
                ],
                'libraries': [
                  '<!@(pkg-config --libs-only-l xmlsec1)',
                ],
              },
            },
           ],
          ],
        }
      ],  # targets
    }, {  # tizen == 0
      'targets': [
        {
          'target_name': 'libnotify',
          'type': 'none',
          'conditions': [
            ['building_crosswalk_bin==1', {
              'direct_dependent_settings': {
                'defines': ['USE_LIBNOTIFY=1'],
                'cflags': [
                  '<!@(pkg-config --cflags libnotify)',
                ],
              },
              'link_settings': {
                'ldflags': [
                  '<!@(pkg-config --libs-only-L --libs-only-other libnotify)',
                ],
                'libraries': [
                  '<!@(pkg-config --libs-only-l libnotify)',
                ],
              },
            },
           ],
          ],
        },
      ],
    }],
  ],  # conditions
}
