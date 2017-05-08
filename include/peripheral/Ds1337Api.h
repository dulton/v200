#ifndef _DS_1337_API_H_
#define _DS_1337_API_H_


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

#if 1 //def HIS_DEMB
#define   DS1307_READ_REG    _IOR('p', 0x01,rtc_time_t)
#define HI_RTC_RD_TIME   _IOR('p', 0x09,  rtc_time_t)
#define HI_RTC_SET_TIME  _IOW('p', 0x0a,  rtc_time_t)

int  DS_RTC_Open(void);
int  DS_RTC_Close(void);
void TestDSRTC();

int 	DS_RTC_GetTime(rtc_time_t *ntime);
int 	DS_RTC_SetTime(rtc_time_t *ntime);
int 	SYS_RTC_GetTime(rtc_time_t* time_struct);
#else
#define DS1337_WRITE   	0x12//_IOW('p', 0x12,  rtc_time_t) 
#define DS1337_READ  	0x11//_IOR('p', 0x11,  rtc_time_t) 

#if __cplusplus
extern "C"{
#endif

int  DS_RTC_Open(void);
int  DS_RTC_Close(void);

int 	DS_RTC_GetTime(rtc_time_t *ntime);
int 	DS_RTC_SetTime(rtc_time_t *ntime);

#if __cplusplus
}
#endif

#endif


#endif//_DS_1337_API_H_

