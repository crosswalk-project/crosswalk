{
  'sources': [
    'test/context_destruction.cc',
    'test/external_extension_browsertest.cc',
    'test/internal_extension_browsertest.cc',
    'test/internal_extension_browsertest.h',
    'test/internal_extension_browsertest_api.js',
    'test/xwalk_extensions_browsertest.cc',
    'test/xwalk_extensions_test_base.cc',
    'test/xwalk_extensions_test_base.h',
  ],

  'dependencies': [
    'extensions/external_extension_sample.gyp:external_extension_sample',
    'extensions/jsapi_test.gyp:api_test',
  ],
  'includes': [
    'xwalk_js2c.gypi',
  ],
}
