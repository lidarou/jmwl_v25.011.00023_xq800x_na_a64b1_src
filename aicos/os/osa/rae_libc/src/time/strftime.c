/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e3524
#define LOG_MODULE    AMOD_AOS

#include <time.h>
#include <rtc.h>
#include <aic_errno.h>

#define RTC_INITIAL_YR          1970/* RTC_INITIAL_YR = Jan 1 1970 00:00:00 */
#define ABB_LENGTH              3

const char *_days[] = {
            "Sunday", "Monday", "Tuesday", "Wednesday",
            "Thursday", "Friday", "Saturday"
        };

const char *_months[] = {
            "January", "February", "March",
            "April", "May", "June",
            "July", "August", "September",
            "October", "November", "December"
        };

static char *
s_prt(char *s, size_t maxsize, const char *str, int width)
{
    while(width > 0 || (width < 0 && *str)){
        if(!maxsize) break;
        *s++ = *str++;
        maxsize--;
        width--;
    }
    return s;
}

static char *
i_prt(char *s, size_t maxsize, int val, int width)
{
    int c;

    c = val % 10;
    val = val / 10;
    if (--width > 0 || (width < 0 && val != 0))
        s = i_prt(s, (maxsize ? maxsize - 1 : 0), val, width);
    if(maxsize) *s++ = c + '0';
    return s;
}

size_t strftime(char *s, size_t maxsize, const char *format, const struct tm *tmptr)
{
    size_t n;
    char *firsts, *olds;

    if(!format) return 0;

    firsts = s;
    while(maxsize && *format){
        while(maxsize && *format && *format != '%'){
            *s++ = *format++;
            maxsize--;
        }
        if(!maxsize || !*format) break;
        format++;

        olds = s;
        switch(*format++){
        case 'a':
            s = s_prt(s, maxsize, _days[tmptr->tm_wday], ABB_LENGTH);
            maxsize -= s - olds;
            break;

        case 'A':
            s = s_prt(s, maxsize, _days[tmptr->tm_wday], -1);
            maxsize -= s - olds;
            break;

        case 'b':
            s = s_prt(s, maxsize, _months[tmptr->tm_mon], ABB_LENGTH);
            maxsize -= s - olds;
            break;

        case 'B':
            s = s_prt(s, maxsize, _months[tmptr->tm_mon], -1);
            maxsize -= s - olds;
            break;
        case 'c':
            n = strftime(s, maxsize,
                    "%a %b %d %H:%M:%S %Y", tmptr);
            if (n) maxsize -= n;
            else maxsize = 0;
            s += n;
            break;
        case 'd':
            s = i_prt(s, maxsize, tmptr->tm_mday, 2);
            maxsize -= s - olds;
            break;
        case 'H':
            s = i_prt(s, maxsize, tmptr->tm_hour, 2);
            maxsize -= s - olds;
            break;
        case 'I':
            s = i_prt(s, maxsize, (tmptr->tm_hour + 11) % 12 + 1, 2);
            maxsize -= s - olds;
            break;
        case 'j':
            s = i_prt(s, maxsize, tmptr->tm_yday + 1, 3);
            maxsize -= s - olds;
            break;
        case 'm':
            s = i_prt(s, maxsize, tmptr->tm_mon + 1, 2);
            maxsize -= s - olds;
            break;
        case 'M':
            s = i_prt(s, maxsize, tmptr->tm_min, 2);
            maxsize -= s - olds;
            break;
        case 'p':
            s = s_prt(s, maxsize, (tmptr->tm_hour < 12) ? "AM" : "PM", 2);
            maxsize -= s - olds;
            break;
        case 'S':
            s = i_prt(s, maxsize, tmptr->tm_sec, 2);
            maxsize -= s - olds;
            break;
        case 'U':
            s = i_prt(s, maxsize, (tmptr->tm_yday + 7 - tmptr->tm_wday) / 7, 2);
            maxsize -= s - olds;
            break;
        case 'w':
            s = i_prt(s, maxsize, tmptr->tm_wday, 2);
            maxsize -= s - olds;
            break;
        case 'W':
            s = i_prt(s, maxsize, (tmptr->tm_yday + 7 - (tmptr->tm_wday + 6)%7) / 7, 2);
            maxsize -= s - olds;
            break;
        case 'x':
            n = strftime(s, maxsize, "%a %b %d %Y", tmptr);
            if(n)maxsize -= n;
            else maxsize = 0;
            s += n;
            break;
        case 'X':
            n = strftime(s, maxsize, "%H:%M:%S", tmptr);
            if(n)maxsize -= n;
            else maxsize = 0;
            s += n;
            break;
        case 'y':
            s = i_prt(s, maxsize, (tmptr->tm_year + TM_YEAR_BASE) % 100, 2);
            maxsize -= s - olds;
            break;
        case 'Y':
            s = i_prt(s, maxsize, (tmptr->tm_year + TM_YEAR_BASE), -1);
            maxsize -= s - olds;
            break;
        case '%':
            *s++ = '%';
            maxsize--;
            break;
        default:
            while(*format) format++;
            break;
        }
    }
    if(maxsize){
        *s = '\0';
        return s - firsts;
    }
    return 0;
}

