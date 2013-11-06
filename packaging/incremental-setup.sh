#!/bin/sh

echo "Usage: $0 chromium-crosswalk(the dir containing 'src') gbs-build-root(e.g. ~/GBS-ROOT/)"
echo "Example 1: $0"
echo "           -- use default arguments: chromium-crosswalk='../../../', gbs-build-root='~/GBS-ROOT/'"
echo "Example 2: $0 gbs-build-root"
echo "           -- use default argument: chromium-crosswalk='../../../'"
echo "Example 3: $0 chromium-crosswalk gbs-build-root"

if [ $(id -u) -ne 0 ]
then
  echo "Please re-run $0 as root."
  exit
fi

src_in_host="../../../"
gbs_root="$HOME/GBS-ROOT/"
conf_file=$(cd "$(dirname "$0")"; pwd)/../../../.incremental.conf
if [ -f $conf_file ]
then
  echo "Please clean the last setup first!"
  exit
fi

if [ $# -ge 3 ]
then
  echo "Invalid arguments!"
  exit
elif [ $# -eq 2 ]
then
  src_in_host=$1
  gbs_root=$2
elif [ $# -eq 1 ]
then
  gbs_root=$1
fi

if [ -d $src_in_host ]
then
  cd $src_in_host
  src_in_host=$(pwd)
  cd -
else
  echo "invalid path [$src_in_host]!"
  exit
fi

if [ ! -d "$src_in_host/src/" ]
then
  echo "no 'src' in '$src_in_host'"
  exit
fi

if [ -d $gbs_root ]; then
  cd $gbs_root
  gbs_root="$(pwd)"
  src_in_chroot="$(pwd)/local/BUILD-ROOTS/scratch.i586.0/incremental"
  cd -
else
  echo "[$gbs_root] isn't a ready gbs chroot, typically it can be created with the below command:"
  echo "gbs build -B $gbs_root -A i586"
  echo "Once it's completed, please re-run this script!"
  exit
fi

# bypass the mount-fs checking of gbs tool
patched=$(grep "#mount_source_check" /usr/bin/depanneur)
if [ -z "$patched" ]
then
  sed -i 's/mount_source_check("$scratch_dir.$i");/#mount_source_check("$scratch_dir.$i");/g' /usr/bin/depanneur
fi

# umount 'src_in_chroot' in fear that gbs will delete it while initializing the
# build root.
patched=$(grep "BUILD_ROOT/incremental" /usr/lib/build/init_buildsystem 2>/dev/null)
if [ -z "$patched" ]
then
  sed -i "128 i\            umount \"\$BUILD_ROOT/incremental\" 2> /dev/null || true" /usr/lib/build/init_buildsystem
fi

# mount the chromium-crosswalk from host to chroot system
if [ -d $src_in_chroot ]
then
  echo "setup failed -- [$src_in_chroot]:Already used in chroot!"
  exit
else
  mkdir -p $src_in_chroot
  chmod 777 $src_in_chroot
  mkdir $src_in_chroot/src
  chmod 777 $src_in_chroot/src
  mount --bind $src_in_host/src/ $src_in_chroot/src/
fi

echo "chromium=$src_in_host" > $conf_file
echo "gbs-root=$gbs_root" >> $conf_file
chmod 666 $conf_file

echo "setup completed"
