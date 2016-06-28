{
  'targets': [
    {
      'target_name': 'sysapps',
      'type': 'static_library',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../net/net.gyp:net',
        '../../ui/base/ui_base.gyp:ui_base',
        '../../ui/gfx/gfx.gyp:gfx',
        '../../ui/gfx/gfx.gyp:gfx_geometry',
        '../extensions/extensions.gyp:xwalk_extensions',
        'sysapps_resources.gyp:xwalk_sysapps_resources',
      ],
      'variables': {
        'jsapi_component': 'sysapps',
      },
      'includes': [
        '../../build/filename_rules.gypi',
        '../xwalk_jsapi.gypi',
      ],
      'sources': [
        'common/binding_object.h',
        'common/binding_object_store.cc',
        'common/binding_object_store.h',
        'common/common.idl',
        'common/event_target.cc',
        'common/event_target.h',
        'common/sysapps_manager.cc',
        'common/sysapps_manager.h',
        'raw_socket/raw_socket.idl',
        'raw_socket/raw_socket_extension.cc',
        'raw_socket/raw_socket_extension.h',
        'raw_socket/raw_socket_object.cc',
        'raw_socket/raw_socket_object.h',
        'raw_socket/tcp_server_socket.idl',
        'raw_socket/tcp_server_socket_object.cc',
        'raw_socket/tcp_server_socket_object.h',
        'raw_socket/tcp_socket.idl',
        'raw_socket/tcp_socket_object.cc',
        'raw_socket/tcp_socket_object.h',
        'raw_socket/udp_socket.idl',
        'raw_socket/udp_socket_object.cc',
        'raw_socket/udp_socket_object.h',
      ],
      'conditions': [
        ['OS!="android"', {
          'dependencies': [
            '../../components/components.gyp:storage_monitor',
            '../../media/media.gyp:media',
            '../../third_party/ffmpeg/ffmpeg.gyp:ffmpeg',
          ],
        }],
        ['OS=="win"', {
          'link_settings': {
            'libraries': [
              '-lPdh.lib',
            ],
          },
        }]
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          # Build units including this module should have this
          # on theirs include path because of the code we generate
          # from the IDL files.
          '<(SHARED_INTERMEDIATE_DIR)',
        ]
      },
    },
  ],
}
