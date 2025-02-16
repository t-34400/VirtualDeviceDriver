@echo off
set DRIVER_NAME=virtual_device_driver

:: Remove existing release directory
rd /s /q release\%DRIVER_NAME%
mkdir release\%DRIVER_NAME%\bin\win64
mkdir release\%DRIVER_NAME%\resources

:: Copy necessary files
copy bin\win64\driver_%DRIVER_NAME%.dll release\%DRIVER_NAME%\bin\win64\
xcopy resources release\%DRIVER_NAME%\resources\ /E /I
copy driver.vrdrivermanifest release\%DRIVER_NAME%\

echo Release package created: release\%DRIVER_NAME%
