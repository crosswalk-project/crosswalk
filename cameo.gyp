# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'content_shell_product_name': 'Cameo',
    'content_shell_version': '0.28.0.1',
    'conditions': [
      ['OS=="linux"', {
       'use_custom_freetype%': 1,
      }, {
       'use_custom_freetype%': 0,
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'content_shell_lib',
      'type': 'static_library',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'defines': ['CONTENT_SHELL_VERSION="<(content_shell_version)"'],
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        '../content/content.gyp:content_app',
        '../content/content.gyp:content_browser',
        '../content/content.gyp:content_common',
        '../content/content.gyp:content_gpu',
        '../content/content.gyp:content_plugin',
        '../content/content.gyp:content_ppapi_plugin',
        '../content/content.gyp:content_renderer',
        '../content/content.gyp:content_utility',
        '../content/content.gyp:content_worker',
        '../content/content.gyp:test_support_content',
        '../content/content_resources.gyp:content_resources',
        '../base/base.gyp:base',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '../ipc/ipc.gyp:ipc',
        '../media/media.gyp:media',
        '../net/net.gyp:net',
        '../net/net.gyp:net_resources',
        '../skia/skia.gyp:skia',
        '../ui/gl/gl.gyp:gl',
        '../ui/ui.gyp:ui',
        '../v8/tools/gyp/v8.gyp:v8',
        '../webkit/support/webkit_support.gyp:webkit_resources',
        '../webkit/support/webkit_support.gyp:webkit_support',
        '../third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
        '../third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit_test_support',
        '../third_party/WebKit/Tools/DumpRenderTree/DumpRenderTree.gyp/DumpRenderTree.gyp:TestRunner',
        'cameo_resources'
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'src/android/shell_jni_registrar.cc',
        'src/android/shell_jni_registrar.h',
        'src/android/shell_manager.cc',
        'src/android/shell_manager.h',
        'src/app/shell_main_delegate.cc',
        'src/app/shell_main_delegate.h',
        'src/browser/minimal_ash.cc',
        'src/browser/minimal_ash.h',
        'src/browser/shell.cc',
        'src/browser/shell.h',
        'src/browser/shell_android.cc',
        'src/browser/shell_aura.cc',
        'src/browser/shell_gtk.cc',
        'src/browser/shell_mac.mm',
        'src/browser/shell_win.cc',
        'src/browser/shell_application_mac.h',
        'src/browser/shell_application_mac.mm',
        'src/browser/shell_browser_context.cc',
        'src/browser/shell_browser_context.h',
        'src/browser/shell_browser_main.cc',
        'src/browser/shell_browser_main.h',
        'src/browser/shell_browser_main_parts.cc',
        'src/browser/shell_browser_main_parts.h',
        'src/browser/shell_browser_main_parts_mac.mm',
        'src/browser/shell_content_browser_client.cc',
        'src/browser/shell_content_browser_client.h',
        'src/browser/shell_devtools_delegate.cc',
        'src/browser/shell_devtools_delegate.h',
        'src/browser/shell_devtools_frontend.cc',
        'src/browser/shell_devtools_frontend.h',
        'src/browser/shell_download_manager_delegate.cc',
        'src/browser/shell_download_manager_delegate.h',
        'src/browser/shell_javascript_dialog_manager.cc',
        'src/browser/shell_javascript_dialog_manager.h',
        'src/browser/shell_javascript_dialog_gtk.cc',
        'src/browser/shell_javascript_dialog_mac.mm',
        'src/browser/shell_javascript_dialog_win.cc',
        'src/browser/shell_javascript_dialog.h',
        'src/browser/shell_login_dialog_gtk.cc',
        'src/browser/shell_login_dialog_mac.mm',
        'src/browser/shell_login_dialog.cc',
        'src/browser/shell_login_dialog.h',
        'src/browser/shell_message_filter.cc',
        'src/browser/shell_message_filter.h',
        'src/browser/shell_network_delegate.cc',
        'src/browser/shell_network_delegate.h',
        'src/browser/shell_quota_permission_context.cc',
        'src/browser/shell_quota_permission_context.h',
        'src/browser/shell_resource_dispatcher_host_delegate.cc',
        'src/browser/shell_resource_dispatcher_host_delegate.h',
        'src/browser/shell_url_request_context_getter.cc',
        'src/browser/shell_url_request_context_getter.h',
        'src/browser/shell_web_contents_view_delegate_android.cc',
        'src/browser/shell_web_contents_view_delegate_creator.h',
        'src/browser/shell_web_contents_view_delegate_gtk.cc',
        'src/browser/shell_web_contents_view_delegate_mac.mm',
        'src/browser/shell_web_contents_view_delegate_win.cc',
        'src/browser/shell_web_contents_view_delegate.h',
        'src/browser/ui/native_app_window.h',
        'src/browser/ui/native_app_window_win.cc',
        'src/browser/ui/native_app_window_win.h',
        'src/browser/ui/native_toolbar_win.cc',
        'src/browser/ui/native_toolbar_win.h',
        'src/common/paths_mac.h',
        'src/common/paths_mac.mm',
        'src/common/shell_content_client.cc',
        'src/common/shell_content_client.h',
        'src/common/shell_messages.cc',
        'src/common/shell_messages.h',
        'src/common/shell_switches.cc',
        'src/common/shell_switches.h',
        'src/geolocation/shell_access_token_store.cc',
        'src/geolocation/shell_access_token_store.h',
        'src/renderer/shell_content_renderer_client.cc',
        'src/renderer/shell_content_renderer_client.h',
        'src/renderer/shell_render_process_observer.cc',
        'src/renderer/shell_render_process_observer.h',
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
            '../webkit/support/webkit_support.gyp:webkit_strings',
            '../ui/views/controls/webview/webview.gyp:webview',
            '../ui/views/views.gyp:views',
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
        }],
        ['OS=="android"', {
          'dependencies': [
            'content_shell_jni_headers',
          ],
        }, {  # else: OS!="android"
          'dependencies': [
            # This dependency is for running DRT against the content shell, and
            # this combination is not yet supported on Android.
            '../webkit/support/webkit_support.gyp:webkit_support',
          ],
        }],  # OS=="android"
        ['os_posix==1 and use_aura==1 and linux_use_tcmalloc==1', {
          'dependencies': [
            # This is needed by content/app/content_main_runner.cc
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:aura',
            '../ui/base/strings/ui_strings.gyp:ui_strings',
            '../ui/views/controls/webview/webview.gyp:webview',
            '../ui/views/views.gyp:views',
            '../ui/views/views.gyp:views_test_support',
            '../ui/ui.gyp:ui_resources',
          ],
          'sources/': [
            ['exclude', 'src/shell_gtk.cc'],
            ['exclude', 'src/shell_win.cc'],
          ],
        }],  # use_aura==1
        ['chromeos==1', {
          'dependencies': [
            '../ash/ash.gyp:ash',
            '../chromeos/chromeos.gyp:chromeos',
           ],
        }], # chromeos==1
        ['use_custom_freetype==1', {
          'dependencies': [
             '../third_party/freetype2/freetype2.gyp:freetype2',
          ],
        }],
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
            'grit_grd_file': 'src/resources/cameo_resources.grd',
          },
          'includes': [ '../build/grit_action.gypi' ],
        },
      ],
    },
    {
      # We build a minimal set of resources so WebKit in content_shell has
      # access to necessary resources.
      'target_name': 'cameo_pak',
      'type': 'none',
      'dependencies': [
        '../content/browser/devtools/devtools_resources.gyp:devtools_resources',
        'cameo_resources',
        '../ui/ui.gyp:ui_resources',
      ],
      'variables': {
        'repack_path': '../tools/grit/grit/format/repack.py',
      },
      'actions': [
        {
          'action_name': 'repack_cameo_pack',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/cameo/cameo_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/app_locale_settings/app_locale_settings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/webui_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_strings/ui_strings_en-US.pak',
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
          'conditions': [
            ['OS!="android"', {
              'outputs': [
                '<(PRODUCT_DIR)/cameo.pak',
              ],
            }, {
              'outputs': [
                '<(PRODUCT_DIR)/content_assets/cameo.pak',
              ],
            }],
          ],
        },
      ],
    },
    {
      'target_name': 'cameo',
      'type': 'executable',
      'mac_bundle': 1,
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        'content_shell_lib',
        'cameo_pak',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'src/app/shell_main.cc',
      ],
      'mac_bundle_resources': [
        'src/mac/app.icns',
        'src/mac/app-Info.plist',
      ],
      # TODO(mark): Come up with a fancier way to do this.  It should only
      # be necessary to list app-Info.plist once, not the three times it is
      # listed here.
      'mac_bundle_resources!': [
        'src/mac/app-Info.plist',
      ],
      'xcode_settings': {
        'INFOPLIST_FILE': 'src/mac/app-Info.plist',
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
            'src/browser/shell.rc',
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
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '../build/linux/system.gyp:gtk',
          ],
        }],  # toolkit_uses_gtk
        ['OS=="mac"', {
          'product_name': '<(content_shell_product_name)',
          'dependencies!': [
            'content_shell_lib',
          ],
          'dependencies': [
            'content_shell_framework',
            'content_shell_helper_app',
          ],
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/<(content_shell_product_name).app/Contents/Frameworks',
              'files': [
                '<(PRODUCT_DIR)/<(content_shell_product_name) Helper.app',
              ],
            },
          ],
          'postbuilds': [
            {
              'postbuild_name': 'Copy <(content_shell_product_name) Framework.framework',
              'action': [
                '../build/mac/copy_framework_unversioned.sh',
                '${BUILT_PRODUCTS_DIR}/<(content_shell_product_name) Framework.framework',
                '${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/Frameworks',
              ],
            },
            {
              'postbuild_name': 'Fix Framework Link',
              'action': [
                'install_name_tool',
                '-change',
                '/Library/Frameworks/<(content_shell_product_name) Framework.framework/Versions/A/<(content_shell_product_name) Framework',
                '@executable_path/../Frameworks/<(content_shell_product_name) Framework.framework/<(content_shell_product_name) Framework',
                '${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}'
              ],
            },
            {
              # Modify the Info.plist as needed.
              'postbuild_name': 'Tweak Info.plist',
              'action': ['../build/mac/tweak_info_plist.py',
                         '--scm=1',
                         '--version=<(content_shell_version)'],
            },
            {
              # This postbuid step is responsible for creating the following
              # helpers:
              #
              # Content Shell Helper EH.app and Content Shell Helper NP.app are
              # created from Content Shell Helper.app.
              #
              # The EH helper is marked for an executable heap. The NP helper
              # is marked for no PIE (ASLR).
              'postbuild_name': 'Make More Helpers',
              'action': [
                '../build/mac/make_more_helpers.sh',
                'Frameworks',
                '<(content_shell_product_name)',
              ],
            },
            {
              # Make sure there isn't any Objective-C in the shell's
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
  ],
  'conditions': [
    ['OS=="mac"', {
      'targets': [
        {
          'target_name': 'content_shell_framework',
          'type': 'shared_library',
          'product_name': '<(content_shell_product_name) Framework',
          'mac_bundle': 1,
          'mac_bundle_resources': [
            'src/mac/English.lproj/HttpAuth.xib',
            'src/mac/English.lproj/MainMenu.xib',
            '<(PRODUCT_DIR)/cameo.pak'
          ],
          'dependencies': [
            'content_shell_lib',
          ],
          'include_dirs': [
            '..',
          ],
          'sources': [
            'src/app/shell_content_main.cc',
            'src/app/shell_content_main.h',
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
                ['libpeer_target_type=="shared_library"', {
                  'copies': [{
                   'destination': '<(PRODUCT_DIR)/$(CONTENTS_FOLDER_PATH)/Libraries',
                   'files': [
                      '<(PRODUCT_DIR)/Libraries/libpeerconnection.dylib',
                    ],
                  }],
                }],
              ],
            }],
          ],
        },  # target content_shell_framework
        {
          'target_name': 'content_shell_helper_app',
          'type': 'executable',
          'variables': { 'enable_wexit_time_destructors': 1, },
          'product_name': '<(content_shell_product_name) Helper',
          'mac_bundle': 1,
          'dependencies': [
            'content_shell_framework',
          ],
          'sources': [
            'src/app/shell_main.cc',
            'src/mac/helper-Info.plist',
          ],
          # TODO(mark): Come up with a fancier way to do this.  It should only
          # be necessary to list helper-Info.plist once, not the three times it
          # is listed here.
          'mac_bundle_resources!': [
            'src/mac/helper-Info.plist',
          ],
          # TODO(mark): For now, don't put any resources into this app.  Its
          # resources directory will be a symbolic link to the browser app's
          # resources directory.
          'mac_bundle_resources/': [
            ['exclude', '.*'],
          ],
          'xcode_settings': {
            'INFOPLIST_FILE': 'src/mac/helper-Info.plist',
          },
          'postbuilds': [
            {
              # The framework defines its load-time path
              # (DYLIB_INSTALL_NAME_BASE) relative to the main executable
              # (chrome).  A different relative path needs to be used in
              # content_shell_helper_app.
              'postbuild_name': 'Fix Framework Link',
              'action': [
                'install_name_tool',
                '-change',
                '/Library/Frameworks/<(content_shell_product_name) Framework.framework/Versions/A/<(content_shell_product_name) Framework',
                '@executable_path/../../../../Frameworks/<(content_shell_product_name) Framework.framework/<(content_shell_product_name) Framework',
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
                         '--version=<(content_shell_version)'],
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
                  # Get back from Content Shell.app/Contents/Frameworks/
                  #                                 Helper.app/Contents/MacOS
                  '@loader_path/../../../../../..',
                ],
              },
            }],
          ],
        },  # target content_shell_helper_app
      ],
    }],  # OS=="mac"
    ['OS=="android"', {
      'targets': [
        {
          # TODO(jrg): Update this action and other jni generators to only
          # require specifying the java directory and generate the rest.
          'target_name': 'content_shell_jni_headers',
          'type': 'none',
          'sources': [
            'src/android/browsertests_apk/src/org/chromium/content_browsertests_apk/BrowserTestSystemMessageHandler.java',
            'src/android/browsertests_apk/src/org/chromium/content_browsertests_apk/ContentBrowserTestsActivity.java',
            'src/android/java/src/org/chromium/content_ShellManager.java',
            'src/android/java/src/org/chromium/content_Shell.java',
          ],
          'direct_dependent_settings': {
            'include_dirs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/shell',
            ],
          },
          'variables': {
            'jni_gen_package': 'content/shell',
          },
          'includes': [ '../build/jni_generator.gypi' ],
        },
        {
          'target_name': 'libcontent_shell_content_view',
          'type': 'shared_library',
          'dependencies': [
            'content_shell_jni_headers',
            'content_shell_lib',
            'cameo_pak',
            # Skia is necessary to ensure the dependencies needed by
            # WebContents are included.
            '../skia/skia.gyp:skia',
            '../media/media.gyp:player_android',
          ],
          'sources': [
            'src/android/shell_library_loader.cc',
            'src/android/shell_library_loader.h',
          ],
          'conditions': [
            ['android_webview_build==1', {
              'ldflags': [
                '-lgabi++',  # For rtti
              ],
            }],
          ],
        },
        {
          'target_name': 'content_shell_java',
          'type': 'none',
          'dependencies': [
            'content_java',
          ],
          'variables': {
            'java_in_dir': '../content/android/java',
            'has_java_resources': 1,
            'R_package': 'org.chromium.content_shell',
            'R_package_relpath': 'org/chromium/content_shell',
          },
          'includes': [ '../build/java.gypi' ],
        },
        {
          # content_shell_apk creates a .jar as a side effect. Any java targets
          # that need that .jar in their classpath should depend on this target,
          # content_shell_apk_java. Dependents of content_shell_apk receive its
          # jar path in the variable 'apk_output_jar_path'. This target should
          # only be used by targets which instrument content_shell_apk.
          'target_name': 'content_shell_apk_java',
          'type': 'none',
          'dependencies': [
            'content_shell_apk',
          ],
          'includes': [ '../build/apk_fake_jar.gypi' ],
        },
        {
          'target_name': 'content_shell_apk',
          'type': 'none',
          'dependencies': [
            'content_java',
            'content_shell_java',
            'libcontent_shell_content_view',
            '../base/base.gyp:base_java',
            '../media/media.gyp:media_java',
            '../net/net.gyp:net_java',
            '../ui/ui.gyp:ui_java',
          ],
          'variables': {
            'apk_name': 'ContentShell',
            'manifest_package_name': 'org.chromium.content_shell_apk',
            'java_in_dir': 'android/shell_apk',
            'resource_dir': 'android/shell_apk/res',
            'native_lib_target': 'libcontent_shell_content_view',
            'additional_input_paths': ['<(PRODUCT_DIR)/content_assets/cameo.pak'],
            'asset_location': '<(ant_build_out)/content_assets',
          },
          'includes': [ '../build/java_apk.gypi' ],
        },
      ],
    }],  # OS=="android"
  ]
}
