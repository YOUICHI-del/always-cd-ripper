@echo off
setlocal
set QT=C:\Qt\6.11.0\msvc2022_64
set VCPKG=C:\vcpkg\installed\x64-windows\bin
set PRJ=C:\Users\jokun\Downloads\AlwaysCDRipper2
set SRC=%PRJ%\out\build\x64-Release
set WIX=C:\Program Files (x86)\WiX Toolset v3.14\bin
set OUT=%PRJ%\wix
echo [1/4] windeployqt...
cd "%SRC%"
"%QT%\bin\windeployqt.exe" AlwaysCDRipper.exe
echo [2/4] Copy DLLs...
copy /y "%VCPKG%\FLAC.dll" "%SRC%"
copy /y "%VCPKG%\tag.dll" "%SRC%"
copy /y "%VCPKG%\ogg.dll" "%SRC%"
copy /y "%VCPKG%\zlib1.dll" "%SRC%"
copy /y "%PRJ%\resources\icon.ico" "%SRC%"
echo [3/4] candle...
cd "%OUT%"
"%WIX%\candle.exe" "%OUT%\AlwaysCDRipper.wxs" -out "%OUT%\AlwaysCDRipper.wixobj" -arch x64
if errorlevel 1 ( echo ERROR: candle failed & pause & exit /b 1 )
echo [4/4] light...
"%WIX%\light.exe" "%OUT%\AlwaysCDRipper.wixobj" -ext WixUIExtension -ext WixUtilExtension -out "%OUT%\AlwaysCDRipper_Setup.msi" -b "%SRC%"
if errorlevel 1 ( echo ERROR: light failed & pause & exit /b 1 )
echo === Done ===
pause
