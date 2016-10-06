{
  'variables': {
    'xwalk_product_name': 'XWalk',
    'xwalk_version': '<!(python ../build/util/version.py -f VERSION -t "@MAJOR@.@MINOR@.@BUILD@.@PATCH@")',
    'chrome_version': '<!(python ../build/util/version.py -f ../chrome/VERSION -t "@MAJOR@.@MINOR@.@BUILD@.@PATCH@")',
  },
  'includes' : [
    'xwalk_tests.gypi',
    'application/xwalk_application.gypi',
    'extensions/xesh/xesh.gypi',
    'xwalk_installer.gypi',
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
        '../base/base.gyp:base_static',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../cc/cc.gyp:cc',
        '../components/components.gyp:autofill_content_browser',
        '../components/components.gyp:autofill_content_renderer',
        '../components/components.gyp:autofill_core_browser',
        '../components/components.gyp:cdm_renderer',
        '../components/components_resources.gyp:components_resources',
        '../components/components_strings.gyp:components_strings',
        '../components/components.gyp:devtools_http_handler',
        '../components/components.gyp:user_prefs',
        '../components/components.gyp:visitedlink_browser',
        '../components/components.gyp:visitedlink_renderer',
        '../components/url_formatter/url_formatter.gyp:url_formatter',
        '../content/content.gyp:content',
        '../content/content.gyp:content_app_both',
        '../content/content.gyp:content_browser',
        '../content/content.gyp:content_common',
        '../content/content.gyp:content_gpu',
        '../content/content.gyp:content_ppapi_plugin',
        '../content/content.gyp:content_renderer',
        '../content/content.gyp:content_utility',
        '../gin/gin.gyp:gin',
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
        '../v8/src/v8.gyp:v8',
        'generate_upstream_blink_version',
        'xwalk_application_lib',
        'xwalk_pak',
        'xwalk_resources',
        'extensions/extensions.gyp:xwalk_extensions',
        'sysapps/sysapps.gyp:sysapps',
        '../third_party/boringssl/boringssl.gyp:boringssl',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        '../extensions/common/constants.cc',
        '../extensions/common/constants.h',
        '../extensions/common/url_pattern.cc',
        '../extensions/common/url_pattern.h',
        'experimental/native_file_system/native_file_system.idl',
        'experimental/native_file_system/native_file_system_extension.cc',
        'experimental/native_file_system/native_file_system_extension.h',
        'experimental/native_file_system/virtual_root_provider.cc',
        'experimental/native_file_system/virtual_root_provider.h',
        'experimental/native_file_system/virtual_root_provider_android.cc',
        'experimental/native_file_system/virtual_root_provider_linux.cc',
        'experimental/native_file_system/virtual_root_provider_mac.cc',
        'experimental/native_file_system/virtual_root_provider_win.cc',
        'runtime/app/android/xwalk_main_delegate_android.cc',
        'runtime/app/android/xwalk_main_delegate_android.h',
        'runtime/app/xwalk_main_delegate.cc',
        'runtime/app/xwalk_main_delegate.h',
        'runtime/browser/android/cookie_manager.cc',
        'runtime/browser/android/cookie_manager.h',
        'runtime/browser/android/find_helper.cc',
        'runtime/browser/android/find_helper.h',
        'runtime/browser/android/net/android_protocol_handler.cc',
        'runtime/browser/android/net/android_protocol_handler.h',
        'runtime/browser/android/net/android_stream_reader_url_request_job.cc',
        'runtime/browser/android/net/android_stream_reader_url_request_job.h',
        'runtime/browser/android/net/init_native_callback.h',
        'runtime/browser/android/net/input_stream.h',
        'runtime/browser/android/net/input_stream_impl.cc',
        'runtime/browser/android/net/input_stream_impl.h',
        'runtime/browser/android/net/input_stream_reader.cc',
        'runtime/browser/android/net/input_stream_reader.h',
        'runtime/browser/android/net/url_constants.cc',
        'runtime/browser/android/net/url_constants.h',
        'runtime/browser/android/net/xwalk_cookie_store_wrapper.cc',
        'runtime/browser/android/net/xwalk_cookie_store_wrapper.h',
        'runtime/browser/android/net/xwalk_url_request_job_factory.cc',
        'runtime/browser/android/net/xwalk_url_request_job_factory.h',
        'runtime/browser/android/net_disk_cache_remover.cc',
        'runtime/browser/android/net_disk_cache_remover.h',
        'runtime/browser/android/renderer_host/xwalk_render_view_host_ext.cc',
        'runtime/browser/android/renderer_host/xwalk_render_view_host_ext.h',
        'runtime/browser/android/scoped_allow_wait_for_legacy_web_view_api.h',
        'runtime/browser/android/state_serializer.cc',
        'runtime/browser/android/state_serializer.h',
        'runtime/browser/android/xwalk_autofill_client_android.cc',
        'runtime/browser/android/xwalk_autofill_client_android.h',
        'runtime/browser/android/xwalk_content.cc',
        'runtime/browser/android/xwalk_content.h',
        'runtime/browser/android/xwalk_content_lifecycle_notifier.cc',
        'runtime/browser/android/xwalk_content_lifecycle_notifier.h',
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
        'runtime/browser/android/xwalk_http_auth_handler.cc',
        'runtime/browser/android/xwalk_http_auth_handler.h',
        'runtime/browser/android/xwalk_http_auth_handler_base.cc',
        'runtime/browser/android/xwalk_http_auth_handler_base.h',
        'runtime/browser/android/xwalk_login_delegate.cc',
        'runtime/browser/android/xwalk_login_delegate.h',
        'runtime/browser/android/xwalk_path_helper.cc',
        'runtime/browser/android/xwalk_path_helper.h',
        'runtime/browser/android/xwalk_presentation_host.cc',
        'runtime/browser/android/xwalk_presentation_host.h',
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
        'runtime/browser/android/xwalk_web_resource_response.cc',
        'runtime/browser/android/xwalk_web_resource_response.h',
        'runtime/browser/android/xwalk_web_resource_response_impl.cc',
        'runtime/browser/android/xwalk_web_resource_response_impl.h',
        'runtime/browser/application_component.cc',
        'runtime/browser/application_component.h',
        'runtime/browser/devtools/remote_debugging_server.cc',
        'runtime/browser/devtools/remote_debugging_server.h',
        'runtime/browser/devtools/xwalk_devtools_frontend.cc',
        'runtime/browser/devtools/xwalk_devtools_frontend.h',
        'runtime/browser/devtools/xwalk_devtools_manager_delegate.cc',
        'runtime/browser/devtools/xwalk_devtools_manager_delegate.h',
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
        'runtime/browser/runtime_notification_permission_context.cc',
        'runtime/browser/runtime_notification_permission_context.h',
        'runtime/browser/runtime_platform_util.h',
        'runtime/browser/runtime_platform_util_android.cc',
        'runtime/browser/runtime_platform_util_aura.cc',
        'runtime/browser/runtime_platform_util_linux.cc',
        'runtime/browser/runtime_platform_util_mac.mm',
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
        'runtime/browser/runtime_ui_delegate_desktop.cc',
        'runtime/browser/runtime_ui_delegate_desktop.h',
        'runtime/browser/runtime_url_request_context_getter.cc',
        'runtime/browser/runtime_url_request_context_getter.h',
        'runtime/browser/speech/speech_recognition_manager_delegate.cc',
        'runtime/browser/speech/speech_recognition_manager_delegate.h',
        'runtime/browser/ssl_error_page.cc',
        'runtime/browser/ssl_error_page.h',
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
        'runtime/browser/ui/native_app_window_android.cc',
        'runtime/browser/ui/native_app_window_desktop.cc',
        'runtime/browser/ui/native_app_window_desktop.h',
        'runtime/browser/ui/native_app_window_mac.h',
        'runtime/browser/ui/native_app_window_mac.mm',
        'runtime/browser/ui/native_app_window_views.cc',
        'runtime/browser/ui/native_app_window_views.h',
        'runtime/browser/ui/desktop/exclusive_access_bubble_views.cc',
        'runtime/browser/ui/desktop/exclusive_access_bubble_views.h',
        'runtime/browser/ui/desktop/exclusive_access_bubble.cc',
        'runtime/browser/ui/desktop/exclusive_access_bubble.h',
        'runtime/browser/ui/desktop/exclusive_access_bubble_views_context.h',
        'runtime/browser/ui/desktop/download_views.cc',
        'runtime/browser/ui/desktop/download_views.h',
        'runtime/browser/ui/desktop/xwalk_autofill_popup_controller.cc',
        'runtime/browser/ui/desktop/xwalk_autofill_popup_controller.h',
        'runtime/browser/ui/desktop/xwalk_autofill_popup_view.cc',
        'runtime/browser/ui/desktop/xwalk_autofill_popup_view.h',
        'runtime/browser/ui/desktop/xwalk_permission_modal_dialog.cc',
        'runtime/browser/ui/desktop/xwalk_permission_modal_dialog.h',
        'runtime/browser/ui/desktop/xwalk_permission_modal_dialog_views.h',
        'runtime/browser/ui/desktop/xwalk_permission_modal_dialog_views.cc',
        'runtime/browser/ui/desktop/xwalk_permission_dialog_manager.cc',
        'runtime/browser/ui/desktop/xwalk_permission_dialog_manager.h',
        'runtime/browser/ui/desktop/xwalk_popup_controller.cc',
        'runtime/browser/ui/desktop/xwalk_popup_controller.h',
        'runtime/browser/ui/top_view_layout_views.cc',
        'runtime/browser/ui/top_view_layout_views.h',
        'runtime/browser/ui/xwalk_javascript_native_dialog_factory_views.cc',
        'runtime/browser/ui/xwalk_views_delegate.cc',
        'runtime/browser/ui/xwalk_views_delegate.h',
        'runtime/browser/wifidirect_component_win.cc',
        'runtime/browser/wifidirect_component_win.h',
        'runtime/browser/xwalk_app_extension_bridge.cc',
        'runtime/browser/xwalk_app_extension_bridge.h',
        'runtime/browser/xwalk_application_mac.h',
        'runtime/browser/xwalk_application_mac.mm',
        'runtime/browser/xwalk_autofill_client.cc',
        'runtime/browser/xwalk_autofill_client.h',
        'runtime/browser/xwalk_autofill_client_desktop.cc',
        'runtime/browser/xwalk_autofill_client_desktop.h',
        'runtime/browser/xwalk_browser_context.cc',
        'runtime/browser/xwalk_browser_context.h',
        'runtime/browser/xwalk_browser_main_parts.cc',
        'runtime/browser/xwalk_browser_main_parts.h',
        'runtime/browser/xwalk_browser_main_parts_android.cc',
        'runtime/browser/xwalk_browser_main_parts_android.h',
        'runtime/browser/xwalk_browser_main_parts_mac.h',
        'runtime/browser/xwalk_browser_main_parts_mac.mm',
        'runtime/browser/xwalk_component.h',
        'runtime/browser/xwalk_autofill_manager.cc',
        'runtime/browser/xwalk_autofill_manager.h',
        'runtime/browser/xwalk_content_browser_client.cc',
        'runtime/browser/xwalk_content_browser_client.h',
        'runtime/browser/xwalk_content_settings.cc',
        'runtime/browser/xwalk_content_settings.h',
        'runtime/browser/xwalk_form_database_service.cc',
        'runtime/browser/xwalk_form_database_service.h',
        'runtime/browser/xwalk_notification_manager_linux.cc',
        'runtime/browser/xwalk_notification_manager_linux.h',
        'runtime/browser/xwalk_notification_manager_win.cc',
        'runtime/browser/xwalk_notification_manager_win.h',
        'runtime/browser/xwalk_notification_win.cc',
        'runtime/browser/xwalk_notification_win.h',
        'runtime/browser/xwalk_permission_manager.cc',
        'runtime/browser/xwalk_permission_manager.h',
        'runtime/browser/xwalk_platform_notification_service.cc',
        'runtime/browser/xwalk_platform_notification_service.h',
        'runtime/browser/xwalk_pref_store.cc',
        'runtime/browser/xwalk_pref_store.h',
        'runtime/browser/xwalk_presentation_service_delegate.cc',
        'runtime/browser/xwalk_presentation_service_delegate.h',
        'runtime/browser/xwalk_presentation_service_delegate_android.cc',
        'runtime/browser/xwalk_presentation_service_delegate_android.h',
        'runtime/browser/xwalk_presentation_service_delegate_win.cc',
        'runtime/browser/xwalk_presentation_service_delegate_win.h',
        'runtime/browser/xwalk_presentation_service_helper.cc',
        'runtime/browser/xwalk_presentation_service_helper.h',
        'runtime/browser/xwalk_presentation_service_helper_android.cc',
        'runtime/browser/xwalk_presentation_service_helper_android.h',
        'runtime/browser/xwalk_presentation_service_helper_win.cc',
        'runtime/browser/xwalk_presentation_service_helper_win.h',
        'runtime/browser/xwalk_render_message_filter.cc',
        'runtime/browser/xwalk_render_message_filter.h',
        'runtime/browser/xwalk_runner.cc',
        'runtime/browser/xwalk_runner.h',
        'runtime/browser/xwalk_runner_win.cc',
        'runtime/browser/xwalk_runner_win.h',
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
        'runtime/common/logging_xwalk.cc',
        'runtime/common/logging_xwalk.h',
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
        'runtime/common/xwalk_resource_delegate.cc',
        'runtime/common/xwalk_resource_delegate.h',
        'runtime/common/xwalk_runtime_features.cc',
        'runtime/common/xwalk_runtime_features.h',
        'runtime/common/xwalk_switches.cc',
        'runtime/common/xwalk_switches.h',
        'runtime/common/xwalk_system_locale.cc',
        'runtime/common/xwalk_system_locale.h',
        'runtime/renderer/android/xwalk_render_thread_observer.cc',
        'runtime/renderer/android/xwalk_render_thread_observer.h',
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
        'runtime/renderer/xwalk_content_renderer_client.cc',
        'runtime/renderer/xwalk_content_renderer_client.h',
        'runtime/renderer/xwalk_render_thread_observer_generic.cc',
        'runtime/renderer/xwalk_render_thread_observer_generic.h',
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
        ['OS=="android"',{
          'dependencies':[
            '../components/components.gyp:cdm_browser',
            'xwalk_core_jar_jni',
            'xwalk_core_native_jni',
          ],
          'sources!':[
            'runtime/browser/devtools/xwalk_devtools_frontend.cc',
            'runtime/browser/devtools/xwalk_devtools_frontend.h',
            'runtime/browser/runtime_ui_delegate_desktop.cc',
            'runtime/browser/runtime_ui_delegate_desktop.h',
            'runtime/browser/ui/desktop/download_views.cc',
            'runtime/browser/ui/desktop/download_views.h',
            'runtime/browser/ui/desktop/xwalk_autofill_popup_controller.cc',
            'runtime/browser/ui/desktop/xwalk_autofill_popup_controller.h',
            'runtime/browser/ui/desktop/xwalk_autofill_popup_view.cc',
            'runtime/browser/ui/desktop/xwalk_autofill_popup_view.h',
            'runtime/browser/ui/desktop/xwalk_permission_modal_dialog.cc',
            'runtime/browser/ui/desktop/xwalk_permission_modal_dialog.h',
            'runtime/browser/ui/desktop/xwalk_permission_modal_dialog_views.h',
            'runtime/browser/ui/desktop/xwalk_permission_modal_dialog_views.cc',
            'runtime/browser/ui/desktop/xwalk_permission_dialog_manager.cc',
            'runtime/browser/ui/desktop/xwalk_permission_dialog_manager.h',
            'runtime/browser/ui/desktop/xwalk_popup_controller.cc',
            'runtime/browser/ui/desktop/xwalk_popup_controller.h',
            'runtime/browser/ui/native_app_window_desktop.cc',
            'runtime/browser/ui/native_app_window_desktop.h',
            'runtime/browser/xwalk_autofill_client_desktop.cc',
            'runtime/browser/xwalk_autofill_client_desktop.h',
            'runtime/renderer/xwalk_render_thread_observer_generic.cc',
            'runtime/renderer/xwalk_render_thread_observer_generic.h',
          ],
        }],
        ['OS=="win"', {
          'conditions': [
            ['win_use_allocator_shim==1', {
              'dependencies': [
                '../base/allocator/allocator.gyp:allocator',
              ],
            }],
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
          'link_settings': {
            'libraries': [
              '-lruntimeobject.lib',
            ],
          },
          # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
          'msvs_disabled_warnings': [ 4267, ],
        }],  # OS=="win"
        ['OS=="linux"', {
          'dependencies': [
            'build/system.gyp:libnotify',
          ],
        }],  # OS=="linux"
        ['toolkit_views==1', {
          'dependencies': [
            '../components/components.gyp:app_modal',
            '../components/components.gyp:constrained_window',
            '../components/components.gyp:guest_view_browser',
            '../components/components.gyp:ui_zoom',
            '../ui/events/events.gyp:events',
            '../ui/strings/ui_strings.gyp:ui_strings',
            '../ui/views/controls/webview/webview.gyp:webview',
            '../ui/views/views.gyp:views',
            '../ui/resources/ui_resources.gyp:ui_resources',
          ],
          'sources': [
            'runtime/browser/ui/xwalk_javascript_native_dialog_factory.h',
          ]
        }, { # toolkit_views==0
          'sources/': [
            ['exclude', 'runtime/browser/ui/xwalk_views_delegate.cc'],
          ],
        }],  # toolkit_views==1
        ['use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:aura',
            '../ui/wm/wm.gyp:wm',
            '../ui/base/ime/ui_base_ime.gyp:ui_base_ime',
          ],
        }],
        ['OS=="linux" and use_ozone!=1', {
          'defines': ['USE_GTK_UI'],
          'dependencies': [
            '../chrome/browser/ui/libgtk2ui/libgtk2ui.gyp:gtk2ui',
          ],
        }],  # OS=="linux"
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
                    '../components/nacl/renderer/plugin/plugin.gyp:nacl_trusted_plugin',
                  ],
                }],
            ],
        }],
        ['enable_plugins==1', {
          'sources': [
            '../chrome/renderer/pepper/pepper_flash_drm_renderer_host.cc',
            '../chrome/renderer/pepper/pepper_flash_drm_renderer_host.h',
            '../chrome/renderer/pepper/pepper_flash_font_file_host.cc',
            '../chrome/renderer/pepper/pepper_flash_font_file_host.h',
            '../chrome/renderer/pepper/pepper_flash_fullscreen_host.cc',
            '../chrome/renderer/pepper/pepper_flash_fullscreen_host.h',
            '../chrome/renderer/pepper/pepper_flash_menu_host.cc',
            '../chrome/renderer/pepper/pepper_flash_menu_host.h',
            '../chrome/renderer/pepper/pepper_flash_renderer_host.cc',
            '../chrome/renderer/pepper/pepper_flash_renderer_host.h',
          ],
          'dependencies': [
            '../components/components.gyp:pdf_renderer',
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
        ['OS!="android"', {
          'dependencies': [
            'xwalk_strings',
          ],
        }],
        ['disable_bundled_extensions==1', {
          'defines': ['DISABLE_BUNDLED_EXTENSIONS'],
        }],
      ],
    },
    {
      # While we could just call lastchange.py here and generate the header
      # directly, it would only work if there is a git checkout (ie. it does
      # not work with a tarball, for example).
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
      'target_name': 'xwalk_strings',
      'type': 'none',
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/xwalk/locales/xwalk',
      },
      'direct_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)/xwalk/locales',
        ],
      },
      'actions': [
        {
          'action_name': 'generate_xwalk_strings',
          'variables': {
            'grit_grd_file': 'runtime/resources/xwalk_strings.grd',
            'grit_resource_ids': 'resources/resource_ids',
          },
          'includes': [ '../build/grit_action.gypi' ],
          'outputs': [ '<(grit_out_dir)' ]
        },
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/locales/',
          'files': [ '<(grit_out_dir)/' ],
        },
      ],
    },
    {
      # Build a minimal set of resources so Blink in XWalk has
      # access to necessary resources.
      'target_name': 'xwalk_pak',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/components/components_resources.gyp:components_resources',
        '<(DEPTH)/content/app/resources/content_resources.gyp:content_resources',
        '<(DEPTH)/ui/strings/ui_strings.gyp:ui_strings',
        '<(DEPTH)/ui/app_list/resources/app_list_resources.gyp:app_list_resources',
        '<(DEPTH)/ui/resources/ui_resources.gyp:ui_resources',
        'xwalk_resources',
      ],
      'conditions': [
        [ 'OS!="android"', {
          'dependencies': [
            '<(DEPTH)/components/components_strings.gyp:components_strings',
            '<(DEPTH)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
          ],
        }],
        ['toolkit_views==1', {
          'dependencies': [
            '<(DEPTH)/ui/views/resources/views_resources.gyp:views_resources',
          ],
        }],
      ],
      'actions': [
        {
          'action_name': 'repack_xwalk_resources',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_application_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_extensions_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_sysapps_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/strings/app_locale_settings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/strings/ui_strings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/components/components_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/blink/public/resources/blink_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/content/app/strings/content_strings_en-US.pak',
            ],
            'pak_output': '<(PRODUCT_DIR)/xwalk.pak',
          },
          'conditions': [
            [ 'OS!="android"', {
              'variables': {
                'pak_inputs+': [
                  '<(SHARED_INTERMEDIATE_DIR)/blink/devtools_resources.pak',
                  '<(SHARED_INTERMEDIATE_DIR)/components/strings/components_strings_en-US.pak',
                ],
              },
            }],
          ],
          'includes': ['../build/repack_action.gypi'],
        },
        {
          'action_name': 'repack_xwalk_resources_100_percent',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/app/resources/content_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/resources/ui_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/components/components_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/blink/public/resources/blink_image_resources_100_percent.pak',
            ],
            'pak_output': '<(PRODUCT_DIR)/xwalk_100_percent.pak',
          },
          'conditions': [
            ['toolkit_views==1', {
              'variables': {
                'pak_inputs+': [
                  '<(SHARED_INTERMEDIATE_DIR)/ui/views/resources/views_resources_100_percent.pak',
                ],
              },
            }],
          ],
          'includes': ['../build/repack_action.gypi'],
        },
        {
          'action_name': 'repack_xwalk_resources_200_percent',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/app/resources/content_resources_200_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/resources/ui_resources_200_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/components/components_resources_200_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/blink/public/resources/blink_image_resources_200_percent.pak',
            ],
            'pak_output': '<(PRODUCT_DIR)/xwalk_200_percent.pak',
          },
          'conditions': [
            ['toolkit_views==1', {
              'variables': {
                'pak_inputs+': [
                  '<(SHARED_INTERMEDIATE_DIR)/ui/views/resources/views_resources_200_percent.pak',
                ],
              },
            }],
          ],
          'includes': ['../build/repack_action.gypi'],
        },
        {
          'action_name': 'repack_xwalk_resources_300_percent',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/ui/resources/ui_resources_300_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/components/components_resources_300_percent.pak',
            ],
            'pak_output': '<(PRODUCT_DIR)/xwalk_300_percent.pak',
          },
          'includes': ['../build/repack_action.gypi'],
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
        ['OS=="win"', {
          'conditions': [
            ['win_use_allocator_shim==1', {
              'dependencies': [
                '../base/allocator/allocator.gyp:allocator',
              ],
            }],
          ],
          'dependencies': [
            '../sandbox/sandbox.gyp:sandbox',
            'dotnet/dotnet_bridge.gyp:dotnet_bridge',
            'experimental/wifidirect/wifidirect_extension.gyp:*'
          ],
          'sources': [
            '../content/app/sandbox_helper_win.cc', # Needed by InitializedSandbox
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
      'target_name': 'generate_crosswalk_win_zip',
      'type': 'none',
      'includes': [
        'xwalk_win_zip.gypi',
      ],
    },
    {
      'target_name': 'xwalk_builder',
      'type': 'none',
      'conditions': [
        ['OS=="android"', {
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
            'xwalk_core_library',
            'xwalk_shared_library',
            'xwalk_core_library_documentation',
            'xwalk_runtime_lib_apk',
            'xwalk_runtime_lib_lzma_apk',
            'xwalk_app_hello_world_apk',
            'xwalk_app_template',
            'xwalk_core_sample_apk',
            'xwalk_core_library_aar',
            'xwalk_shared_library_aar',
          ],
        }, 'OS=="win"', {
          'dependencies': [
            'xwalk',
            'xwalk_all_tests',
            'generate_crosswalk_win_zip',
          ],
        }, {  # OS!="android" and OS!="win"
          'dependencies': [
            'xwalk',
            'xwalk_all_tests',
          ],
        }],
      ],
    },
  ], # targets
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
            '<(PRODUCT_DIR)/xwalk.pak',
            '<(PRODUCT_DIR)/xwalk_100_percent.pak',
            '<(PRODUCT_DIR)/xwalk_200_percent.pak',
            '<(PRODUCT_DIR)/xwalk_300_percent.pak',
            '<(PRODUCT_DIR)/snapshot_blob.bin',
            '<(PRODUCT_DIR)/natives_blob.bin',
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
        'api_version': '<!(python ../build/util/version.py -f API_VERSION -t "@API@")',
        'min_api_version': '<!(python ../build/util/version.py -f API_VERSION -t "@MIN_API@")',
        'xwalk_version_code': '<!(python build/android/generate_version_code.py --version <(xwalk_version) --abi-name <(android_app_abi))',
      },
      'includes': [
        'xwalk_android.gypi',
        'xwalk_android_tests.gypi',
        'xwalk_android_app.gypi',
        'xwalk_core_library_android.gypi',
      ],
    }], # OS=="android"
  ]
}
