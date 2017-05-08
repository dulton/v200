
#include "Lock.h"

CLock::CLock()
{
	pthread_mutex_init(&m_MutexLock,NULL);//
}

CLock::~CLock()
{
	pthread_mutex_destroy(&m_MutexLock);
}

void CLock::Lock()
{
	pthread_mutex_lock(&m_MutexLock);
}

void CLock::Unlock()
{
	pthread_mutex_unlock(&m_MutexLock);	
}

