{
  'variables': {
    'tizenos%': 0,
  },
  'target_defaults': {
    'conditions': [
      ['tizenos==1', {
        'defines': ['OS_TIZEN=1'],
      }],
    ],
  },
}
