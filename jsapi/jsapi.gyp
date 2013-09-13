{
  'targets': [
    {
      'target_name': 'xwalk_jsapi',
      'type': 'static_library',
      'sources': [
        '<@(schema_files)',
      ],
      'includes': [
        '../../build/json_schema_compile.gypi',
      ],
      'variables': {
        'schema_files': [
          'dialog.idl',
          'runtime.idl',
          'sysapps_common.idl',
          'sysapps_raw_socket.idl',
          'sysapps_raw_socket_tcp_socket.idl',
        ],
        'cc_dir': 'xwalk/jsapi',
        'root_namespace': 'xwalk::jsapi',
      },
   }],
}
