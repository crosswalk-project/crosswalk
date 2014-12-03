{
  'variables': {
    'tizen%': 0,
    'tizen_mobile%': 0,
    'shared_process_mode%': 0,
    'use_cynara%': 0,
    'enable_murphy%': 0,
  },
  'target_defaults': {
    'variables': {
      'tizen%': '<(tizen)',
      'tizen_mobile%': '<(tizen_mobile)',
      'enable_murphy%': '<(enable_murphy)',
    },
    'conditions': [
      ['tizen==1', {
        'conditions': [
          ['use_cynara==1', {
            'defines': ['OS_TIZEN=1', 'USE_CYNARA=1']
          },{
            'defines': ['OS_TIZEN=1', 'USE_CYNARA=0']
          }]
        ]
      }],
      ['tizen_mobile==1', {
        'defines': ['OS_TIZEN_MOBILE=1', 'OS_TIZEN=1'],
      }],
      ['shared_process_mode==1', {
        'defines': ['SHARED_PROCESS_MODE=1'],
      }],
      ['enable_murphy==1', {
        'defines': ['ENABLE_MURPHY=1'],
      }],
    ],
    'includes': [
      'xwalk_filename_rules.gypi',
    ],
  },
}
