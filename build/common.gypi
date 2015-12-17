{
  'variables': {
    'use_webui_file_picker%': 0,
    'disable_bundled_extensions%': 0,

    # Name of Crosswalk Maven artifacts used to generate their
    # respective POM files.
    'xwalk_core_library_artifact_id%': 'xwalk_core_library_canary',
    'xwalk_shared_library_artifact_id%': 'xwalk_shared_library_canary',

    # Flags defined for CrosswalkLite
    'use_icu_alternatives_on_android%': 0,
    'disable_xslt%': 0,
    'disable_webp%': 0,
    'disable_angle%': 0,
    'disable_quic_support%': 0,
    'disable_sync_compositor%': 0,
    'disable_webaudio%': 0,
    'disable_webaudio_hrtf%': 0,
    'use_minimum_resources%': 0,
    'disable_builtin_extensions%': 0,
    'disable_devtools%': 0,
    'disable_web_video%': 0,
    'disable_speech%': 0,
    'use_optimize_for_size_compile_option%': 0,
    'disable_notifications%': 0,
    'disable_indexeddb%': 0,
    'disable_accessibility%': 0,
    'disable_geo_features%': 0,
    'disable_bluetooth%': 0,
    'disable_webdatabase%': 0,
    'disable_webmidi%': 0,
    'disable_mediastream%': 0,

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
      # Whether to enable WebCL.
      'enable_webcl%': '<(enable_webcl)',
    },
    'conditions': [
      ['disable_accessibility==1', {
        'defines': ['DISABLE_ACCESSIBILITY=1'],
      }],

      ['disable_bluetooth==1', {
        'defines': ['DISABLE_BLUETOOTH'],
      }],

      ['disable_devtools==1', {
        'defines': ['DISABLE_DEVTOOLS=1'],
      }],

      ['disable_geo_features==1', {
        'defines' : ['DISABLE_GEO_FEATURES'],
      }],

      ['disable_indexeddb==1', {
        'defines': ['DISABLE_INDEXEDDB=1'],
      }],

      ['disable_mediastream==1', {
        'defines': ['DISABLE_MEDIASTREAM=1'],
      }],

      ['disable_notifications==1', {
        'defines' : ['DISABLE_NOTIFICATIONS'],
      }],

      ['enable_plugins==0', {
        'defines' : ['DISABLE_PLUGINS'],
      }],

      ['disable_speech==1', {
        'defines': ['DISABLE_SPEECH'],
      }],

      ['disable_webaudio==1', {
        'defines': ['DISABLE_WEB_AUDIO=1'],
      }],

      ['enable_webcl==1', {
        'defines': ['ENABLE_WEBCL=1'],
      }],

      ['disable_webdatabase==1', {
        'defines' : ['DISABLE_WEBDATABASE'],
      }],

      ['disable_webmidi==1', {
        'defines' : ['DISABLE_WEBMIDI'],
      }],

      ['disable_web_video==1', {
        'defines': ['DISABLE_WEB_VIDEO=1'],
      }],
    ],
  },
}
