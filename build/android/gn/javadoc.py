#!/usr/bin/env python
#
# Copyright (c) 2015 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401

import subprocess
import sys

def main():
  subprocess.check_call(['javadoc'] + sys.argv[1:])

if __name__ == '__main__':
  sys.exit(main())
