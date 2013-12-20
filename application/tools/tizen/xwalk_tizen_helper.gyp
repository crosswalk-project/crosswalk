{
  'targets': [
    {
      'target_name': 'xwalk-pkg-helper',
      'type': 'executable',
      'product_name': 'xwalk-pkg-helper',
      'conditions': [
        ['tizen_mobile==1', {
          'dependencies': [
            '../../../build/system.gyp:tizen',
            '../../../../base/base.gyp:base',
            ],
          }],
        ['OS=="linux"', {
          'dependencies': [
            '../../../../base/base.gyp:base',
          ],
        }],
      ],
      'include_dirs': [
        '../../../..',
      ],
      'sources': [
        'xwalk_pkg_helper.cc',
      ],
    },
  ],
}
