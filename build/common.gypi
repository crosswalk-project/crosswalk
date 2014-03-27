{
  'variables': {
    'tizen%': 0,
    'tizen_mobile%': 0,
  },
  'target_defaults': {
    'variables': {
      'tizen%': '<(tizen)',
      'tizen_mobile%': '<(tizen_mobile)',
    },
    'conditions': [
      ['tizen==1', {
        'defines': ['OS_TIZEN=1'],
      }],
      ['tizen_mobile==1', {
        'defines': ['OS_TIZEN_MOBILE=1', 'OS_TIZEN=1'],
      }],
    ],
    'includes': [
      'xwalk_filename_rules.gypi',
    ],
  },
}
