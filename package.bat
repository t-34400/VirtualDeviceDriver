@echo off
set DRIVER_NAME=virtual_device_driver

if "%1"=="" (
    set CONFIGURATION=Release
) else (
    set CONFIGURATION=%1
)

set BUILD_DIR=build\%CONFIGURATION%
set OUTPUT_DIR=release\%DRIVER_NAME%

echo Packaging %CONFIGURATION% build...

rd /s /q %OUTPUT_DIR%
mkdir %OUTPUT_DIR%\bin\win64
mkdir %OUTPUT_DIR%\resources

if exist %BUILD_DIR%\%DRIVER_NAME%.dll (
    copy %BUILD_DIR%\%DRIVER_NAME%.dll %OUTPUT_DIR%\bin\win64\driver_%DRIVER_NAME%.dll
) else (
    echo ERROR: %BUILD_DIR%\%DRIVER_NAME%.dll not found!
    exit /b 1
)

xcopy resources %OUTPUT_DIR%\resources\ /E /I

if exist driver.vrdrivermanifest (
    copy driver.vrdrivermanifest %OUTPUT_DIR%\
) else (
    echo WARNING: driver.vrdrivermanifest not found!
)

echo Release package created: %OUTPUT_DIR%
