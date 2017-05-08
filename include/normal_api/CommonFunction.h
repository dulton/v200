/******************************************************************************
  File Name     : ViVo_Api.cpp
  Version       : Initial Draft
  Last Modified :
  Description   : the functions of vi and vi inplement  
  Function List :
  History       :

******************************************************************************/
#ifndef _COMMON_FUNCTION_H_
#define _COMMON_FUNCTION_H_

typedef	void *(*ThreadEntryPtrType)(void *);

#define ADD_THREAD_LOCK(mutex)			pthread_mutex_lock(&mutex)	
#define FREE_THREAD_LOCK(mutex)			pthread_mutex_unlock(&mutex)
	
void mSleep(unsigned int  MilliSecond);

int CreateNormalThreadJpeg(ThreadEntryPtrType entry, void *para, pthread_t *pid);

int CreateNormalThread(ThreadEntryPtrType entry, void *para, pthread_t *pid);

void ExitNormalThread(int ThreadHandle);

void ShutdownSystem();

//#define DEBUG_LOG
 #ifdef DEBUG_LOG
/*****************************************************************************
函数功能:调试日志打开处理函数
输入参数:无
输出参数:无
返回参数:成功返回0
备注信息:
******************************************************************************/
int OpenDebugLog();

 /*****************************************************************************
函数功能:写调试日志处理函数
输入参数:无
输出参数:无
返回参数:成功返回0
备注信息:
******************************************************************************/
int WriteDebugLog(char *logstr);

 /*****************************************************************************
函数功能:关闭调试日志处理函数
输入参数:无
输出参数:无
返回参数:成功返回0
备注信息:
******************************************************************************/
int CloseDebugLog();
 #else
 #define OpenDebugLog()	
 #define WriteDebugLog(logstr)
 #define CloseDebugLog()
#endif
#endif
