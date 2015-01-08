{
  'variables': {
    'xwalk_product_name': 'XWalk',
    'xwalk_version': '<!(python ../build/util/version.py -f VERSION -t "@MAJOR@.@MINOR@.@BUILD@.@PATCH@")',
    'chrome_version': '<!(python ../build/util/version.py -f ../chrome/VERSION -t "@MAJOR@.@MINOR@.@BUILD@.@PATCH@")',
    'conditions': [
      ['OS=="win" or OS=="mac"', {
        'disable_nacl': 1,
      }],
      ['OS=="android"', {
        'enable_extensions': 1,
      }],
    ], # conditions
  },
  'includes' : [
    'xwalk_tests.gypi',
    'application/xwalk_application.gypi',
  ],
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
        '../ui/snapshot/snapshot.gyp:snapshot',
        '../url/url.gyp:url_lib',
        '../v8/tools/gyp/v8.gyp:v8',
        'generate_upstream_blink_version',
        'xwalk_application_lib',
        'xwalk_resources',
        'extensions/extensions.gyp:xwalk_extensions',
        'sysapps/sysapps.gyp:sysapps',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        '../extensions/common/constants.cc',
        '../extensions/common/constants.h',
        '../extensions/common/url_pattern.cc',
        '../extensions/common/url_pattern.h',
        'experimental/native_file_system/native_file_system_extension.cc',
        'experimental/native_file_system/native_file_system_extension.h',
        'experimental/native_file_system/virtual_root_provider_mac.cc',
        'experimental/native_file_system/virtual_root_provider_tizen.cc',
        'experimental/native_file_system/virtual_root_provider_win.cc',
        'experimental/native_file_system/virtual_root_provider.cc',
        'experimental/native_file_system/virtual_root_provider.h',
        'runtime/app/android/xwalk_main_delegate_android.cc',
        'runtime/app/android/xwalk_main_delegate_android.h',
        'runtime/app/xwalk_main_delegate.cc',
        'runtime/app/xwalk_main_delegate.h',
        'runtime/browser/android/cookie_manager.cc',
        'runtime/browser/android/cookie_manager.h',
        'runtime/browser/android/intercepted_request_data.h',
        'runtime/browser/android/intercepted_request_data_impl.cc',
        'runtime/browser/android/intercepted_request_data_impl.h',
        'runtime/browser/android/net/android_protocol_handler.cc',
        'runtime/browser/android/net/android_protocol_handler.h',
        'runtime/browser/android/net/android_stream_reader_url_request_job.cc',
        'runtime/browser/android/net/android_stream_reader_url_request_job.h',
        'runtime/browser/android/net/input_stream.h',
        'runtime/browser/android/net/input_stream_impl.cc',
        'runtime/browser/android/net/input_stream_impl.h',
        'runtime/browser/android/net/input_stream_reader.cc',
        'runtime/browser/android/net/input_stream_reader.h',
        'runtime/browser/android/net/url_constants.cc',
        'runtime/browser/android/net/url_constants.h',
        'runtime/browser/android/net/xwalk_url_request_job_factory.cc',
        'runtime/browser/android/net/xwalk_url_request_job_factory.h',
        'runtime/browser/android/net_disk_cache_remover.cc',
        'runtime/browser/android/net_disk_cache_remover.h',
        'runtime/browser/android/renderer_host/xwalk_render_view_host_ext.cc',
        'runtime/browser/android/renderer_host/xwalk_render_view_host_ext.h',
        'runtime/browser/android/state_serializer.cc',
        'runtime/browser/android/state_serializer.h',
        'runtime/browser/android/xwalk_content.cc',
        'runtime/browser/android/xwalk_content.h',
        'runtime/browser/android/xwalk_contents_client_bridge.cc',
        'runtime/browser/android/xwalk_contents_client_bridge.h',
        'runtime/browser/android/xwalk_contents_client_bridge_base.cc',
        'runtime/browser/android/xwalk_contents_client_bridge_base.h',
        'runtime/browser/android/xwalk_contents_io_thread_client.h',
        'runtime/browser/android/xwalk_contents_io_thread_client_impl.cc',
        'runtime/browser/android/xwalk_contents_io_thread_client_impl.h',
        'runtime/browser/android/xwalk_cookie_access_policy.cc',
        'runtime/browser/android/xwalk_cookie_access_policy.h',
        'runtime/browser/android/xwalk_dev_tools_server.cc',
        'runtime/browser/android/xwalk_dev_tools_server.h',
        'runtime/browser/android/xwalk_download_resource_throttle.cc',
        'runtime/browser/android/xwalk_download_resource_throttle.h',
        'runtime/browser/android/xwalk_http_auth_handler.cc',
        'runtime/browser/android/xwalk_http_auth_handler.h',
        'runtime/browser/android/xwalk_http_auth_handler_base.cc',
        'runtime/browser/android/xwalk_http_auth_handler_base.h',
        'runtime/browser/android/xwalk_login_delegate.cc',
        'runtime/browser/android/xwalk_login_delegate.h',
        'runtime/browser/android/xwalk_path_helper.cc',
        'runtime/browser/android/xwalk_path_helper.h',
        'runtime/browser/android/xwalk_icon_helper.cc',
        'runtime/browser/android/xwalk_icon_helper.h',
        'runtime/browser/android/xwalk_request_interceptor.cc',
        'runtime/browser/android/xwalk_request_interceptor.h',
        'runtime/browser/android/xwalk_settings.cc',
        'runtime/browser/android/xwalk_view_delegate.cc',
        'runtime/browser/android/xwalk_view_delegate.h',
        'runtime/browser/android/xwalk_web_contents_delegate.cc',
        'runtime/browser/android/xwalk_web_contents_delegate.h',
        'runtime/browser/android/xwalk_web_contents_view_delegate.cc',
        'runtime/browser/android/xwalk_web_contents_view_delegate.h',
        'runtime/browser/application_component.cc',
        'runtime/browser/application_component.h',
        'runtime/browser/devtools/remote_debugging_server.cc',
        'runtime/browser/devtools/remote_debugging_server.h',
        'runtime/browser/devtools/xwalk_devtools_delegate.cc',
        'runtime/browser/devtools/xwalk_devtools_delegate.h',
        'runtime/browser/geolocation/tizen/location_provider_tizen.cc',
        'runtime/browser/geolocation/tizen/location_provider_tizen.h',
        'runtime/browser/geolocation/xwalk_access_token_store.cc',
        'runtime/browser/geolocation/xwalk_access_token_store.h',
        'runtime/browser/image_util.cc',
        'runtime/browser/image_util.h',
        'runtime/browser/media/media_capture_devices_dispatcher.cc',
        'runtime/browser/media/media_capture_devices_dispatcher.h',
        'runtime/browser/renderer_host/pepper/xwalk_browser_pepper_host_factory.cc',
        'runtime/browser/renderer_host/pepper/xwalk_browser_pepper_host_factory.h',
        'runtime/browser/runtime.cc',
        'runtime/browser/runtime.h',
        'runtime/browser/runtime_download_manager_delegate.cc',
        'runtime/browser/runtime_download_manager_delegate.h',
        'runtime/browser/runtime_file_select_helper.cc',
        'runtime/browser/runtime_file_select_helper.h',
        'runtime/browser/runtime_geolocation_permission_context.cc',
        'runtime/browser/runtime_geolocation_permission_context.h',
        'runtime/browser/runtime_javascript_dialog_manager.cc',
        'runtime/browser/runtime_javascript_dialog_manager.h',
        'runtime/browser/runtime_network_delegate.cc',
        'runtime/browser/runtime_network_delegate.h',
        'runtime/browser/runtime_platform_util.h',
        'runtime/browser/runtime_platform_util_android.cc',
        'runtime/browser/runtime_platform_util_aura.cc',
        'runtime/browser/runtime_platform_util_linux.cc',
        'runtime/browser/runtime_platform_util_mac.mm',
        'runtime/browser/runtime_platform_util_tizen.cc',
        'runtime/browser/runtime_platform_util_win.cc',
        'runtime/browser/runtime_quota_permission_context.cc',
        'runtime/browser/runtime_quota_permission_context.h',
        'runtime/browser/runtime_resource_dispatcher_host_delegate.cc',
        'runtime/browser/runtime_resource_dispatcher_host_delegate.h',
        'runtime/browser/runtime_resource_dispatcher_host_delegate_android.cc',
        'runtime/browser/runtime_resource_dispatcher_host_delegate_android.h',
        'runtime/browser/runtime_select_file_policy.cc',
        'runtime/browser/runtime_select_file_policy.h',
        'runtime/browser/runtime_ui_delegate.cc',
        'runtime/browser/runtime_ui_delegate.h',
        'runtime/browser/runtime_url_request_context_getter.cc',
        'runtime/browser/runtime_url_request_context_getter.h',
        'runtime/browser/speech/speech_recognition_manager_delegate.cc',
        'runtime/browser/speech/speech_recognition_manager_delegate.h',
        'runtime/browser/sysapps_component.cc',
        'runtime/browser/sysapps_component.h',
        'runtime/browser/storage_component.cc',
        'runtime/browser/storage_component.h',
        'runtime/browser/ui/color_chooser.cc',
        'runtime/browser/ui/color_chooser.h',
        'runtime/browser/ui/color_chooser_android.cc',
        'runtime/browser/ui/color_chooser_aura.cc',
        'runtime/browser/ui/color_chooser_mac.cc',
        'runtime/browser/ui/native_app_window.cc',
        'runtime/browser/ui/native_app_window.h',
        'runtime/browser/ui/native_app_window_aura.cc',
        'runtime/browser/ui/native_app_window_aura.h',
        'runtime/browser/ui/native_app_window_android.cc',
        'runtime/browser/ui/native_app_window_mac.h',
        'runtime/browser/ui/native_app_window_mac.mm',
        'runtime/browser/ui/native_app_window_tizen.cc',
        'runtime/browser/ui/native_app_window_tizen.h',
        'runtime/browser/ui/native_app_window_views.cc',
        'runtime/browser/ui/native_app_window_views.h',
        'runtime/browser/ui/splash_screen_tizen.cc',
        'runtime/browser/ui/splash_screen_tizen.h',
        'runtime/browser/ui/taskbar_util.h',
        'runtime/browser/ui/taskbar_util_win.cc',
        'runtime/browser/ui/top_view_layout_views.cc',
        'runtime/browser/ui/top_view_layout_views.h',
        'runtime/browser/ui/xwalk_views_delegate.cc',
        'runtime/browser/ui/xwalk_views_delegate.h',
        'runtime/browser/xwalk_app_extension_bridge.cc',
        'runtime/browser/xwalk_app_extension_bridge.h',
        'runtime/browser/xwalk_application_mac.h',
        'runtime/browser/xwalk_application_mac.mm',
        'runtime/browser/xwalk_browser_context.cc',
        'runtime/browser/xwalk_browser_context.h',
        'runtime/browser/xwalk_browser_main_parts.cc',
        'runtime/browser/xwalk_browser_main_parts.h',
        'runtime/browser/xwalk_browser_main_parts_android.cc',
        'runtime/browser/xwalk_browser_main_parts_android.h',
        'runtime/browser/xwalk_browser_main_parts_mac.h',
        'runtime/browser/xwalk_browser_main_parts_mac.mm',
        'runtime/browser/xwalk_browser_main_parts_tizen.cc',
        'runtime/browser/xwalk_browser_main_parts_tizen.h',
        'runtime/browser/xwalk_component.h',
        'runtime/browser/xwalk_content_browser_client.cc',
        'runtime/browser/xwalk_content_browser_client.h',
        'runtime/browser/xwalk_render_message_filter.cc',
        'runtime/browser/xwalk_render_message_filter.h',
        'runtime/browser/xwalk_runner.cc',
        'runtime/browser/xwalk_runner.h',
        'runtime/browser/xwalk_runner_android.cc',
        'runtime/browser/xwalk_runner_android.h',
        'runtime/browser/xwalk_runner_tizen.cc',
        'runtime/browser/xwalk_runner_tizen.h',
        'runtime/browser/xwalk_ssl_host_state_delegate.cc',
        'runtime/browser/xwalk_ssl_host_state_delegate.h',
        'runtime/common/android/xwalk_globals_android.cc',
        'runtime/common/android/xwalk_globals_android.h',
        'runtime/common/android/xwalk_hit_test_data.cc',
        'runtime/common/android/xwalk_hit_test_data.h',
        'runtime/common/android/xwalk_message_generator.cc',
        'runtime/common/android/xwalk_message_generator.h',
        'runtime/common/android/xwalk_render_view_messages.cc',
        'runtime/common/android/xwalk_render_view_messages.h',
        'runtime/common/paths_mac.h',
        'runtime/common/paths_mac.mm',
        'runtime/common/xwalk_common_messages.cc',
        'runtime/common/xwalk_common_messages.h',
        'runtime/common/xwalk_common_message_generator.cc',
        'runtime/common/xwalk_common_message_generator.h',
        'runtime/common/xwalk_content_client.cc',
        'runtime/common/xwalk_content_client.h',
        'runtime/common/xwalk_localized_error.cc',
        'runtime/common/xwalk_localized_error.h',
        'runtime/common/xwalk_paths.cc',
        'runtime/common/xwalk_paths.h',
        'runtime/common/xwalk_runtime_features.cc',
        'runtime/common/xwalk_runtime_features.h',
        'runtime/common/xwalk_switches.cc',
        'runtime/common/xwalk_switches.h',
        'runtime/common/xwalk_system_locale.cc',
        'runtime/common/xwalk_system_locale.h',
        'runtime/renderer/android/xwalk_render_process_observer.cc',
        'runtime/renderer/android/xwalk_render_process_observer.h',
        'runtime/renderer/android/xwalk_permission_client.cc',
        'runtime/renderer/android/xwalk_permission_client.h',
        'runtime/renderer/android/xwalk_render_view_ext.cc',
        'runtime/renderer/android/xwalk_render_view_ext.h',
        'runtime/renderer/isolated_file_system.cc',
        'runtime/renderer/isolated_file_system.h',
        'runtime/renderer/pepper/pepper_helper.cc',
        'runtime/renderer/pepper/pepper_helper.h',
        'runtime/renderer/pepper/pepper_uma_host.cc',
        'runtime/renderer/pepper/pepper_uma_host.h',
        'runtime/renderer/pepper/xwalk_renderer_pepper_host_factory.cc',
        'runtime/renderer/pepper/xwalk_renderer_pepper_host_factory.h',
        'runtime/renderer/tizen/xwalk_content_renderer_client_tizen.cc',
        'runtime/renderer/tizen/xwalk_content_renderer_client_tizen.h',
        'runtime/renderer/tizen/xwalk_render_view_ext_tizen.cc',
        'runtime/renderer/tizen/xwalk_render_view_ext_tizen.h',
        'runtime/renderer/xwalk_content_renderer_client.cc',
        'runtime/renderer/xwalk_content_renderer_client.h',
        'runtime/renderer/xwalk_render_process_observer_generic.cc',
        'runtime/renderer/xwalk_render_process_observer_generic.h',
      ],
      'includes': [
        'xwalk_jsapi.gypi',
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
            '../ui/compositor/compositor.gyp:compositor',
            'build/system.gyp:tizen_geolocation',
            'build/system.gyp:tizen_tzplatform_config',
            'sysapps/sysapps_resources.gyp:xwalk_sysapps_resources',
            'tizen/xwalk_tizen.gypi:xwalk_tizen_lib',
            '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
            '../components/components.gyp:web_modal',
            '../components/components.gyp:renderer_context_menu',
          ],
          'sources': [
            'runtime/browser/tizen/tizen_locale_listener.cc',
            'runtime/browser/tizen/tizen_locale_listener.h',
            'runtime/browser/tizen/xwalk_web_contents_view_delegate.cc',
            'runtime/browser/tizen/xwalk_web_contents_view_delegate.h',
            'runtime/browser/tizen/render_view_context_menu_impl.cc',
            'runtime/browser/tizen/render_view_context_menu_impl.h',
          ],
          'sources!':[
            'runtime/browser/runtime_platform_util_linux.cc',
            'runtime/browser/android/xwalk_web_contents_view_delegate.cc',
            'runtime/browser/android/xwalk_web_contents_view_delegate.h',
          ],
        }],
        ['OS=="android"',{
          'dependencies':[
            'xwalk_core_jar_jni',
            'xwalk_core_native_jni',
          ],
          'sources': [
            'experimental/native_file_system/virtual_root_provider_android.cc',
          ],
          'sources!':[
            'runtime/renderer/xwalk_render_process_observer_generic.cc',
            'runtime/renderer/xwalk_render_process_observer_generic.h',
          ],
        }],
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
            '../build/linux/system.gyp:dbus',
          ],
          'sources': [
            'experimental/native_file_system/virtual_root_provider_linux.cc',
          ]
        }],  # OS=="linux"
        ['os_posix==1 and OS != "mac" and use_allocator=="tcmalloc"', {
          'dependencies': [
            # This is needed by content/app/content_main_runner.cc
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],  # os_posix==1 and OS != "mac" and use_allocator=="tcmalloc"
        ['toolkit_views==1', {
          'dependencies': [
            '../ui/events/events.gyp:events',
            '../ui/strings/ui_strings.gyp:ui_strings',
            '../ui/views/controls/webview/webview.gyp:webview',
            '../ui/views/views.gyp:views',
            '../ui/resources/ui_resources.gyp:ui_resources',
          ],
          'sources/': [
            ['exclude', 'runtime/browser/ui/native_app_window_aura.cc'],
          ],
        }, { # toolkit_views==0
          'sources/': [
            ['exclude', 'runtime/browser/ui/xwalk_views_delegate.cc'],
            ['exclude', 'runtime/browser/ui/color_chooser_aura.cc'],
          ],
        }],  # toolkit_views==1
        ['use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:aura',
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
      'target_name': 'generate_upstream_blink_version',
      'type': 'none',
      'actions': [
        {
          'action_name': 'generate_blink_upstream_version',
          'inputs': [
            '<(script)',
            '<(upstream)',
            '<(template)',
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/blink_upstream_version.h',
          ],
          'action': ['python',
                     '<(script)',
                     '-f', '<(upstream)',
                     '<(template)',
                     '<@(_outputs)',
          ],
          'variables': {
            'script': '<(DEPTH)/build/util/version.py',
            'upstream': '<(DEPTH)/xwalk/build/UPSTREAM.blink',
            'template': 'runtime/browser/blink_upstream_version.h.in',
          },
        },
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
        'xwalk_resources',
      ],
      'conditions': [
        [ 'OS!="android"', {
          'dependencies': [
            '<(DEPTH)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
          ],
        }],
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
              '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_application_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_extensions_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_sysapps_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/content/app/resources/content_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/resources/ui_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/strings/app_locale_settings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/strings/ui_strings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/blink/public/resources/blink_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/content/app/strings/content_strings_en-US.pak',
            ],
          },
          'conditions': [
            [ 'OS!="android"', {
              'variables': {
                'pak_inputs+': [
                  '<(SHARED_INTERMEDIATE_DIR)/webkit/devtools_resources.pak',
                ],
              },
            }],
            [ 'tizen_mobile == 1', {
              'variables': {
                'pak_inputs+': [
                  '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_sysapps_resources.pak',
                ],
              },
            }],
            [ 'tizen==1', {
              'variables': {
                'pak_inputs+': [
                  # Add WebUI resources for Tizen.
                  '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
                  '<(SHARED_INTERMEDIATE_DIR)/ui/resources/webui_resources.pak',
                ],
              },
            }],
          ],
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
        ['OS == "win"', {
          'dependencies': [
            '../sandbox/sandbox.gyp:sandbox',
          ],
        }],  # OS=="win"
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
      'conditions': [
        ['OS!="android"', {
          'dependencies': [
            'xwalk',
            'xwalk_all_tests',
          ],
        },
        {
          'dependencies': [
            # For internal testing.
            'xwalk_core_internal_shell_apk',
            'xwalk_core_internal_test_apk',
            'xwalk_core_shell_apk',
            'xwalk_core_test_apk',
            'xwalk_runtime_client_embedded_shell_apk',
            'xwalk_runtime_client_embedded_test_apk',
            'xwalk_runtime_client_shell_apk',
            'xwalk_runtime_client_test_apk',

            # For external testing.
            'pack_xwalk_core_library',
            'xwalk_core_library_documentation',
            'xwalk_runtime_lib_apk',
            'xwalk_app_hello_world_apk',
            'xwalk_app_template',
            'xwalk_core_sample_apk',
            'xwalk_core_library_aar',
            'xwalk_packaging_tool_test',
          ],
        }],
      ],
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
            ['android_app_abi=="x86_64"', {
              'version_code_shift%': 4,
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
