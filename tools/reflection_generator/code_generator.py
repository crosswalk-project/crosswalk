# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class CodeGenerator(object):
  """Basic class of code generator"""
  def __init__(self, java_data, class_loader):
    self._java_data = java_data
    self._class_loader = class_loader
    self._generated_code = ''
    self._generated_class_name = ''

  def RunTask(self):
    pass

  def GetJavaData(self, clazz):
    return self._class_loader.GetJavaData(clazz)

  def IsInternalClass(self, clazz):
    return self._class_loader.IsInternalClass(clazz)

  def GenerateDoc(self, doc):
    return self._class_loader.GenerateDoc(doc)

  def GetGeneratedCode(self):
    return self._generated_code

  def GetGeneratedClassFileName(self):
    return self._generated_class_name + '.java'

  def GenerateImportRules(self):
    imports = ''
    for imported in self._java_data.imports:
      import_string = 'import ' + imported + ";\n"
      imports += import_string
    # Add the reflection helper import.
    imports += '\n'
    imports += 'import java.lang.reflect.Constructor;\n'
    imports += 'import java.lang.reflect.Method;\n'
    return imports
