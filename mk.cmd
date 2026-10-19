@echo off

REM =====================================================================================
set ARM_TOOLCHAIN_PATH=C:\Program Files\BuildTools\gcc-arm-none-eabi-10-2020-q4-major\bin
set AICPATH1=C:\Program Files\BuildTools\ninja
set AICPATH2=C:\Program Files\CMake\bin
set PATH=%AICPATH1%;%AICPATH2%;%PATH%;
REM =====================================================================================

python ./build/env.py
python ./lunch.py 3
python ./make.py all