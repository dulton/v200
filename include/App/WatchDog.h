#ifndef _WDTTEST_H
#define _WDTTEST_H
#include <linux/ioctl.h>
#include <linux/types.h>
#define	WATCHDOG_IOCTL_BASE	'W'
#define	WDIOC_KEEPALIVE		_IOR(WATCHDOG_IOCTL_BASE, 5, int)

class CWatchDog
{
public:
	int m_fdDog;
	CWatchDog();
	~CWatchDog();
	static CWatchDog* Instance();
	int FeedDog();
	int Dogclose();
	int  Wdt_thread_ID;	
private:
	
	static CWatchDog *m_pInstance;
};

void FeedWatchDog();

#endif 

