# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Writes C++ header/cc source files for embedding resources into C++."""

import os


def WriteSource(base_name,
                dir_from_src,
                output_dir,
                global_string_map):
  """Writes C++ header/cc source files for the given map of string variables.

  Args:
      base_name: The basename of the file, without the extension.
      dir_from_src: Path from src to the directory that will contain the file,
          using forward slashes.
      output_dir: Directory to output the sources to.
      global_string_map: Map of variable names to their string values. These
          variables will be available as globals.
  """
  copyright = '\n'.join([
      '// Copyright 2013 The Chromium Authors. All rights reserved.',
      '// Use of this source code is governed by a BSD-style license that '
          'can be',
      '// found in the LICENSE file.'])

  # Write header file.
  externs = []
  for name in global_string_map.iterkeys():
    externs += ['extern const char %s[];' % name]

  temp = '_'.join(dir_from_src.split('/') + [base_name])
  define = temp.upper() + '_H_'
  header = '\n'.join([
      copyright,
      '',
      '#ifndef ' + define,
      '#define ' + define,
      '',
      '\n'.join(externs),
      '',
      '#endif  // ' + define])
  header += '\n'

  with open(os.path.join(output_dir, base_name + '.h'), 'w') as f:
    f.write(header)

  # Write cc file.
  def EscapeLine(line):
    return line.replace('\\', '\\\\').replace('"', '\\"')

  definitions = []
  for name, contents in global_string_map.iteritems():
    lines = []
    if '\n' not in contents:
      lines = ['    "%s"' % EscapeLine(contents)]
    else:
      for line in contents.split('\n'):
        lines += ['    "%s\\n"' % EscapeLine(line)]
    definitions += ['const char %s[] =\n%s;' % (name, '\n'.join(lines))]

  cc = '\n'.join([
      copyright,
      '',
      '#include "%s"' % (dir_from_src + '/' + base_name + '.h'),
      '',
      '\n'.join(definitions)])
  cc += '\n'

  with open(os.path.join(output_dir, base_name + '.cc'), 'w') as f:
    f.write(cc)
