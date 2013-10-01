#!/usr/bin/env python
# Copyright (c) 2013 The Intel Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generator for injected JS api from json files.
"""

import optparse
import os
import sys

from xwalk_ext_generator import XWalkExtGenerator
import json_schema
from model import Model, UnixName
from schema_loader import SchemaLoader

# Names of supported code generators, as specified on the command-line.
# First is default.
GENERATORS = ['xwalk-js-api']

def GenerateSchema(generator,
                   in_filename,
                   root,
                   destdir,
                   root_namespace):
  schema_loader = SchemaLoader(os.path.dirname(os.path.relpath(
      os.path.normpath(in_filename), root)))

  schema = os.path.normpath(in_filename)
  schema_filename, schema_extension = os.path.splitext(schema)
  api_def = schema_loader.LoadSchema(schema)

  api_model = Model()
  relpath = os.path.relpath(os.path.normpath(schema_filename), root)
  namespace = api_model.AddNamespace(api_def[0],
                                     relpath,
                                     include_compiler_options=True)

  short_filename, extension = os.path.splitext(schema_filename)

  # Filenames are checked against the unix_names of the namespaces they
  # generate because the gyp uses the names of the JSON files to generate
  # the names of the .cc and .h files. We want these to be using unix_names.
#  if namespace.unix_name != short_filename:
#    sys.exit("Filename %s is illegal. Name files using unix_hacker style." %
#             schema_filename)

  # When cpp generator involved we will replace this stub type_generator
  type_generator = ''
  if generator == 'xwalk-js-api':
    extension_generator = XWalkExtGenerator(type_generator, root_namespace)
    generators = [
      ('%s_api.js' % namespace.unix_name, extension_generator.js_generator)
    ]
  else:
    raise Exception('Unrecognised generator %s' % generator)

  output_code = []
  for filename, generator in generators:
    code = generator.Generate(namespace).Render()
    if destdir:
      with open(os.path.join(destdir, namespace.source_file_dir,
          filename), 'w') as f:
        f.write(code)
    output_code += [filename, '', code, '']

  return '\n'.join(output_code)

if __name__ == '__main__':
  parser = optparse.OptionParser(
      description='Generates extension files from JSON schema',
      usage='usage: %prog [option]... schema')
  parser.add_option('-r', '--root', default='.',
      help='logical include root directory. Path to schema files from specified'
      'dir will be the include path.')
  parser.add_option('-d', '--destdir',
      help='root directory to output generated files.')
  parser.add_option('-n', '--namespace', default='generated_api_schemas',
      help='C++ namespace for generated files. e.g extensions::api.')
  parser.add_option('-g', '--generator', default=GENERATORS[0],
      choices=GENERATORS,
      help='The generator to use to build the output code. Supported values are'
      ' %s' % GENERATORS)

  (opts, filenames) = parser.parse_args()

  if not filenames:
    sys.exit(0) # This is OK as a no-op

  # Only one file should be specified.
  if len(filenames) > 1:
    raise Exception("Only one file can be specified at a time.")

  result = GenerateSchema(opts.generator, filenames[0], opts.root, opts.destdir,
                          opts.namespace)
  if not opts.destdir:
    print result
