{
  'targets': [
    {
      'target_name': 'xwalk_extensions_unittest',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:run_all_unittests',
        '../../testing/gtest.gyp:gtest',
        'extensions.gyp:xwalk_extensions',
      ],
      'sources': [
        'browser/xwalk_extension_function_handler_unittest.cc',
        'common/xwalk_extension_server_unittest.cc',
      ],
    },
    {
      'target_name': 'xwalk_extensions_browsertest',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../content/content.gyp:content_browser',
        '../../content/content_shell_and_tests.gyp:test_support_content',
        '../../net/net.gyp:net',
        '../../skia/skia.gyp:skia',
        '../../testing/gtest.gyp:gtest',
        '../test/base/base.gyp:xwalk_test_base',
        '../xwalk.gyp:xwalk_runtime',
        'extensions.gyp:xwalk_extensions',
        'extensions_resources.gyp:xwalk_extensions_resources',
        'external_extension_sample.gyp:*',
      ],
      'defines': [
        'HAS_OUT_OF_PROC_TEST_RUNNER',
      ],
      'variables': {
        'jsapi_component': 'extensions',
      },
      'includes': [
        '../xwalk_jsapi.gypi',
      ],
      'sources': [
        'test/bad_extension_test.cc',
        'test/conflicting_entry_points.cc',
        'test/context_destruction.cc',
        'test/crash_extension_process.cc',
        'test/export_object.cc',
        'test/extension_in_iframe.cc',
        'test/external_extension.cc',
        'test/external_extension_multi_process.cc',
        'test/in_process_threads_browsertest.cc',
        'test/internal_extension_browsertest.cc',
        'test/internal_extension_browsertest.h',
        'test/nested_namespace.cc',
        'test/namespace_read_only.cc',
        'test/test.idl',
        'test/v8tools_module.cc',
        'test/xwalk_extensions_browsertest.cc',
        'test/xwalk_extensions_test_base.cc',
        'test/xwalk_extensions_test_base.h',
      ],
    },
  ],
}
