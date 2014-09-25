#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re

from collections import OrderedDict
from string import Template

def ConvertClassExpressionToClassType(class_name):
  """ Turn "final HashMap<String>" to HashMap.class. """
  return '%s.class' % class_name.split()[-1].split('<')[0]


def ConvertPrimitiveTypeToObject(class_name):
  primitive_map = {
      'byte': 'Byte',
      'short': 'Short',
      'int': 'Integer',
      'long': 'Long',
      'float': 'Float',
      'double': 'Double',
      'char': 'Character',
      'boolean': 'Boolean',
  }
  return primitive_map.get(class_name, class_name)


def HasCreateInternally(java_data):
  java_data_instance = java_data.GetInstanceJavaData()
  if java_data_instance:
    return java_data_instance.HasCreateInternallyAnnotation()
  else:
    return java_data.HasCreateInternallyAnnotation()


class ParamType(object):
  """Internal representation of the type of a parameter of a method."""
  def __init__(self, expression, class_loader):
    self._expression = expression
    self._modifier = ''
    self._generic_type = ''
    self._generic_type_parameters = []
    self._contains_internal_class = False
    self.ParseType(class_loader)
    self._contains_internal_class = self._contains_internal_class or\
        class_loader.IsInternalClass(self._generic_type)

  def ParseType(self, class_loader):
    param_type_re = re.compile('(?P<modifier>(\w+ )*)'
                               '(?P<generic>(\w+))(?P<type_params>(<.*>)?)')
    for match in re.finditer(param_type_re, self._expression):
      self._modifier = match.group('modifier')
      self._generic_type = match.group('generic')
      type_params = match.group('type_params')
      if len(type_params) > 1:
        type_params = type_params[1:-1]
        self._generic_type_parameters = [ParamType(param.strip(),
            class_loader) for param in type_params.split(',')]

    for type_param in self._generic_type_parameters:
      if self.generic_type == 'ValueCallback':
        print 'value callback with %s' % type_param.generic_type
      if type_param.contains_internal_class:
        self._contains_internal_class = True
        break

  @property
  def expression(self):
    return self._expression

  @property
  def modifier(self):
    return self._modifier

  @property
  def generic_type(self):
    return self._generic_type

  @property
  def generic_type_parameters(self):
    return self._generic_type_parameters

  @property
  def contains_internal_class(self):
    return self._contains_internal_class


class ParamStringType(object):
  INTERNAL_DECLARE = 1
  BRIDGE_DECLARE = 2
  BRIDGE_DECLARE_FOR_WRAPPER = 3
  BRIDGE_PASS_TO_SUPER = 4
  BRIDGE_PASS_TO_WRAPPER = 5
  INTERNAL_PASS_TO_BRIDGE = 6
  BRIDGE_OVERRIDE_CONDITION = 7
  WRAPPER_DECLARE = 8
  WRAPPER_DECLARE_FOR_BRIDGE = 9
  WRAPPER_PASS_TO_BRIDGE = 10


class MethodStringType(object):
  BRIDGE_CONSTRUCTOR = 1
  BRIDGE_STATIC = 2
  BRIDGE_SUPER = 3
  BRIDGE_OVERRIDE = 4
  BRIDGE_WRAPPER = 5
  WRAPPER_CONSTRUCTOR = 6
  WRAPPER_STATIC = 7
  WRAPPER_BRIDGE = 8
  WRAPPER_INTERFACE = 9


class Method(object):
  """Internal representaion of a method."""
  ANNOTATION_PRE_WRAPLINE = 'preWrapperLines'
  ANNOTATION_POST_WRAPLINE = 'postWrapLines'

  def __init__(self, class_name, class_loader,
      is_constructor, is_static, is_abstract,
      method_name, method_return, params, annotation, doc=''):
    self._class_name = class_name
    self._class_loader = class_loader
    self._is_constructor = is_constructor
    self._is_static = is_static
    self._is_abstract = is_abstract
    self._method_name = method_name
    self._method_return = method_return
    self._params = OrderedDict() # Use OrderedDict to avoid parameter misorder.
    self._typed_params = OrderedDict()
    self._method_annotations = {}
    self._method_doc = doc
    self._class_java_data = ''
    self._method_declare_name = ''
    self._internal_params_declare = ''
    self._bridge_params_declare = ''
    self._bridge_params_declare_for_wrapper = ''
    self._bridge_params_pass_to_super = ''
    self._bridge_params_pass_to_wrapper = ''
    self._internal_params_pass_to_bridge = ''
    self._bridge_override_condition = ''
    self._wrapper_params_declare = ''
    self._wrapper_params_declare_for_bridge = ''
    self._wrapper_params_pass_to_bridge = ''
    self._strings_prepared = False
    self.ParseMethodParams(params)
    self.ParseMethodAnnotation(annotation)

  def IsInternalClass(self, clazz):
    return self._class_loader.IsInternalClass(clazz)

  def LoadJavaClass(self, clazz):
    return self._class_loader.LoadJavaClass(clazz)

  def GenerateDoc(self, doc):
    return self._class_loader.GenerateDoc(doc)

  @property
  def is_constructor(self):
    return self._is_constructor

  @property
  def is_static(self):
    return self._is_static

  @property
  def is_abstract(self):
    return self._is_abstract

  @property
  def method_name(self):
    return self._method_name

  @property
  def method_return(self):
    return self._method_return

  @property
  def params(self):
    return self._params

  @property
  def typed_params(self):
    return self._typed_params

  @property
  def method_annotations(self):
    return self._method_annotations

  @property
  def method_doc(self):
    return self._method_doc

  def ParseMethodParams(self, params):
    # TODO(shouqun): Currently, generic parameters are not supported.
    # The support of generic types should be added if such cases happen.
    if not params or params == '':
      return
    for param in params.split(','):
      param = param.strip()
      param_list = param.split()
      param_type = ' '.join(param_list[:-1]) # To handle modifiers
      param_name = param_list[-1]
      self._params[param_name] = param_type
      self._typed_params[param_name] = ParamType(param_type, self._class_loader)

  def ParseMethodAnnotation(self, annotation):
    pre_wrapline_re = re.compile('preWrapperLines\s*=\s*\{\s*('
        '?P<pre_wrapline>(".*")(,\s*".*")*)\s*\}')
    for match in re.finditer(pre_wrapline_re, annotation):
      pre_wrapline = self.FormatWrapperLine(match.group('pre_wrapline'))
      self._method_annotations[self.ANNOTATION_PRE_WRAPLINE] = pre_wrapline

    post_wrapline_re = re.compile('postWrapperLines\s*=\s*\{\s*('
        '?P<post_wrapline>(".*")(,\s*".*")*)\s*\}')
    for match in re.finditer(post_wrapline_re, annotation):
      post_wrapline = self.FormatWrapperLine(match.group('post_wrapline'))
      self._method_annotations[self.ANNOTATION_POST_WRAPLINE] = post_wrapline

  def FormatWrapperLine(self, annotation_value):
    """ annotaion_value is a java string array which each element is an
        individual line. Probably like: '    "line1",\n    "line2"'
        This method is turnning it to '    line1\n    line2'
    """
    lines = []
    exec('lines = [%s]' % annotation_value.replace('\n', ''))
    template = Template('\n'.join(lines))
    values = {}
    for arg in range(1, len(self.params.keys())+1):
      values['param%d' % arg] = self.params.keys()[arg-1]
    return template.substitute(values)

  def GetBridgeParamsStringDeclareForWrapper(self):
    self.PrepareStrings()
    return self._bridge_params_declare_for_wrapper

  def GetWrapperParamsStringDeclareForBridge(self):
    self.PrepareStrings()
    return self._wrapper_params_declare_for_bridge

  def GetMethodDeclareName(self):
    self.PrepareStrings()
    return self._method_declare_name

  def PrepareStrings(self):
    if self._strings_prepared:
      return
    self._class_java_data = self.LoadJavaClass(self._class_name)
    self._method_declare_name = self.GenerateMethodDeclareName()

    self._internal_params_declare = ', '.join(
        self.GetFormattedParamArray(ParamStringType.INTERNAL_DECLARE))
    self._bridge_params_declare = ', '.join(
        self.GetFormattedParamArray(ParamStringType.BRIDGE_DECLARE))
    self._bridge_params_declare_for_wrapper = ', '.join(
        self.GetFormattedParamArray(
            ParamStringType.BRIDGE_DECLARE_FOR_WRAPPER, insert_empty=True))
    self._bridge_params_pass_to_super = ', '.join(
        self.GetFormattedParamArray(ParamStringType.BRIDGE_PASS_TO_SUPER))
    self._bridge_params_pass_to_wrapper = ', '.join(
        self.GetFormattedParamArray(
            ParamStringType.BRIDGE_PASS_TO_WRAPPER, insert_empty=True))
    self._internal_params_pass_to_bridge = ', '.join(
        self.GetFormattedParamArray(ParamStringType.INTERNAL_PASS_TO_BRIDGE))
    self._bridge_override_condition = ' && '.join(
        self.GetFormattedParamArray(ParamStringType.BRIDGE_OVERRIDE_CONDITION))
    self._wrapper_params_declare = ', '.join(
        self.GetFormattedParamArray(ParamStringType.WRAPPER_DECLARE))
    self._wrapper_params_declare_for_bridge = ', '.join(
        self.GetFormattedParamArray(ParamStringType.WRAPPER_DECLARE_FOR_BRIDGE,
                                    insert_empty=True))
    self._wrapper_params_pass_to_bridge = ', '.join(
        self.GetFormattedParamArray(
            ParamStringType.WRAPPER_PASS_TO_BRIDGE, insert_empty=True))

    self._strings_prepared = True

  def GetFormattedParamArray(self, param_string_type,
      append_empty=False, insert_empty=False):
    """ Return the array of params with specified format.
        append or insert an empty string on demand for cases
        that need extra splitter when using the array.
    """
    formatted_params = []
    for param_name in self._params:
      param_type = self._params[param_name]
      formatted_param = self.FormatSingleParam(
          param_type, param_name, param_string_type)
      if formatted_param:
        formatted_params.append(formatted_param)
    if append_empty:
      formatted_params.append('')
    if insert_empty:
      formatted_params.insert(0, '')
    return formatted_params

  def FormatSingleParam(self, param_type, param_name, param_string_type):
    is_internal_class = self.IsInternalClass(param_type)
    if is_internal_class:
      java_data = self.LoadJavaClass(param_type)
    typed_param = self._typed_params[param_name]
    if param_string_type == ParamStringType.INTERNAL_DECLARE:
      # the way internal declares its params, will be used in bridge's override
      # call.
      #   XWalkViewInternal view => XWalkViewInternal view
      return '%s %s' % (param_type, param_name)
    elif param_string_type == ParamStringType.BRIDGE_DECLARE:
      # the way bridge declares its params, will be used in bridge's wrapper
      # call and super call.
      #   XWalkViewInternal view => XWalkViewBridge view
      if is_internal_class:
        return '%s %s' % (java_data.UseAsTypeInBridgeAndBridgeSuperCall(),
                          param_name)
      else:
        return '%s %s' % (param_type, param_name)
    elif param_string_type == ParamStringType.BRIDGE_DECLARE_FOR_WRAPPER:
      # the way bridge declares its params for wrapper, will turn the param
      # type to class<?> value for reflection to use.
      #   XWalkViewInternal view => "org.xwalk.core.XWalkView"
      #   DirectionInternal direnction => enumDirectionClass
      #   String name => String.class
      if is_internal_class:
        return '"%s"' % java_data.GetFullWrapperName()
      else:
        # TODO(wang16): Here only detects enum declared in the same class as
        # the method itself. Using enum across class is not supported.
        enums = self._class_java_data.enums
        if param_type in enums:
          return enums[param_type].EnumClassName()
        else:
          return ConvertClassExpressionToClassType(param_type)
    elif param_string_type == ParamStringType.BRIDGE_PASS_TO_SUPER:
      # the way bridge passes the param to super
      #   XWalkViewInternal view => view
      if is_internal_class:
        return java_data.UseAsInstanceInBridgeSuperCall(param_name)
      else:
        return param_name
    elif param_string_type == ParamStringType.BRIDGE_PASS_TO_WRAPPER:
      # the way bridge passes the param to wrapper
      #   XWalkViewInternal view => view.getWrapper()
      #   DirectionInternal direction => ConvertDirectionInternal(direction)
      if is_internal_class:
        return java_data.UseAsInstanceInBridgeCall(param_name)
      elif (typed_param.generic_type == 'ValueCallback' and
          typed_param.contains_internal_class):
        assert len(typed_param.generic_type_parameters) == 1
        internal_generic_type_param = typed_param.generic_type_parameters[0]
        internal_generic_type_class = self.LoadJavaClass(
            internal_generic_type_param.generic_type)
        return ('new ValueCallback<Object>() {\n' +
          '                @Override\n' +
          '                public void onReceiveValue(Object value) {\n' +
          '                    %sFinal.onReceiveValue((%s) ' % (
              param_name, internal_generic_type_class.bridge_name) +
          'ReflectionHelper.getBridgeOrWrapper(value));\n' +
          '                }\n' +
          '            }')
      else:
        # TODO(wang16): Here only detects enum declared in the same class as
        # the method itself. Using enum across class is not supported.
        enums = self._class_java_data.enums
        if param_type in enums:
          return 'Convert%s(%s)' % (param_type, param_name)
        else:
          return param_name
    elif param_string_type == ParamStringType.INTERNAL_PASS_TO_BRIDGE:
      # the way bridge accepts param from internal
      #   XWalkViewInternal view => (XWalkViewBridge) view
      if is_internal_class:
        return java_data.UseAsInstanceInBridgeOverrideCall(param_name)
      else:
        return param_name
    elif param_string_type == ParamStringType.BRIDGE_OVERRIDE_CONDITION:
      # the way bridge uses as the condition for whether call super or
      # call wrapper in override call
      #   XWalkViewInternal view => (view instanceof XWalkViewBridge)
      if is_internal_class and not HasCreateInternally(java_data):
        return'(%s instanceof %s)' % (
            param_name,
            java_data.UseAsTypeInBridgeAndBridgeSuperCall())
      else:
        return None
    elif param_string_type == ParamStringType.WRAPPER_DECLARE:
      # the way wrapper declare the param
      #   XWalkViewInternal view => XWalkView view
      #   DirectionInternal direction => Direction direction
      if is_internal_class:
        return '%s %s' % (java_data.UseAsTypeInWrapperCall(), param_name)
      elif param_type in self._class_java_data.enums:
        # TODO(wang16): Here only detects enum declared in the same class as
        # the method itself. Using enum across class is not supported.
        return '%s %s' % (param_type.replace('Internal', ''), param_name)
      else:
        return '%s %s' % (param_type, param_name)
    elif param_string_type == ParamStringType.WRAPPER_DECLARE_FOR_BRIDGE:
      # the way wrapper declares its params for bridge, will turn the param
      # type to class<?> value for reflection to use.
      #   XWalkViewInternal view => "org.xwalk.core.internal.XWalkViewBridge"
      #   DirectionInternal direction => enumDirectionClass
      #   String name => String.class

      # TODO(wang16): Currently there is no internal classes for static method.
      # Need to support it in future.
      if is_internal_class:
        return '"%s"' % java_data.GetFullBridgeName()
      else:
        # TODO(wang16): Here only detects enum declared in the same class as
        # the method itself. Using enum across class is not supported.
        enums = self._class_java_data.enums
        if param_type in enums:
          return enums[param_type].EnumClassName()
        else:
          return ConvertClassExpressionToClassType(param_type)
    elif param_string_type == ParamStringType.WRAPPER_PASS_TO_BRIDGE:
      # the way wrapper passes param to bridge
      #   XWalkViewInternal view => view.getBridge()
      #   DirectionInternal direction => ConvertDirection(direction)
      if is_internal_class:
        return java_data.UseAsInstanceInWrapperCall(param_name)
      elif param_type in self._class_java_data.enums:
        # TODO(wang16): Here only detects enum declared in the same class as
        # the method itself. Using enum across class is not supported.
        return 'Convert%s(%s)' % (param_type.replace('Internal', ''),
                                  param_name)
      else:
        return param_name
    else:
      pass

  def GenerateMethodDeclareName(self):
    name = self.method_name
    for param_name in self.params:
      # Remove modifier and generic type.
      name += ConvertClassExpressionToClassType(
          self.params[param_name]).replace('.class', '')
    if self._is_constructor:
      return '%sConstructor' % name
    else:
      return '%sMethod' % name

  def GenerateBridgeConstructor(self):
    template = Template(
        '    public ${NAME}(${PARAMS}, Object wrapper) {\n' +
        '        super(${PARAMS_PASSING});\n' +
        '        this.wrapper = wrapper;\n' +
        '        try {\n' +
        '            reflectionInit();\n' +
        '        } catch (Exception e) {\n' +
        '            ReflectionHelper.handleException(e);\n'+
        '        }\n' +
        '    }\n\n')
    value = {'NAME': self._class_java_data.bridge_name,
             'PARAMS': self._bridge_params_declare,
             'PARAMS_PASSING': self._bridge_params_pass_to_super}
    return template.substitute(value)

  def GenerateBridgeStaticMethod(self):
    no_return_value = self._method_return == 'void'
    template = Template(
        '    public static ${RETURN_TYPE} ${NAME}($PARAMS) {\n' +
        '        ${RETURN}${CLASS_NAME}.${NAME}(${PARAMS_PASSING});\n' +
        '    }\n\n')
    value = {'RETURN_TYPE': self.method_return,
             'NAME': self.method_name,
             'PARAMS': self._bridge_params_declare,
             'RETURN': '' if no_return_value else 'return ',
             'CLASS_NAME': self._class_name,
             'PARAMS_PASSING': self._bridge_params_pass_to_super}
    return template.substitute(value)

  def GenerateBridgeOverrideMethod(self):
    no_return_value = self._method_return == 'void'
    if not self._bridge_override_condition:
      return '    @Override\n'
    template = Template(
        '    @Override\n' +
        '    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {\n' +
        '        if (${IF_CONDITION}) {\n' +
        '            ${RETURN}${NAME}(${BRIDGE_PARAMS_PASSING});\n' +
        '        } else {\n' +
        '            ${RETURN}super.${NAME}(${PARAMS_PASSING});\n' +
        '        }\n' +
        '    }\n\n')
    value = {'NAME': self.method_name,
             'RETURN_TYPE': self.method_return,
             'PARAMS': self._internal_params_declare,
             'RETURN': '' if no_return_value else 'return ',
             'IF_CONDITION': self._bridge_override_condition,
             'PARAMS_PASSING': self._bridge_params_pass_to_super,
             'BRIDGE_PARAMS_PASSING': self._internal_params_pass_to_bridge}
    return template.substitute(value)

  def GenerateBridgeWrapperMethod(self):
    no_return_value = self._method_return == 'void'
    return_is_internal = self.IsInternalClass(self._method_return)
    if return_is_internal:
      return_type_java_data = self.LoadJavaClass(self._method_return)

    if return_is_internal:
      template = Template(
          '    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {\n' +
          '${GENERIC_TYPE_DECLARE}' +
          '        ${RETURN}ReflectionHelper.getBridgeOrWrapper(' + 
          'ReflectionHelper.invokeMethod(\n' +
          '            ${METHOD_DECLARE_NAME}, wrapper${PARAMS_PASSING}));\n' +
          '    }\n\n')
    else :
      template = Template(
          '    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {\n' +
          '${GENERIC_TYPE_DECLARE}' +
          '        ${RETURN}ReflectionHelper.invokeMethod(\n' +
          '            ${METHOD_DECLARE_NAME}, wrapper${PARAMS_PASSING});\n' +
          '    }\n\n')

    if no_return_value:
      return_statement = ''
    elif return_is_internal:
      return_statement = 'return (%s)' % return_type_java_data.bridge_name
    else:
      return_statement = 'return (%s)' % (
          ConvertPrimitiveTypeToObject(self.method_return))

    # Handling generic types, current only ValueCallback will be handled.
    generic_type_declare = ''
    for param_name in self._typed_params:
      typed_param = self._typed_params[param_name]
      if typed_param.generic_type != 'ValueCallback':
        continue
      if typed_param.contains_internal_class:
        generic_type_declare += '        final %s %sFinal = %s;\n' % (
            typed_param.expression, param_name, param_name)

    value = {'RETURN_TYPE': self.method_return,
             'NAME': self.method_name,
             'METHOD_DECLARE_NAME': self._method_declare_name,
             'PARAMS': self._bridge_params_declare,
             'RETURN': return_statement,
             'GENERIC_TYPE_DECLARE': generic_type_declare,
             'PARAMS_PASSING': self._bridge_params_pass_to_wrapper}
    return template.substitute(value)

  def GenerateBridgeSuperMethod(self):
    no_return_value = self._method_return == 'void'
    return_is_internal = self.IsInternalClass(self._method_return)
    if return_is_internal:
      return_type_java_data = self.LoadJavaClass(self._method_return)

    if self._is_abstract:
      return ''

    if self._class_java_data.HasCreateInternallyAnnotation():
      if no_return_value:
        template = Template(
            '    public void ${NAME}Super(${PARAMS}) {\n' +
            '        if (internal == null) {\n' +
            '            super.${NAME}(${PARAM_PASSING});\n' +
            '        } else {\n' +
            '            internal.${NAME}(${PARAM_PASSING});\n' +
            '        }\n' +
            '    }\n\n')
      else:
        template = Template(
            '    public ${RETURN_TYPE} ${NAME}Super(${PARAMS}) {\n' +
            '        ${INTERNAL_RETURN_TYPE} ret;\n' +
            '        if (internal == null) {\n' +
            '            ret = super.${NAME}(${PARAM_PASSING});\n' +
            '        } else {\n' +
            '            ret = internal.${NAME}(${PARAM_PASSING});\n' +
            '        }\n' +
            '        ${IF_NULL_RETURN_NULL}\n' +
            '        return ${RETURN_VALUE};\n' +
            '    }\n\n')
    else:
      if no_return_value:
        template = Template(
            '    public void ${NAME}Super(${PARAMS}) {\n' +
            '        super.${NAME}(${PARAM_PASSING});\n' +
            '    }\n\n')
      else:
        template = Template(
            '    public ${RETURN_TYPE} ${NAME}Super(${PARAMS}) {\n' +
            '        ${INTERNAL_RETURN_TYPE} ret;\n' +
            '        ret = super.${NAME}(${PARAM_PASSING});\n' +
            '        ${IF_NULL_RETURN_NULL}\n' +
            '        return ${RETURN_VALUE};\n' +
            '    }\n\n')

    if return_is_internal:
      return_value = return_type_java_data.UseAsReturnInBridgeSuperCall('ret')
      method_return = return_type_java_data.bridge_name
    else:
      return_value = 'ret'
      method_return = self._method_return

    if ConvertPrimitiveTypeToObject(method_return) != method_return:
      # it's returning prmitive type, so it can't be null.
      if_null_return_null = ''
    else:
      if_null_return_null = 'if (ret == null) return null;'
    value = {
        'RETURN_TYPE': method_return,
        'INTERNAL_RETURN_TYPE': self.method_return,
        'NAME': self.method_name,
        'PARAM_PASSING': self._bridge_params_pass_to_super,
        'PARAMS': self._bridge_params_declare,
        'IF_NULL_RETURN_NULL': if_null_return_null,
        'RETURN_VALUE': return_value
    }

    return template.substitute(value)

  def GenerateWrapperConstructor(self):
    # TODO(wang16): Currently, only support pre/post wrapper lines for
    # Constructors.
    template = Template(
        '${DOC}\n' +
        '    public ${CLASS_NAME}(${PARAMS}) {\n' +
        '${PRE_WRAP_LINES}\n' +
        '        bridge = ReflectionHelper.createInstance(' +
        '\"${CONSTRUCTOR_DECLARE_NAME}\"${PARAMS_PASSING}, this);\n' +
        '        try {\n' +
        '            reflectionInit();\n' +
        '        } catch(Exception e) {\n' +
        '            ReflectionHelper.handleException(e);\n' +
        '        }\n' +
        '${POST_WRAP_LINES}\n' +
        '    }\n\n')
    value = {'CLASS_NAME': self._class_java_data.wrapper_name,
             'DOC': self.GenerateDoc(self.method_doc),
             'PARAMS': self._wrapper_params_declare,
             'PARAMS_PASSING': self._wrapper_params_pass_to_bridge,
             'CONSTRUCTOR_DECLARE_NAME': self._method_declare_name,
             'PRE_WRAP_LINES': self._method_annotations.get(
                 self.ANNOTATION_PRE_WRAPLINE, ''),
             'POST_WRAP_LINES': self._method_annotations.get(
                 self.ANNOTATION_POST_WRAPLINE, '')}
    return template.substitute(value)

  def GenerateWrapperStaticMethod(self):
    no_return_value = self._method_return == 'void'
    template = Template(
        '${DOC}\n' +
        '    public static ${RETURN_TYPE} ${NAME}(${PARAMS}) {\n' +
        '       Class<?> clazz = ReflectionHelper.loadClass(' +
        '\"${FULL_BRIDGE_NAME}\");\n' +
        '       Method method = ReflectionHelper.loadMethod(clazz, ' +
        '\"${NAME}\"${PARAMS_DECLARE_FOR_BRIDGE});\n' +
        '       ${RETURN}ReflectionHelper.invokeMethod(method, null' +
        '${PARAMS_PASSING});\n' +
        '    }\n\n')
    if no_return_value:
      return_state = ''
    else:
      return_state = 'return (%s)' % ConvertPrimitiveTypeToObject(
          self.method_return)
    value = {'RETURN_TYPE': self.method_return,
             'DOC': self.GenerateDoc(self.method_doc),
             'NAME': self.method_name,
             'FULL_BRIDGE_NAME': self._class_java_data.GetFullBridgeName(),
             'PARAMS_DECLARE_FOR_BRIDGE':
                 self._wrapper_params_declare_for_bridge,
             'PARAMS_PASSING': self._wrapper_params_pass_to_bridge,
             'PARAMS': self._wrapper_params_declare,
             'RETURN': return_state}
    return template.substitute(value)

  def GenerateWrapperBridgeMethod(self):
    no_return_value = self._method_return == 'void'
    return_is_internal = self.IsInternalClass(self._method_return)
    if return_is_internal:
      return_type_java_data = self.LoadJavaClass(self._method_return)

    if self.is_abstract:
      template = Template(
          '${DOC}\n' +
          '    public abstract ${RETURN_TYPE} ${NAME}(${PARAMS});\n\n')
    elif return_is_internal:
      template = Template(
          '${DOC}\n' +
          '    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {\n' +
          '        return (${RETURN_TYPE})ReflectionHelper.' +
          'getBridgeOrWrapper(\n' +
          '            ReflectionHelper.invokeMethod(' +
          '${METHOD_DECLARE_NAME}, bridge${PARAMS_PASSING}));\n' +
          '    }\n\n')
    else:
      template = Template(
          '${DOC}\n' +
          '    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {\n' +
          '        ${RETURN}ReflectionHelper.invokeMethod(' +
          '${METHOD_DECLARE_NAME}, bridge${PARAMS_PASSING});\n' +
          '    }\n\n')
    if return_is_internal:
      return_type = return_type_java_data.wrapper_name
    else:
      return_type = self.method_return
    if no_return_value:
      return_state = ''
    else:
      return_state = 'return (%s)' % ConvertPrimitiveTypeToObject(return_type)
    value = {'RETURN_TYPE': return_type,
             'RETURN': return_state,
             'DOC': self.GenerateDoc(self.method_doc),
             'NAME': self.method_name,
             'PARAMS': re.sub(r'ValueCallback<([A-Za-z]+)Internal>', 
                  r'ValueCallback<\1>',self._wrapper_params_declare),
             'METHOD_DECLARE_NAME': self._method_declare_name,
             'PARAMS_PASSING': self._wrapper_params_pass_to_bridge}

    return template.substitute(value)

  def GenerateWrapperInterface(self):
    return_is_internal = self.IsInternalClass(self._method_return)
    if return_is_internal:
      return_type_java_data = self.LoadJavaClass(self._method_return)

    template = Template(
        '${DOC}\n' +
        '    public ${RETURN_TYPE} ${NAME}(${PARAMS});\n\n')
    if return_is_internal:
      return_type = return_type_java_data.wrapper_name
    else:
      return_type = self.method_return
    value = {'RETURN_TYPE': return_type,
             'DOC': self.GenerateDoc(self.method_doc),
             'NAME': self.method_name,
             'PARAMS': self._wrapper_params_declare}
    return template.substitute(value)

  def GenerateMethodsStringForBridge(self):
    self.PrepareStrings()
    if self._is_constructor:
      return self.GenerateBridgeConstructor()
    elif self._is_static:
      return self.GenerateBridgeStaticMethod()
    else:
      return '%s%s%s%s' % (
          '    private Method %s;\n' % self._method_declare_name,
          self.GenerateBridgeOverrideMethod(),
          self.GenerateBridgeWrapperMethod(),
          self.GenerateBridgeSuperMethod())

  def GenerateMethodsStringForWrapper(self):
    self.PrepareStrings()
    if self._is_constructor:
      return self.GenerateWrapperConstructor()
    elif self._is_static:
      return self.GenerateWrapperStaticMethod()
    elif self._is_abstract:
      return self.GenerateWrapperBridgeMethod()
    else:
      return '%s%s' % (
          '    private Method %s;\n' % self._method_declare_name,
          self.GenerateWrapperBridgeMethod())

  def GenerateMethodsStringForInterface(self):
    self.PrepareStrings()
    return self.GenerateWrapperInterface()
