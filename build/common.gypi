{
  'variables': {
    'use_webui_file_picker%': 0,
    'disable_bundled_extensions%': 0,

    # Name of Crosswalk Maven artifacts used to generate their
    # respective POM files.
    'xwalk_core_library_artifact_id%': 'xwalk_core_library_canary',
    'xwalk_shared_library_artifact_id%': 'xwalk_shared_library_canary',

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
      ['enable_webcl==1', {
        'defines': ['ENABLE_WEBCL=1'],
      }],
    ],
  },
}
