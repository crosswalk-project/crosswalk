# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

TEMPLATE = """\
extern const char %s[];
const char %s[] = { %s, 0 };
"""

js_code = sys.argv[1]
lines = file(js_code).read()
c_code = ', '.join(str(ord(c)) for c in lines)

symbol_name = sys.argv[2]
output = open(sys.argv[3], "w")
output.write(TEMPLATE % (symbol_name, symbol_name, c_code))
output.close()
