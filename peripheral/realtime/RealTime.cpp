#include <unistd.h>
#include <string.h>

#include "RealTime.h"
#include "assert.h"

//#include "TdFeedDog.h"
//#include "GpioManage.h"

//#include "GpioManage.h"
extern int startupdate;
rtc_time_t CRealTime::m_rtcTime = {0};
bool CRealTime::m_bBegin = false;

CRealTime* CRealTime::m_pInstance = NULL;

CRealTime* CRealTime::Instance()
{
	if(m_pInstance == NULL)
	{
		m_pInstance = new  CRealTime();
		if(DS_RTC_Open() < 0)
		{
			printf("Open misc rtc file failure!\n");
		}
	}
	
	return m_pInstance;
}

CRealTime::CRealTime()
{
	m_bBegin = false;
}

CRealTime::~CRealTime()
{
	m_bBegin = false;
	DS_RTC_Close();
}

bool CRealTime::Start()
{	
	m_bBegin = true;
	if(pthread_create(&m_rtcId, NULL,CreateGetTimeTd , (void *)this) == 0)//创建线程
	{
		printf("Create Real Time thread successful!\n");
		return 0;
	}
	return -1;
}

bool CRealTime::Stop()
{
	m_bBegin = false;
	DestroyTimeTd();
	return false;
}

rtc_time_t* CRealTime::GetCurRTCtime()
{
	return &m_rtcTime;
}

bool CRealTime::SetRTCtime(rtc_time_t * pTime)
{
	printf("set rtc time   \n");
	assert(pTime != NULL);
	DS_RTC_SetTime(pTime);   
	return true;
}

void* CRealTime::CreateGetTimeTd(void*)
{	
	rtc_time_t rtm;

	fprintf(stderr, "******rtc thread pid = %d\n", (unsigned)pthread_self());
	
	while(m_bBegin)
	{
		if(startupdate == 0)
		{
			printf("system update thread exit file:%s line:%d pid:%d\n",__FILE__,__LINE__,(unsigned)pthread_self());
			pthread_exit(0);
		}
		SYS_RTC_GetTime(&rtm);			
		memcpy(&m_rtcTime,&rtm,sizeof(rtc_time_t));
		//printf("year = %d,mon = %d,date = %d,hour = %d,min = %d,sec = %d week %d \n",
		//	rtm.year,rtm.month,rtm.date,rtm.hour,rtm.minute,rtm.second, rtm.weekday);
		usleep(400000);	 // 400ms 	
	}

	return 0;	
}

void CRealTime::DestroyTimeTd()
{
	pthread_join(m_rtcId,NULL);
}


