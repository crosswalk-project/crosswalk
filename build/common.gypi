{
  'variables': {
    'tizen%': 0,
    'tizen_mobile%': 0,
    'enable_murphy%': 0,
    'disable_builtin_extensions%': 0,
    'use_webui_file_picker%': 0,
    'disable_bundled_extensions%': 0,

    'conditions': [
      ['OS=="android"', {
        # Enable WebCL by default on android.
        'enable_webcl%': 1,
        'v8_use_external_startup_data%': 0,
      }, {
        'enable_webcl%': 0,
      }],
      ['OS=="linux"', {
        # Since M44, ffmpeg is built as a static library by default. On Linux,
        # keep the previous behavior or building it as a shared library while
        # we figure out if it makes sense to switch to a static library by
        # default.
        'ffmpeg_component%': 'shared_library',
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
      'disable_builtin_extensions%': '<(disable_builtin_extensions)',
    },
    'conditions': [
      ['disable_indexeddb==1', {
        'defines': ['DISABLE_INDEXEDDB=1'],
      }],

      ['disable_accessibility==1', {
        'defines': ['DISABLE_ACCESSIBILITY=1'],
      }],

      ['disable_web_audio==1', {
        'defines': ['DISABLE_WEB_AUDIO=1'],
      }],

      ['disable_web_video==1', {
        'defines': ['DISABLE_WEB_VIDEO=1'],
      }],

      ['disable_speech==1', {
        'defines': ['DISABLE_SPEECH'],
      }],

      ['disable_notifications==1', {
        'defines' : ['DISABLE_NOTIFICATIONS'],
      }],

      ['disable_geo_features==1', {
        'defines' : ['DISABLE_GEO_FEATURES'],
      }],

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
      ['disable_devtools==1', {
        'defines': ['DISABLE_DEVTOOLS=1'],
      }],
    ],
    'includes': [
      'xwalk_filename_rules.gypi',
    ],
  },
}
