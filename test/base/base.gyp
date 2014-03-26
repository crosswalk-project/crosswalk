{
  'targets': [
    {
      'target_name': 'xwalk_test_base',
      'type': 'static_library',
      'dependencies': [
        # FIXME(tmpsantos): we should depend on runtime
        # here but it is not really a module yet.
        '../../../base/base.gyp:base',
        '../../../base/base.gyp:test_support_base',
        '../../../content/content.gyp:content_browser',
        '../../../content/content_shell_and_tests.gyp:test_support_content'
        '../../../net/net.gyp:net',
        '../../../skia/skia.gyp:skia',
        '../../../testing/gtest.gyp:gtest',
        '../../../ui/base/ui_base.gyp:ui_base',
        '../../../url/url.gyp:url_lib',
      ],
      'sources': [
        'in_process_browser_test.cc',
        'in_process_browser_test.h',
        'xwalk_test_launcher.cc',
        'xwalk_test_suite.cc',
        'xwalk_test_suite.h',
        'xwalk_test_utils.cc',
        'xwalk_test_utils.h',
      ],
    },
  ],
}
