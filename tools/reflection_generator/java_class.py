# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re

from string import Template

from java_class_component import Enum, Field
from java_method import Method

class JavaClassLoader(object):
  """Manager class maintains all loaded java classes."""
  def __init__(self, src_path, class_list):
    self._src_path = src_path
    self._class_list = class_list
    self._java_data_map = {}

    for clazz in self._class_list:
      self.LoadJavaFile(clazz)

    for key,java_data in self._java_data_map.items():
      for method in java_data._methods:
        method.PrepareStrings()

  def IsInternalClass(self, clazz):
    return clazz in self._class_list

  def GetJavaData(self, clazz):
    return self._java_data_map.get(clazz)

  def LoadJavaFile(self, clazz):
    if self._java_data_map.has_key(clazz):
      return

    file_name = os.path.join(self._src_path, '%s.java' % clazz)
    try:
      file_handle = open(file_name, 'r')
      file_content = file_handle.read()
      file_handle.close()
    except Exception:
      print 'Error reading input Java file, please check.'
      return

    java_data = InternalJavaFileData(self)
    java_data.SetClassContent(file_content)
    self._java_data_map[clazz] = java_data

  def GenerateDoc(self, doc):
    if not doc:
      return ''

    def ReplaceInternal(matchobj):
      match = matchobj.group(0)
      if self.IsInternalClass(match):
        return self.GetJavaData(match).wrapper_name
      else:
        return match

    return re.sub('XWalk[a-zA-Z_0-9]*Internal',
        ReplaceInternal, doc).lstrip('\n')

class InternalJavaFileData(object):
  """Data class stores the generator information of internal class."""
  ANNOTATION_CREATE_INTERNALLY = 'createInternally'
  ANNOTATION_CREATE_EXTERNALLY = 'createExternally'
  ANNOTATION_EXTEND_CLASS = 'extendClass'
  ANNOTATION_NO_INSTANCE = 'noInstance'
  ANNOTATION_INSTANCE = 'instance'
  ANNOTATION_IMPL = 'impl'

  def __init__(self, class_loader):
    self._class_loader = class_loader
    self._class_name = ''
    self._bridge_name = ''
    self._wrapper_name = ''
    self._class_type = ''  # class or interface
    self._class_doc = ''
    self._class_annotations = {}
    self._methods = []
    self._fields = []
    self._imports = []
    self._enums = {}
    self._package_name = ''
    self._need_default_constructor = True

  @property
  def class_name(self):
    return self._class_name

  @property
  def bridge_name(self):
    return self._bridge_name

  @property
  def wrapper_name(self):
    return self._wrapper_name

  @property
  def class_type(self):
    return self._class_type

  @property
  def class_doc(self):
    return self._class_doc

  @property
  def class_annotations(self):
    return self._class_annotations

  @property
  def methods(self):
    return self._methods

  @property
  def fields(self):
    return self._fields

  @property
  def imports(self):
    return self._imports

  @property
  def enums(self):
    return self._enums

  @property
  def package_name(self):
    return self._package_name

  @property
  def need_default_constructor(self):
    return self._need_default_constructor

  def GetJavaData(self, clazz):
    return self._class_loader.GetJavaData(clazz)

  def IsInternalClass(self, clazz):
    return self._class_loader.IsInternalClass(clazz)

  def MangleInternalNameToBridgeName(self, internal_name):
    if not self.IsInternalClass(internal_name):
      return internal_name
    else:
      return internal_name.replace('Internal', 'Bridge')

  def MangleInternalNameToWrapperName(self, internal_name):
    if not self.IsInternalClass(internal_name):
      return internal_name
    else:
      return internal_name.replace('Internal', '')

  def SetClassContent(self, content):
    self.ExtractPackageName(content)
    self.ExtractImports(content)
    self.ExtractClassProperties(content)
    self.ExtractMethods(content)
    self.ExtractFields(content)
    self.ExtractEnums(content)

  def ExtractPackageName(self, java_content):
    package_re = re.compile('\s*package\s+(?P<package>[a-zA-Z0-9._]+)\s*;')
    for match in re.finditer(package_re, java_content):
      self._package_name = match.group('package')

  def ExtractImports(self, java_content):
    imports_re = re.compile('\s*import\s+(?P<imported>[a-zA-Z0-9._*]+)\s*;')
    for match in re.finditer(imports_re, java_content):
      imported = match.group('imported')
      # Determine whether the import rule should be ignored for generated code.
      # TODO: Currently we only use a blacklist to filter the import rule.
      if imported.startswith('org.xwalk.core.internal') or \
          imported.startswith('org.xwalk.core') or \
          imported.startswith('org.chromium'):
        continue
      self._imports.append(imported)

  def ExtractClassProperties(self, java_content):
    class_re = re.compile(
        '(?P<class_doc>(\n\s*/\*\*.*\n(\s+\*(.)*\n)+\s+\*/\s*)?)\n'
        '\s*@XWalkAPI\(?'
        '(?P<annotation_content>[a-zA-Z0-9.,=\s]*)\)?'
        '\s*public\s+([a-z]+\s+)*'
        '(?P<type>(class|interface))\s+'
        '(?P<class_name>[a-zA-Z0-9]*)')
    for match in re.finditer(class_re, java_content):
      annotation_content = match.group('annotation_content')
      self._class_name = match.group('class_name')
      self._bridge_name = \
          self.MangleInternalNameToBridgeName(self._class_name)
      self._wrapper_name = \
          self.MangleInternalNameToWrapperName(self._class_name)
      self._class_type = match.group('type')
      self._class_doc = match.group('class_doc')
      self.ParseClassAnnotations(annotation_content)

  def ParseClassAnnotations(self, annotation):
    """Class annotation contains the following optional attributes:
        'extendClass' - The class have to extend
        'createExternally' - boolean
        'craeteInternally' - boolean
        'noInstance' - boolean
        'isConst' - boolean
        'impl' - Class to impl
        'instance - instance'"""
    extend_class_re = re.compile('extendClass\s*=\s*'
        '(?P<extend_class>[a-zA-Z0-9.]+)')
    for match in re.finditer(extend_class_re, annotation):
      extend_class = match.group('extend_class')
      self._class_annotations['extendClass'] = extend_class

    create_internally_re = re.compile('createInternally\s*=\s*'
        '(?P<create_internally>(true|false))')
    for match in re.finditer(create_internally_re, annotation):
      create_internally = match.group('create_internally')
      if create_internally == 'true':
        self._class_annotations['createInternally'] = True
        self._need_default_constructor = False
      elif create_internally == 'false':
        self._class_annotations['createInternally'] = False

    create_externally_re = re.compile('createExternally\s*=\s*'
        '(?P<create_externally>(true|false))')
    for match in re.finditer(create_externally_re, annotation):
      create_externally = match.group('create_externally')
      if create_externally == 'true':
        self._class_annotations['createExternally'] = True
      elif create_externally == 'false':
        self._class_annotations['createExternally'] = False

    no_instance_re = re.compile('noInstance\s*=\s*'
        '(?P<no_instance>(true|false))')
    for match in re.finditer(no_instance_re, annotation):
      no_instance = match.group('no_instance')
      if no_instance == 'true':
        self._class_annotations['noInstance'] = True
        self._need_default_constructor = False
      elif no_instance == 'false':
        self._class_annotations['noInstance'] = False

    is_const_re = re.compile('isConst\s*=\s*'
        '(?P<is_const>(true|false))')
    for match in re.finditer(is_const_re, annotation):
      is_const = match.group('is_const')
      if is_const == 'true':
        self._class_annotations['isConst'] = True
      elif is_const == 'false':
        self._class_annotations['isConst'] = False

    impl_re = re.compile('impl\s*=\s*'
        '(?P<impl>[a-zA-Z0-9.]+)')
    for match in re.finditer(impl_re, annotation):
      impl = match.group('impl')
      self._class_annotations['impl'] = impl

    instance_re = re.compile('instance\s*=\s*'
        '(?P<instance>[a-zA-Z0-9.]+)')
    for match in re.finditer(instance_re, annotation):
      instance = match.group('instance')
      self._class_annotations['instance'] = instance

  def ExtractMethods(self, java_content):
    constructor_re = re.compile(
        '(?P<method_doc>(\n\s*/\*\*.*\n(\s+\*(.)*\n)+\s+\*/\s*)?)\n'
        '(?P<method_deprecated1>\s*@Deprecated\s*\n)?'
        '\s*@XWalkAPI\(?'
        '(?P<method_annotation>[a-zA-Z0-9\$\!%,\s\(\)\{\}\\\\;._"=]*)\)?'
        '(?P<method_deprecated2>\s*@Deprecated\s*\n)?'
        '\s*public\s(?P<method_name>[a-zA-Z0-9]+)\('
        '(?P<method_params>[a-zA-Z0-9\s,\[\]\>\<]*)\)')
    for match in re.finditer(constructor_re, java_content):
      method_annotation = match.group('method_annotation')
      method_name = match.group('method_name')
      method_params = match.group('method_params')
      method_doc = match.group('method_doc')
      method_deprecated1 = match.group('method_deprecated1')
      method_deprecated2 = match.group('method_deprecated2')
      method = Method(
          self._class_name,
          self._class_loader,
          True, # is_constructor
          False, # is_static
          False, # is_abstract
          method_deprecated1 != None or method_deprecated2 != None,
          method_name, None,
          method_params, method_annotation, method_doc)
      self._methods.append(method)
      self._need_default_constructor = False

    method_re = re.compile(
        '(?P<method_doc>(\n\s*/\*\*.*\n(\s+\*(.)*\n)+\s+\*/\s*)?)\n'
        '(?P<method_deprecated1>\s*@Deprecated\s*\n)?'
        '\s*@XWalkAPI\(?'
        '(?P<method_annotation>[a-zA-Z0-9%,\s\(\)\{\};._"=]*)\)?'
        '(?P<method_deprecated2>\s*@Deprecated\s*\n)?'
        '\s*public\s+(?P<method_return>[a-zA-Z0-9]+(\<[a-zA-Z0-9]+,\s[a-zA-Z0-9]+\>)*(\[\s*\])*)\s+'
        '(?P<method_name>[a-zA-Z0-9]+)\('
        '(?P<method_params>[a-zA-Z0-9\s,\]\[\<\>]*)\)')
    for match in re.finditer(method_re, java_content):
      method_annotation = match.group('method_annotation')
      method_name = match.group('method_name')
      method_params = match.group('method_params')
      method_return = match.group('method_return')
      method_doc = match.group('method_doc')
      method_deprecated1 = match.group('method_deprecated1')
      method_deprecated2 = match.group('method_deprecated2')
      method = Method(
          self._class_name,
          self._class_loader,
          False, # is_constructor
          False, # is_static
          False, # is_abstract
          method_deprecated1 != None or method_deprecated2 != None,
          method_name, method_return, method_params,
          method_annotation, method_doc)
      self._methods.append(method)

    method_re = re.compile(
        '(?P<method_doc>(\n\s*/\*\*.*\n(\s+\*(.)*\n)+\s+\*/\s*)?)\n'
        '\s*@XWalkAPI\(?'
        '(?P<method_annotation>[a-zA-Z0-9%,\s\(\)\{\};._"=]*)\)?'
        '\s*public\s+static\s+(synchronized\s+)*'
        '(?P<method_return>[a-zA-Z0-9]+)\s+'
        '(?P<method_name>[a-zA-Z0-9]+)\('
        '(?P<method_params>[a-zA-Z0-9\s,\[\]\<\>]*)\)')
    for match in re.finditer(method_re, java_content):
      method_annotation = match.group('method_annotation')
      method_name = match.group('method_name')
      method_params = match.group('method_params')
      method_return = match.group('method_return')
      method_doc = match.group('method_doc')
      method = Method(
          self._class_name,
          self._class_loader,
          False, # is_constructor
          True, # is_static
          False, # is_abstract
          False,
          method_name, method_return, method_params,
          method_annotation, method_doc)
      self._methods.append(method)

    method_re = re.compile(
        '(?P<method_doc>(\n\s*/\*\*.*\n(\s+\*(.)*\n)+\s+\*/\s*)?)\n'
        '\s*@XWalkAPI\(?'
        '(?P<method_annotation>[a-zA-Z0-9%,\s\(\)\{\};._"=]*)\)?'
        '\s*public\s+abstract\s+(synchronized\s+)*'
        '(?P<method_return>[a-zA-Z0-9]+)\s+'
        '(?P<method_name>[a-zA-Z0-9]+)\('
        '(?P<method_params>[a-zA-Z0-9\s,\[\]\<\>]*)\)')
    for match in re.finditer(method_re, java_content):
      method_annotation = match.group('method_annotation')
      method_name = match.group('method_name')
      method_params = match.group('method_params')
      method_return = match.group('method_return')
      method_doc = match.group('method_doc')
      method = Method(
          self._class_name,
          self._class_loader,
          False, # is_constructor
          False, # is_static
          True, # is_abstract
          False,
          method_name, method_return, method_params,
          method_annotation, method_doc)
      self._methods.append(method)

  def ExtractFields(self, java_content):
    field_re = re.compile(
        '(?P<field_doc>(\n\s*/\*\*.*\n(\s+\*(.)*\n)+\s+\*/\s*)?)\n'
        '\s*@XWalkAPI\s*public\s+static\s+final\s+'
        '(?P<field_type>[a-zA-Z0-9_]+)\s+'
        '(?P<field_name>[a-zA-Z0-9_]+)\s*=\s*'
        '(?P<field_value>[a-zA-Z0-9-_"]+)\s*;')
    for match in re.finditer(field_re, java_content):
      field_type = match.group('field_type')
      field_name = match.group('field_name')
      field_value = match.group('field_value')
      field_doc = match.group('field_doc')
      field_object = Field(field_type, field_name, field_value, field_doc)
      self._fields.append(field_object)

  def ExtractEnums(self, java_content):
    enum_re = re.compile(
        '(?P<enum_doc>(\n\s*/\*\*.*\n(\s+\*(.)*\n)+\s+\*/\s*)?)\n'
        '\s*@XWalkAPI\s*public\s+enum\s+'
        '(?P<enum_name>[a-zA-Z0-9_]+)\s+{'
        '(?P<enum_content>(.|\n)*?)\s*}')
    for match in re.finditer(enum_re, java_content):
      enum_name = match.group('enum_name')
      enum_content = match.group('enum_content')
      enum_doc = match.group('enum_doc')
      enum_object = Enum(enum_name, enum_content, enum_doc)
      self._enums[enum_name] = enum_object

  def HasNoInstanceAnnotation(self):
    return self._class_annotations.get(
        InternalJavaFileData.ANNOTATION_NO_INSTANCE, False)

  def HasCreateInternallyAnnotation(self):
    return self._class_annotations.get(
        InternalJavaFileData.ANNOTATION_CREATE_INTERNALLY, False)

  def HasInstanceCreateInternallyAnnotation(self):
    instance = None
    clazz = self._class_annotations.get(
        InternalJavaFileData.ANNOTATION_INSTANCE, None)
    if clazz:
      instance = self.GetJavaData(clazz.replace('.class', ''))

    if instance:
      return instance.HasCreateInternallyAnnotation()
    else:
      return self.HasCreateInternallyAnnotation()

  def UseAsInstanceInBridgeCall(self, var):
    return '%s.getWrapper()' % self.UseAsReturnInBridgeSuperCall(var)

  def UseAsInstanceInBridgeOverrideCall(self, var):
    clazz = self._class_annotations.get(
        InternalJavaFileData.ANNOTATION_INSTANCE, self._class_name)
    clazz = clazz.replace('.class', '')

    if self.GetJavaData(clazz).class_annotations.get(
        InternalJavaFileData.ANNOTATION_CREATE_INTERNALLY, False):
      return self.UseAsReturnInBridgeSuperCall(var)
    return '(%s) %s' % (self.GetJavaData(clazz).bridge_name, var)

  def UseAsReturnInBridgeSuperCall(self, var):
    clazz = self._class_annotations.get(
        InternalJavaFileData.ANNOTATION_INSTANCE, self._class_name)
    clazz = clazz.replace('.class', '')

    if self.GetJavaData(clazz).class_annotations.get(
        InternalJavaFileData.ANNOTATION_CREATE_INTERNALLY, False):
      typed_var_template = Template('(${VAR} instanceof ${BRIDGE_TYPE} ?'\
          ' ((${BRIDGE_TYPE}) ${VAR} ) : new ${BRIDGE_TYPE}(${INTERNAL_VAR}))')
      value = {'VAR': var,
               'BRIDGE_TYPE': self.GetJavaData(clazz).bridge_name,
               'INTERNAL_VAR': var if clazz == self._class_name else\
                                   '(%s) %s' % (clazz, var)}
      var = typed_var_template.substitute(value)
    else:
      typed_var_template = Template('(${VAR} instanceof ${BRIDGE_TYPE} ?'\
          ' ((${BRIDGE_TYPE}) ${VAR} ) : null)')
      value = {'VAR': var,
               'BRIDGE_TYPE': self.GetJavaData(clazz).bridge_name}
      var = typed_var_template.substitute(value)
    return var

  def UseAsInstanceInBridgeSuperCall(self, var):
    # pylint: disable=R0201
    return var

  def UseAsInstanceInWrapperCall(self, var):
    clazz = self._class_annotations.get('instance', self._class_name)
    clazz = clazz.replace('.class', '')

    if clazz != self._class_name:
      var = '((%s) %s)' % (self.GetJavaData(clazz).wrapper_name, var)
    return '%s.getBridge()' % var

  def UseAsTypeInWrapperCall(self):
    return self._wrapper_name

  def GetBridgeName(self, subclass=None):
    if not self.IsInternalClass(self._class_name):
      return self._class_name
    else:
      clazz = self._class_annotations.get(
          InternalJavaFileData.ANNOTATION_INSTANCE, self._class_name)
      clazz = clazz.replace('.class', '')
      if not subclass:
        return self.GetJavaData(clazz).bridge_name
      else:
        return clazz + '$' + subclass

  def GetWrapperName(self, subclass=None):
    if not self.IsInternalClass(self._class_name):
      return self._class_name
    else:
      if not subclass:
        return self._wrapper_name
      else:
        return "%s$%s" % (self._wrapper_name, subclass.replace('Internal', ''))
