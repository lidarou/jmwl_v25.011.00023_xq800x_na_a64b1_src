# SPDX-License-Identifier: Apache-2.0

if(CONFIG_COMPILER_ISA_THUMB2)
  list(APPEND TOOLCHAIN_C_FLAGS --thumb)
endif()
list(APPEND TOOLCHAIN_LD_FLAGS  --entry=_mainCRTStartup)

# Defines a mapping from GCC_M_CPU to FPU
if(CONFIG_CPU_HAS_FPU_DOUBLE_PRECISION)
  set(PRECISION_TOKEN)
else()
  set(PRECISION_TOKEN sp-)
endif()

set(FPU_FOR_cortex-m4        fpv4-${PRECISION_TOKEN}d16)
set(FPU_FOR_cortex-m7        fpv5-${PRECISION_TOKEN}d16)
set(FPU_FOR_cortex-m33       fpv5-${PRECISION_TOKEN}d16)
set(FPU_FOR_cortex-m33+nodsp fpv5-${PRECISION_TOKEN}d16)

if(CONFIG_FPU)
  list(APPEND TOOLCHAIN_C_FLAGS     -mfpu=${FPU_FOR_${GCC_M_CPU}})
  if(CONFIG_FP_SOFTABI)
    list(APPEND TOOLCHAIN_C_FLAGS   -mfloat-abi=softfp)
  elseif(CONFIG_FP_HARDABI)
    list(APPEND TOOLCHAIN_C_FLAGS   -mfloat-abi=hard)
  endif()
else()
  list(APPEND TOOLCHAIN_C_FLAGS     --fpu=SoftVFP)
  if ("${GCC_M_CPU}" STREQUAL "cortex-r5")
    list(APPEND CMAKE_C_FLAGS       --cpu=Cortex-R5)
    list(APPEND CMAKE_ASM_FLAGS     --cpu=Cortex-R5)
    list(APPEND TOOLCHAIN_LD_FLAGS  --cpu=Cortex-R5)
  endif()
endif()
