# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Copyright (c) 2015 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Much simplified version of the code in src/chrome/chrome_installer.gypi.

{
  'conditions': [
    ['OS=="linux"', {
      'variables': {
        'packaging_files_common': [
          'tools/installer/common/crosswalk.info',
          'tools/installer/common/generate-changelog.sh',
          'tools/installer/common/installer.include',
          'tools/installer/common/wrapper',
        ],
        'packaging_files_deb': [
          'tools/installer/debian/build.sh',
          'tools/installer/debian/changelog.template',
          'tools/installer/debian/control.template',
        ],
        'packaging_files_binaries': [
          '<(PRODUCT_DIR)/xwalk',

          # Commented out for the time being because non-Ozone Linux builds
          # depend on the gtk2ui target in src/chrome, which can cause
          # Chromium's language files to overwrite Crosswalk's in the directory
          # below. Remove once we stop depending on the gtk2ui target.
          # '<(PRODUCT_DIR)/locales/en-US.pak',
        ],
        'flock_bash': ['flock', '--', '/tmp/linux_package_lock', 'bash'],
        'deb_build': '<(PRODUCT_DIR)/installer/debian/build.sh',
        'deb_cmd': ['<@(flock_bash)', '<(deb_build)', '-o' '<(PRODUCT_DIR)',
                    '-b', '<(PRODUCT_DIR)', '-a', '<(target_arch)'],
        'conditions': [
          ['ffmpeg_component=="shared_library"', {
            'packaging_files_binaries': [
              '<(PRODUCT_DIR)/lib/libffmpeg.so',
            ],
          }],
          ['target_arch=="ia32"', {
            'deb_arch': 'i386',
            'packaging_files_common': [
              '<(DEPTH)/build/linux/bin/eu-strip',
            ],
          }],
          ['target_arch=="x64"', {
            'deb_arch': 'amd64',
            'packaging_files_common': [
              '<!(which eu-strip)',
            ],
          }],
        ],
      },
      'targets': [
        {
          'target_name': 'xwalk_installer_linux_prepare',
          'type': 'none',
          # Add these files to the build output so the build archives will be
          # "hermetic" for packaging.
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/installer/debian/',
              'files': [
                '<@(packaging_files_deb)',
              ]
            },
            {
              'destination': '<(PRODUCT_DIR)/installer/common/',
              'files': [
                '<@(packaging_files_common)',
              ]
            },
            {
              'destination': '<(PRODUCT_DIR)/installer/',
              'files': [
                '<(DEPTH)/xwalk/VERSION',
              ]
            },
          ],
          'actions': [
            {
              'action_name': 'generate_changelog_from_git',
              'inputs': [
                '<(DEPTH)/xwalk/tools/installer/common/generate-changelog.sh',
              ],
              'outputs': [
                '<(PRODUCT_DIR)/installer/changes.txt',
              ],
              'action': [
                'bash',
                '<(DEPTH)/xwalk/tools/installer/common/generate-changelog.sh',
                '<@(_outputs)',
              ],
            }
          ],
        },
        {
          'target_name': 'xwalk_installer_linux_debian',
          'suppress_wildcard': 1,
          'type': 'none',
          'dependencies': [
            'xwalk',
            'xwalk_installer_linux_prepare',
          ],
          'actions': [
            {
              'action_name': 'deb_packages',
              'process_outputs_as_sources': 1,
              'inputs': [
                '<(deb_build)',
                '<@(packaging_files_binaries)',
                '<@(packaging_files_common)',
                '<@(packaging_files_deb)',
              ],
              'outputs': [
                '<(PRODUCT_DIR)/crosswalk-<(xwalk_version)-1_<(deb_arch).deb',
              ],
              'action': [ '<@(deb_cmd)', ],
            },
          ],
        },
      ],
    }],
  ],
}
