#!/bin/sh

conf_file=$(cd "$(dirname "$0")"; pwd)/../../../.incremental.conf

if [ $(id -u) -ne 0 ]
then
  echo "Please re-run $0 as root."
  exit
fi

if [ ! -f $conf_file ]
then
  echo "please run incremental-setup.sh first"
  exit
fi

while read line;
do
  name=$(echo $line | awk -F '=' '{print $1}')
  value=$(echo $line | awk -F '=' '{print $2}')
  case $name in
    "gbs-root")
      src_in_chroot="$value/local/BUILD-ROOTS/scratch.i586.0/incremental/"
    ;;
    *)
    ;;
esac
done < $conf_file

#umount
umount $src_in_chroot/src/
rm -rf $src_in_chroot
rm $conf_file
