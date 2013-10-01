#!/usr/bin/env python
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from json_comment_eater import Nom
import unittest

class JsonCommentEaterTest(unittest.TestCase):
  def _Load(self, test_name):
    '''Loads the input and expected output for |test_name| as given by reading
    in |test_name|.json and |test_name|_expected.json, and returns the string
    contents as a tuple in that order.
    '''
    def read(file_name):
      with open(file_name, 'r') as f:
        return f.read()
    return [read(pattern % test_name)
            for pattern in ('%s.json', '%s_expected.json')]

  def testEverything(self):
    json, expected_json = self._Load('everything')
    self.assertEqual(expected_json, Nom(json))

if __name__ == '__main__':
  unittest.main()
