{
  'targets': [
    {
      'target_name': 'xwalk_extension_shell',
      'type': 'executable',
      'defines': ['XWALK_VERSION="<(xwalk_version)"'],
      'product_name': 'xesh',
      'dependencies': [
        '../base/allocator/allocator.gyp:allocator',
        '../base/base.gyp:base',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../content/content.gyp:content',
        '../ipc/ipc.gyp:ipc',
        '../third_party/WebKit/public/blink.gyp:blink',
        '../url/url.gyp:url_lib',
        '../v8/tools/gyp/v8.gyp:v8',
        '../webkit/glue/webkit_glue.gyp:glue',
        '../webkit/child/webkit_child.gyp:webkit_child',
        'extensions/extensions.gyp:xwalk_extensions',
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
