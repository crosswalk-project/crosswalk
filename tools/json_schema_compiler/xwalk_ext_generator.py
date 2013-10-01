# Copyright (c) 2013 The Intel Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from xwalk_js_generator import XWalkJSGenerator
#from xwalk_cc_generator import XWalkCCGenerator

class XWalkExtGenerator(object):
  def __init__(self, type_generator, namespace):
      self.js_generator = XWalkJSGenerator(type_generator, namespace)
#      self.cc_generator = XWalkCCGenerator(type_generator, namespace)
