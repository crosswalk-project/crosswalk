# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from string import Template

from code_generator import CodeGenerator

class InterfaceGenerator(CodeGenerator):
  """Generator class that generates interfade code in wrapper layer"""
  def __init__(self, java_data, class_loader):
    super(InterfaceGenerator, self).__init__(java_data, class_loader)

  def RunTask(self):
    self._generated_class_name = self._java_data.wrapper_name
    self._generated_code = self.GenerateInterface()

  def GenerateDoc(self, doc):
    return self._class_loader.GenerateDoc(doc)

  def GenerateInterface(self):
    interface_template = Template("""
${PACKAGE_SECTION}

${IMPORT_SECTION}
${DOC}
public interface ${INTERFACE_NAME} {
${METHOD_SECTION}
}
""")
    package_section = 'package %s;' % \
        (self._java_data.package_name.replace('.internal', ''))
    import_section = self.GenerateImportRules()
    method_section = self.GenerateMethods()
    interface_name = self._generated_class_name
    value = {'PACKAGE_SECTION': package_section,
             'IMPORT_SECTION': import_section,
             'DOC': self.GenerateDoc(self._java_data.class_doc),
             'INTERFACE_NAME': interface_name,
             'METHOD_SECTION': method_section}
    interface_code = interface_template.substitute(value)
    return interface_code

  def GenerateMethods(self):
    methods_string = ''
    for method in self._java_data.methods:
      methods_string += method.GenerateMethodsStringForInterface()
    return methods_string
