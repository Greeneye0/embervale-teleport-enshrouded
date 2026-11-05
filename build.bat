@echo off
setlocal
cd /d "%~dp0"
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
if not exist build mkdir build
if not defined MT_OUT set "MT_OUT=bin\loader"
if not exist "%MT_OUT%" mkdir "%MT_OUT%"
cl /nologo /std:c++20 /utf-8 /O2 /MT /W4 /EHa /DUNICODE %MT_DEFINES% /Fobuild\ /LD src\main.cpp src\travel.cpp src\protection.cpp src\popup.cpp /link /MAP:build\map-teleport.map /IMPLIB:build\map-teleport.lib /OUT:%MT_OUT%\map-teleport.dll user32.lib kernel32.lib || exit /b 1
copy /y mod.json %MT_OUT%\mod.json >nul || exit /b 1
copy /y map-teleport.ini %MT_OUT%\map-teleport.ini >nul || exit /b 1
echo Built %MT_OUT%\map-teleport.dll for Greeneye Loader API 2
