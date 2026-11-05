@echo off
rem A build whose keys and pad can be driven by tools\inputtest.py, for tests without focus. Never ship it.
setlocal
set "MT_DEFINES=/DGREENEYE_INPUT_TEST"
set "MT_OUT=bin-dev\loader"
call "%~dp0build.bat"
