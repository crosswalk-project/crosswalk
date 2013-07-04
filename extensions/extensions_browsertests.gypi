{
  'sources': [
    'test/xwalk_extensions_browsertest.cc',
    'test/external_extension_browsertest.cc',
    'test/xwalk_extensions_test_base.cc',
    'test/xwalk_extensions_test_base.h',
  ],

  'dependencies': [
    'extensions/external_extension_sample.gyp:external_extension_sample',
  ],
}
