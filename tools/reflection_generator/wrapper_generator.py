#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from string import Template

from code_generator import CodeGenerator
from java_class import InternalJavaFileData

class WrapperGenerator(CodeGenerator):
  """ Generator class thar generates wrapper layer code."""
  def __init__(self, java_data, class_loader):
    super(WrapperGenerator, self).__init__(java_data, class_loader)

  def RunTask(self):
    self._generated_class_name = self._java_data.wrapper_name
    self._generated_code = self.GenerateWrapperClass()

  def GenerateDoc(self, doc):
    return self._class_loader.GenerateDoc(doc)

  def GenerateWrapperClass(self):
    wrapper_template = Template("""\
${PACKAGE_SECTION}

${IMPORT_SECTION}

${DOC_SECTION}
public ${MODIFIER}class ${CLASS_NAME} ${CLASS_EXTENDS} ${CLASS_IMPLEMENTS}{

${FIELD_SECTION}

${ENUM_SECTION}

    private final static String BRIDGE_CLASS = "${BRIDGE_CLASS_FULL_NAME}";
    private Object bridge;

    Object getBridge() {
        return bridge;
    }
${CREATE_INTERNALLY_CONSTRUCORS}
${METHODS_SECTION}

${REFLECTION_SECTION}

${STATIC_INITIALIZER}
}
""")
    package_string = self.GeneratePackageString()
    imports = self.GenerateImportRules()
    modifier = self.GenerateModifier()
    class_name = self._java_data.wrapper_name
    class_extends = self.GenerateClassExtends()
    class_implements = self.GenerateClassImplements()
    fields = self.GenerateClassFields()
    enums = self.GenerateClassEnums()
    bridge_full_class_name = self._java_data.GetFullBridgeName()
    create_internally_constructor = self.GenerateCreateInternallyConstructor()
    methods = self.GenerateMethods()
    reflections = self.GenerateReflectionInitString()
    static_initializer = self.GenerateStaticInitializerString()
    if self._java_data.class_annotations.has_key(
        InternalJavaFileData.ANNOTATION_NO_INSTANCE):
      create_internally_constructor = ''
      static_initializer = ''
      reflections = ''
    value = {'PACKAGE_SECTION': package_string,
             'IMPORT_SECTION': imports,
             'DOC_SECTION': self.GenerateDoc(self._java_data.class_doc),
             'MODIFIER': modifier,
             'CLASS_NAME': class_name,
             'CLASS_EXTENDS': class_extends,
             'CLASS_IMPLEMENTS': class_implements,
             'FIELD_SECTION': fields,
             'ENUM_SECTION': enums,
             'BRIDGE_CLASS_FULL_NAME': bridge_full_class_name,
             'CREATE_INTERNALLY_CONSTRUCORS': create_internally_constructor,
             'METHODS_SECTION': methods,
             'REFLECTION_SECTION': reflections,
             'STATIC_INITIALIZER': static_initializer}
    return wrapper_template.substitute(value)

  def HasCreateInternallyAnnotation(self):
    return self._java_data.class_annotations.get(
        InternalJavaFileData.ANNOTATION_CREATE_INTERNALLY, False)

  def GeneratePackageString(self):
    # Remove the 'internal' folder from internal package.
    package_name = self._java_data.package_name.replace('.internal', '')
    return 'package %s;' % (package_name)

  def GenerateModifier(self):
    for method in self._java_data.methods:
      if method.is_abstract:
        return 'abstract '
    return ''

  def GenerateClassExtends(self):
    annotations = self._java_data.class_annotations
    if annotations.has_key(InternalJavaFileData.ANNOTATION_EXTEND_CLASS):
      to_extend = annotations[InternalJavaFileData.ANNOTATION_EXTEND_CLASS]
      return ' extends %s ' % (to_extend.replace('.class', ''))
    return ''

  def GenerateClassImplements(self):
    annotations = self._java_data.class_annotations
    if annotations.has_key(InternalJavaFileData.ANNOTATION_IMPL):
      to_implement = annotations[InternalJavaFileData.ANNOTATION_IMPL]
      impl_interface = to_implement.replace('.class', '')
      if self.IsInternalClass(impl_interface):
        impl_interface = self.LoadJavaClass(impl_interface).wrapper_name
      return ' implements %s ' % (impl_interface)
    return ''

  def GenerateClassFields(self):
    fields_string = ''
    field_template = Template("""\
${DOC}
    public final static ${TYPE} ${NAME} = ${VALUE};
""")
    for field in self._java_data.fields:
      value = {'TYPE': field.field_type,
               'NAME': field.field_name,
               'VALUE': field.field_value,
               'DOC': self.GenerateDoc(field.field_doc)}
      fields_string += field_template.substitute(value)
    return fields_string

  def GenerateClassEnums(self):
    enums_string = ''
    enum_template = Template("""\
${DOC}
    public enum ${NAME} {${CONTENT}
    }
    private Class<?> ${ENUM_CLASS_NAME};
    private Method ${ENUM_VALUE_OF_METHOD};
    private Object Convert${NAME}(${NAME} type) {
        return ReflectionHelper.invokeMethod(${ENUM_VALUE_OF_METHOD}, \
null, type.toString());
    }
""")
    for enum in self._java_data.enums.values():
      value = {'NAME': enum.enum_name.replace('Internal', ''),
               'CONTENT': enum.enum_declaration,
               'DOC': self.GenerateDoc(enum.enum_doc),
               'ENUM_CLASS_NAME': enum.EnumClassName(),
               'ENUM_VALUE_OF_METHOD': enum.EnumMethodValueOfName()}
      enums_string += enum_template.substitute(value)
    return enums_string

  def GenerateCreateInternallyConstructor(self):
    if not self.HasCreateInternallyAnnotation():
      return ''
    constructor_template = Template("""\
    public ${CLASS_NAME}(Object bridge) {
        this.bridge = bridge;
        try {
            reflectionInit();
        } catch (Exception e) {
            ReflectionHelper.handleException(e);
        }
    }
""")
    class_name = self._java_data.wrapper_name
    return constructor_template.substitute({'CLASS_NAME': class_name})

  def GenerateMethods(self):
    methods_string = ''
    # Generate method definitions.
    for method in self._java_data.methods:
      methods_string += method.GenerateMethodsStringForWrapper()
    return methods_string

  def GenerateReflectionInitString(self):
    ref_methods_string = ''

    ref_enum_template = Template("""\
        ${ENUM} = clazz.getClassLoader().loadClass("${ENUM_CLASS}");
        ${METHOD} = ${ENUM}.getMethod("valueOf", String.class);
""")
    for enum in self._java_data.enums.values():
      value = { 'ENUM': enum.EnumClassName(),
                'ENUM_CLASS': self._java_data.GetFullBridgeName(enum.enum_name),
                'METHOD': enum.EnumMethodValueOfName()}
      ref_methods_string += ref_enum_template.substitute(value)

    ref_method_template = Template("""\
        ${METHOD_DECLARE_NAME} = ReflectionHelper.loadMethod(\
clazz, \"${METHOD}Super\"${PARAMS});
""")
    for method in self._java_data.methods:
      if method.is_constructor or method.is_static or method.is_abstract:
        continue
      value = { 'METHOD_DECLARE_NAME': method.GetMethodDeclareName(),
                'METHOD': method.method_name,
                'PARAMS': method.GetWrapperParamsStringDeclareForBridge()}
      ref_methods_string += ref_method_template.substitute(value)

    ref_init_template = Template("""\
    private void reflectionInit() throws NoSuchMethodException,
            ClassNotFoundException {
        Class<?> clazz = bridge.getClass();
${REF_METHODS}
    }
""")
    value = {'REF_METHODS': ref_methods_string}
    ref_init_string = ref_init_template.substitute(value)
    return ref_init_string

  def FormatStaticInitializer(self, method):
    static_initializer_template = Template("""\
        ReflectionHelper.registerConstructor(\"${CONSTRUCTOR_NAME}\", \
\"${FULL_CLASS_NAME}\"${PARAM_LIST}, Object.class);
""")
    value = {"CONSTRUCTOR_NAME": method.GetMethodDeclareName(),
             "FULL_CLASS_NAME": self._java_data.GetFullBridgeName(),
             "PARAM_LIST": method.GetWrapperParamsStringDeclareForBridge()}
    return static_initializer_template.substitute(value)

  def GenerateStaticInitializerString(self):
    if self._java_data.class_annotations.has_key(
        InternalJavaFileData.ANNOTATION_CREATE_INTERNALLY):
      return ''
    static_initializer_template = Template("""\
    static {
${STATIC_INITIALIZER_LIST}
    }
""")
    static_initializer_list = ''
    for method in self._java_data.methods:
      if method.is_constructor:
        static_initializer_list += self.FormatStaticInitializer(method)
    value = {'STATIC_INITIALIZER_LIST': static_initializer_list}
    return static_initializer_template.substitute(value)
