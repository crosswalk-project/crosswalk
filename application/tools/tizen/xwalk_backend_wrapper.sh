#!/bin/bash

# Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#parse installation options
while [ "$1" != "" ]
do
    case "$1" in
        "-i") option="-i"
              id="$2"
              shift
              ;;
        "-d") option="-u"
              id="$2"
              shift
              ;;
        "-q") quiet="-q"
              ;;
        "-k") key="-k"
              keyvalue="$2"
              shift
              ;;
        "-r") option="-r"
              id="$2"
              ;;
    esac
    shift
done

if [ "`whoami`" == "root" ]
then
	exit 1
else
    # correct UID was set by pkgmgr
    /usr/bin/xwalkctl ${option} ${id} ${key} ${keyvalue} ${quiet}
fi
