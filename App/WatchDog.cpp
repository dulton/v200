
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "WatchDog.h"

#define WatchDogDEVNAME		"/dev/watchdog"

CWatchDog* CWatchDog::m_pInstance = NULL;

CWatchDog::CWatchDog():m_fdDog(-1)
{
	m_fdDog = open(WatchDogDEVNAME, O_RDWR);
	if (m_fdDog < 0)
	{
		printf("open dog err, %s, %d\n", __FILE__, __LINE__);
	}
}

CWatchDog::~CWatchDog()
{
	if (m_fdDog > 0)
	{
		close(m_fdDog);
	}
}

CWatchDog* CWatchDog::Instance()
{
	if (!m_pInstance)
	{
		m_pInstance = new CWatchDog;
	}
	
	return m_pInstance;
}

int CWatchDog::FeedDog()
{
	unsigned long ulTm = 3;
	if (ioctl(m_fdDog, WDIOC_KEEPALIVE, &ulTm) < 0)
	{
		//printf("WDIOC_KEEPALIVE err, %s, %d\n", __FILE__, __LINE__);
		return -1;
	}
	//printf("--------------------FeedDog\n");
	return 0;
}

int CWatchDog::Dogclose()
{	
	FeedDog();
	if(m_fdDog >=0)
	{
		
		close(m_fdDog);
		m_fdDog = -1;
		printf("Dogclose success m_fdDog:%d!!!!!\n",m_fdDog);
	}
	sleep(1);
	//pthread_join((pthread_t)Wdt_thread_ID, NULL);
	return 0;
}
void FeedWatchDog()
{
	CWatchDog  *pWDT = CWatchDog::Instance();
	pWDT->FeedDog();
	
}


