#!/bin/bash
function absolute_path () {
    echo $(cd $1 && pwd)
}
THIS_SCRIPT=$0
SCRIPT_DIR=$(absolute_path $(dirname $THIS_SCRIPT))
TEMP_DIR=/tmp/cameo_build
while [ "$1" != "" ]; do
    DASHED_PARAM=$1
    PARAM=${DASHED_PARAM/--/}
    if [ "$PARAM" == "$DASHED_PARAM" ]; then
      APP_PATH=$PARAM
    else
        if [[ "$PARAM" != *"="* ]]; then
            eval ${PARAM^^}=true
        else
            read PARAM_NAME PARAM_VALUE <<<$(IFS="="; echo $PARAM)
            eval ${PARAM_NAME^^}=$PARAM_VALUE
        fi
    fi
    shift
done

if [ "x$HELP" != "x" ]; then
    echo
    echo usage: $THIS_SCRIPT [options] [app_path]
    echo
    echo This script is used to create a standalone installer for cameo applications. It
    echo depends on checkinstall and cameo to function properly.
    echo The following options are supported:
    echo "
     app_path                Path to the cameo application. If not specified, the
                               current directory is used.
     --cameo_path=<path>     Path to Cameo binaries. If not specified, the script
                               will try to find them through PATH, the app path, or
                               the current directory.
     --app_name=<name>       Name of the application. If not specified, the name
                               of the application directory is used.
     --version=<version>     The version of the application, defaults to 1.0.0
     --app_index=<path>      Path of app index file, relative to app_path. If not
                               specified, index.html is used.
     --out=<path>            Path of the output package file, defaults to
                               $TEMP_DIR/<app_name>
     --publisher=<name>      The manufacturer of this application, defaults to "Me"
     --help                  Print this message "
    exit 1
fi

if [ "x`which checkinstall`" == "x" ]; then
    echo
    echo "checkinstall is required to create an application package. You can install it by:
    sudo apt-get install checkinstall"
    exit 1
fi
if [ "x$APP_PATH" == "x" ]; then
    APP_PATH=`pwd`
fi
if [ "x$APP_INDEX" == "x" ]; then
    APP_INDEX=index.html
fi
if [ "x$VERSION" == "x" ]; then
    VERSION=1.0.0
fi
if [ "x$CAMEO_PATH" == "x" ]; then
    CAMEO_PATH=`which cameo`
    if [ "x$CAMEO_PATH" != "x" ]; then
        CAMEO_PATH=$(dirname $CAMEO_PATH)
    fi
    if [ "x$CAMEO_PATH" == "x" ]; then
        CAMEO_PATH=$APP_PATH
        if ! test -f "$CAMEO_PATH/cameo"; then
            CAMEO_PATH=`pwd`
            if ! test -f "$CAMEO_PATH/cameo"; then
                echo Please make sure you have installed cameo and setup the PATH enviroment variable
                echo on your system properly. Or you can specify the cameo path through --cameo_path
                echo command line parameter.
                exit 1
            fi
        fi
    fi
fi
if [ "x$APP_NAME" == "x" ]; then
    APP_NAME=$(basename $APP_PATH)
fi
if [ "x$PUBLISHER" == "x" ]; then
    PUBLISHER=Me
fi

if [ "x$OUT" != "x" ]; then
    OUT_OPT="--pakdir=$OUT"
fi


CAMEO_PATH=$(absolute_path $CAMEO_PATH)
APP_PATH=$(absolute_path $APP_PATH)
BUILD_DIR=$TEMP_DIR/$(basename $APP_PATH)
#export some variables so that Makefile can use them
export INSTALL_DIR=/opt/${PUBLISHER}/${APP_NAME}

#Prepare for the build dir
mkdir -p $TEMP_DIR
cp -r $APP_PATH $TEMP_DIR
if ! test -f $APP_PATH/Makefile; then
    cp $SCRIPT_DIR/Makefile.templ $BUILD_DIR/Makefile
fi
if ! ls *.desktop &> /dev/null; then
    eval echo "\"$(cat <<EOF_$RANDOM
$(<$SCRIPT_DIR/app.desktop.templ)
EOF_$RANDOM
)\"" > $BUILD_DIR/$APP_NAME.desktop
fi
cd $CAMEO_PATH && cp cameo cameo.pak libffmpegsumo.so $BUILD_DIR

#build the package
cd $BUILD_DIR && checkinstall --pkgname=$APP_NAME --pkgversion=$VERSION --backup=no --install=no --exclude=Makefile --fstrans=yes $OUT_OPT
