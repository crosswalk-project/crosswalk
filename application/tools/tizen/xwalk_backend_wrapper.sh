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
        "-r") echo "Reinstall not supported"
              exit 128 # ErrorNotSupportRDSUpdate == 128
              ;; #TODO(t.iwanek) fix me - sending signals for reinstall option
    esac
    shift
done

if [ "`whoami`" == "root" ]
then
    #
    # TODO(t.iwanek): fix me
    # this is workaround that will work only for 'app' user
    # pkgmgr-server is running as root and then backend too
    # 1) backend need to know the user
    #  or
    # 2) pkgmgr must change user before launching backend
    #

    # Find requesting process...
    #  (will fail for multiple installation at same time)
    user=`ps aux | grep -v 'grep' | grep pkgcmd | cut -f1 -d" "`

    su - ${user} -c "/usr/bin/xwalkctl ${option} ${id} ${key} ${keyvalue} ${quiet}"
else
    # correct UID was set by pkgmgr
    /usr/bin/xwalkctl ${option} ${id} ${key} ${keyvalue} ${quiet}
fi
