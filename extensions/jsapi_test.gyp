{
  'targets': [
    {
      'target_name': 'api_test',
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
          'test/test.idl',
        ],
        'cc_dir': 'xwalk/extensions/test',
        'root_namespace': 'xwalk::jsapi_test',
      },
   }],
}
