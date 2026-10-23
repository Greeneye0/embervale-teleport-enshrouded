@echo off
setlocal
cd /d "%~dp0.."
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
if not exist build mkdir build
cl /nologo /std:c++20 /W4 /EHsc /MT /Fobuild\tests.obj /Febuild\tests.exe tools\tests.cpp || exit /b 1
build\tests.exe || exit /b 1
cl /nologo /std:c++20 /W4 /EHa /MT /Fobuild\ /Febuild\protection-tests.exe tools\protection_tests.cpp src\protection.cpp || exit /b 1
build\protection-tests.exe || exit /b 1
cl /nologo /std:c++20 /W4 /EHa /MT /DMAP_TELEPORT_TEST /Fobuild\ /Febuild\travel-tests.exe tools\travel_tests.cpp src\travel.cpp || exit /b 1
build\travel-tests.exe || exit /b 1
build\travel-tests.exe deluxe || exit /b 1
build\travel-tests.exe deluxe unknown-bounds || exit /b 1
build\travel-tests.exe prank local || exit /b 1
build\travel-tests.exe prank remote || exit /b 1
build\travel-tests.exe prank restart || exit /b 1
build\travel-tests.exe prank disconnect || exit /b 1
build\travel-tests.exe prank failed-move || exit /b 1
build\travel-tests.exe prank prediction || exit /b 1
python tools\verify_signatures.py || exit /b 1
