#!/bin/bash
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

BUILD_DIR=$1

if [ $# -eq 0 ]; then
   echo -e "\nUsage: $0 PATH_TO_BUILD_DIR (i.e.: $0 ../../../out/Release/)."
   exit 1
fi

if [ ! $BUILD_DIR/xesh ]; then
   echo -e "\nPlease make sure XEsh is built in $BUILD_DIR"
   exit 1
fi

echo "echo.syncEcho(\"\");" > temp_test.js
echo "quit();" >> temp_test.js

$BUILD_DIR/xesh --external-extensions-path=$BUILD_DIR/tests/extension/echo_extension --input-file=temp_test.js 1> test_stdout 2> test_stderr

RESULT=`cat test_stdout`
EXPECTED="Instance 1 created!"

rm temp_test.js
rm test_stdout
rm test_stderr

if [ "$RESULT" = "$EXPECTED" ]; then
   echo -e "XESh Test: PASS."
   exit 0
else
   echo -e "XESh Test: FAIL."
   exit 1
fi
