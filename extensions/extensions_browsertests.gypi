{
  'sources': [
    'test/bad_extension_test.cc',
    'test/context_destruction.cc',
    'test/extension_in_iframe.cc',
    'test/external_extension.cc',
    'test/internal_extension_browsertest.cc',
    'test/internal_extension_browsertest.h',
    'test/internal_extension_browsertest_api.js',
    'test/xwalk_extensions_browsertest.cc',
    'test/xwalk_extensions_test_base.cc',
    'test/xwalk_extensions_test_base.h',
    'test/v8tools_module.cc',
  ],

  'dependencies': [
    'extensions/external_extension_sample.gyp:*',
    'extensions/jsapi_test.gyp:api_test',
  ],
  'includes': [
    'xwalk_js2c.gypi',
  ],
}
