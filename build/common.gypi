{
  'variables': {
    'tizen%': 0,
    'tizen_mobile%': 0,
    'enable_murphy%': 0,
    'use_webui_file_picker%': 0,
    'conditions': [
      ['OS=="android"', {
        # Enable WebCL by default on android.
        'enable_webcl%': 1,
        'v8_use_external_startup_data%': 0,
      }, {
        'enable_webcl%': 0,
      }],
    ],
  },
  'target_defaults': {
    'variables': {
      'tizen%': '<(tizen)',
      'tizen_mobile%': '<(tizen_mobile)',
      'enable_murphy%': '<(enable_murphy)',
      # Whether to enable WebCL.
      'enable_webcl%': '<(enable_webcl)',
    },
    'conditions': [
      ['tizen==1', {
        'defines': ['OS_TIZEN=1'],
      }],
      ['tizen_mobile==1', {
        'defines': ['OS_TIZEN_MOBILE=1', 'OS_TIZEN=1'],
      }],
      ['enable_murphy==1', {
        'defines': ['ENABLE_MURPHY=1'],
      }],
      ['enable_webcl==1', {
        'defines': ['ENABLE_WEBCL=1'],
      }],
    ],
    'includes': [
      'xwalk_filename_rules.gypi',
    ],
  },
}
