#ifndef __HI_RTC_API_H
#define __HI_RTC_API_H

#define HI_RTC_AIE_ON    0x01//_IO('p', 0x01)
#define HI_RTC_AIE_OFF   0x02//_IO('p', 0x02)
#define HI_RTC_ALM_SET    0x07 //_IOW('p', 0x07,  rtc_time_t)
#define HI_RTC_ALM_READ   0x08//_IOR('p', 0x08,  rtc_time_t)
//#define HI_RTC_RD_TIME   0x09//_IOR('p', 0x09,  rtc_time_t)
//#define HI_RTC_SET_TIME    0x0a//_IOW('p', 0x0a,  rtc_time_t)

typedef struct
{
        unsigned int  year;
        unsigned int  month;
        unsigned int  date;
        unsigned int  hour;
        unsigned int  minute;
        unsigned int  second;
        unsigned int  weekday;
} rtc_time_t;

int Hi_Rtc_Open();
int Hi_Rtc_Close();
int Hi_Rtc_Settime(rtc_time_t* time_struct);
int Hi_Rtc_Gettime(rtc_time_t* time_struct);
#endif
