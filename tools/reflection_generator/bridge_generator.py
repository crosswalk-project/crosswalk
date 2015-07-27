#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from string import Template

from code_generator import CodeGenerator
from java_method import Method

class BridgeGenerator(CodeGenerator):
  """ Generator class that generates bridge layer code."""
  def __init__(self, java_data, class_loader):
    super(BridgeGenerator, self).__init__(java_data, class_loader)

  def RunTask(self):
    self._generated_class_name = self._java_data.bridge_name
    self._generated_code = self.GenerateBridgeClass()

  def GenerateBridgeClass(self):
    bridge_class_template = Template("""\
${PACKAGE_SECTION}
${IMPORT_SECTION}
public class ${CLASS_NAME} extends ${PARENT_CLASS} {
    private XWalkCoreBridge coreBridge;
    private Object wrapper;

    public Object getWrapper() {
        return wrapper;
    }

${ENUMS_SECTION}
${INTERNAL_CONSTRUCTOR}
${METHODS_SECTION}
${REFLECTION_INIT_SECTION}}
""")

    value = {'PACKAGE_SECTION': self.GeneratePackage(),
             'IMPORT_SECTION': self.GenerateImportRules(),
             'CLASS_NAME': self._java_data.bridge_name,
             'PARENT_CLASS': self._java_data.class_name,
             'INTERNAL_CONSTRUCTOR': self.GenerateInternalConstructor(),
             'ENUMS_SECTION': self.GenerateEnums(),
             'METHODS_SECTION': self.GenerateMethods(),
             'REFLECTION_INIT_SECTION': self.GenerateReflectionInit()}
    return bridge_class_template.substitute(value)

  def GeneratePackage(self):
    if self._java_data.package_name == '':
      return ''
    return 'package ' + self._java_data.package_name + ";\n"

  def GenerateEnums(self):
    enum_template = Template("""\
    private ReflectMethod ${ENUM_VALUE_OF_METHOD} = new ReflectMethod();

    private Object Convert${ENUM_TYPE}(${ENUM_TYPE} type) {
        return ${ENUM_VALUE_OF_METHOD}.invoke(type.toString());
    }

""")

    enums_string = ''
    for enum in self._java_data.enums.values():
      value = {'ENUM_VALUE_OF_METHOD': enum.EnumMethodValueOfName(),
               'ENUM_TYPE': enum.enum_name}
      enums_string += enum_template.substitute(value)
    return enums_string

  def GenerateInternalConstructor(self):
    if not self._java_data.HasCreateInternallyAnnotation():
      return ''

    constructor_template = Template("""\
    private ${INTERNAL_CLASS_NAME} internal;

    ${BRIDGE_CLASS_NAME}(${INTERNAL_CLASS_NAME} internal) {
        this.internal = internal;
        reflectionInit();
    }
""")

    value = {'INTERNAL_CLASS_NAME': self._java_data.class_name,
             'BRIDGE_CLASS_NAME': self._java_data.bridge_name}
    return constructor_template.substitute(value)

  def GenerateBridgeDefaultConstructor(self):
    template = Template("""\
    public ${NAME}(Object wrapper) {
        this.wrapper = wrapper;
        reflectionInit();
    }

""")
    value = {'NAME': self._java_data.bridge_name}
    return template.substitute(value)

  def GenerateMethods(self):
    methods_string = ''
    if self._java_data.need_default_constructor:
      methods_string += self.GenerateBridgeDefaultConstructor()
    for method in self._java_data.methods:
      methods_string += method.GenerateMethodsStringForBridge()
    return methods_string

  def GenerateReflectionInit(self):
    ref_init_string = """\
        coreBridge = XWalkCoreBridge.getInstance();
        if (coreBridge == null) return;
"""

    if self._java_data.HasCreateInternallyAnnotation():
      ref_init_templete = Template("""
        ReflectConstructor constructor = new ReflectConstructor(
                coreBridge.getWrapperClass("${WRAPPER_NAME}"), Object.class);
        this.wrapper = constructor.newInstance(this);
""")
      value = {'WRAPPER_NAME': self._java_data.GetWrapperName()}
      ref_init_string += ref_init_templete.substitute(value)

    ref_enum_template = Template("""\
        ${METHOD}.init(null,
                coreBridge.getWrapperClass("${ENUM}"), "valueOf", String.class);
""")

    ref_methods_string = ''
    for enum in self._java_data.enums.values():
      value = {'METHOD': enum.EnumMethodValueOfName(),
               'ENUM': self._java_data.GetWrapperName(enum.enum_name)}
      ref_methods_string += ref_enum_template.substitute(value)

    ref_method_template = Template("""\
        ${METHOD_DECLARE_NAME}.init(wrapper, null,
                "${METHOD}"${PARAMS});
""")

    if (ref_methods_string != ''):
      ref_methods_string += "\n"
    for method in self._java_data.methods:
      if method.is_constructor or method.is_static:
        continue
      value = { 'METHOD_DECLARE_NAME': method._method_declare_name,
                'METHOD': method.method_name,
                'PARAMS': method._bridge_params_declare_for_wrapper}
      ref_methods_string += ref_method_template.substitute(value)

    ref_init_template = Template("""\
    void reflectionInit() {
${REF_INIT}
${REF_METHODS}    }
""")

    value = {'REF_INIT': ref_init_string,
             'REF_METHODS': ref_methods_string}
    return ref_init_template.substitute(value)
