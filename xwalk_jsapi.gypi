# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'api_gen_dir': '<(DEPTH)/tools/json_schema_compiler',
    'api_gen': '<(api_gen_dir)/compiler.py',
    'root_namespace': 'xwalk::jsapi::%(namespace)s',
    'jsapi_component%': '',
  },
  'rules': [
    {
      'rule_name': 'genapi_idl',
      'msvs_external_rule': 1,
      'extension': 'idl',
      'inputs': [
        '<(api_gen_dir)/cc_generator.py',
        '<(api_gen_dir)/code.py',
        '<(api_gen_dir)/compiler.py',
        '<(api_gen_dir)/cpp_generator.py',
        '<(api_gen_dir)/cpp_type_generator.py',
        '<(api_gen_dir)/cpp_util.py',
        '<(api_gen_dir)/h_generator.py',
        '<(api_gen_dir)/idl_schema.py',
        '<(api_gen_dir)/model.py',
        '<(api_gen_dir)/util.cc',
        '<(api_gen_dir)/util.h',
        '<(api_gen_dir)/util_cc_helper.py',
      ],
      'outputs': [
        '<(SHARED_INTERMEDIATE_DIR)/xwalk/<(jsapi_component)/<(RULE_INPUT_DIRNAME)/<(RULE_INPUT_ROOT).cc',
        '<(SHARED_INTERMEDIATE_DIR)/xwalk/<(jsapi_component)/<(RULE_INPUT_DIRNAME)/<(RULE_INPUT_ROOT).h',
      ],
      'action': [
        'python',
        '<(api_gen)',
        '<(RULE_INPUT_PATH)',
        '--root=.',
        '--destdir=<(SHARED_INTERMEDIATE_DIR)/xwalk/<(jsapi_component)',
        '--namespace=<(root_namespace)',
        '--generator=cpp',
      ],
      'message': 'Generating C++ code from <(RULE_INPUT_PATH) IDL files',
      'process_outputs_as_sources': 1,
    },
  ],
  'include_dirs': [
    '<(SHARED_INTERMEDIATE_DIR)',
    '<(SHARED_INTERMEDIATE_DIR)/xwalk/<(jsapi_component)',
    '<(DEPTH)',
  ],
  'dependencies':[
    '<(DEPTH)/tools/json_schema_compiler/api_gen_util.gyp:api_gen_util',
  ],
  'direct_dependent_settings': {
    'include_dirs': [
      '<(SHARED_INTERMEDIATE_DIR)',
    ]
  },
  'hard_dependency': 1,
}
