/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This aic auto int test file.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1cbd07
#define LOG_MODULE    AMOD_INIT

#include <aic_init.h>
#include <alog_init.h>

/* reserve the auto init test codes. */
#define AUTO_INIT_TEST 0
#if AUTO_INIT_TEST
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926
#endif

#ifndef _PID_T_DECLARED
#define _PID_T_DECLARED
typedef int pid_t;
#endif

#define test_log    alog_info
int kill (pid_t, int);
pid_t   getpid (void);

int test_cmp(const void *p1, const void *p2)
{
   return  *((u8 *)p1) - *((u8 *)p2);
}

static int minilib_test(void)
{
#if 1
    int year,month,day, num;
    char str[30];

    test_log("----test sprintf start----\n");
    snprintf(str, sizeof(str), "%s %02d, %.*s\n", "test", 2, 4, "hello!");
    test_log("----test sprintf end----\n");

    test_log("----test sscanf start----\n");

    sscanf("123456","%d",&num);
    test_log("exp: 123456, rel: %d\n",num);

    sscanf("2022.09.21","%d.%d.%d",&year,&month,&day);
    test_log("exp: 2022 09 21, rel: %d %d %d\n",year,month,day);

    sscanf("20220901000","%8s", str);
    test_log("exp: 20220901, rel: %s\n",str);

    sscanf("test 01","%[^ ]",str);
    test_log("exp: test, rel: %s\n",str);

    sscanf("test/01","%[^/]",str);
    test_log("exp: test, rel: %s\n",str);

    sscanf("123456testSIX","%[1-9a-z]",str);
    test_log("exp: 123456test, rel: %s\n",str);

    sscanf("test TEST","%*s%s",str);
    test_log("exp: TEST, rel: %s\n",str);

    sscanf("test/unique_11@qq.com","%*[^/]/%[^@]",str);
    test_log("exp: unique_11, rel: %s\n",str);

    sscanf("123456abcdedfBCDEF","%[1-9A-Z]",str);
    test_log("exp: 123456, rel: %s\n",str);

    test_log("----test sscanf end----\n");

    test_log("----test math start----\n");
    test_log("sin(30)=%f\n", sin(M_PI/6));
    test_log("asin(0.5)=%f\n", asin(0.5)*180/M_PI);
    test_log("cos(60)=%f\n", cos(M_PI/3));
    test_log("acos(0.5)=%f\n", acos(0.5)*180/M_PI);

    test_log("atan2(1)=%f\n", atan2(1, M_PI/4));
    test_log("sinh(30)=%f\n", sinh(M_PI/6));
    test_log("tanh(45)=%f\n", tanh(M_PI/4));
{
    double x = 1024, fraction, f_test;
    long long ll = 0x808080000000;
    int e;

    test_log("ll=(0x%llx, %llu)\n", ll, ll);

    test_log("exp(2)=%f\n", exp(2));
    test_log("frexp(x, e)\n");
    fraction = frexp(x, &e);
    test_log("x=%.21f = %.21f *2^%d\n", x, fraction, e);
    test_log("ldexp(3, 2)=%f\n", ldexp(3, 2));
    test_log("log(2)=%f\n", log(2));
    x=3.1415;
    fraction = modf(x, &f_test);
    test_log("modf(3.14), fractpart=%f, intpart=%d\n", x, e);
    test_log("fmod(3.14/0.3) = %f \n", fmod(3.14, 0.3));
    test_log("sqrt(2) = %f \n", sqrt(2));
    test_log("ceil(3.14) = %d \n", ceil(3.14));
    test_log("floor(3.14) = %f \n", floor(3.14));
    test_log("fabs(3.14) = %f \n", fabs(3.14));
    test_log("log10(100) = %f \n", log10(100));
    test_log("pow(3, 4) = %f \n", pow(3, 4));
    test_log("pow(3, 4) = %f \n", pow(3, 4));
    test_log("----test math end----\n");
}

{
    char str1[30], *str2;
    char arry[] = {'a', 'c', 'f', 'd', '\0'};

    test_log("----test str start\n----");
    strncpy(str1, "test1, test2, test3, test4", sizeof(str1) - 1);

    test_log("strtok %s with ','\n", str1);
    str2 = strtok(str1, ", ");
    while (str2) {
        test_log("strtok: %s\n", str2);
        str2 = strtok(NULL, ",");
    }

    test_log("qsort: %s\n", arry);
    qsort(arry, sizeof(arry), sizeof(arry[0]), test_cmp);
    test_log("after qsort: %s\n", arry);
}

{
    char *end = NULL;
    test_log("atoi 123456=%d\n", atoi("123456"));
    test_log("atol 123456=%d\n", atol("123456"));
    test_log("strtol %s ==>%l\n", "0x123456789", strtol("0x123456789", &end, 10));
    test_log("strtcoll %s cmp %s, ret=%d\n", "1234", "12345", strcoll("1234", "12345"));

    memset(str, 0, sizeof(str));
    strxfrm(str, "abcdef", 6);
    test_log("strxfrm %s ==>%s\n", "abcdef", str);
    test_log("'a' iscntrl=%d\n", iscntrl('a'));
    test_log("'\t' iscntrl=%d\n", iscntrl('\t'));
    test_log("'\b' iscntrl=%d\n", iscntrl('\b'));

    test_log("';' ispunct=%d\n", ispunct(';'));
    test_log("',' ispunct=%d\n", ispunct(','));
    test_log("'a' ispunct=%d\n", ispunct('a'));

    test_log("'a' islower=%d\n", islower('a'));
    test_log("'A' islower=%d\n", islower('A'));
    test_log("----test str end----\n");

    test_log("atof 1234.56=%f\n", atof("1234.56"));
    test_log("strtod %s ==>%f\n", "123456789", strtod("0x123456789", NULL));
    test_log("strtof %s ==>%f\n", "123456.789", strtof("123456.789", NULL));
}

{
    srand(0);
    test_log("srand(0)");
    test_log("rand1=%d, rand2=%d, rand3=%d, rand4=%d, rand5=%d\n",
             rand(), rand(), rand(), rand(), rand());

    test_log("srand(1)");
    test_log("rand1=%d, rand2=%d, rand3=%d, rand4=%d, rand5=%d\n",
             rand(), rand(), rand(), rand(), rand());

    test_log("srand(2)");
    test_log("rand1=%d, rand2=%d, rand3=%d, rand4=%d, rand5=%d\n",
             rand(), rand(), rand(), rand(), rand());
}
#endif

#if 0
{
    exit(0);
    test_log("exit(0)\n");

    abort();
    test_log("abort(0)\n");

    test_log("_getpid = %d\n", getpid());

    test_log("_getpid = %d\n", kill(getpid(), 0));
}
#endif
    return 0;
}
AIC_NORMAL_DRV_P1_INIT_EXPORT(minilib_test);
#include <setjmp.h>
jmp_buf buf_test;

static int drv_p1_test()
{
    u32 tmp = 0;
    volatile int count = 0;

    tmp = setjmp(buf_test);

    aicx_info(0x0f0f30, "du", "setjmp[%d] tmp is %u\n", count++, tmp);

    if (count < 5)
        longjmp(buf_test, 1);

    aicx_info(0x106458, "", "drv_p1_test\n");
    return 0;
}
AIC_NORMAL_DRV_P1_INIT_EXPORT(drv_p1_test);

static int drv_p2_test()
{
    aicx_info(0x099f27, "", "drv_p2_test\n");
    return 0;

}
AIC_NORMAL_DRV_P2_INIT_EXPORT(drv_p2_test);

static int comp_p1_test()
{
    aicx_info(0x092db8, "", "comp_p1_test\n");
    return 0;

}
AIC_NORMAL_COMP_P1_INIT_EXPORT(comp_p1_test);

static int comp_p2_test()
{
    aicx_info(0x0ec4c1, "", "comp_p2_test\n");
    return 0;

}
AIC_NORMAL_COMP_P2_INIT_EXPORT(comp_p2_test);

static int app_p1_test()
{
    aicx_info(0x0deebf, "", "app_p1_test\n");
    return 0;

}
AIC_NORMAL_APP_P1_INIT_EXPORT(app_p1_test);

static int app_p2_test()
{
    aicx_info(0x12e287, "", "app_p2_test\n");
    return 0;

}
AIC_NORMAL_APP_P2_INIT_EXPORT(app_p2_test);

static int drv_l1_test()
{
    aicx_info(0x0adacb, "", "drv_l1_test\n");
    return 0;
}
AIC_NORMAL_DRV_L1_INIT_EXPORT(drv_l1_test);

static int drv_l2_test()
{
    aicx_info(0x096ef8, "", "drv_l2_test\n");
    return 0;

}
AIC_NORMAL_DRV_L2_INIT_EXPORT(drv_l2_test);

static int comp_l1_test()
{
    aicx_info(0x1717da, "", "comp_l1_test\n");
    return 0;

}
AIC_NORMAL_COMP_L1_INIT_EXPORT(comp_l1_test);

static int comp_l2_test()
{
    aicx_info(0x030abd, "", "comp_l2_test\n");
    return 0;

}
AIC_COMP_L2_INIT_EXPORT(comp_l2_test);

static int app_l1_test()
{
    aicx_info(0x0bbecb, "", "app_l1_test\n");
    return 0;

}
AIC_NORMAL_APP_L1_INIT_EXPORT(app_l1_test);

static int app_l2_test()
{
    aicx_info(0x0219af, "", "app_l2_test\n");
    return 0;

}
AIC_NORMAL_APP_L2_INIT_EXPORT(app_l2_test);
#endif


