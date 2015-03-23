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
    private final static String WRAPPER_CLASS = "org.xwalk.core.Object";
    private Object wrapper;

    public Object getWrapper() {
        return wrapper;
    }
${CREATE_INTERNALLY_CONSTRUCTOR}
${ENUMS_SECTION}
${METHODS_SECTION}

${REFLECTION_INIT_SECTION}
${STATIC_INITIALIZER}
}
""")
    package_name = ''
    if self._java_data.package_name != '':
      package_name = 'package ' + self._java_data.package_name + ";"
    imports_string = self.GenerateImportRules()
    internal_class_name = self._java_data.class_name
    bridge_class_name = self._generated_class_name
    create_internally_constructor = self.GenerateCreateInternallyConstructor()
    bridge_enums = self.GenerateBridgeEnums()
    bridge_methods = self.GenerateBridgeMethods()
    reflection_init = self.GenerateReflectionInitString()
    static_initializer = self.GenerateStaticInitializerString()
    value = {'PACKAGE_SECTION': package_name,
             'IMPORT_SECTION': imports_string,
             'CLASS_NAME': bridge_class_name,
             'PARENT_CLASS': internal_class_name,
             'ENUMS_SECTION': bridge_enums,
             'METHODS_SECTION': bridge_methods,
             'REFLECTION_INIT_SECTION': reflection_init,
             'CREATE_INTERNALLY_CONSTRUCTOR': create_internally_constructor,
             'STATIC_INITIALIZER': static_initializer}
    class_content = bridge_class_template.substitute(value)
    return class_content

  def GenerateCreateInternallyConstructor(self):
    if not self._java_data.HasCreateInternallyAnnotation():
      return ''
    constructor_template = Template("""\
    private ${INTERNAL_CLASS_NAME} internal = null;
    ${BRIDGE_CLASS_NAME}(${INTERNAL_CLASS_NAME} internal) {
        this.internal = internal;
        this.wrapper = ReflectionHelper.createInstance(\
"${STATIC_CONSTRUCTOR_NAME}", this);
        try {
          reflectionInit();
        } catch (Exception e) {
          ReflectionHelper.handleException(e);
        }
    }
""")
    internal_class_name = self._java_data.class_name
    bridge_class_name = self._generated_class_name
    constructor_method = Method(self._java_data.class_name, self._class_loader,
        True, False, False, bridge_class_name, '', '', '')
    static_constructor_name = constructor_method.GetMethodDeclareName()
    value = {'INTERNAL_CLASS_NAME': internal_class_name,
             'BRIDGE_CLASS_NAME': bridge_class_name,
             'STATIC_CONSTRUCTOR_NAME': static_constructor_name}
    return constructor_template.substitute(value)

  def GenerateBridgeEnums(self):
    enums_string = ''
    enum_template = Template("""\
    private Class<?> ${ENUM_CLASS_NAME};
    private Method ${ENUM_VALUE_OF_METHOD};
    private Object Convert${ENUM_TYPE}(${ENUM_TYPE} type) {
        return ReflectionHelper.invokeMethod(${ENUM_VALUE_OF_METHOD}, \
null, type.toString());
    }
""")
    for enum in self._java_data.enums.values():
      value = {'ENUM_CLASS_NAME': enum.EnumClassName(),
               'ENUM_VALUE_OF_METHOD': enum.EnumMethodValueOfName(),
               'ENUM_TYPE': enum.enum_name}
      enums_string += enum_template.substitute(value)
    return enums_string

  def GenerateBridgeMethods(self):
    methods_string = ''
    for method in self._java_data.methods:
      methods_string += method.GenerateMethodsStringForBridge()
    return methods_string

  def GenerateReflectionInitString(self):
    ref_methods_string = ''

    ref_enum_template = Template("""\
        ${ENUM} = clazz.getClassLoader().loadClass("${ENUM_CLASS}");
        ${METHOD} = ${ENUM}.getMethod("valueOf", String.class);
""")
    for enum in self._java_data.enums.values():
      value = { 'ENUM': enum.EnumClassName(),
                'ENUM_CLASS': self._java_data.GetFullWrapperName(
                    enum.enum_name),
                'METHOD': enum.EnumMethodValueOfName()}
      ref_methods_string += ref_enum_template.substitute(value)

    ref_method_template = Template("""\
        ${METHOD_DECLARE_NAME} = ReflectionHelper.loadMethod(\
clazz, \"${METHOD}\"${PARAMS});
""")
    for method in self._java_data.methods:
      if method.is_constructor or method.is_static:
        continue
      value = { 'METHOD_DECLARE_NAME': method.GetMethodDeclareName(),
                'METHOD': method.method_name,
                'PARAMS': method.GetBridgeParamsStringDeclareForWrapper()}
      ref_methods_string += ref_method_template.substitute(value)

    ref_init_template = Template("""\
    private void reflectionInit() throws NoSuchMethodException,
            ClassNotFoundException {
        Class<?> clazz = wrapper.getClass();
${REF_METHODS}
    }
""")
    value = {'REF_METHODS': ref_methods_string}
    ref_init_string = ref_init_template.substitute(value)
    return ref_init_string

  def GenerateStaticInitializerString(self):
    if not self._java_data.HasCreateInternallyAnnotation():
      return ''
    static_initializer_template = Template("""\
    static {
        ReflectionHelper.registerConstructor("${STATIC_CONSTRUCTOR_NAME}", \
"${FULL_CLASS_NAME}", Object.class);
    }
""")
    bridge_class_name = self._java_data.bridge_name
    constructor_method = Method(self._java_data.class_name, self._class_loader,
        True, False, False, bridge_class_name, '', '', '')
    static_constructor_name = constructor_method.GetMethodDeclareName()
    full_class_name = self._java_data.GetFullWrapperName()
    value = {'STATIC_CONSTRUCTOR_NAME': static_constructor_name,
             'FULL_CLASS_NAME': full_class_name}
    return static_initializer_template.substitute(value)
