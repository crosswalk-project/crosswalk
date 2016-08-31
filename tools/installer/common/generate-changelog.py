#!/usr/bin/env python
#
# Copyright (c) 2016 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys

# In GN the target "Action" cannot run shell script directly.
# Run the "generate-changelog.sh", pass the output-file as parameter.
exit(subprocess.call([sys.argv[1], sys.argv[2]]))
