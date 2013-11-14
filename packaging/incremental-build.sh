#!/bin/sh

conf_file=$(cd "$(dirname "$0")"; pwd)/../../../.incremental.conf

if [ ! -f $conf_file ]
then
  echo "please run incremental-setup.sh first"
  exit
fi

while read line;
do
  name=$(echo $line | awk -F '=' '{print $1}')
  value=$(echo $line | awk -F '=' '{print $2}')
  echo "$name $value"
  case $name in
    "chromium")
      src_in_host=$value
    ;;
    "gbs-root")
      gbs_root=$value
    ;;
    *)
      echo "invalid setup"
    ;;
esac
done < $conf_file

#build
cd $src_in_host/src/xwalk
gbs build --debug --overwrite -B $gbs_root -A i586 --define "INCREMENTAL /incremental"
cd -
