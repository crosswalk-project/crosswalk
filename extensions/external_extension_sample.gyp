{
  'targets': [
  {
    'target_name': 'echo_extension',
    'type': 'loadable_module',
    'include_dirs': [
      '../..',
    ],
    'sources': [
      'test/echo_extension.c',
    ],
    'conditions': [
      ['OS=="win"', {
        'product_dir': '<(PRODUCT_DIR)\\tests\\extension\\echo_extension\\'
      }, {
        'product_dir': '<(PRODUCT_DIR)/tests/extension/echo_extension/'
      }],
    ], # conditions
  },
  {
    'target_name': 'bad_extension',
    'type': 'loadable_module',
    'include_dirs': [
      '../..',
    ],
    'sources': [
      'test/bad_extension.c',
    ],
    'conditions': [
      ['OS=="win"', {
        'product_dir': '<(PRODUCT_DIR)\\tests\\extension\\bad_extension\\'
      }, {
        'product_dir': '<(PRODUCT_DIR)/tests/extension/bad_extension/'
      }],
    ], # conditions
  },
  {
    'target_name': 'multiple_entry_points_extension',
    'type': 'loadable_module',
    'include_dirs': [
      '../..',
    ],
    'sources': [
      'test/multiple_entry_points_extension.c',
    ],
    'conditions': [
      ['OS=="win"', {
        'product_dir': '<(PRODUCT_DIR)\\tests\\extension\\multiple_extension\\'
      }, {
        'product_dir': '<(PRODUCT_DIR)/tests/extension/multiple_extension/'
      }],
    ], # conditions
  },
  {
    'target_name': 'crash_extension',
    'type': 'loadable_module',
    'include_dirs': [
      '../..',
    ],
    'sources': [
      'test/crash_extension.c',
    ],
    'conditions': [
      ['OS=="win"', {
        'product_dir': '<(PRODUCT_DIR)\\tests\\extension\\crash_extension\\'
      }, {
        'product_dir': '<(PRODUCT_DIR)/tests/extension/crash_extension/'
      }],
    ], # conditions
  },
  ],
}
