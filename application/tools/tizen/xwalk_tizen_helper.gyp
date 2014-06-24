{
  'targets': [
    {
      'target_name': 'xwalk-pkg-helper',
      'type': 'executable',
      'product_name': 'xwalk-pkg-helper',
      'dependencies': [
        '../../../build/system.gyp:tizen',
        '../../../../base/base.gyp:base',
      ],
      'include_dirs': [
        '../../../..',
      ],
      'cflags': [
        '<!@(pkg-config --cflags libtzplatform-config)',
      ],
      'link_settings': {
        'libraries': [
          '<!@(pkg-config --libs libtzplatform-config)',
        ],
      },
      'sources': [
        'xwalk_package_helper.cc',
        'xwalk_package_installer_helper.cc',
        'xwalk_package_installer_helper.h',
      ],
    },
  ],
}
