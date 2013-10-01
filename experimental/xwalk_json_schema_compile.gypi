# Copyright (c) 2012 The Intel Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    # When including this gypi, the following variables must be set:
    #   schema_files: a list of json or IDL files that comprise the api model.
    #   cc_dir: path to generated files
    #   root_namespace: the C++ namespace that all generated files go under
    # Functions and namespaces can be excluded by setting "nocompile" to true.
    'api_gen_dir': '<(DEPTH)/tools/json_schema_compiler',
    'api_gen': '<(api_gen_dir)/xwalk_extension_compiler.py',
    'root_namespace': 'xwalk',
  },
  'inputs': [
    '<(api_gen_dir)/xwalk_ext_generator.py',
    '<(api_gen_dir)/code.py',
    '<(api_gen_dir)/xwalk_extension_compiler.py',
    '<(api_gen_dir)/xwalk_js_generator.py',
    '<(api_gen_dir)/json_schema.py',
    '<(api_gen_dir)/model.py',
  ],
  'outputs': [
    '<(SHARED_INTERMEDIATE_DIR)/<(out_filename)'
  ],
  'action': [
    'python',
    '<(api_gen)',
    '<(input_json_file)',
    '--root=<(input_root)',
    '--destdir=<(SHARED_INTERMEDIATE_DIR)',
    '--namespace=<(root_namespace)',
    '--generator=xwalk-js-api',
  ],
  'message': 'Generating JS api code from <(RULE_INPUT_PATH) json files',
}
