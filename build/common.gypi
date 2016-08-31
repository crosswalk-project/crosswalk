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
#
# The assignments below can look daunting. It is due to the way gyp merges this
# file, src/build/common.gypi and the other build system files and how
# variables and conditions are evaluated.
# Important things to keep in mind:
# - We try to change variables coming from a different file at the same scope
#   level they are originally defined so that the value we set propagate
#   through all scope levels all the way to the outermost "variables" block.
# - It is not possible to define a variable in a "variables" block and use it
#   in a check in a "conditions" block at the same scope level does not work.
# - "conditions" blocks inside a "variables" block are evaluated _after_ the
#    variable assignments outside the "conditions" block. This can lead to
#    unexpected results.

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
    # Lastly, we have some extra conditional blocks (like for |use_sysroot|)
    # because src/build/common.gypi might define some variables in nested
    # blocks and to override their value we must set them at least within the
    # same scope level.
    'variables': {
      'variables': {
        'variables': {
          # From src/build/common.gypi.
          # Android and Windows do not use the sysroot, and we cannot use it on
          # Linux because we use libnotify that is not present there.
          'use_sysroot%': 0,
        },
        'use_sysroot%': '<(use_sysroot)',

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
      'use_sysroot%': '<(use_sysroot)',

      # Whether to disable NaCl support.
      'disable_nacl%': 0,

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
        # This condition check looks confusing, but it effectively acts like an
        # unconditional assignment such as the ones directly above. We cannot
        # use an unconditional assignment because the files would look like
        # this:
        #
        # (src/build/common.gypi)
        #   'variables': {
        #     'variables': {
        #       'proprietary_codecs%': 0,
        #     },
        #     'conditions': [
        #       ['OS=="android"', {
        #         'proprietary_codecs%': '<(proprietary_codecs)',
        #       }],
        #     ],
        #   },
        # (src/xwalk/build/common.gypi)
        #   'variables': {
        #     'proprietary_codecs%': 1,
        #   },
        #
        # When gyp merges both files, the `conditions' block is evaluated after
        # the variable assignments, so |proprietary_codecs| ends up with the
        # value "<(proprietary_codecs)", which evaluates to 0 since it's the
        # value coming from the inner scope.
        # The safest approach is to just turn the assignment into a conditional
        # so that it is evaluated at the same stage as the one from Chromium's
        # common.gypi and overrides its value.
        ['1==1', {
          # From src/build/common.gypi.
          # Whether to include support for proprietary codecs.
          'proprietary_codecs%': 1,
        }],

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
    'ffmpeg_branding%': '<(ffmpeg_branding)',
    'ffmpeg_component%': '<(ffmpeg_component)',
    'mediacodecs_EULA%': '<(mediacodecs_EULA)',
    'release_unwind_tables%': '<(release_unwind_tables)',
    'use_rssdk%': '<(use_rssdk)',
    'use_sysroot%': '<(use_sysroot)',
    'v8_use_external_startup_data%': '<(v8_use_external_startup_data)',

    # Whether to build and use Crosswalk's internal extensions (device
    # capabilities, sysapps etc).
    'disable_bundled_extensions%': 0,

    # From src/build/common.gypi.
    # Disable WebVR support. The code is still experimental and and ends up
    # pulling additional dependencies into our JARs (XWALK-6597).
    'enable_webvr%': 0,

    # Android: whether the integrity of the Crosswalk runtime library should be
    # verified when Crosswalk is run in shared mode.
    'verify_xwalk_apk%': 0,

    # Name of Crosswalk Maven artifacts used to generate their
    # respective POM files.
    'xwalk_core_library_artifact_id%': 'xwalk_core_library_canary',
    'xwalk_shared_library_artifact_id%': 'xwalk_shared_library_canary',
  },
}
