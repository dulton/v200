#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <getopt.h>
#include <asm/types.h>

#include "McuCommunication.h"
#include "ModuleFuncInterface.h"

#if 1 //def HIS_DEMB
#include "Ds1337Api.h"

static const char* dev_name1="/dev/pcf8563dev";
static const char* dev_name2="/dev/ds1307dev";

static signed int hirtcfd=-1;

int DS_RTC_Open()
{
       unsigned int  value = 0x00;
	int  retval = 0xff;

	
	if(hirtcfd<=0)
	{
		if((hirtcfd=open(dev_name2,O_RDWR))<=0)/*failed*/
		{
			printf("open rtc  ds1307dev device failed!\n");
			//return -1;
		}
		else /*success*/
		{
			retval = ioctl(hirtcfd,DS1307_READ_REG,&value);
			
			printf("open rtc ds1307dev device %d !\n",value );
			
			if (value != 0xff)
			{
				printf("open rtc ds1307dev device OK  %d !\n",value );
				return 0;				 
			}
		}
		
		if((hirtcfd=open(dev_name1,O_RDWR))<=0)
		{
			printf("open rtc pcf8563dev device failed!\n");
			return -1;
		}
		printf("open %s success \n",dev_name1);
		
		
	}
	printf("rtc fd %d  %d\n",hirtcfd, __LINE__);
	return 0;
}

int DS_RTC_Close()
{

#if 1
	if(hirtcfd<=0)
	{
		printf("device close already!\n");		
	}
	close(hirtcfd);
	hirtcfd=-1;
#endif 

	return 0;

}

int DS_RTC_SetTime(rtc_time_t* time_struct)
{
	//MCUCOMM *pMcuCom = NULL;
	//datetime_setting  DateTime;
	
	if(0==time_struct)
	{
		printf("invalid parameter!\n");
		return -1;
	}
#if 1
	rtc_time_t tm;
	unsigned int tmp = 0;
	memcpy(&tm,time_struct,sizeof(rtc_time_t));
	if((tmp=ioctl(hirtcfd, HI_RTC_SET_TIME, &tm)) != 0) 
	{
		printf("set time failed!\n");
		return -1;		
	}
#else 

	DateTime.year = time_struct->year;
	DateTime.month =  time_struct->month;
	DateTime.day =  time_struct->date;
	DateTime.hour =  time_struct->hour;
	DateTime.minute =  time_struct->minute;
	DateTime.second =  time_struct->second;

	pMcuCom = MCUCOMM::Instance();

	pMcuCom->SetMcuRtc(DateTime);

#endif 

	return 0;
}

int DS_RTC_GetTime(rtc_time_t* time_struct)
{
	if(time_struct == NULL)
	{
		printf("invalid parameter!\n");
		return -1;
	}

	if((ioctl(hirtcfd, HI_RTC_RD_TIME, time_struct)) != 0) 
	{
		printf("get time failed!\n");
		return -1;		
	}
	return 0;	
}


int SYS_RTC_GetTime(rtc_time_t* time_struct)
{
	struct tm *t ;  
	time_t timer = 0;
	
	timer = time(NULL);
	t = localtime(&timer);
	if( t->tm_year > 99) // 1900 Îª»ù×¼
		time_struct->year = t->tm_year -100;
	else 
		time_struct->year = 9;

	time_struct->month = t->tm_mon + 1;
	time_struct->date = t->tm_mday;
	time_struct->hour = t->tm_hour;
	time_struct->minute = t->tm_min;
	time_struct->second = t->tm_sec;
	time_struct->weekday= CaculateWeek(time_struct->date, time_struct->month, time_struct->year);
	return 0;	
}


void TestDSRTC()
{
	DS_RTC_Open();
	rtc_time_t time_struct;
	
	while(1)
	{
		DS_RTC_GetTime(&time_struct);
		sleep(1);
		printf("year = %d,min = %d\n",time_struct.year,time_struct.minute);
		time_struct.year = 2008;
		DS_RTC_SetTime(&time_struct);
	}

	DS_RTC_Close();
}

#else

#include "Ds1337Api.h" 

static int rtcfd = -1;
const char rtc_devicename[] ="/dev/misc/ds1337";

int DS_RTC_Open(void)
{
	if(rtcfd <= 0)
	{
		if((rtcfd=open(rtc_devicename,O_RDWR))<=0)
		{
			printf("open rtc device failed!\n");
			return -1;
		}
	}	
	
	return 0;
}


int DS_RTC_Close(void)
{
	if(rtcfd<=0)
	{
		printf("device close already!\n");		
	}
	close(rtcfd);
	rtcfd=-1;

	return 0;
}

int DS_RTC_GetTime(rtc_time_t * ntime)
{
	unsigned int tmp=0;
	
	if(0==ntime)
	{
		printf("invalid parameter!\n");
		return -1;
	}
	
	if((tmp=ioctl(rtcfd, DS1337_READ, ntime)) != 0) 
	{
		printf("get time failed!\n");
		return -1;		
	}
    
	return 0;
}    

int DS_RTC_SetTime(rtc_time_t * ntime)
{
	unsigned int tmp=0;
	
	if(0==ntime)
	{
		printf("invalid parameter!\n");
		return -1;
	}
	
	if((tmp=ioctl(rtcfd, DS1337_WRITE, ntime)) != 0) 
	{
		printf("set time failed!\n");
		return -1;		
	}	

	return 0;
}

#endif


