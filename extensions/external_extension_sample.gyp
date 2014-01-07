{
  'target_defaults': {
    'include_dirs': [
      '../..',
    ],
  },
  'targets': [
    {
      'target_name': 'echo_extension',
      'type': 'loadable_module',
      'variables': {
        # We do not strip the binaries on Mac because the tool that does that
        # gets confused when you change the 'product_dir' (the binary output
        # directory). Since these are only for testing, no harm is done.
        'mac_strip': 0,
      },
      'sources': [
        'test/echo_extension.c',
      ],
      'conditions': [
        ['OS=="win"', {
          'product_dir': '<(PRODUCT_DIR)\\tests\\extension\\echo_extension\\'
        }, {
          'product_dir': '<(PRODUCT_DIR)/tests/extension/echo_extension/'
        }],
      ],
    },
    {
      'target_name': 'bad_extension',
      'type': 'loadable_module',
      'variables': {
        'mac_strip': 0,
      },
      'sources': [
        'test/bad_extension.c',
      ],
      'conditions': [
        ['OS=="win"', {
          'product_dir': '<(PRODUCT_DIR)\\tests\\extension\\bad_extension\\'
        }, {
          'product_dir': '<(PRODUCT_DIR)/tests/extension/bad_extension/'
        }],
      ],
    },
    {
      'target_name': 'multiple_entry_points_extension',
      'type': 'loadable_module',
      'variables': {
        'mac_strip': 0,
      },
      'sources': [
        'test/multiple_entry_points_extension.c',
      ],
      'conditions': [
        ['OS=="win"', {
          'product_dir': '<(PRODUCT_DIR)\\tests\\extension\\multiple_extension\\'
        }, {
          'product_dir': '<(PRODUCT_DIR)/tests/extension/multiple_extension/'
        }],
      ],
    },
    {
      'target_name': 'crash_extension',
      'type': 'loadable_module',
      'variables': {
        'mac_strip': 0,
      },
      'sources': [
        'test/crash_extension.c',
      ],
      'conditions': [
        ['OS=="win"', {
          'product_dir': '<(PRODUCT_DIR)\\tests\\extension\\crash_extension\\'
        }, {
          'product_dir': '<(PRODUCT_DIR)/tests/extension/crash_extension/'
        }],
      ],
    },
  ],
}
