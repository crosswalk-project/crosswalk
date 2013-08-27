{
  'sources': [
    'browser/xwalk_extension_function_handler.cc',
    'browser/xwalk_extension_function_handler.h',
    'browser/xwalk_extension_internal.cc',
    'browser/xwalk_extension_internal.h',
    'browser/xwalk_extension_process_host.cc',
    'browser/xwalk_extension_process_host.h',
    'browser/xwalk_extension_service.cc',
    'browser/xwalk_extension_service.h',
    'common/xwalk_extension.cc',
    'common/xwalk_extension.h',
    'common/xwalk_extension_external.cc',
    'common/xwalk_extension_external.h',
    'common/xwalk_extension_messages.cc',
    'common/xwalk_extension_messages.h',
    'common/xwalk_extension_runner.cc',
    'common/xwalk_extension_runner.h',
    'common/xwalk_extension_threaded_runner.cc',
    'common/xwalk_extension_threaded_runner.h',
    'common/xwalk_extension_server.cc',
    'common/xwalk_extension_server.h',
    'common/xwalk_extension_switches.cc',
    'common/xwalk_extension_switches.h',
    'common/xwalk_external_adapter.cc',
    'common/xwalk_external_adapter.h',
    'common/xwalk_external_context.cc',
    'common/xwalk_external_context.h',
    'common/xwalk_external_extension.cc',
    'common/xwalk_external_extension.h',
    'extension_process/xwalk_extension_process_main.cc',
    'extension_process/xwalk_extension_process_main.h',
    'extension_process/xwalk_extension_process.cc',
    'extension_process/xwalk_extension_process.h',
    'public/xwalk_extension_public.h',
    'public/XW_Extension.h',
    'public/XW_Extension_SyncMessage.h',
    'renderer/xwalk_extension_renderer_controller.cc',
    'renderer/xwalk_extension_renderer_controller.h',
    'renderer/xwalk_api.js',
    'renderer/xwalk_extension_module.cc',
    'renderer/xwalk_extension_module.h',
    'renderer/xwalk_module_system.cc',
    'renderer/xwalk_module_system.h',
    'renderer/xwalk_v8tools_module.cc',
    'renderer/xwalk_v8tools_module.h',
    'renderer/xwalk_remote_extension_runner.cc',
    'renderer/xwalk_remote_extension_runner.h',
    'renderer/xwalk_extension_client.cc',
    'renderer/xwalk_extension_client.h',
  ],
  'conditions': [
    ['OS=="android"',{
      'sources': [
        'common/android/xwalk_extension_android.cc',
        'common/android/xwalk_extension_android.h',
      ],
    }],
  ],
  'includes': [
    'xwalk_js2c.gypi',
  ],
}
