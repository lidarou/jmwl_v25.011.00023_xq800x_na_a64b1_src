#ifndef __RTC_NV_H__
#define __RTC_NV_H__

#pragma pack(push,1)

typedef struct _RTC_NV{
    int year;    /* year */
    int month;    /* month */
    int wday;    /* week day */
    int day;    /* day */
    int hour;    /* hour */
    int minute;    /* minute */
    int second;    /* second */
}RTC_NV;


#define RTC_NV_STRUCT_SIZE    28

#pragma pack(pop)

#endif /* __RTC_NV_H__ */
