#ifndef _FEED_DOG_H_
#define _FEED_DOG_H_

#include "MyList.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "Lock.h"
#include   <sys/time.h> 

struct FDDG_TDINFO
{
	int threadID;		/*线程的ID*/
	int tdfood;		/*线程狗吃的食物数*/
	int timenum;		/*在固定时间段内的次数*/
	int hungernum;	/*某个线程饥饿的次数，常态是0*/
	int bDetect;		/*自检过没有*/
	int Interval;		/*时间间隔，一个线程正常转一圈的时间(毫秒)*/
	unsigned long StartTime;	/*线程启动时间*/
	CLock	lock;
};

class CFeedDog
{
public:
	CFeedDog();
	~CFeedDog();
	static CFeedDog	*Instance();	
	bool	Add2List(int thdId);
	bool SubList(int thdId);
	bool FeedFood();
	bool EatFood(int thdId);	
	bool Start();
	void Stop();
	bool 	Arbitration()	;//仲裁
protected:
	//
private:
	//
	unsigned long long GetCurUsecTime();
	CMyList<FDDG_TDINFO> m_tdlist;
	static CFeedDog	*m_pInstance;		
	bool 	m_tdRuning;
	int 		m_thread_ID;	
};



#endif//_FEED_DOG_H_


