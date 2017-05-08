#ifndef _LOCK_H_
#define _LOCK_H_

#include <pthread.h>

class CLock
{

public:
	CLock();
	void Lock();
	void Unlock();
	virtual ~CLock();
protected:
	//
private:
	pthread_mutex_t    m_MutexLock;
	pthread_mutexattr_t m_MutexAttr;
};
#endif//_LOCK_H_

