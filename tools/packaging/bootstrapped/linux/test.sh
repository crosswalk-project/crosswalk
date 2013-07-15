#!/bin/sh
init_env() {
  APP_NAME=$1
  VERSION=$2
  if [ "x$VERSION" = "x" ]; then
    VERSION=1.0.0
  fi
  DEB_FILE=${APP_NAME}_$VERSION-*.deb 
  DEB_PATH=$BUILD_DIR/$DEB_FILE
  DESKTOP_FILE=$APP_NAME.desktop
  DESKTOP_PATH=$BUILD_DIR/$DESKTOP_FILE
  DESKTOP_PATH_IN_DEB=/usr/share/applications/$DESKTOP_FILE
  if test -f $DEB_PATH; then
    rm $DEB_PATH
  fi 
}

file_exist_in_deb() {
    run_test "Check file $1 existence in deb" "dpkg --contents $DEB_PATH |grep $1"
}

run_test () {
    if ! eval $2; then
        echo $1 failed!
        exit 1
    fi
}

APP_NAME=test-app
TMP_DIR=/tmp
XWALK_PATH=--xwalk_path=../../../../../../out/Release 
BUILD_DIR=$TMP_DIR/xwalk_build/$APP_NAME
if test -e $BUILD_DIR; then
    rm -r $BUILD_DIR
fi

mkdir -p $APP_NAME
cd $APP_NAME
echo hello > index.html

init_env $APP_NAME

run_test "Check app_arguments help message" "../create_linux_installer.sh --help |grep app_arguments"

case $1 in 
    1) 
        init_env test-app 1.0.1
        DEB_PATH=$TMP_DIR/$DEB_FILE
        run_test "Create package" "../create_linux_installer.sh $XWALK_PATH --version=$VERSION --out=$TMP_DIR"

        #test if the version argument works
        run_test "Check DEB existence" "test -e $DEB_PATH"
        #test if files installed successfully
        file_exist_in_deb index.html
        file_exist_in_deb xwalk
        file_exist_in_deb xwalk.pak
        file_exist_in_deb libffmpegsumo.so
        file_exist_in_deb $DESKTOP_PATH_IN_DEB
        ;;

    2) 
        init_env custom-app-name 
        run_test "Create package with customized app name" "../create_linux_installer.sh $XWALK_PATH --app_name=$APP_NAME && test -e $DEB_PATH"
        file_exist_in_deb $DESKTOP_PATH_IN_DEB
        run_test "Check desktop file" "grep $APP_NAME $DESKTOP_PATH"
;;
    3)
        CUSTOM_INDEX=src/custom_index.html
        APP_ARGUMENTS=--allow-file-access-from-files
        run_test "Test app_index" "../create_linux_installer.sh $XWALK_PATH --app_arguments=$APP_ARGUMENTS --app_index=$CUSTOM_INDEX && grep $CUSTOM_INDEX $DESKTOP_PATH && grep -- $APP_ARGUMENTS $DESKTOP_PATH"
;;
esac
