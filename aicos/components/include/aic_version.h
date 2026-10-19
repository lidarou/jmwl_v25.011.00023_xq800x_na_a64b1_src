/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC version configurations.
 */

#ifndef __AOS_VERSION_H__
#define __AOS_VERSION_H__

#include <aic_type.h>
#if !defined(WIN32)
#include <sprintf_nano.h>
#endif

#define MAX_VERSION_LEN        30
#define MAX_PROJECT_LEN        30
#define MAX_BUILDTYPE_LEN      25
#define MAX_BUILDNUM_LEN       20
#define MAX_BRANCH_LEN         40
#define COMPLILE_TIME_LEN      40

#ifdef CONFIG_SUBSYS_CP
#define AIC_VERSION_LEN (MAX_VERSION_LEN + \
                         MAX_PROJECT_LEN + \
                         MAX_BRANCH_LEN + \
                         COMPLILE_TIME_LEN)
#else
#define AIC_VERSION_LEN (MAX_VERSION_LEN + \
                         MAX_PROJECT_LEN + \
                         MAX_BUILDTYPE_LEN + \
                         MAX_BUILDNUM_LEN + \
                         MAX_BRANCH_LEN + \
                         COMPLILE_TIME_LEN)
#endif

/*
 * The sw version like "v21.001_00001".
 * The whole vesion info like below:
 *
 * Version:
 *    v21_001_00001
 * Project:
 *    gx318l_h4
 * Build type:
 *    debug
 * Build number:
 *    J569
 * Branch:
 *    master
 * Time:
 *    Oct 26 2022 08:00:45
 */
#define AIC_SW_VERSION      "v25.011"
#define AIC_SW_BRANCH       "jmwl_v24.009"

#ifdef CONFIG_SUBSYS_CP
#define AIC_VERSION_FMT     \
    "CP version:" \
    "  %s\n" \
    "Project:" \
    "  %s\n" \
    "Branch:" \
    "  %s\n" \
    "Build time:" \
    "  %s %s\n"
#else
#define AIC_VERSION_FMT     \
    "Version:" \
    "  %s.%s\n" \
    "Project:" \
    "  %s\n" \
    "Build type:" \
    "  %s\n" \
    "Build num:" \
    "  %s\n" \
    "Branch:" \
    "  %s\n" \
    "Build time:" \
    "  %s %s\n"
#endif

struct aic_version {
    char *version;
    char *branch;
    char *board;
    char *date;
    char *time;
};

void aic_get_version(char *buf, size_t size);

#endif /* __AOS_VERSION_H__ */

