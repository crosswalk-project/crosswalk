# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from command_executor import Command


class WebElement(object):
  """Represents an HTML element."""
  def __init__(self, xwalkdriver, id_):
    self._xwalkdriver = xwalkdriver
    self._id = id_

  def _Execute(self, command, params=None):
    if params is None:
      params = {}
    params['id'] = self._id;
    return self._xwalkdriver.ExecuteCommand(command, params)

  def FindElement(self, strategy, target):
    return self._Execute(
        Command.FIND_CHILD_ELEMENT, {'using': strategy, 'value': target})

  def FindElements(self, strategy, target):
    return self._Execute(
        Command.FIND_CHILD_ELEMENTS, {'using': strategy, 'value': target})

  def HoverOver(self):
    self._Execute(Command.HOVER_OVER_ELEMENT)

  def Click(self):
    self._Execute(Command.CLICK_ELEMENT)

  def SingleTap(self):
    self._Execute(Command.TOUCH_SINGLE_TAP)

  def Clear(self):
    self._Execute(Command.CLEAR_ELEMENT)

  def SendKeys(self, *values):
    typing = []
    for value in values:
      if isinstance(value, int):
        value = str(value)
      for i in range(len(value)):
        typing.append(value[i])
    self._Execute(Command.SEND_KEYS_TO_ELEMENT, {'value': typing})

  def GetLocation(self):
    return self._Execute(Command.GET_ELEMENT_LOCATION)
