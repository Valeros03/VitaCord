#ifndef _PSP2_RTC_H_
#define _PSP2_RTC_H_
typedef struct SceDateTime {
    unsigned short year;
    unsigned short month;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
    unsigned int microsecond;
} SceDateTime;

#ifdef __cplusplus
extern "C" {
#endif
int sceRtcGetCurrentClockLocalTime(SceDateTime *time);
#ifdef __cplusplus
}
#endif
#endif
