{
  'variables': {
    'xwalk_product_name': 'XWalk',
    'xwalk_version': '<!(python ../build/util/version.py -f VERSION -t "@MAJOR@.@MINOR@.@BUILD@.@PATCH@")',
    'chrome_version': '<!(python ../build/util/version.py -f ../chrome/VERSION -t "@MAJOR@.@MINOR@.@BUILD@.@PATCH@")',
    'conditions': [
      ['OS=="linux"', {
       'use_custom_freetype%': 1,
      }, {
       'use_custom_freetype%': 0,
      }],
      ['OS=="win" or OS=="mac"', {
        'disable_nacl': 1,
      }],
    ], # conditions
  },
  'targets': [
    {
      'target_name': 'xwalk_runtime',
      'type': 'static_library',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'defines': ['XWALK_VERSION="<(xwalk_version)"','CHROME_VERSION="<(chrome_version)"'],
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../cc/cc.gyp:cc',
        '../components/components.gyp:visitedlink_browser',
        '../components/components.gyp:visitedlink_renderer',
        '../content/content.gyp:content',
        '../content/content.gyp:content_app_both',
        '../content/content.gyp:content_browser',
        '../content/content.gyp:content_common',
        '../content/content.gyp:content_gpu',
        '../content/content.gyp:content_plugin',
        '../content/content.gyp:content_ppapi_plugin',
        '../content/content.gyp:content_renderer',
        '../content/content.gyp:content_utility',
        '../ipc/ipc.gyp:ipc',
        '../media/media.gyp:media',
        '../net/net.gyp:net',
        '../net/net.gyp:net_resources',
        '../skia/skia.gyp:skia',
        '../storage/storage_browser.gyp:storage',
        '../storage/storage_common.gyp:storage_common',
        '../third_party/WebKit/public/blink.gyp:blink',
        '../ui/base/ui_base.gyp:ui_base',
        '../ui/gl/gl.gyp:gl',
        '../ui/shell_dialogs/shell_dialogs.gyp:shell_dialogs',
        '../url/url.gyp:url_lib',
        '../v8/tools/gyp/v8.gyp:v8',
      ],
      'include_dirs': [
        '..',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
        },
      },
      'conditions': [
        ['tizen==1', {
          'dependencies': [
            '../content/app/resources/content_resources.gyp:content_resources',
            '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
            '../components/components.gyp:web_modal',
            '../components/components.gyp:renderer_context_menu',
          ],
        }],
        ['os_posix==1 and OS != "mac" and use_allocator=="tcmalloc"', {
          'dependencies': [
            # This is needed by content/app/content_main_runner.cc
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],  # os_posix==1 and OS != "mac" and use_allocator=="tcmalloc"
        ['use_custom_freetype==1', {
          'dependencies': [
             '../third_party/freetype2/freetype2.gyp:freetype2',
          ],
        }],  # use_custom_freetype==1
        ['toolkit_views==1', {
          'dependencies': [
            '../ui/strings/ui_strings.gyp:ui_strings',
            '../ui/views/controls/webview/webview.gyp:webview',
            '../ui/views/views.gyp:views',
            '../ui/views/views.gyp:views_test_support',
            '../ui/resources/ui_resources.gyp:ui_resources',
          ],
        }],  # toolkit_views==1
        ['use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:aura',
          ],
        }, {  # use_aura==0
          'sources/': [
            ['exclude', '_aura\\.cc$'],
          ],
        }],
        ['disable_nacl==0', {
            'conditions': [
                ['OS=="linux"', {
                  'sources': [
                    'runtime/browser/nacl_host/nacl_browser_delegate_impl.cc',
                    'runtime/browser/nacl_host/nacl_browser_delegate_impl.h',
                  ],
                  'dependencies': [
                    '../components/nacl.gyp:nacl',
                    '../components/nacl.gyp:nacl_browser',
                    '../components/nacl.gyp:nacl_common',
                    '../components/nacl.gyp:nacl_renderer',
                    '../components/nacl.gyp:nacl_helper',
                    '../components/nacl.gyp:nacl_linux',
                    '../native_client/src/trusted/service_runtime/linux/nacl_bootstrap.gyp:nacl_helper_bootstrap',
                    '../ppapi/native_client/src/trusted/plugin/plugin.gyp:nacl_trusted_plugin',
                  ],
                }],
            ],
        }],
        ['enable_plugins==1', {
          'dependencies': [
            '../ppapi/ppapi_internal.gyp:ppapi_host',
            '../ppapi/ppapi_internal.gyp:ppapi_proxy',
            '../ppapi/ppapi_internal.gyp:ppapi_ipc',
            '../ppapi/ppapi_internal.gyp:ppapi_shared',
          ],
        }, {  # enable_plugins==0
          'sources/': [
            ['exclude', '^runtime/browser/renderer_host/pepper/'],
            ['exclude', '^runtime/renderer/pepper/'],
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
            'grit_resource_ids': 'resources/resource_ids',
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
        '<(DEPTH)/ui/strings/ui_strings.gyp:ui_strings',
        '<(DEPTH)/ui/resources/ui_resources.gyp:ui_resources',
        '<(DEPTH)/content/app/resources/content_resources.gyp:content_resources',
      ],
      'conditions': [
        [ 'OS!="android"', {
          'dependencies': [
            '<(DEPTH)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
          ],
        }],
      ],
    },
    {
      'target_name': 'xwalk-thirdparty',
      'type': 'none',
      'mac_bundle': 1,
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'dependencies': [
        'xwalk_runtime',
        'xwalk_pak',
      ],
      'include_dirs': [
        '..',
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
    },
  ], # targets
  'conditions': [
    ['OS=="linux"', {
      'includes': [ 'extensions/xesh/xesh.gypi' ],
    }],
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
            ['icu_use_data_file_flag==1', {
              'mac_bundle_resources': [
                '<(PRODUCT_DIR)/icudtl.dat',
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
    ['OS=="android"', {
      'variables': {
        'variables': {
          'conditions': [
            ['android_app_abi=="x86"', {
              'version_code_shift%': 1,
            }],
            ['android_app_abi=="armeabi-v7a"', {
              'version_code_shift%': 2,
            }],
            ['android_app_abi=="armeabi"', {
              'version_code_shift%': 3,
            }],
          ], # conditions
        },
        'version_code_shift%': '<(version_code_shift)',
        'xwalk_version_code': '<!(python tools/build/android/generate_version_code.py -f VERSION -s <(version_code_shift))',
      },
      'includes': [
        'xwalk_android.gypi',
        'xwalk_android_tests.gypi',
        'xwalk_android_app.gypi',
        'xwalk_core_library_android.gypi',
      ],
      'targets': [
      {
        'target_name': 'All',
        'type': 'none',
        'dependencies': [
          'xwalk',
        ],
      }, # target_name: All
    ],  # targets
    }], # OS=="android"
  ]
}
