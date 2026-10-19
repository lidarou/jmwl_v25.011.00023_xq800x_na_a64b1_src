#!/bin/bash
#
# Script to install the DDK
#
# This script creates the directory tree, unpacks the archives
# and builds the examples.
#

#------------------------------------------------------------------------------
#
#    Module        : DDK-130_bsd
#    Version       : 3.1
#    Configuration : DDK_EIP130_BSD
#
#    Date          : 2022-Feb-14
#
# Copyright (c) 2014-2022 by Rambus, Inc. and/or its subsidiaries.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# check_file_exists
#
# Arguments:
#  1. path to file to check, may include whitespaces.
#
# This function checks if the given file exists. The function returns when
# the file exists. Otherwise a message is printed and the script is aborted
# with exit code 1.
#
function check_file_exists()
{
  if [ ! -f "$1" ]
  then
      echo "[INSTALL] Missing file $1"
      echo "[INSTALL] DDK installation failed"
      exit 1
  fi
}

#-------------------------------------------------------------------------------
# check_warning_flag
#
# Arguments:
#  1. gcc warning flag e.g. -Wall
#
# This function checks if the given compiler flag is supported or not. It
# tries to compile dummy '.c' file with used compiler or cross compiler and
# add that string to the WARNING_FLAGS variable if compilation was successful.
# WARNING_FLAGS should be passed to the setvars.sh file and used in Makefiles.
#
function check_warning_flag()
{
    touch foo.c
    if [ "$CROSS_COMPILE" != '' ]
    then
        CC_LOCAL=${CROSS_COMPILE}gcc
    else
        CC_LOCAL=gcc
    fi
    WARNING_FLAGS+=$(${CC_LOCAL} $1 -c foo.c -o foo.o &> /dev/null && echo " $1")
    rm -f foo.c foo.o
}

# main script starts here

echo ""
echo "[INSTALL] Please check for availability of recent DDK patches!"
echo ""

echo "[INSTALL] Do you want to set the DDK environment variables [y/n]?"
read nvar
if [ "$nvar" != 'y' ]
then
    echo "[INSTALL] Skipping the DDK environment variables setup"
    echo ""
    echo "[INSTALL] Installation completed successfully"
    echo ""
    exit 0
fi

if [ "$SYSTEMTEST_CONFIGURATION" != '' ]
then
    # Make sure we use the default system configuration
    unset SYSTEMTEST_CONFIGURATION
fi

export DDKDIR=`pwd`
export EARLY_USER_LFLAGS=

if [ -d Config/FPGA_V2M_Juno ]
then
    export HOST_HW_PLATFORM=FPGA_V2M_Juno
    export EARLY_USER_LFLAGS="-L${DDKDIR}/Examples/Driver/build -lmemrepl -fno-builtin-memset -fno-builtin-memcpy"
elif [ -d Config/FPGA_Virtex6_PCI ]
then
    export HOST_HW_PLATFORM=FPGA_Virtex6_PCI
elif [ -d Config/FPGA_Zynq_ZC702 ]
then
    export HOST_HW_PLATFORM=FPGA_Zynq_ZC702
else
    echo "[INSTALL] FPGA type not recognized, DDK installation failed"
    exit 1
fi

echo "[INSTALL] Host hardware platform (HOST_HW_PLATFORM), fixed at $HOST_HW_PLATFORM"
echo "Other options: only available after driver customization"
echo
echo "These variables can be changed later if the driver is configured"
echo "for different hardware"
echo

echo "[INSTALL] Set HW platform architecture (ARCH) variable"
if [ "$ARCH" = '' ]
then
    if [ "$HOST_HW_PLATFORM" = "FPGA_Virtex6_PCI" ]
    then
        echo "[INSTALL] Type x86 or powerpc"
        read nvar
        if [ "$nvar" != '' ]
        then
            export ARCH=$nvar
        fi
        case $ARCH in
            x86 | X86) echo "[INSTALL] ARCH is set to $ARCH";;
            powerpc | POWERPC) echo "[INSTALL] ARCH is set to $ARCH";;
            *) echo "[INSTALL] $ARCH is not supported, defaulting to x86"; export ARCH=x86;;
        esac
    fi

    if [ "$HOST_HW_PLATFORM" = "FPGA_V2M_Juno" ]
    then
        export ARCH=arm64
        echo "[INSTALL] ARCH is set to $ARCH"
    fi

    if [ "$HOST_HW_PLATFORM" = "FPGA_Zynq_ZC702" ]
    then
        echo "[INSTALL] Type arm or mb"
        read nvar
        if [ "$nvar" != '' ]
        then
            export ARCH=$nvar
        fi
        case $ARCH in
            arm | ARM) echo "[INSTALL] ARCH is set to $ARCH";;
            mb | MB) echo "[INSTALL] ARCH is set to $ARCH";;
            *) echo "[INSTALL] $ARCH is not supported, defaulting to arm"; export ARCH=arm;;
        esac
    fi

    if [ "$ARCH" = '' ]
    then
        echo "[INSTALL] ARCH is not set"
        exit 1
    fi
else
    echo "[INSTALL] ARCH is set to $ARCH"
fi

if [ "$ARCH" = "mb" ]
then
    echo "[INSTALL] MicroBlaze Little Endian configuration [y/n]?"
    read nvar
    if [ "$nvar" = 'y' ]
    then
        export ARCH_MB=ARCH_MB_LE
    else
        export ARCH_MB=ARCH_MB_BE
    fi

    echo "[INSTALL] Set Xilinx SDK path (XSDK_PATH)"
    if [ "$XSDK_PATH" = '' ]
    then
        echo "[INSTALL] Type path"
        read nvar
        if [ "$nvar" != '' ]
        then
            export XSDK_PATH=$nvar
        else
            echo "[INSTALL] XSDK_PATH is not set"
            exit 1
        fi
    else
        echo "[INSTALL] XSDK_PATH is set to $XSDK_PATH"
    fi

    echo "[INSTALL] Set Xilinx MicroBlaze FreeRTOS BSP path (XBSP_PATH)"
    if [ "$XBSP_PATH" = '' ]
    then
        echo "[INSTALL] Type path"
        read nvar
        if [ "$nvar" != '' ]
        then
            export XBSP_PATH=$nvar
        else
            echo "[INSTALL] XBSP_PATH is not set"
            exit 1
        fi
    else
        echo "[INSTALL] XBSP_PATH is set to $XBSP_PATH"
    fi

    if [ ! -f $XBSP_PATH/bitstream.bit ]
    then
        echo "[INSTALL] Missing BSP file $XBSP_PATH/bitstream.bit"
        exit 1
    fi

    if [ ! -f $XBSP_PATH/fsbl.elf ]
    then
        echo "[INSTALL] Missing BSP file $XBSP_PATH/fsbl.elf"
        exit 1
    fi

    if [ ! -f $XBSP_PATH/lscript.ld ]
    then
        echo "[INSTALL] Missing BSP file $XBSP_PATH/lscript.ld"
        exit 1
    fi

    if [ ! -d $XBSP_PATH/include -o ! -d $XBSP_PATH/lib ]
    then
        echo "[INSTALL] Missing BSP direcotries inlcude and lib"
        exit 1
    fi

    if [ -d ${DDKDIR}/TestTools/Regress_vip/mb_freertos/bsp ]
    then
        rm -rf ${DDKDIR}/TestTools/Regress_vip/mb_freertos/bsp
    fi

    ln -s $XBSP_PATH ${DDKDIR}/TestTools/Regress_vip/mb_freertos/bsp

    export CROSS_COMPILE=mb-
    export CROSS_LIB_PATH=${XBSP_PATH}/lib
    export REGRESS_LIB_PATH="-L. -L${DDKDIR}/Examples/DemoApp_OTPKeyBlob/build -L${DDKDIR}/Examples/DemoApp_OTPProgram/build -L${DDKDIR}/Examples/DemoApp_SecureDebug/build -L${DDKDIR}/Examples/DemoApp_VAL_Specials/build -L${DDKDIR}/Examples/DemoApp_VAL/build -L${DDKDIR}/TestTools/TestTool_VAL/build"
    export REGRESS_LIB="-lda_otpkeyblob_combined_u -lda_otpprogram_combined_u -lda_securedebug_combined_u -lda_val_specials_combined_u -lda_val_combined_u -ltest_val_combined_u"
    export USER_CFLAGS="-fmessage-length=0 -mcpu=v10.0 -mxl-soft-mul -Wl,--no-relax -ffunction-sections -fdata-sections -I${XBSP_PATH}/include"
    export USER_LFLAGS="-Wl,--start-group,-lmbfreertos,-lxil,-lfreertos,-lgcc,-lc,--end-group -mcpu=v10.0 -mxl-soft-mul -Wl,--no-relax -Wl,--gc-sections"
    if [ "$ARCH_MB" = "ARCH_MB_LE" ]
    then
        export USER_CFLAGS="-mlittle-endian "$USER_CFLAGS
        export USER_LFLAGS="-mlittle-endian "$USER_LFLAGS
    fi
    export EARLY_USER_LFLAGS="-Wl,-T -Wl,${XBSP_PATH}/lscript.ld"
    export DDK_HOST64=n
    export PATH=${XSDK_PATH}/gnu/microblaze/lin/bin:${XSDK_PATH}/bin:$PATH
else
    echo "[INSTALL] Set Linux kernel source directory (KERNEL_DIR) variable"
    if [ "$KERNEL_DIR" = '' ]
    then
        read nvar
        if [ "$nvar" != '' ]
        then
            export KERNEL_DIR=$nvar
        else
            echo "[INSTALL] KERNEL_DIR is not set"
            exit 1
        fi
    else
        echo "[INSTALL] KERNEL_DIR is set to $KERNEL_DIR"
    fi

    if [ ! -d "$KERNEL_DIR" ]
    then
        echo "[INSTALL] Directory does not exist $KERNEL_DIR"
        echo "[INSTALL] Set KERNEL_DIR correctly"
        unset KERNEL_DIR
        exit 1
    fi

    echo "[INSTALL] Set cross-compiler (CROSS_COMPILE) variable if used"
    if [ "$CROSS_COMPILE" = '' ]
    then
        echo "[INSTALL] CROSS_COMPILE is not set"
        echo "[INSTALL] Type value or press ENTER to disable it"
        read nvar
        if [ "$nvar" != '' ]
        then
            export CROSS_COMPILE=$nvar
        else
            export CROSS_COMPILE=''
        fi
    else
        echo "[INSTALL] CROSS_COMPILE is set to $CROSS_COMPILE"
    fi

    echo "[INSTALL] Set cross library path (CROSS_LIB_PATH) variable if used"
    if [ "$CROSS_LIB_PATH" = '' ]
    then
        echo "[INSTALL] CROSS_LIB_PATH is not set"
        echo "[INSTALL] Type path or press ENTER to clear it"
        read nvar
        if [ "$nvar" != '' ]
        then
            export CROSS_LIB_PATH=$nvar
        else
            export CROSS_LIB_PATH=''
        fi
    else
        echo "[INSTALL] CROSS_LIB_PATH is set to $CROSS_LIB_PATH"
    fi

    if [ "$CROSS_COMPILE" != '' -a ! -d "$CROSS_LIB_PATH" ]
    then
        echo "[INSTALL] Cross-compiler: $CROSS_COMPILE"
        echo "[INSTALL] Cross library path does not exist: $CROSS_LIB_PATH"
        echo "[INSTALL] Set CROSS_COMPILE and CROSS_LIB_PATH correctly and re-try"
        exit 1
    fi

    if grep CONFIG_64BIT=y $KERNEL_DIR/.config >/dev/null
    then
       echo "[INSTALL] Kernel is found to be configured for 64 bit."
       export DDK_HOST64=y
    else
       echo "[INSTALL] Kernel is found to be configured for 32 bit."
       export DDK_HOST64=n
    fi
    export USER_LFLAGS=

    # Check if some warning flag is supported
    WARNING_FLAGS=
    check_warning_flag -Wrestrict
    check_warning_flag -Wduplicated-cond
    check_warning_flag -Wduplicated-branches
    export WARNING_FLAGS=$WARNING_FLAGS
fi

export DDK_DEVICE64=y

# Save the relevant exports to file
echo export ARCH=$ARCH >setvars.sh
echo export HOST_HW_PLATFORM=$HOST_HW_PLATFORM >>setvars.sh
echo export DDK_HOST64=$DDK_HOST64 >>setvars.sh
echo export DDK_DEVICE64=$DDK_DEVICE64 >>setvars.sh
if [ "$ARCH" = "mb" ]
then
    echo export XSDK_PATH=$XSDK_PATH >>setvars.sh
    echo export XBSP_PATH=$XBSP_PATH >>setvars.sh
    echo export REGRESS_LIB_PATH=\"${REGRESS_LIB_PATH}\" >>setvars.sh
    echo export REGRESS_LIB=\"${REGRESS_LIB}\" >>setvars.sh
    echo export PATH=$PATH >>setvars.sh
    echo export USER_CFLAGS=\"${USER_CFLAGS}\" >>setvars.sh
    echo export USER_LFLAGS=\"${USER_LFLAGS}\" >>setvars.sh
else
    echo export KERNEL_DIR=$KERNEL_DIR >>setvars.sh
    echo export USER_LFLAGS=$USER_LFLAGS >>setvars.sh
fi
echo export EARLY_USER_LFLAGS=\"${EARLY_USER_LFLAGS}\" >>setvars.sh
echo export CROSS_COMPILE=$CROSS_COMPILE >>setvars.sh
echo export CROSS_LIB_PATH=$CROSS_LIB_PATH >>setvars.sh
echo export BUILD_CONFIGS= >>setvars.sh
echo export DDKDIR=$PWD >>setvars.sh
echo export WARNING_FLAGS=\"${WARNING_FLAGS}\" >>setvars.sh
echo alias gd=\'cd $DDKDIR\' >>setvars.sh

echo "[INSTALL] Do you want to build the DDK components now [y/n]?"
read nvar
if [ "$nvar" != 'y' ]
then
    echo "[INSTALL] Skipping the DDK SW components build ($nvar)"
    echo ""
    echo "[INSTALL] Source setvars.sh file to set environment variables"
    echo "[INSTALL] Change to TestTools/Regress_vip directory to build and run regression tests"
    echo "[INSTALL] Installation completed successfully"
    echo ""
    exit 0
fi

# Build the combined driver
echo ""
echo "[INSTALL] Building the EIP-13x combined driver"
cd Examples/Driver/build
make
if [ "$ARCH" = "mb" ]
then
    check_file_exists libdriver_vip_u.a
else
    check_file_exists driver_vip_k.ko
    check_file_exists libdriver_vip_u.a
    make DDK_VEX_PROXY=y driver_vip_u
    check_file_exists libdriver_vip_up.a

    # Build the VAL driver
    echo "[INSTALL] Building the EIP-13x VAL driver"
    cd ../../../Examples/Driver_VAL/build
    make clean; make
    check_file_exists driver_val_k.ko
    make DDK_VEX_PROXY=y driver_val_up
    check_file_exists libdriver_val_up.a

    # Build the GC driver
    echo "[INSTALL] Building the EIP-13x GC driver"
    cd ../../../Examples/Driver_GC/build
    make clean; make
    check_file_exists driver_gc_k.ko
fi
cd ../../..

# Build the VAL Test Tool
if [ -d TestTools/TestTool_VAL/build ]
then
    echo "[INSTALL] Building the VAL Test Tool"
    cd TestTools/TestTool_VAL/build
    make clean; make
    if [ "$ARCH" = "mb" ]
    then
        check_file_exists libtest_val_combined_u.a
    else
        check_file_exists test_val_k.ko
        check_file_exists test_val_up
        check_file_exists test_val_combined_u
        check_file_exists test_val_combined_up
    fi
    cd ../../..
fi

# Build the Demo Applications
if [ -d Examples/DemoApp_VAL/build ]
then
    echo "[INSTALL] Building the Demo Application for VAL"
    cd Examples/DemoApp_VAL/build
    make clean; make
    if [ "$ARCH" = "mb" ]
    then
        check_file_exists libda_val_combined_u.a
    else
        check_file_exists da_val_k.ko
        check_file_exists da_val_up
        check_file_exists da_val_combined_u
        check_file_exists da_val_combined_up
    fi
    cd ../../..
fi

if [ -d Examples/DemoApp_VAL_Specials/build ]
then
    echo "[INSTALL] Building the Demo Application for VAL Specials"
    cd Examples/DemoApp_VAL_Specials/build
    make clean; make
    if [ "$ARCH" = "mb" ]
    then
        check_file_exists libda_val_specials_combined_u.a
    else
        check_file_exists da_val_specials_up
        check_file_exists da_val_specials_combined_u
        check_file_exists da_val_specials_combined_up
    fi
    cd ../../..
fi

if [ -d Examples/DemoApp_SecureDebug/build ]
then
    echo "[INSTALL] Building the Demo Application for Secure Debug"
    cd Examples/DemoApp_SecureDebug/build
    make clean; make
    if [ "$ARCH" = "mb" ]
    then
        check_file_exists libda_securedebug_combined_u.a
    else
        check_file_exists da_securedebug_k.ko
        check_file_exists da_securedebug_up
        check_file_exists da_securedebug_combined_u
        check_file_exists da_securedebug_combined_up
    fi
    cd ../../..
fi

if [ -d Examples/DemoApp_OTPKeyBlob/build ]
then
    echo "[INSTALL] Building the Demo Application for OTP Key Blob"
    cd Examples/DemoApp_OTPKeyBlob/build
    make clean; make
    if [ "$ARCH" = "mb" ]
    then
        check_file_exists libda_otpkeyblob_combined_u.a
    else
        check_file_exists da_otpkeyblob_up
        check_file_exists da_otpkeyblob_combined_u
        check_file_exists da_otpkeyblob_combined_up
    fi
    cd ../../..
fi

if [ -d Examples/DemoApp_OTPProgram/build ]
then
    echo "[INSTALL] Building the Demo Application for OTP Programming"
    cd Examples/DemoApp_OTPProgram/build
    make clean; make
    if [ "$ARCH" = "mb" ]
    then
        check_file_exists libda_otpprogram_combined_u.a
    else
        check_file_exists da_otpprogram_up
        check_file_exists da_otpprogram_combined_u
        check_file_exists da_otpprogram_combined_up
    fi
    cd ../../..
fi

# install - end
echo ""
echo "[INSTALL] Source setvars.sh file to set environment variables"
echo "[INSTALL] Change to TestTools/Regress_vip directory to build and run regression tests"
echo "[INSTALL] Installation completed successfully"
echo ""

# end of file
