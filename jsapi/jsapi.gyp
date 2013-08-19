{
  'targets': [
    {
      'target_name': 'xwalk_jsapi',
      'type': 'static_library',
      'sources': [
        '<@(schema_files)',
      ],
      'includes': [
        '../../build/json_schema_bundle_compile.gypi',
        '../../build/json_schema_compile.gypi',
      ],
      'variables': {
        'schema_files': [
          'dialog.idl',
          'runtime.idl',
        ],
        'cc_dir': 'xwalk/jsapi',
        'root_namespace': 'xwalk::jsapi',
      },
   }],
}
