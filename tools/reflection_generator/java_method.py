# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re

from collections import OrderedDict
from string import Template

def ConvertClassExpressionToClassType(class_name):
  """ Turn "Map<String, String>" to Map.class. """
  generic_re = re.compile('[a-zA-z0-9]+(\<[a-zA-Z0-9]+,\s[a-zA-z0-9]+\>)')
  if re.match(generic_re, class_name):
    return '%s.class' % class_name.split('<')[0]
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


def GetPrimitiveTypeDefaultValue(class_name):
  primitive_map = {
      'byte': '0',
      'short': '0',
      'int': '0',
      'long': '0L',
      'float': '0.0f',
      'double': '0.0d',
      'char': "'\u0000'",
      'boolean': 'false',
  }
  return primitive_map.get(class_name, 'null')


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
  ANNOTATION_POST_WRAPLINE = 'postWrapperLines'
  ANNOTATION_POST_BRIDGELINE = 'postBridgeLines'

  def __init__(self, class_name, class_loader,
      is_constructor, is_static, is_abstract, is_deprecated,
      method_name, method_return, params, annotation, doc=''):
    self._class_name = class_name
    self._class_loader = class_loader
    self._is_constructor = is_constructor
    self._is_static = is_static
    self._is_abstract = is_abstract
    self._is_deprecated = is_deprecated
    self._is_delegate = False
    self._disable_reflect_method = False
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
    self._is_reservable = False
    self._parameter_is_internal = False

    self.ParseMethodParams(params)
    self.ParseMethodAnnotation(annotation)

  def IsInternalClass(self, clazz):
    return self._class_loader.IsInternalClass(clazz)

  def GetJavaData(self, clazz):
    return self._class_loader.GetJavaData(clazz)

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
  def is_deprecated(self):
    return self._is_deprecated

  @property
  def is_reservable(self):
    return self._is_reservable

  @property
  def is_delegate(self):
    return self._is_delegate

  @property
  def disable_reflect_method(self):
    return self._disable_reflect_method

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
    subparams = re.findall("<.*?>", params) # To handle Map type
    for index in range(len(subparams)):
      params = params.replace(subparams[index], subparams[index].replace(", ", "-"))
    for param in params.split(','):
      param = param.strip()
      param_list = param.split()
      param_type = ' '.join(param_list[:-1]) # To handle modifiers
      if re.search("<.*?>", param_type):
        param_type = param_type.replace("-", ", ")
      param_name = param_list[-1]
      self._params[param_name] = param_type
      self._typed_params[param_name] = ParamType(param_type, self._class_loader)

  def ParseMethodAnnotation(self, annotation):
    if annotation.find('reservable = true') >= 0:
      self._is_reservable = True

    delegate_re = re.compile('delegate\s*=\s*'
        '(?P<delegate>(true|false))')
    for match in re.finditer(delegate_re, annotation):
      delegate = match.group('delegate')
      if delegate == 'true':
        self._is_delegate = True
      elif delegate == 'false':
        self._is_delegate = False

    disable_reflect_method_re = re.compile('disableReflectMethod\s*=\s*'
        '(?P<disableReflectMethod>(true|false))')
    for match in re.finditer(disable_reflect_method_re, annotation):
      disable_reflect_method = match.group('disableReflectMethod')
      if disable_reflect_method == 'true':
        self._disable_reflect_method = True
      else:
        self._disable_reflect_method = False

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

    post_bridgeline_re = re.compile('postBridgeLines\s*=\s*\{\s*('
        '?P<post_bridgeline>(".*")(,\s*".*")*)\s*\}')
    for match in re.finditer(post_bridgeline_re, annotation):
      post_bridgeline = self.FormatWrapperLine(match.group('post_bridgeline'))
      self._method_annotations[self.ANNOTATION_POST_BRIDGELINE] = post_bridgeline

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

  def PrepareStrings(self):
    self._class_java_data = self.GetJavaData(self._class_name)
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
        self.GetFormattedParamArray(ParamStringType.BRIDGE_PASS_TO_WRAPPER))
    self._internal_params_pass_to_bridge = ', '.join(
        self.GetFormattedParamArray(ParamStringType.INTERNAL_PASS_TO_BRIDGE))
    self._bridge_override_condition = ' && '.join(
        self.GetFormattedParamArray(ParamStringType.BRIDGE_OVERRIDE_CONDITION))
    self._wrapper_params_declare = ', '.join(
        self.GetFormattedParamArray(ParamStringType.WRAPPER_DECLARE))
    self._wrapper_params_declare_for_bridge = ', '.join(
        self.GetFormattedParamArray(
            ParamStringType.WRAPPER_DECLARE_FOR_BRIDGE, insert_empty=True))
    self._wrapper_params_pass_to_bridge = ', '.join(
        self.GetFormattedParamArray(ParamStringType.WRAPPER_PASS_TO_BRIDGE))

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
      java_data = self.GetJavaData(param_type)
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
        return '%s %s'% (java_data.GetBridgeName(), param_name)
      else:
        return '%s %s' % (param_type, param_name)
    elif param_string_type == ParamStringType.BRIDGE_DECLARE_FOR_WRAPPER:
      # the way bridge declares its params for wrapper, will turn the param
      # type to class<?> value for reflection to use.
      #   XWalkViewInternal view => coreBridge.getWrapperClass("XWalkView")
      #   DirectionInternal direnction =>
      #       coreBridge.getWrapperClass("XWalkView$Direction")
      #   String name => String.class
      if is_internal_class:
        return 'coreBridge.getWrapperClass("%s")' % java_data.GetWrapperName()
      else:
        # TODO(wang16): Here only detects enum declared in the same class as
        # the method itself. Using enum across class is not supported.
        if param_type in self._class_java_data.enums:
          return ('coreBridge.getWrapperClass("%s")' %
              self._class_java_data.GetWrapperName(param_type))
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
        internal_generic_type_class = self.GetJavaData(
            internal_generic_type_param.generic_type)
        return ('new ValueCallback<Object>() {\n' +
          '                @Override\n' +
          '                public void onReceiveValue(Object value) {\n' +
          '                    %sFinal.onReceiveValue((%s) ' % (
              param_name, internal_generic_type_class.bridge_name) +
          'coreBridge.getBridgeObject(value));\n' +
          '                }\n' +
          '            }')
      else:
        # TODO(wang16): Here only detects enum declared in the same class as
        # the method itself. Using enum across class is not supported.
        if param_type in self._class_java_data.enums:
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
      if is_internal_class:
        if not java_data.HasInstanceCreateInternallyAnnotation():
          return'(%s instanceof %s)' % (param_name, java_data.GetBridgeName())
        else:
          self._parameter_is_internal = True
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
      #   XWalkViewInternal view =>
      #       coreWrapper.getBridgeClass("XWalkViewBridge")
      #   DirectionInternal direction => enumDirectionClass
      #   String name => String.class

      # TODO(wang16): Currently there is no internal classes for static method.
      # Need to support it in future.
      if is_internal_class:
        return 'coreWrapper.getBridgeClass("%s")' % java_data.GetBridgeName()
      else:
        # TODO(wang16): Here only detects enum declared in the same class as
        # the method itself. Using enum across class is not supported.
        enums = self._class_java_data.enums
        if param_type in enums:
          return ('coreWrapper.getBridgeClass("%s")' %
              self._class_java_data.GetBridgeName(param_type))
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
    name = name.replace('[]', 'Array');
    if self._is_constructor:
      return '%sConstructor' % name
    else:
      return '%sMethod' % name

  def GenerateBridgeConstructor(self):
    if (self._bridge_params_declare != ''):
      template = Template("""\
    public ${NAME}(${PARAMS}, Object wrapper) {
        super(${PARAMS_PASSING});

        this.wrapper = wrapper;
        reflectionInit();
${POST_BRIDGE_LINES}
    }

    """)

      post_bridge_string = self._method_annotations.get(
          self.ANNOTATION_POST_BRIDGELINE, '')

      value = {'NAME': self._class_java_data.bridge_name,
               'PARAMS': self._bridge_params_declare,
               'PARAMS_PASSING': self._bridge_params_pass_to_super,
               'POST_BRIDGE_LINES': post_bridge_string}
      return template.substitute(value)
    else:
      template = Template("""\
    public ${NAME}(Object wrapper) {
        super();

        this.wrapper = wrapper;
        reflectionInit();
    }

    """)
      value = {'NAME': self._class_java_data.bridge_name,
               'PARAMS': self._bridge_params_declare,
               'PARAMS_PASSING': self._bridge_params_pass_to_super}
      return template.substitute(value)

  def GenerateBridgeStaticMethod(self):
    template = Template("""\
    public static ${RETURN_TYPE} ${NAME}($PARAMS) {
        ${RETURN}${CLASS_NAME}.${NAME}(${PARAMS_PASSING});
    }
""")

    value = {'RETURN_TYPE': self.method_return,
             'NAME': self.method_name,
             'PARAMS': self._bridge_params_declare,
             'RETURN': '' if self._method_return == 'void' else 'return ',
             'CLASS_NAME': self._class_name,
             'PARAMS_PASSING': self._bridge_params_pass_to_super}
    return template.substitute(value)

  def GenerateBridgeOverrideMethod(self):
    if not self._bridge_override_condition and not self._parameter_is_internal:
      return '    @Override'
    # If _bridge_override_condition and _parameter_is_internal are True
    # concurrently, _bridge_override_condition should be treated in priority.
    if self._bridge_override_condition:
      template = Template("""\
    @Override
    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {
        if (${IF_CONDITION}) {
            ${RETURN}${NAME}(${BRIDGE_PARAMS_PASSING});
        } else {
            ${RETURN}super.${NAME}(${PARAMS_PASSING});
        }
    }
""")
    else:
      template = Template("""\
    @Override
    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {
        ${RETURN}${NAME}(${BRIDGE_PARAMS_PASSING});
    }
""")

    value = {'NAME': self.method_name,
             'RETURN_TYPE': self.method_return,
             'PARAMS': self._internal_params_declare,
             'RETURN': '' if self._method_return == 'void' else 'return ',
             'IF_CONDITION': self._bridge_override_condition,
             'PARAMS_PASSING': self._bridge_params_pass_to_super,
             'BRIDGE_PARAMS_PASSING': self._internal_params_pass_to_bridge}
    return template.substitute(value)

  def GenerateBridgeWrapperMethod(self):
    return_is_internal = self.IsInternalClass(self._method_return)
    if return_is_internal:
      return_type_java_data = self.GetJavaData(self._method_return)

    if return_is_internal:
      template = Template("""\
    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {
        if (${METHOD_DECLARE_NAME} == null || ${METHOD_DECLARE_NAME}.isNull()) {
            ${RETURN_SUPER}${NAME}Super(${PARAMS_PASSING_SUPER});
        } else {
            ${GENERIC_TYPE_DECLARE}${RETURN}coreBridge.getBridgeObject(\
${METHOD_DECLARE_NAME}.invoke(${PARAMS_PASSING}));
        }
    }
""")
    elif self._is_abstract:
      template = Template("""\
    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {
        ${GENERIC_TYPE_DECLARE}${RETURN}${METHOD_DECLARE_NAME}.invoke(\
${PARAMS_PASSING});
    }
""")
    else :
      template = Template("""\
    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {
        if (${METHOD_DECLARE_NAME} == null || ${METHOD_DECLARE_NAME}.isNull()) {
            ${RETURN_SUPER}${NAME}Super(${PARAMS_PASSING_SUPER});
        } else {
            ${GENERIC_TYPE_DECLARE}${RETURN}${METHOD_DECLARE_NAME}.invoke(\
${PARAMS_PASSING});
        }
    }
""")

    if self._method_return == 'void':
      return_statement = ''
      return_statement_super = ''
    elif return_is_internal:
      return_statement = 'return (%s)' % return_type_java_data.bridge_name
      return_statement_super = 'return '
    else:
      return_statement = ('return (%s)' %
          ConvertPrimitiveTypeToObject(self.method_return))
      return_statement_super = 'return '

    # Handling generic types, current only ValueCallback will be handled.
    generic_type_declare = ''
    for param_name in self._typed_params:
      typed_param = self._typed_params[param_name]
      if typed_param.generic_type != 'ValueCallback':
        continue
      if typed_param.contains_internal_class:
        generic_type_declare += 'final %s %sFinal = %s;\n            ' % (
            typed_param.expression, param_name, param_name)

    value = {'RETURN_TYPE': self.method_return,
             'NAME': self.method_name,
             'METHOD_DECLARE_NAME': self._method_declare_name,
             'PARAMS': self._bridge_params_declare,
             'RETURN': return_statement,
             'RETURN_SUPER': return_statement_super,
             'GENERIC_TYPE_DECLARE': generic_type_declare,
             'PARAMS_PASSING_SUPER': self._bridge_params_pass_to_super,
             'PARAMS_PASSING': self._bridge_params_pass_to_wrapper}
    return template.substitute(value)

  def GenerateBridgeSuperMethod(self):
    no_return_value = self._method_return == 'void'
    return_is_internal = self.IsInternalClass(self._method_return)
    if return_is_internal:
      return_type_java_data = self.GetJavaData(self._method_return)

    if self._is_abstract:
      return ''

    if self._class_java_data.HasCreateInternallyAnnotation():
      if no_return_value:
        template = Template("""\
    public void ${NAME}Super(${PARAMS}) {
        if (internal == null) {
            super.${NAME}(${PARAM_PASSING});
        } else {
            internal.${NAME}(${PARAM_PASSING});
        }
    }
""")
      else:
        template = Template("""\
    public ${RETURN_TYPE} ${NAME}Super(${PARAMS}) {
        ${INTERNAL_RETURN_TYPE} ret;
        if (internal == null) {
            ret = super.${NAME}(${PARAM_PASSING});
        } else {
            ret = internal.${NAME}(${PARAM_PASSING});
        }
        ${IF_NULL_RETURN_NULL}
        return ${RETURN_VALUE};
    }
""")
    else:
      if no_return_value:
        template = Template("""\
    public void ${NAME}Super(${PARAMS}) {
        super.${NAME}(${PARAM_PASSING});
    }
""")
      else:
        template = Template("""\
    public ${RETURN_TYPE} ${NAME}Super(${PARAMS}) {
        ${INTERNAL_RETURN_TYPE} ret;
        ret = super.${NAME}(${PARAM_PASSING});
        ${IF_NULL_RETURN_NULL}
        return ${RETURN_VALUE};
    }
""")

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
    template = Template("""\
${DOC}
    public ${CLASS_NAME}(${PARAMS}) {
${PRE_WRAP_LINES}
        reflectionInit();
    }

""")

    pre_wrap_string = self._method_annotations.get(
        self.ANNOTATION_PRE_WRAPLINE, '')
    post_wrap_string = self._method_annotations.get(
        self.ANNOTATION_POST_WRAPLINE, '')

    if (pre_wrap_string != ''):
      pre_wrap_string += "\n\n"

    pre_wrap_string += "        constructorTypes = new ArrayList<Object>();\n"
    for param_type in self._wrapper_params_declare_for_bridge.split(', '):
      if (param_type != ''):
        param_type = param_type.replace('coreWrapper.getBridgeClass(', '')
        param_type = param_type.replace(')', '')
        pre_wrap_string  += ("        constructorTypes.add(%s);\n" % param_type)

    pre_wrap_string += "\n"
    pre_wrap_string += "        constructorParams = new ArrayList<Object>();\n"
    for param_name in self._wrapper_params_pass_to_bridge.split(', '):
      if (param_name != ''):
        param_name = param_name.replace('.getBridge()', '')
        pre_wrap_string += "        constructorParams.add(%s);\n" % param_name

    if (post_wrap_string != ''):
      pre_wrap_string += ("""
        postWrapperMethod = new ReflectMethod(this,
                \"post%s\");\n""" % self._method_declare_name)

    doc_string = self.GenerateDoc(self.method_doc)
    if self._is_deprecated :
      doc_string += "\n    @Deprecated"

    value = {'DOC': doc_string,
             'CLASS_NAME': self._class_java_data.wrapper_name,
             'PARAMS': self._wrapper_params_declare,
             'PRE_WRAP_LINES': pre_wrap_string}
    ret = template.substitute(value)

    if (post_wrap_string != ''):
      template = Template("""\
    public void post${POST_WRAP_METHOD}() {
${POST_WRAP_LINES}
    }

""")
      value = {'POST_WRAP_METHOD': self._method_declare_name,
               'POST_WRAP_LINES': post_wrap_string}
      ret += template.substitute(value)

    return ret

  def GenerateWrapperStaticMethod(self):
    if self.is_reservable:
      template = Template("""\
${DOC}
    public static ${RETURN_TYPE} ${NAME}(${PARAMS}) {
        reflectionInit();
        try {
            ${RETURN}${METHOD_DECLARE_NAME}.invoke(${PARAMS_PASSING});
        } catch (UnsupportedOperationException e) {
            if (coreWrapper == null) {
                ${METHOD_DECLARE_NAME}.setArguments(${PARAMS_PASSING});
                XWalkCoreWrapper.reserveReflectMethod(${METHOD_DECLARE_NAME});
            } else {
                XWalkCoreWrapper.handleRuntimeError(e);
            }
        }
        ${RETURN_NULL}
    }
""")
    else:
      template = Template("""\
${DOC}
    public static ${RETURN_TYPE} ${NAME}(${PARAMS}) {
        reflectionInit();
        try {
            ${RETURN}${METHOD_DECLARE_NAME}.invoke(${PARAMS_PASSING});
        } catch (UnsupportedOperationException e) {
            if (coreWrapper == null) {
                throw new RuntimeException("Crosswalk's APIs are not ready yet");
            } else {
                XWalkCoreWrapper.handleRuntimeError(e);
            }
        }
        ${RETURN_NULL}
    }
""")

    return_type = self.method_return
    if self._method_return == 'void':
      return_state = ''
      return_null = ''
    else:
      return_state = 'return (%s) ' % ConvertPrimitiveTypeToObject(return_type)
      return_null = 'return %s;' % GetPrimitiveTypeDefaultValue(return_type)

    doc_string = self.GenerateDoc(self.method_doc)
    if self._is_deprecated :
      doc_string += "\n    @Deprecated"

    value = {'RETURN_TYPE': self.method_return,
             'RETURN': return_state,
             'RETURN_NULL': return_null,
             'DOC': doc_string,
             'NAME': self.method_name,
             'PARAMS': self._wrapper_params_declare,
             'METHOD_DECLARE_NAME': self._method_declare_name,
             'PARAMS_PASSING': self._wrapper_params_pass_to_bridge}
    return template.substitute(value)

  def GenerateWrapperBridgeMethod(self):
    return_is_internal = self.IsInternalClass(self._method_return)
    if return_is_internal:
      return_type_java_data = self.GetJavaData(self._method_return)

    if self.is_abstract:
      template = Template(
          '${DOC}\n' +
          '    public abstract ${RETURN_TYPE} ${NAME}(${PARAMS});\n\n')
    elif return_is_internal:
      template = Template("""\
${DOC}
    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {
        try {
            return (${RETURN_TYPE}) coreWrapper.getWrapperObject(\
${METHOD_DECLARE_NAME}.invoke(${PARAMS_PASSING}));
        } catch (UnsupportedOperationException e) {
            if (coreWrapper == null) {
                throw new RuntimeException("Crosswalk's APIs are not ready yet");
            } else {
                XWalkCoreWrapper.handleRuntimeError(e);
            }
        }
        ${RETURN_NULL}
    }
""")
    elif self.is_reservable:
      template = Template("""\
${DOC}
    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {
        try {
            ${RETURN}${METHOD_DECLARE_NAME}.invoke(${PARAMS_PASSING});
        } catch (UnsupportedOperationException e) {
            if (coreWrapper == null) {
                ${METHOD_DECLARE_NAME}.setArguments(${PARAMS_RESERVING});
                XWalkCoreWrapper.reserveReflectMethod(${METHOD_DECLARE_NAME});
            } else {
                XWalkCoreWrapper.handleRuntimeError(e);
            }
        }
        ${RETURN_NULL}
    }
""")
    elif self._is_delegate:
      template = Template("""\
    private ${RETURN_TYPE} ${NAME}(${PARAMS}){
        ${PRE_WRAP_LINES}
    }
""")
    elif self._disable_reflect_method:
      template = Template("""\
${DOC}
    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {
${PRE_WRAP_LINES}
    }
""")
    else:
      prefix_str = """\
${DOC}
    public ${RETURN_TYPE} ${NAME}(${PARAMS}) {
        try {\n"""
      suffix_str = """\n        } catch (UnsupportedOperationException e) {
            if (coreWrapper == null) {
                throw new RuntimeException("Crosswalk's APIs are not ready yet");
            } else {
                XWalkCoreWrapper.handleRuntimeError(e);
            }
        }
        ${RETURN_NULL}
    }
"""
      return_str = """            ${RETURN}${METHOD_DECLARE_NAME}.invoke(\
${PARAMS_PASSING});"""
      if self._method_return in self._class_java_data.enums:
        # Here only detects enum declared in the same class as
        # the method itself. Using enum across class is not supported.
        self._method_return = self._method_return.replace('Internal', '')
        return_str = """            ${RETURN} %s.valueOf(\
${METHOD_DECLARE_NAME}.invoke(\
${PARAMS_PASSING}).toString());""" % self._method_return

      template = Template(prefix_str + return_str + suffix_str)
    if return_is_internal:
      return_type = return_type_java_data.wrapper_name
    else:
      return_type = self.method_return

    if self._method_return == 'void':
      return_state = ''
      return_null = ''
    else:
      return_state = 'return (%s)' % ConvertPrimitiveTypeToObject(return_type)
      return_null = 'return %s;' % GetPrimitiveTypeDefaultValue(return_type)

    params_reserving = []
    for param in self._wrapper_params_pass_to_bridge.split(', '):
      if (param.find("getBridge()") > 0):
        param = param.replace('.getBridge()', '')
        params_reserving.append(
            'new ReflectMethod(%s, "getBridge")' % param)
      else:
        params_reserving.append(param)

    pre_wrap_string = self._method_annotations.get(
        self.ANNOTATION_PRE_WRAPLINE, '')

    doc_string = self.GenerateDoc(self.method_doc)
    if self._is_deprecated :
      doc_string += "\n    @Deprecated"

    value = {'RETURN_TYPE': return_type,
             'RETURN': return_state,
             'RETURN_NULL': return_null,
             'DOC': doc_string,
             'NAME': self.method_name,
             'PARAMS': re.sub(r'ValueCallback<([A-Za-z]+)Internal>',
                  r'ValueCallback<\1>',self._wrapper_params_declare),
             'METHOD_DECLARE_NAME': self._method_declare_name,
             'PARAMS_RESERVING': ', '.join(params_reserving),
             'PARAMS_PASSING': self._wrapper_params_pass_to_bridge,
             'PRE_WRAP_LINES': pre_wrap_string}

    return template.substitute(value)

  def GenerateWrapperInterface(self):
    return_is_internal = self.IsInternalClass(self._method_return)
    if return_is_internal:
      return_type_java_data = self.GetJavaData(self._method_return)

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
    if self._is_constructor:
      return self.GenerateBridgeConstructor()
    elif self._is_static:
      return self.GenerateBridgeStaticMethod()
    else:
      return '%s\n%s\n%s\n%s\n' % (
          self.GenerateBridgeOverrideMethod(),
          self.GenerateBridgeWrapperMethod(),
          self.GenerateBridgeSuperMethod(),
          '    private ReflectMethod %s = new ReflectMethod(null, "%s");\n' %
              (self._method_declare_name, self._method_name))

  def GenerateMethodsStringForWrapper(self):
    if self._is_constructor:
      return self.GenerateWrapperConstructor()
    elif self._is_static:
      return '%s\n%s\n' % (
          self.GenerateWrapperStaticMethod(), """\
    private static ReflectMethod %s = new ReflectMethod(null, "%s");\n""" %
              (self._method_declare_name, self._method_name))
    elif self._is_abstract or self._is_delegate or self._disable_reflect_method:
      return self.GenerateWrapperBridgeMethod()
    else:
      return '%s\n%s\n' % (
          self.GenerateWrapperBridgeMethod(),
          '    private ReflectMethod %s = new ReflectMethod(null, "%s");\n' %
              (self._method_declare_name, self._method_name))

  def GenerateMethodsStringForInterface(self):
    return self.GenerateWrapperInterface()
