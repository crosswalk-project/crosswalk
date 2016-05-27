# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from string import Template

from code_generator import CodeGenerator
from collections import OrderedDict
from java_class import InternalJavaFileData

class WrapperGenerator(CodeGenerator):
  """ Generator class thar generates wrapper layer code."""
  def __init__(self, java_data, class_loader):
    super(WrapperGenerator, self).__init__(java_data, class_loader)

  def RunTask(self):
    self._generated_class_name = self._java_data.wrapper_name
    self._generated_code = self.GenerateWrapperClass()

  def GenerateWrapperClass(self):
    wrapper_template = Template("""\
${PACKAGE_SECTION}
${IMPORT_SECTION}
${DOC_SECTION}

public ${CLASS_MODIFIER}class ${CLASS_NAME} \
${CLASS_EXTENDS} ${CLASS_IMPLEMENTS} {

${FIELDS_SECTION}
${ENUMS_SECTION}
${MEMBERSS_SECTION}
${INTERNALLY_CONSTRUCOR_SECTION}
${METHODS_SECTION}
${REFLECTION_SECTION}
}
""")

    import_string = self.GenerateImportRules()
    import_string += "import java.util.ArrayList;\n"

    value = {'PACKAGE_SECTION': self.GeneratePackage(),
             'IMPORT_SECTION': import_string,
             'DOC_SECTION': self.GenerateDoc(self._java_data.class_doc),
             'CLASS_MODIFIER': self.GenerateClassModifier(),
             'CLASS_NAME': self._java_data.wrapper_name,
             'CLASS_EXTENDS': self.GenerateClassExtends(),
             'CLASS_IMPLEMENTS': self.GenerateClassImplements(),
             'ENUMS_SECTION': self.GenerateEnums(),
             'FIELDS_SECTION': self.GenerateFields(),
             'MEMBERSS_SECTION': self.GenerateMembers(),
             'INTERNALLY_CONSTRUCOR_SECTION':
                  self.GenerateInternallyConstructor(),
             'METHODS_SECTION': self.GenerateMethods(),
             'REFLECTION_SECTION': self.GenerateReflectionInit()}
    return wrapper_template.substitute(value)

  def GeneratePackage(self):
    # Remove the 'internal' folder from internal package.
    package_name = self._java_data.package_name.replace('.internal', '')
    return 'package %s;\n' % (package_name)

  def GenerateClassModifier(self):
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
        impl_interface = self.GetJavaData(impl_interface).wrapper_name
      return ' implements %s ' % (impl_interface)
    return ''

  def GenerateFields(self):
    field_template = Template("""\
${DOC}
    public final static ${TYPE} ${NAME} = ${VALUE};

""")

    fields_string = ''
    for field in self._java_data.fields:
      value = {'DOC': self.GenerateDoc(field.field_doc),
               'TYPE': field.field_type,
               'NAME': field.field_name,
               'VALUE': field.field_value}
      fields_string += field_template.substitute(value)

    if not self._java_data.HasNoInstanceAnnotation():
      fields_string += "    private ArrayList<Object> constructorTypes;\n"
      fields_string += "    private ArrayList<Object> constructorParams;\n"
      fields_string += "    private ReflectMethod postWrapperMethod;\n"
    if self._java_data.wrapper_name == "XWalkView":
      fields_string += "    private String mAnimatable;\n"
      fields_string += "    \n"
      fields_string += "    private static final String ANIMATABLE = \"animatable\";\n"
      fields_string += "    private static final String XWALK_ATTRS_NAMESPACE\
 = \"http://schemas.android.com/apk/res-auto\";"

    return fields_string

  def GenerateEnums(self):
    enum_template = Template("""\
${DOC}
    public enum ${NAME} {${CONTENT}
    }

    private ReflectMethod ${ENUM_VALUE_OF_METHOD} = new ReflectMethod();

    private Object Convert${NAME}(${NAME} type) {
        return ${ENUM_VALUE_OF_METHOD}.invoke(type.toString());
    }
""")

    enums_string = ''
    for enum in self._java_data.enums.values():
      value = {'DOC': self.GenerateDoc(enum.enum_doc),
               'NAME': enum.enum_name.replace('Internal', ''),
               'CONTENT': enum.enum_declaration,
               'ENUM_VALUE_OF_METHOD': enum.EnumMethodValueOfName()}
      enums_string += enum_template.substitute(value)
    return enums_string

  def GenerateMembers(self):
    if self._java_data.HasNoInstanceAnnotation():
      members_string = """\
    private static XWalkCoreWrapper coreWrapper;
"""
    else:
      members_string = """\
    private XWalkCoreWrapper coreWrapper;
    private Object bridge;

    protected Object getBridge() {
        return bridge;
    }
"""

    return members_string;

  def GenerateInternallyConstructor(self):
    if self._java_data.HasNoInstanceAnnotation():
      return ''
    if not self._java_data.HasCreateInternallyAnnotation():
      return ''

    constructor_template = Template("""\
    public ${CLASS_NAME}(Object bridge) {
        this.bridge = bridge;
        reflectionInit();
    }
""")
    value = {'CLASS_NAME': self._java_data.wrapper_name};
    return constructor_template.substitute(value)

  def GenerateWrapperDefaultConstructor(self):
    template = Template("""\
    public ${NAME}() {
${WRAP_LINES}
        reflectionInit();
    }

""")
    wrap_string = "        constructorTypes = new ArrayList<Object>();\n"
    wrap_string += "        constructorParams = new ArrayList<Object>();\n"
    value = {'NAME': self._java_data.wrapper_name,
             'WRAP_LINES': wrap_string}
    return template.substitute(value)

  def GenerateMethods(self):
    methods_string = ''
    # Generate method definitions.
    if self._java_data.need_default_constructor:
      methods_string += self.GenerateWrapperDefaultConstructor()
    for method in self._java_data.methods:
      methods_string += method.GenerateMethodsStringForWrapper()
    return methods_string

  def GenerateReflectionInit(self):
    if self._java_data.HasNoInstanceAnnotation():
      ref_methods_string = ''

      ref_method_template = Template("""\
        ${METHOD_DECLARE_NAME}.init(null, bridgeClass, "${METHOD_NAME}"${PARAMS});
""")

      for method in self._java_data.methods:
        if not method.is_static:
          continue
        value = { 'METHOD_DECLARE_NAME': method._method_declare_name,
                  'METHOD_NAME': method.method_name,
                  'PARAMS': method._wrapper_params_declare_for_bridge}
        ref_methods_string += ref_method_template.substitute(value)

      ref_init_template = Template("""\
    static void reflectionInit() {
        if (coreWrapper != null) return;

        XWalkCoreWrapper.initEmbeddedMode();

        coreWrapper = XWalkCoreWrapper.getInstance();
        if (coreWrapper == null) {
            XWalkCoreWrapper.reserveReflectClass(${CLASS_NAME}.class);
            return;
        }

        Class<?> bridgeClass = coreWrapper.getBridgeClass("${BRIDGE_NAME}");

${REF_METHODS}    }
  """)

      value = { 'CLASS_NAME': self._java_data.wrapper_name,
                'BRIDGE_NAME': self._java_data._bridge_name,
                'REF_METHODS': ref_methods_string}
      return ref_init_template.substitute(value)

    ref_init_string = """\
        coreWrapper = XWalkCoreWrapper.getInstance();
        if (coreWrapper == null) {
            XWalkCoreWrapper.reserveReflectObject(this);
            return;
        }
"""

    if not self._java_data.HasCreateInternallyAnnotation():
      init_templete = Template("""
        int length = constructorTypes.size();
        Class<?>[] paramTypes = new Class<?>[length+1];
        for (int i = 0; i < length; ++i) {
            Object type = constructorTypes.get(i);
            if (type instanceof String) {
                paramTypes[i] = coreWrapper.getBridgeClass((String) type);
                constructorParams.set(i, \
coreWrapper.getBridgeObject(constructorParams.get(i)));
            } else if (type instanceof Class<?>) {
                paramTypes[i] = (Class<?>) type;
            } else {
                assert(false);
            }
        }

        paramTypes[length] = Object.class;
        constructorParams.add(this);

        ReflectConstructor constructor = new ReflectConstructor(
                coreWrapper.getBridgeClass(\"${BRIDGE_NAME}\"), paramTypes);
        try {
            bridge = constructor.newInstance(constructorParams.toArray());
        } catch (UnsupportedOperationException e) {
            return;
        }

        if (postWrapperMethod != null) postWrapperMethod.invoke();
""")

      value = {'BRIDGE_NAME': self._java_data._bridge_name}
      ref_init_string += init_templete.substitute(value)


    ref_enum_template = Template("""\
        ${METHOD}.init(null,
                coreWrapper.getBridgeClass("${ENUM}"), "valueOf", String.class);
""")

    ref_methods_string = ''
    for enum in self._java_data.enums.values():
      value = {'METHOD': enum.EnumMethodValueOfName(),
               'ENUM': self._java_data.GetBridgeName(enum.enum_name)}
      ref_methods_string += ref_enum_template.substitute(value)

    ref_method_template = Template("""\
        ${METHOD_DECLARE_NAME}.init(bridge, null,
                "${METHOD}Super"${PARAMS});
""")

    if (ref_methods_string != ''):
      ref_methods_string += "\n"
    for method in self._java_data.methods:
      if method.is_constructor or method.is_static or method.is_abstract \
        or method.is_delegate or method.disable_reflect_method:
        continue
      value = { 'METHOD_DECLARE_NAME': method._method_declare_name,
                'METHOD': method.method_name,
                'PARAMS': method._wrapper_params_declare_for_bridge}
      ref_methods_string += ref_method_template.substitute(value)

    ref_init_template = Template("""\
    void reflectionInit() {
        XWalkCoreWrapper.initEmbeddedMode();

${REF_INIT}
${REF_METHODS}    }
""")

    value = {'REF_INIT': ref_init_string,
             'REF_METHODS': ref_methods_string}
    return ref_init_template.substitute(value)
