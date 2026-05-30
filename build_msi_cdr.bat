@echo off
cd /d C:\Users\jokun\Downloads\AlwaysCDRipper2

set WIX=C:\Program Files (x86)\WiX Toolset v3.14\bin
set SRC=C:\Users\jokun\Downloads\AlwaysCDRipper2\out\build\x64-Release

echo [1/2] Candle...
"%WIX%\candle.exe" -nologo -dSourceDir="%SRC%" AlwaysCDRipper.wxs
if errorlevel 1 ( echo Candle failed & pause & exit /b 1 )

echo [2/2] Light...
"%WIX%\light.exe" -nologo -ext WixUIExtension AlwaysCDRipper.wixobj -out "%USERPROFILE%\Downloads\Always_CD_Ripper_Setup_1.0.0.msi"
if errorlevel 1 ( echo Light failed & pause & exit /b 1 )

echo Done! Always_CD_Ripper_Setup_1.0.0.msi created.
pause
