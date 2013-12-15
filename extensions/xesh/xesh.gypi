{
  'targets': [
    {
      'target_name': 'xwalk_extension_shell',
      'type': 'executable',
      'defines': ['XWALK_VERSION="<(xwalk_version)"'],
      'product_name': 'xesh',
      'dependencies': [
        '../base/base.gyp:base',
        '../content/content.gyp:content',
        '../ipc/ipc.gyp:ipc',
        '../url/url.gyp:url_lib',
        '../v8/tools/gyp/v8.gyp:v8',
        'extensions/extensions.gypi:xwalk_extensions_lib',
      ],
      'include_dirs': [
        '../../..',
      ],
      'sources': [
        'xesh_main.cc',
        'xesh_v8_runner.h',
        'xesh_v8_runner.cc',
      ],
    },
  ],
}
