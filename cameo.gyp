{
  'variables': {
    'cameo_product_name': 'Cameo',
    # TODO: define cameo version format.
    'cameo_version': '0.28.0.1',
    'conditions': [
      ['OS=="linux"', {
       'use_custom_freetype%': 1,
      }, {
       'use_custom_freetype%': 0,
      }],
    ], # conditions
  },
  'includes' : [
    'cameo_tests.gypi',
  ],
  'targets': [
    {
      'target_name': 'cameo_runtime',
      'type': 'static_library',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'defines': ['CAMEO_VERSION="<(cameo_version)"'],
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        '../base/base.gyp:base',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '../content/content.gyp:content_app',
        '../content/content.gyp:content_browser',
        '../content/content.gyp:content_common',
        '../content/content.gyp:content_gpu',
        '../content/content.gyp:content_plugin',
        '../content/content.gyp:content_ppapi_plugin',
        '../content/content.gyp:content_renderer',
        '../content/content.gyp:content_utility',
        '../content/content.gyp:content_worker',
        '../content/content_resources.gyp:content_resources',
        '../ipc/ipc.gyp:ipc',
        '../media/media.gyp:media',
        '../net/net.gyp:net',
        '../net/net.gyp:net_resources',
        '../skia/skia.gyp:skia',
        '../third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
        '../ui/gl/gl.gyp:gl',
        '../ui/ui.gyp:ui',
        '../v8/tools/gyp/v8.gyp:v8',
        '../webkit/support/webkit_support.gyp:webkit_resources',
        '../webkit/support/webkit_support.gyp:webkit_support',
        'cameo_resources',
      ],
      'include_dirs': [
        '..',
      ],
      'includes': [
        'extensions/extensions.gypi',
      ],
      'sources': [
        'runtime/app/cameo_main_delegate.cc',
        'runtime/app/cameo_main_delegate.h',
        'runtime/browser/cameo_browser_main_parts.cc',
        'runtime/browser/cameo_browser_main_parts.h',
        'runtime/browser/cameo_content_browser_client.cc',
        'runtime/browser/cameo_content_browser_client.h',
        'runtime/browser/devtools/cameo_devtools_delegate.cc',
        'runtime/browser/devtools/cameo_devtools_delegate.h',
        'runtime/browser/devtools/remote_debugging_server.cc',
        'runtime/browser/devtools/remote_debugging_server.h',
        'runtime/browser/geolocation/cameo_access_token_store.cc',
        'runtime/browser/geolocation/cameo_access_token_store.h',
        'runtime/browser/image_util.cc',
        'runtime/browser/image_util.h',
        'runtime/browser/media/media_capture_devices_dispatcher.cc',
        'runtime/browser/media/media_capture_devices_dispatcher.h',
        'runtime/browser/runtime.cc',
        'runtime/browser/runtime.h',
        'runtime/browser/runtime_context.cc',
        'runtime/browser/runtime_context.h',
        'runtime/browser/runtime_file_select_helper.cc',
        'runtime/browser/runtime_file_select_helper.h',
        'runtime/browser/runtime_network_delegate.cc',
        'runtime/browser/runtime_network_delegate.h',
        'runtime/browser/runtime_platform_util.h',
        'runtime/browser/runtime_platform_util_common_linux.cc',
        'runtime/browser/runtime_platform_util_linux.cc',
        'runtime/browser/runtime_platform_util_win.cc',
        'runtime/browser/runtime_registry.cc',
        'runtime/browser/runtime_registry.h',
        'runtime/browser/runtime_select_file_policy.cc',
        'runtime/browser/runtime_select_file_policy.h',
        'runtime/browser/runtime_url_request_context_getter.cc',
        'runtime/browser/runtime_url_request_context_getter.h',
        'runtime/browser/ui/color_chooser.cc',
        'runtime/browser/ui/color_chooser.h',
        'runtime/browser/ui/color_chooser_dialog_win.cc',
        'runtime/browser/ui/color_chooser_dialog_win.h',
        'runtime/browser/ui/color_chooser_gtk.cc',
        'runtime/browser/ui/color_chooser_win.cc',
        'runtime/browser/ui/native_app_window.h',
        'runtime/browser/ui/native_app_window_win.cc',
        'runtime/browser/ui/native_app_window_win.h',
        'runtime/browser/ui/native_app_window_gtk.cc',
        'runtime/browser/ui/native_app_window_gtk.h',
        'runtime/browser/ui/taskbar_util_win.cc',
        'runtime/browser/ui/taskbar_util.h',
        'runtime/common/cameo_content_client.cc',
        'runtime/common/cameo_content_client.h',
        'runtime/common/cameo_paths.cc',
        'runtime/common/cameo_paths.h',
        'runtime/common/cameo_switches.cc',
        'runtime/common/cameo_switches.h',
        'runtime/renderer/cameo_content_renderer_client.cc',
        'runtime/renderer/cameo_content_renderer_client.h',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
        },
      },
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS=="win"', {
          'resource_include_dirs': [
            '<(SHARED_INTERMEDIATE_DIR)/webkit',
          ],
          'dependencies': [
            '../ui/views/controls/webview/webview.gyp:webview',
            '../ui/views/views.gyp:views',
            '../webkit/support/webkit_support.gyp:webkit_strings',
          ],
          'configurations': {
            'Debug_Base': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
                },
              },
            },
          },
          # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
          'msvs_disabled_warnings': [ 4267, ],
        }],  # OS=="win"
        ['OS=="linux"', {
          'dependencies': [
            '../build/linux/system.gyp:fontconfig',
          ],
        }],  # OS=="linux"
        ['os_posix==1 and linux_use_tcmalloc==1', {
          'dependencies': [
            # This is needed by content/app/content_main_runner.cc
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],  # os_posix==1 and linux_use_tcmalloc==1
        ['use_custom_freetype==1', {
          'dependencies': [
             '../third_party/freetype2/freetype2.gyp:freetype2',
          ],
        }],  # use_custom_freetype==1
        ['toolkit_uses_gtk==1', {
          'dependencies': [
            '../build/linux/system.gyp:gtk',
          ],
        }],  # toolkit_uses_gtk==1
      ],
    },
    {
      'target_name': 'cameo_resources',
      'type': 'none',
      'dependencies': [
        'generate_cameo_resources',
      ],
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/cameo',
      },
      'includes': [ '../build/grit_target.gypi' ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(SHARED_INTERMEDIATE_DIR)/cameo/cameo_resources.pak'
          ],
        },
      ],
    },
    {
      'target_name': 'generate_cameo_resources',
      'type': 'none',
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/cameo',
      },
      'actions': [
        {
          'action_name': 'cameo_resources',
          'variables': {
            'grit_resource_ids': 'runtime/resources/resource_ids',
            'grit_grd_file': 'runtime/resources/cameo_resources.grd',
          },
          'includes': [ '../build/grit_action.gypi' ],
        },
      ],
    },
    {
      # Build a minimal set of resources so Blink in cameo has
      # access to necessary resources.
      'target_name': 'cameo_pak',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
        '<(DEPTH)/ui/base/strings/ui_strings.gyp:ui_strings',
        '<(DEPTH)/ui/ui.gyp:ui_resources',
        'cameo_resources',
      ],
      'variables': {
        'repack_path': '../tools/grit/grit/format/repack.py',
      },
      'actions': [
        {
          'action_name': 'repack_cameo_resources',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/cameo/cameo_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/app_locale_settings/app_locale_settings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_strings/ui_strings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/devtools_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_chromium_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_strings_en-US.pak',
            ],
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)',
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)',
                     '<@(pak_inputs)'],
          'outputs':[
            '<(PRODUCT_DIR)/cameo.pak',
          ],
        },
      ],
    },
    {
      'target_name': 'cameo',
      'type': 'executable',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'dependencies': [
        'cameo_runtime',
        'cameo_pak',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'runtime/app/cameo_main.cc',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
        },
      },
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS=="win"', {
          'sources': [
            '../content/app/startup_helper_win.cc', # Needed by InitializedSandbox
            'runtime/resources/cameo.rc',
          ],
          'configurations': {
            'Debug_Base': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
                },
              },
            },
          },
        }],  # OS=="win"
        ['OS == "win" or (toolkit_uses_gtk == 1 and selinux == 0)', {
          'dependencies': [
            '../sandbox/sandbox.gyp:sandbox',
          ],
        }],  # OS=="win" or (toolkit_uses_gtk == 1 and selinux == 0)
      ],
    },
    {
      'target_name': 'cameo_builder',
      'type': 'none',
      'dependencies': [
        'cameo',
        'cameo_browsertest',
        'cameo_unittest',
      ],
    },
  ],
}
