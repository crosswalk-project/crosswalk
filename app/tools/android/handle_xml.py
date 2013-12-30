#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
The public interface functions of handling xml.
"""


def AddElementAttribute(doc, node, name, value):
  root = doc.documentElement
  item = doc.createElement(node)
  item.setAttribute(name, value)
  root.appendChild(item)


def EditElementAttribute(doc, node, name, value):
  item = doc.getElementsByTagName(node)[0]
  if item.hasAttribute(name):
    item.attributes[name].value = value
  else:
    item.setAttribute(name, value)


def AddElementAttributeAndText(doc, node, name, value, data):
  root = doc.documentElement
  item = doc.createElement(node)
  item.setAttribute(name, value)
  text = doc.createTextNode(data)
  item.appendChild(text)
  root.appendChild(item)


def AddThemeStyle(doc, node, name, value):
  item = doc.getElementsByTagName(node)[0]
  src_str = item.attributes[name].value
  src_str = src_str + '.' + value
  item.attributes[name].value = src_str


def RemoveThemeStyle(doc, node, name, value):
  item = doc.getElementsByTagName(node)[0]
  dest_str = item.attributes[name].value.replace('.' + value, '')
  item.attributes[name].value = dest_str
