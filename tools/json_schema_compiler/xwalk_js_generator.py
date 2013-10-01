# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from code import Code
from model import PropertyType, Type
import model
import schema_util
import sys

class XWalkJSGenerator(object):
  def __init__(self, type_generator, cpp_namespace):
    self._type_generator = type_generator
    self._cpp_namespace = cpp_namespace

  def Generate(self, namespace):
    return _Generator(namespace).Generate()

class _Generator(object):
  """A .cc generator for a namespace.
  """
  def __init__(self, namespace):
    self._namespace = namespace

  def Generate(self):
    """Generates JS api.
    """
    c = Code()
    (c.Append('extension._setupExtensionInternal();')
      .Append('var internal = extension._internal;')
      .Append()
    )
    if self._namespace.functions:
      (c.Append('// Generated APIs')
        .Append()
      )
    for function in self._namespace.functions.values():
      c.Cblock(self._GenerateFunction(function))
    return c

  def _GenerateFunction(self, function):
    c = Code()
    code = ''

    """function name"""
    code += ('exports.%s = function(' % function.name)
    
    """paramaters"""
    params = "";
    if function.params:
      for param in function.params:
        param = ('%s, ' % param.name)
        params += param
    code += (params + 'callback) {') 
    (c.Append(code))

    """body"""
    code = ('  internal.postMessage(\'%s\',' % function.name)
    code += (' [%s], callback);' % params[0:(len(params)-2)])
    (c.Append(code)
      .Append('};')
      .Append()
    )

    return c

