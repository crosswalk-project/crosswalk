{
  'variables': {
    'xwalk_product_name': 'XWalk',
    # TODO: define xwalk version format.
    'xwalk_version': '0.28.0.1',
    'conditions': [
      ['OS=="linux"', {
       'use_custom_freetype%': 1,
      }, {
       'use_custom_freetype%': 0,
      }],
    ], # conditions
  },
  'includes' : [
    'xwalk_tests.gypi',
  ],
  'targets': [
    {
      'target_name': 'xwalk_runtime',
      'type': 'static_library',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'defines': ['XWALK_VERSION="<(xwalk_version)"'],
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
        'xwalk_resources',
      ],
      'include_dirs': [
        '..',
      ],
      'includes': [
        'extensions/extensions.gypi',
        'experimental/dialog/dialog.gypi',
      ],
      'sources': [
        'runtime/app/xwalk_main_delegate.cc',
        'runtime/app/xwalk_main_delegate.h',
        'runtime/browser/xwalk_application_mac.h',
        'runtime/browser/xwalk_application_mac.mm',
        'runtime/browser/xwalk_browser_main_parts.cc',
        'runtime/browser/xwalk_browser_main_parts.h',
        'runtime/browser/xwalk_browser_main_parts_mac.mm',
        'runtime/browser/xwalk_content_browser_client.cc',
        'runtime/browser/xwalk_content_browser_client.h',
        'runtime/browser/devtools/xwalk_devtools_delegate.cc',
        'runtime/browser/devtools/xwalk_devtools_delegate.h',
        'runtime/browser/devtools/remote_debugging_server.cc',
        'runtime/browser/devtools/remote_debugging_server.h',
        'runtime/browser/geolocation/xwalk_access_token_store.cc',
        'runtime/browser/geolocation/xwalk_access_token_store.h',
        'runtime/browser/image_util.cc',
        'runtime/browser/image_util.h',
        'runtime/browser/media/media_capture_devices_dispatcher.cc',
        'runtime/browser/media/media_capture_devices_dispatcher.h',
        'runtime/browser/runtime.cc',
        'runtime/browser/runtime.h',
        'runtime/browser/runtime_context.cc',
        'runtime/browser/runtime_context.h',
        'runtime/browser/runtime_download_manager_delegate.cc',
        'runtime/browser/runtime_download_manager_delegate.h',
        'runtime/browser/runtime_file_select_helper.cc',
        'runtime/browser/runtime_file_select_helper.h',
        'runtime/browser/runtime_network_delegate.cc',
        'runtime/browser/runtime_network_delegate.h',
        'runtime/browser/runtime_platform_util.h',
        'runtime/browser/runtime_platform_util_aura.cc',
        'runtime/browser/runtime_platform_util_gtk.cc',
        'runtime/browser/runtime_platform_util_linux.cc',
        'runtime/browser/runtime_platform_util_win.cc',
        'runtime/browser/runtime_platform_util_mac.mm',
        'runtime/browser/runtime_registry.cc',
        'runtime/browser/runtime_registry.h',
        'runtime/browser/runtime_select_file_policy.cc',
        'runtime/browser/runtime_select_file_policy.h',
        'runtime/browser/runtime_url_request_context_getter.cc',
        'runtime/browser/runtime_url_request_context_getter.h',
        'runtime/browser/ui/color_chooser.cc',
        'runtime/browser/ui/color_chooser.h',
        'runtime/browser/ui/color_chooser_aura.cc',
        'runtime/browser/ui/color_chooser_dialog_win.cc',
        'runtime/browser/ui/color_chooser_dialog_win.h',
        'runtime/browser/ui/color_chooser_gtk.cc',
        'runtime/browser/ui/color_chooser_win.cc',
        'runtime/browser/ui/native_app_window.h',
        'runtime/browser/ui/native_app_window.cc',
        'runtime/browser/ui/native_app_window_views.cc',
        'runtime/browser/ui/native_app_window_views.h',
        'runtime/browser/ui/native_app_window_gtk.cc',
        'runtime/browser/ui/native_app_window_gtk.h',
        'runtime/browser/ui/native_app_window_mac.mm',
        'runtime/browser/ui/native_app_window_mac.h',
        'runtime/browser/ui/taskbar_util_win.cc',
        'runtime/browser/ui/taskbar_util.h',
        'runtime/common/paths_mac.h',
        'runtime/common/paths_mac.mm',
        'runtime/common/xwalk_content_client.cc',
        'runtime/common/xwalk_content_client.h',
        'runtime/common/xwalk_paths.cc',
        'runtime/common/xwalk_paths.h',
        'runtime/common/xwalk_switches.cc',
        'runtime/common/xwalk_switches.h',
        'runtime/renderer/xwalk_content_renderer_client.cc',
        'runtime/renderer/xwalk_content_renderer_client.h',
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
        ['os_posix==1 and OS != "mac" and linux_use_tcmalloc==1', {
          'dependencies': [
            # This is needed by content/app/content_main_runner.cc
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],  # os_posix==1 and OS != "mac" and linux_use_tcmalloc==1
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
        ['toolkit_views==1', {
          'dependencies': [
            '../ui/base/strings/ui_strings.gyp:ui_strings',
            '../ui/views/controls/webview/webview.gyp:webview',
            '../ui/views/views.gyp:views',
            '../ui/views/views.gyp:views_test_support',
            '../ui/ui.gyp:ui_resources',
          ],
        }],  # toolkit_views==1
        ['use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:aura',
          ],
        }],
      ],
    },
    {
      'target_name': 'xwalk_resources',
      'type': 'none',
      'dependencies': [
        'generate_xwalk_resources',
      ],
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/xwalk',
      },
      'includes': [ '../build/grit_target.gypi' ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_resources.pak'
          ],
        },
      ],
    },
    {
      'target_name': 'generate_xwalk_resources',
      'type': 'none',
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/xwalk',
      },
      'actions': [
        {
          'action_name': 'xwalk_resources',
          'variables': {
            'grit_resource_ids': 'runtime/resources/resource_ids',
            'grit_grd_file': 'runtime/resources/xwalk_resources.grd',
          },
          'includes': [ '../build/grit_action.gypi' ],
        },
      ],
    },
    {
      # Build a minimal set of resources so Blink in XWalk has
      # access to necessary resources.
      'target_name': 'xwalk_pak',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
        '<(DEPTH)/ui/base/strings/ui_strings.gyp:ui_strings',
        '<(DEPTH)/ui/ui.gyp:ui_resources',
        'xwalk_resources',
      ],
      'variables': {
        'repack_path': '../tools/grit/grit/format/repack.py',
      },
      'actions': [
        {
          'action_name': 'repack_xwalk_resources',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_resources.pak',
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
            '<(PRODUCT_DIR)/xwalk.pak',
          ],
        },
      ],
    },
    {
      'target_name': 'xwalk',
      'type': 'executable',
      'mac_bundle': 1,
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'dependencies': [
        'xwalk_runtime',
        'xwalk_pak',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'runtime/app/xwalk_main.cc',
      ],
      'mac_bundle_resources': [
        'runtime/app/app.icns',
        'runtime/app/app-Info.plist',
      ],
      # TODO(mark): Come up with a fancier way to do this.  It should only
      # be necessary to list app-Info.plist once, not the three times it is
      # listed here.
      'mac_bundle_resources!': [
        'runtime/app/app-Info.plist',
      ],
      'xcode_settings': {
        'INFOPLIST_FILE': 'runtime/app/app-Info.plist',
      },
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
            'runtime/resources/xwalk.rc',
          ],
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)',
              'files': [
                'tools/packaging/bootstrapped/win/app.wxs.templ',
                'tools/packaging/bootstrapped/win/create_windows_installer.bat',
                'tools/packaging/bootstrapped/win/guid.vbs',
              ],
            },
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
        ['OS == "linux"', {
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)',
              'files': [
                'tools/packaging/bootstrapped/linux/app.desktop.templ',
                'tools/packaging/bootstrapped/linux/create_linux_installer.sh',
                'tools/packaging/bootstrapped/linux/Makefile.templ',
              ],
            }
          ],
        }],
        ['OS=="mac"', {
          'product_name': '<(xwalk_product_name)',
          'dependencies!': [
            'xwalk_runtime',
          ],
          'dependencies': [
            'xwalk_framework',
            'xwalk_helper_app',
          ],
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/<(xwalk_product_name).app/Contents/Frameworks',
              'files': [
                '<(PRODUCT_DIR)/<(xwalk_product_name) Helper.app',
              ],
            },
          ],
          'postbuilds': [
            {
              'postbuild_name': 'Copy <(xwalk_product_name) Framework.framework',
              'action': [
                '../build/mac/copy_framework_unversioned.sh',
                '${BUILT_PRODUCTS_DIR}/<(xwalk_product_name) Framework.framework',
                '${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/Frameworks',
              ],
            },
            {
              'postbuild_name': 'Fix Framework Link',
              'action': [
                'install_name_tool',
                '-change',
                '/Library/Frameworks/<(xwalk_product_name) Framework.framework/Versions/A/<(xwalk_product_name) Framework',
                '@executable_path/../Frameworks/<(xwalk_product_name) Framework.framework/<(xwalk_product_name) Framework',
                '${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}'
              ],
            },
            {
              # Modify the Info.plist as needed.
              'postbuild_name': 'Tweak Info.plist',
              'action': ['../build/mac/tweak_info_plist.py',
                         '--scm=1',
                         '--version=<(xwalk_version)'],
            },
            {
              # This postbuid step is responsible for creating the following
              # helpers:
              #
              # XWalk Helper EH.app and XWalk Helper NP.app are
              # created from XWalk Helper.app.
              #
              # The EH helper is marked for an executable heap. The NP helper
              # is marked for no PIE (ASLR).
              'postbuild_name': 'Make More Helpers',
              'action': [
                '../build/mac/make_more_helpers.sh',
                'Frameworks',
                '<(xwalk_product_name)',
              ],
            },
            {
              # Make sure there isn't any Objective-C in the xwalk's
              # executable.
              'postbuild_name': 'Verify No Objective-C',
              'action': [
                '../build/mac/verify_no_objc.sh',
              ],
            },
          ],
        }],  # OS=="mac"
      ],
    },
    {
      'target_name': 'xwalk_builder',
      'type': 'none',
      'dependencies': [
        'xwalk',
        'xwalk_browsertest',
        'xwalk_unittest',
      ],
    },
  ],
  'conditions': [
    ['OS=="mac"', {
      'targets': [
        {
          'target_name': 'xwalk_framework',
          'type': 'shared_library',
          'product_name': '<(xwalk_product_name) Framework',
          'mac_bundle': 1,
          'mac_bundle_resources': [
            'runtime/app/English.lproj/MainMenu.xib',
            '<(PRODUCT_DIR)/xwalk.pak'
          ],
          'dependencies': [
            'xwalk_runtime',
          ],
          'include_dirs': [
            '..',
          ],
          'sources': [
            'runtime/app/xwalk_content_main.cc',
            'runtime/app/xwalk_content_main.h',
          ],
          'copies': [
            {
              # Copy FFmpeg binaries for audio/video support.
              'destination': '<(PRODUCT_DIR)/$(CONTENTS_FOLDER_PATH)/Libraries',
              'files': [
                '<(PRODUCT_DIR)/ffmpegsumo.so',
              ],
            },
          ],
          'conditions': [
            ['enable_webrtc==1', {
              'variables': {
                'libpeer_target_type%': 'static_library',
              },
              'conditions': [
                ['libpeer_target_type!="static_library"', {
                  'copies': [{
                   'destination': '<(PRODUCT_DIR)/$(CONTENTS_FOLDER_PATH)/Libraries',
                   'files': [
                      '<(PRODUCT_DIR)/libpeerconnection.so',
                    ],
                  }],
                }],
              ],
            }],
          ],
        },  # target xwalk_framework
        {
          'target_name': 'xwalk_helper_app',
          'type': 'executable',
          'variables': { 'enable_wexit_time_destructors': 1, },
          'product_name': '<(xwalk_product_name) Helper',
          'mac_bundle': 1,
          'dependencies': [
            'xwalk_framework',
          ],
          'sources': [
            'runtime/app/xwalk_main.cc',
            'runtime/app/helper-Info.plist',
          ],
          # TODO(mark): Come up with a fancier way to do this.  It should only
          # be necessary to list helper-Info.plist once, not the three times it
          # is listed here.
          'mac_bundle_resources!': [
            'runtime/app/helper-Info.plist',
          ],
          # TODO(mark): For now, don't put any resources into this app.  Its
          # resources directory will be a symbolic link to the browser app's
          # resources directory.
          'mac_bundle_resources/': [
            ['exclude', '.*'],
          ],
          'xcode_settings': {
            'INFOPLIST_FILE': 'runtime/app/helper-Info.plist',
          },
          'postbuilds': [
            {
              # The framework defines its load-time path
              # (DYLIB_INSTALL_NAME_BASE) relative to the main executable
              # (chrome).  A different relative path needs to be used in
              # xwalk_helper_app.
              'postbuild_name': 'Fix Framework Link',
              'action': [
                'install_name_tool',
                '-change',
                '/Library/Frameworks/<(xwalk_product_name) Framework.framework/Versions/A/<(xwalk_product_name) Framework',
                '@executable_path/../../../../Frameworks/<(xwalk_product_name) Framework.framework/<(xwalk_product_name) Framework',
                '${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}'
              ],
            },
            {
              # Modify the Info.plist as needed.  The script explains why this
              # is needed.  This is also done in the chrome and chrome_dll
              # targets.  In this case, --breakpad=0, --keystone=0, and --scm=0
              # are used because Breakpad, Keystone, and SCM keys are
              # never placed into the helper.
              'postbuild_name': 'Tweak Info.plist',
              'action': ['../build/mac/tweak_info_plist.py',
                         '--breakpad=0',
                         '--keystone=0',
                         '--scm=0',
                         '--version=<(xwalk_version)'],
            },
            {
              # Make sure there isn't any Objective-C in the helper app's
              # executable.
              'postbuild_name': 'Verify No Objective-C',
              'action': [
                '../build/mac/verify_no_objc.sh',
              ],
            },
          ],
          'conditions': [
            ['component=="shared_library"', {
              'xcode_settings': {
                'LD_RUNPATH_SEARCH_PATHS': [
                  # Get back from XWalk.app/Contents/Frameworks/
                  #                                 Helper.app/Contents/MacOS
                  '@loader_path/../../../../../..',
                ],
              },
            }],
          ],
        },  # target xwalk_helper_app
      ],
    }],  # OS=="mac"
  ]
}
