#ifndef _REAL_TIME_H_
#define _REAL_TIME_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "Ds1337Api.h"

class CRealTime
{
public:
	static CRealTime* Instance();
	CRealTime();
	~CRealTime();
	bool Start();
	bool Stop();
	static rtc_time_t* GetCurRTCtime();
	bool SetRTCtime(rtc_time_t *pTime);
private:
	//
	static void* CreateGetTimeTd(void*);
	void DestroyTimeTd();
	pthread_t m_rtcId;
	static bool m_bBegin;
	static rtc_time_t	m_rtcTime;
	static CRealTime* m_pInstance;	
};

#endif//_REAL_TIME_H_

