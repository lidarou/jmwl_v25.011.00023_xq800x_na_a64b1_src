/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x18eaf3
#define LOG_MODULE    AMOD_AOS

#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <aic_osa_ex.h>

/**
 * strpbrk - Find the first occurrence of a set of characters
 * @cs: The string to be searched
 * @ct: The characters to search for
 */
char *strpbrk(const char * cs,const char * ct)
{
    const char *sc1,*sc2;

    for( sc1 = cs; *sc1 != '\0'; ++sc1) {
        for( sc2 = ct; *sc2 != '\0'; ++sc2) {
            if (*sc1 == *sc2)
                return (char *) sc1;
        }
    }
    return NULL;
}

/**
 * strsep - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 *
 * strsep() updates @s to point after the token, ready for the next call.
 *
 * It returns empty tokens, too, behaving exactly like the libc function
 * of that name. In fact, it was stolen from glibc2 and de-fancy-fied.
 * Same semantics, slimmer shape. ;)
 */
char *strsep(char **s, const char *ct)
{
    char *sbegin = *s, *end;

    if (sbegin == NULL)
        return NULL;

    end = strpbrk(sbegin, ct);
    if (end)
        *end++ = '\0';
    *s = end;

    return sbegin;
}

char* strndup(const char* s1, size_t n)
{
    char* copy = (char*)malloc(n + 1);
    if (copy) {
        memcpy(copy, s1, n);
        copy[n] = 0;
    }
    return copy;
}

char* strdup(const char* s1)
{
    char* copy = (char*)malloc(strlen(s1) + 1);
    if (copy) {
        memcpy(copy, s1, strlen(s1));
        copy[strlen(s1)] = 0;
    }
    return copy;
}

#ifndef CONFIG_RTOS_LITEOS_M
int strcpy_s(char *dst, size_t n, const char * src)
{
    return (strncpy(dst, src, n) ? 0 : -1);
}

int strcat_s(char *dst, size_t n, const char * src)
{
    return (strncat(dst, src, n) ? 0 : -1);
}
#endif

char *
strcasestr(const char *s, const char *find)
{
    char c, sc;
    size_t len;

    c = *find++;
    if (c != 0) {
        len = strlen(find);
        do {
            unsigned char lower1, lower2;

            do {
                sc = *s++;
                if (sc == 0) {
                    return NULL;
                }
                lower1 = tolower(sc);
                lower2 = tolower(c);
            } while (lower1 != lower2);
        } while (strncasecmp(s, find, len) != 0);
    s--;
    }
    return (char *)s;
}

#ifdef WIN32
char *strtok_r(char *str, const char *sep, char **state)
{
    char *start, *end;

    start = str ? str : *state;

    /* skip leading delimiters */
    while (*start && strchr(sep, *start)) {
        start++;
    }

    if (*start == '\0') {
        *state = start;
        return NULL;
    }

    /* look for token chars */
    end = start;
    while (*end && !strchr(sep, *end)) {
        end++;
    }

    if (*end != '\0') {
        *end = '\0';
        *state = end + 1;
    } else {
        *state = end;
    }

    return start;
}
#endif

