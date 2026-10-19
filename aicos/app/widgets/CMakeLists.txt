# SPDX-License-Identifier: Apache-2.0

aic_library_include_directories(
	.
)

file(GLOB SOURCES ./*.c)
aic_library_sources_ifdef(CONFIG_APP_ENTRY_IDLE_FP ${SOURCES})
