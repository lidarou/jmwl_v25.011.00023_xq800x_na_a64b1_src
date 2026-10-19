# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    system_assert.c
    exception_handle.c
    assert_adpter.c
    assert_cmd.c
    assert_dump.c
    exception_parse.c
)

aic_library_sources_ifdef(CONFIG_ASSERT_TEST assert_test.c)

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/os/osa/common/inc
    ${AIC_SOURCE_ROOT}/os/osa/include
)
