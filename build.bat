@echo off
setlocal
cd /d "%~dp0"
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
if not exist build mkdir build
if not exist bin\loader mkdir bin\loader
cl /nologo /std:c++20 /O2 /MT /W4 /EHa /DUNICODE /Fobuild\ /LD src\main.cpp src\travel.cpp src\protection.cpp src\popup.cpp /link /MAP:build\map-teleport.map /IMPLIB:build\map-teleport.lib /OUT:bin\loader\map-teleport.dll user32.lib kernel32.lib || exit /b 1
copy /y mod.json bin\loader\mod.json >nul || exit /b 1
copy /y map-teleport.ini bin\loader\map-teleport.ini >nul || exit /b 1
echo Built bin\loader\map-teleport.dll for Greeneye Loader API 2
