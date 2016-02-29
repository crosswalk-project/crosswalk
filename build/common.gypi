# This file is similar to src/build/common.gypi. It contains common variable
# definitions and other actions that are then used in other build system files.
#
# The values can be overriden on the GYP command line (-D) or by setting them
# in ~/.gyp/include.gypi.
#
# IMPORTANT: Like src/build/common.gypi, this file must not be included by
# other .gyp or .gypi files, it is included automatically when gyp_xwalk is
# called. This file is always included immediately after src/build/common.gypi
# itself so that variables originally set there can have their values
# overridden here.

{
  # Organization of the variables below:
  # 1. Variables copied from inner scopes.
  # 2. New variables defined in current scope.
  # 3. Conditions.
  # Some variables are new, but most come from other .gyp files and are
  # overridden here. The default values should match the ones in the original
  # .gyp files.
  'variables': {
    # Putting a variables dict inside another variables dict looks kind of
    # weird. This is caused by the way GYP variable expansion and evaluation
    # works: one cannot set a variable in a scope and use it in a conditional
    # block in the same scope level. For example, setting |mediacodecs_EULA|
    # and using it in a conditional in the same scope does not work. Similarly,
    # since this file is automatically included and at the root level into all
    # other .gyp and .gypi files their own |variables| section might have a
    # check for a variable set in a condition block below, so we need to put
    # variables such as |enable_widevine| into an inner scope to avoid having
    # the conditions block here be at the same level as one from another file.
    'variables': {
      'variables': {
        # From src/build/common.gypi.
        # Which target type to build most targets as.
        'component%': 'static_library',

        # This variable used to be interpreted in the gyp_xwalk script and
        # translated into changing |ffmpeg_branding|. We keep it here for
        # compatibility for now.
        # TODO(rakuco): Remove this after a while.
        'mediacodecs_EULA%': 0,
      },
      'component%': '<(component)',
      'mediacodecs_EULA%': '<(mediacodecs_EULA)',

      # Whether to disable NaCl support.
      'disable_nacl%': 0,

      # From src/third_party/widevine/cdm/widevine_cdm.gyp.
      # Whether to build Crosswalk with support for the Widevine CDM.
      'enable_widevine%': 0,

      # From src/third_party/ffmpeg/ffmpeg.gyp.
      # Whether to build the Chromium or Google Chrome version of FFmpeg (the
      # latter contains additional codecs).
      'ffmpeg_branding%': 'Chromium',

      # From src/third_party/ffmpeg/ffmpeg.gyp.
      # Which target type to build libffmpeg as.
      'ffmpeg_component%': '<(component)',

      # From src/build/common.gypi.
      # Whether to include stack unwinding support for backtrace().
      'release_unwind_tables%': 1,

      # From src/build/common.gypi.
      # Whether to use external startup data for V8.
      'v8_use_external_startup_data%': 1,

      # Whether to use the RealSense SDK (libpxc) video capture.
      'use_rssdk%': 0,

      'conditions': [
        ['mediacodecs_EULA==1', {
          'ffmpeg_branding%': 'Chrome',
        }],

        ['OS=="android"', {
          # Make release builds smaller by omitting stack unwind support for
          # backtrace().
          # TODO(rakuco): determine if we only want this in official builds.
          'release_unwind_tables%': 0,

          # Temporarily disable use of external snapshot files (XWALK-3516).
          'v8_use_external_startup_data%': 0,
        }],

        ['OS=="linux"', {
          # Since M44, ffmpeg is built as a static library by default. On Linux,
          # keep the previous behavior or building it as a shared library while
          # we figure out if it makes sense to switch to a static library by
          # default.
          'ffmpeg_component%': 'shared_library',
        }],

        ['OS=="mac" or OS=="win"', {
          'disable_nacl%': 1,
        }],

        ['OS=="win"', {
          'use_rssdk%': 1,
        }],
      ],
    },
    # Copy conditionally-set variables out one scope.
    'component%': '<(component)',
    'disable_nacl%': '<(disable_nacl)',
    'enable_widevine%': '<(enable_widevine)',
    'ffmpeg_branding%': '<(ffmpeg_branding)',
    'ffmpeg_component%': '<(ffmpeg_component)',
    'mediacodecs_EULA%': '<(mediacodecs_EULA)',
    'release_unwind_tables%': '<(release_unwind_tables)',
    'use_rssdk%': '<(use_rssdk)',
    'v8_use_external_startup_data%': '<(v8_use_external_startup_data)',

    # Whether to build and use Crosswalk's internal extensions (device
    # capabilities, sysapps etc).
    'disable_bundled_extensions%': 0,

    # From src/build/common.gypi.
    # Whether to include support for proprietary codecs..
    'proprietary_codecs%': 1,

    # Whether to use a WebUI-based file picker.
    'use_webui_file_picker%': 0,

    # Android: whether the integrity of the Crosswalk runtime library should be
    # verified when Crosswalk is run in shared mode.
    'verify_xwalk_apk%': 0,

    # Name of Crosswalk Maven artifacts used to generate their
    # respective POM files.
    'xwalk_core_library_artifact_id%': 'xwalk_core_library_canary',
    'xwalk_shared_library_artifact_id%': 'xwalk_shared_library_canary',
  },
}
