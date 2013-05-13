# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'cameo_product_name': 'Content Shell',
    # The "19" is so that sites that sniff for version think that this is
    # something reasonably current; the "77.34.5" is a hint that this isn't a
    # standard Chrome.
    'cameo_version': '19.77.34.5',
    'conditions': [
      ['OS=="linux"', {
       'use_custom_freetype%': 1,
      }, {
       'use_custom_freetype%': 0,
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'cameo',
      'type': 'executable',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'defines': ['CAMEO_VERSION="<(cameo_version)"'],
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        '../third_party/mesa/mesa.gyp:osmesa',
        '../content/content.gyp:content_app',
        '../content/content.gyp:content_browser',
        '../content/content.gyp:content_common',
        '../content/content.gyp:content_gpu',
        '../content/content.gyp:content_plugin',
        '../content/content.gyp:content_ppapi_plugin',
        '../content/content.gyp:content_renderer',
        '../content/content.gyp:content_utility',
        '../content/content.gyp:content_worker',
        '../base/base.gyp:base',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '../ipc/ipc.gyp:ipc',
        '../media/media.gyp:media',
        '../net/net.gyp:net',
        '../net/net.gyp:net_resources',
        '../ui/gl/gl.gyp:gl',
        '../ui/ui.gyp:ui',
        '../v8/tools/gyp/v8.gyp:v8',
        '../base/allocator/allocator.gyp:allocator',
	'../skia/skia.gyp:skia',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'main.cc',
	'cameo_main_delegate.cc',
	'cameo_main_delegate.h',
	'cameo_content_browser_client.cc',
	'cameo_content_browser_client.h',
	'cameo_views_delegate.cc',
	'cameo_views_delegate.h',
	'cameo_browser_main_parts.cc',
	'cameo_browser_main_parts.h',
	'cameo_browser_context.cc',
	'cameo_browser_context.h',
	'cameo_resource_context.cc',
	'cameo_resource_context.h',
	'cameo_url_request_context_getter.cc',
	'cameo_url_request_context_getter.h',
	'cameo_widget_delegate.cc',
	'cameo_widget_delegate.h',
	'cameo_web_contents_delegate.cc',
	'cameo_web_contents_delegate.h',
	'cameo_network_delegate.cc',
	'cameo_network_delegate.h',
	'renderer/cameo_content_renderer_client.h',
	'renderer/cameo_content_renderer_client.cc',
      ],
      'conditions': [
        ['OS == "win" or (toolkit_uses_gtk == 1 and selinux == 0)', {
          'dependencies': [
            '../sandbox/sandbox.gyp:sandbox',
          ],
        }],  # OS=="win" or (toolkit_uses_gtk == 1 and selinux == 0)
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '<(DEPTH)/build/linux/system.gyp:gtk',
          ],
        }],  # toolkit_uses_gtk
        ['OS=="linux"', {
          'dependencies': [
            '../build/linux/system.gyp:fontconfig',
          ],
        }],
        ['use_custom_freetype==1', {
          'dependencies': [
             '../third_party/freetype2/freetype2.gyp:freetype2',
          ],
        }],
        ['use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:aura',
            '../ui/base/strings/ui_strings.gyp:ui_strings',
            '../ui/views/controls/webview/webview.gyp:webview',
            '../ui/views/views.gyp:views',
            '../ui/ui.gyp:ui_resources',
          ],
        }],  # use_aura==1
      ],
    },
    {
      'target_name': 'cameo_builder',
      'type': 'none',
      'dependencies': [
        'cameo',
      ],
    },
  ]
}
