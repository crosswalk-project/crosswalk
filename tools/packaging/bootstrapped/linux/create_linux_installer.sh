#!/bin/sh
absolute_path () {
    echo `cd $1 && pwd`
}
THIS_SCRIPT=$0
SCRIPT_DIR=`dirname $THIS_SCRIPT`
SCRIPT_DIR=`absolute_path $SCRIPT_DIR`
TEMP_DIR=/tmp/xwalk_build
while [ "$1" != "" ]; do
    DASHED_PARAM=$1
    PARAM=`echo ${DASHED_PARAM}|sed 's/--//'`
    if [ "$PARAM" = "$DASHED_PARAM" ]; then
      APP_PATH=$PARAM
    else
        if ! echo  $PARAM |grep  = ;  then
            PARAM=`echo $PARAM |sed 's/^\(.*\)/\U\1/'`
            eval $PARAM=true
        else
            PARAM=`echo $PARAM |sed 's/^\(.*=\)/\U\1/'`
            eval $PARAM
        fi
    fi
    shift
done

if [ "x$HELP" != "x" ]; then
    echo
    echo usage: $THIS_SCRIPT [options] [app_path]
    echo
    echo This script is used to create a standalone installer for Crosswalk applications. It
    echo depends on checkinstall and Crosswalk to function properly.
    echo The following options are supported:
    echo "
     app_path                  Path to the Crosswalk application. If not specified, the
                               current directory is used.
     --xwalk_path=<path>       Path to Crosswalk binaries. If not specified, the script
                               will try to find them through PATH, the app path, or
                               the current directory.
     --app_name=<name>         Name of the application. If not specified, the name
                               of the application directory is used.
     --version=<version>       The version of the application, defaults to 1.0.0
     --app_index=<path>        Path of app index file, relative to app_path. If not
                               specified, index.html is used.
     --app_arguments=<path>    Arugments to be passed into Crosswalk executable
                               Example: --app_arguments=--allow-file-access-from-files
     --out=<path>              Path of the output package file, defaults to
                               $TEMP_DIR/<app_name>
     --publisher=<name>        The manufacturer of this application, defaults to "Me"
     --shadow_install=<yes|no> Enable/disable installing app to a temporary directory
                               for testing purposed. Defaults to yes
     --help                    Print this message "
    exit 1
fi

if [ "x`which checkinstall`" = "x" ]; then
    echo
    echo "checkinstall is required to create an application package. You can install it by:
    sudo apt-get install checkinstall"
    exit 1
fi
if [ "x$APP_PATH" = "x" ]; then
    APP_PATH=`pwd`
fi
if [ "x$APP_INDEX" = "x" ]; then
    APP_INDEX=index.html
fi
if [ "x$VERSION" = "x" ]; then
    VERSION=1.0.0
fi
if [ "x$XWALK_PATH" = "x" ]; then
    XWALK_PATH=`which xwalk`
    if [ "x$XWALK_PATH" != "x" ]; then
        XWALK_PATH=`dirname $XWALK_PATH`
    fi
    if [ "x$XWALK_PATH" = "x" ]; then
        XWALK_PATH=$APP_PATH
        if ! test -f "$XWALK_PATH/xwalk"; then
            XWALK_PATH=`pwd`
            if ! test -f "$XWALK_PATH/xwalk"; then
                echo Please make sure you have installed xwalk and setup the PATH enviroment variable
                echo on your system properly. Or you can specify the Crosswalk path through --xwalk_path
                echo command line parameter.
                exit 1
            fi
        fi
    fi
fi
if [ "x$APP_NAME" = "x" ]; then
    APP_NAME=`basename $APP_PATH`
fi
if [ "x$PUBLISHER" = "x" ]; then
    PUBLISHER=Me
fi

if [ "x$OUT" != "x" ]; then
    OUT_OPT="--pakdir=$OUT"
fi

if [ "x$SHADOW_INSTALL" = "x" ]; then
    SHADOW_INSTALL=yes
fi

XWALK_PATH=`absolute_path $XWALK_PATH`
APP_PATH=`absolute_path $APP_PATH`
BUILD_DIR=$TEMP_DIR/`basename $APP_PATH`
#export some variables so that Makefile can use them
export INSTALL_DIR=/opt/${PUBLISHER}/${APP_NAME}
if [ ! -d "$INSTALL_DIR" ]; then
  echo "Creating installation directory."
  mkdir -p $INSTALL_DIR
fi


#Prepare for the build dir
mkdir -p $TEMP_DIR
cp -r $APP_PATH $TEMP_DIR
if ! test -f $APP_PATH/Makefile; then
    cp $SCRIPT_DIR/Makefile.templ $BUILD_DIR/Makefile
fi
if ! ls *.desktop > /dev/null 2>&1; then
    while read line; do eval echo $line; done < $SCRIPT_DIR/app.desktop.templ > $BUILD_DIR/$APP_NAME.desktop
fi
cd $XWALK_PATH && cp xwalk xwalk.pak libffmpegsumo.so $BUILD_DIR

#build the package
cd $BUILD_DIR && checkinstall --pkgname=$APP_NAME --pkgversion=$VERSION \
  --backup=no --install=no --exclude=Makefile --fstrans=$SHADOW_INSTALL $OUT_OPT
if [ $? != 0 -a -f  $APP_PATH/Makefile ]; then
    echo "Warning: Packaging failed. Maybe there are some unsupported operations in your app's Makefile? Please try re-run the script with --shadow_install=no using sudo"
    echo
fi
