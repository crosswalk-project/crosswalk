rem @echo off
setlocal enabledelayedexpansion
set THIS_SCRIPT=%0
set WXS_TEMPL_FILE="%~dp0\app.wxs.templ"
:loop
set DASHED_PARAM=%1
set PARAM=!DASHED_PARAM:--=!
if "!PARAM!" == "!DASHED_PARAM!" (
  set APP_PATH=!PARAM!
) else (
    if x%2==x (set !PARAM!=true) else (set !PARAM!=%2)
    shift
)
@shift
    
@if not "%~1"=="" goto loop

if not x%HELP%==x (
    echo.
    echo usage: %THIS_SCRIPT% [options] [app_path]
    echo.
    echo This script is used to create a standalone installer for cameo applications. It 
    echo depends on Wix toolset and cameo to function properly.
    echo The following options are supported:
    echo.
    echo app_path                Path to the cameo application. If not specified, the
    echo                           current directory is used.
    echo --wix_bin_path=^<path^>   Path to Wix toolset binaries. If not specified, the
    echo                           script will try to find them through PATH
    echo --cameo_path=^<path^>     Path to Cameo binaries. If not specified, the script
    echo                           will try to find them through PATH, the app path, or
    echo                           the current directory.
    echo --app_name=^<name^>       Name of the application. If not specified, the name
    echo                           of the application directory is used.
    echo --version=^<version^>     The version of the application, defaults to 1.0.0
    echo --app_arguments=^<args^>  Arguments that will be passed into cameo executable.
    echo                           If not specified, "index.html" is used. For example,
    echo                           "--allow-file-access-from-files src/index.html"
    echo --out=^<pathname^>        File Path of the output installer file, defaults to the
    echo                           current directory with %%app_name%%.msi as its name.
    echo --publisher=^<name^>      The manufacturer of this application, defaults to "Me"
    echo --help                  Print this message 
    exit /b

)
if x%WIX_BIN_PATH%==x (
    FOR /F "tokens=*" %%A IN ('where candle') DO SET WIX_BIN_PATH="%%A"
)
if not x%WIX_BIN_PATH%==x set WIX_BIN_PATH=!WIX_BIN_PATH:\candle.exe=!
if x%WIX_BIN_PATH%==x (
  set WIX_BIN_PATH="C:\Program Files (x86)\WiX Toolset v3.7\bin"
  if not exist "!WIX_BIN_PATH!\candle.exe" (
    echo Please make sure you have installed Wix and setup the PATH enviroment variable on
    echo your system properly. Or you can specify the wix path through --wix_bin_path
    echo command line parameter. You can download Wix from http://wixtoolset.org
    exit /b
  )
)
if x%APP_PATH%==x set APP_PATH=%CD%
if x%APP_ARGUMENTS%==x set APP_ARGUMENTS=index.html
if x%VERSION%==x set VERSION=1.0.0
if x%CAMEO_PATH%==x (
  FOR /F "tokens=*" %%A IN ('where cameo') DO SET CAMEO_PATH="%%A"
  if x%CAMEO_PATH%==x (
      set CAMEO_PATH=%APP_PATH%
      if not exist "!CAMEO_PATH!\cameo.exe" (
          set CAMEO_PATH=%CD%
          if not exist "!CAMEO_PATH!\cameo.exe" (
            echo Please make sure you have installed cameo and setup the PATH enviroment variable
            echo on your system properly. Or you can specify the cameo path through --cameo_path 
            echo command line parameter.
            
            exit /b
        )
      )
  )
)
if x%APP_NAME%==x  (
    for %%F in (%APP_PATH%) do set APP_NAME=%%~nF
)
if x%PUBLISHER%==x set PUBLISHER=Me

set MAIN_OBJ_FILE=%TEMP%\%APP_NAME%.wixobj

pushd %CAMEO_PATH%
set CAMEO_PATH=%CD%
popd
pushd %APP_PATH%
set APP_PATH=%CD%
popd

if not "x!APP_PATH:%CAMEO_PATH%=!"=="x%APP_PATH%" (
    set __DIRS.1="%APP_PATH%"
    set __HARV_FILES.1=appFiles
) else (
    if not "x!CAMEO_PATH:%APP_PATH%=!"=="x%CAMEO_PATH%" (
        set __DIRS.1="%CAMEO_PATH%"
        set __HARV_FILES.1=cameoFiles
    ) else (
        set __DIRS.1="%APP_PATH%"
        set __HARV_FILES.1=appFiles
        set __DIRS.2="%CAMEO_PATH%"
        set __HARV_FILES.2=cameoFiles
    )
)
set WXS_FILES=
set OBJ_FILES=
if x%OUT%==x (
    set OUT=%APP_NAME%.msi
    if exist !OUT! (
        set /p CONFIRM_DEL="The output file !OUT! already exists, replace it(Y/N)?" %=%
        if /I Y==!CONFIRM_DEL! goto :yes
        if /I YES==!CONFIRM_DEL! goto :yes
        echo.
        echo Installer not created!
        exit /b
        :yes
        del !OUT!
    )
)
for /f "tokens=2* delims=.=" %%A IN ('"SET __HARV_FILES."') do (
    set WXS_FILE=%TEMP%\%%B.wxs
    set OBJ_FILE=%TEMP%\%%B.wixobj
    %WIX_BIN_PATH%\heat dir !__DIRS.%%A! -gg -cg %%BGroup -srd -dr INSTALLDIR -var var.%%BDir -o !WXS_FILE!
    %WIX_BIN_PATH%\candle -dappFilesDir="%APP_PATH%" -dcameoFilesDir="%CAMEO_PATH%" !WXS_FILE! -o !OBJ_FILE!
    set "WXS_FILES=!WXS_FILES! !WXS_FILE!"
    set "OBJ_FILES=!OBJ_FILES! !OBJ_FILE!"
)

%WIX_BIN_PATH%\candle -dappArguments=%APP_ARGUMENTS% -dappFilesDir="%APP_PATH%" -dcameoFilesDir="%CAMEO_PATH%" %WXS_TEMPL_FILE% -o %MAIN_OBJ_FILE%
%WIX_BIN_PATH%\light -spdb %MAIN_OBJ_FILE% %OBJ_FILES% -o %OUT%
