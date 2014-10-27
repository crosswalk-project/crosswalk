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


def EditElementValueByNodeName(doc, node, name, value):
  items = doc.getElementsByTagName(node)
  for item in items:
    if item.attributes['name'].value == name:
      item.firstChild.data = value
      break


def AddElementAttributeAndText(doc, node, name, value, data):
  root = doc.documentElement
  item = doc.createElement(node)
  item.setAttribute(name, value)
  text = doc.createTextNode(data)
  item.appendChild(text)
  root.appendChild(item)


def CompareNodes(node1, node2):
  if node1.tagName != node2.tagName or node1.attributes is None:
    return False
  if node2.attributes is None:
    return True

  for item in node2.attributes.items():
    if not item in node1.attributes.items():
      return False
  return True


def MergeNodes(node1, node2):
  tmp_node_list = []
  for item2 in node2.childNodes:
    if item2.nodeType != item2.ELEMENT_NODE:
      continue
    item1 = None
    for tmp_item in node1.childNodes:
      if tmp_item.nodeType != tmp_item.ELEMENT_NODE:
        continue
      if CompareNodes(tmp_item, item2):
        item1 = tmp_item
        break
    if item1 is not None:
      MergeNodes(item1, item2)
    else:
      tmp_node_list.append(item2)

  for item in tmp_node_list:
    node1.appendChild(item)
