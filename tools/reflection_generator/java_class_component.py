# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class Field:
  """Python class represents static field of a java class"""
  def __init__(self, field_type, name, value, doc):
    self._field_type = field_type
    self._field_name = name
    self._field_value = value
    self._field_doc = doc

  @property
  def field_type(self):
    return self._field_type

  @property
  def field_name(self):
    return self._field_name

  @property
  def field_value(self):
    return self._field_value

  @property
  def field_doc(self):
    return self._field_doc


class Enum:
  """Python class represents enum type in a java class"""
  def __init__(self, name, declaration, doc):
    self._enum_name = name
    self._enum_declaration = declaration
    self._enum_doc = doc

  @property
  def enum_name(self):
    return self._enum_name

  @property
  def enum_declaration(self):
    return self._enum_declaration

  @property
  def enum_doc(self):
    return self._enum_doc

  def EnumWrapperName(self):
    return self._enum_name.replace('Internal', '')

  def EnumClassName(self):
    # return the variable name of the class<?> object for this enum
    # type in parent class.
    return 'enum%sClass' % self._enum_name.replace('Internal', '')

  def EnumMethodValueOfName(self):
    # return the variable name of the Method object for this enum
    # type's valueOf method in parent class.
    return '%sValueOfMethod' % self.EnumClassName()

