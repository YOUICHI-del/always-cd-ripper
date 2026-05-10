@echo off
setlocal

set WIX=C:\Program Files (x86)\WiX Toolset v3.14\bin
set SRC=C:\Users\jokun\Downloads\AlwaysCDRipper2\out\build\x64-Release
set WXS=C:\Users\jokun\Downloads\AlwaysCDRipper2\wix\AlwaysCDRipper.wxs
set OUT=C:\Users\jokun\Downloads\AlwaysCDRipper2\wix

echo.
echo === Always CD Ripper Installer Build ===
echo.

mkdir "%SRC%\platforms"    2>nul
mkdir "%SRC%\styles"       2>nul
mkdir "%SRC%\imageformats" 2>nul

echo [1/2] candle...
"%WIX%\candle.exe" "%WXS%" -out "%OUT%\AlwaysCDRipper.wixobj" -arch x64
if errorlevel 1 (
    echo ERROR: candle failed
    pause
    exit /b 1
)

echo [2/2] light...
"%WIX%\light.exe" "%OUT%\AlwaysCDRipper.wixobj" ^
    -ext WixUIExtension ^
    -ext WixUtilExtension ^
    -cultures:ja-JP ^
    -out "%OUT%\AlwaysCDRipper_Setup.msi" ^
    -b "%SRC%"
if errorlevel 1 (
    echo ERROR: light failed
    pause
    exit /b 1
)

echo.
echo === Done: %OUT%\AlwaysCDRipper_Setup.msi ===
echo.
pause
