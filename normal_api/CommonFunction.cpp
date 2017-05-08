/******************************************************************************
  File Name     : ViVo_Api.cpp
  Version       : Initial Draft
  Last Modified :
  Description   : the functions of vi and vi inplement  
  Function List :
  History       :

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/vfs.h>
#include "CommonFunction.h"
#include <sys/reboot.h>

#ifdef DEBUG_LOG
#define	ADDLOCK(m_mutex)			pthread_mutex_lock(&m_mutex)
#define	RELEASELOCK(m_mutex)		pthread_mutex_unlock(&m_mutex)

static pthread_mutex_t DebugLogMutex = PTHREAD_MUTEX_INITIALIZER;	/*报站日志锁*/

#define DEBUG_LOG_NAME		"/sd/DebugLog.txt"

FILE *DebugLogFile = NULL;
#endif

void mSleep(unsigned int  MilliSecond)
{
	struct timeval time;
	
	time.tv_sec = MilliSecond / 1000;//seconds
	time.tv_usec = MilliSecond * 1000 % 1000000;//microsecond
	
	select(0,NULL,NULL,NULL,&time);
}


int CreateNormalThreadJpeg(ThreadEntryPtrType entry, void *para, pthread_t *pid)
{
	pthread_t ThreadId;
	//pthread_attr_t attr;

	//pthread_attr_init(&attr);	
	//pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);//绑定
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//分离
	if(pthread_create(&ThreadId, NULL, entry, para) == 0)//创建线程
	{
		//pthread_attr_destroy(&attr);

		*pid = ThreadId;
		return 0;
	}

	//pthread_attr_destroy(&attr);

	return -1;
}


int CreateNormalThread(ThreadEntryPtrType entry, void *para, pthread_t *pid)
{
	pthread_t ThreadId;
	//pthread_attr_t attr;

	//pthread_attr_init(&attr);	
	//pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);//绑定
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//分离
	if(pthread_create(&ThreadId, NULL, entry, para) == 0)//创建线程
	{
		//pthread_attr_destroy(&attr);

		*pid = ThreadId;
		return 0;
	}

	//pthread_attr_destroy(&attr);

	return -1;
}

void ExitNormalThread(int ThreadHandle)
{
	pthread_join(ThreadHandle, NULL);
}

/*
//重起应用程序 
static void RestartAppliction()
{
	system("reboot");
	exit(0);
}

*/

void ShutdownSystem()
{
	fprintf(stderr, "ShutdownSystem!\n");

	system("busybox poweroff");

	sleep(2);
	reboot(0x4321fedc);

	//reboot(0xcdef0123);
	
}

 #ifdef DEBUG_LOG
 
/*****************************************************************************
函数功能:调试日志打开处理函数
输入参数:无
输出参数:无
返回参数:成功返回0
备注信息:
******************************************************************************/
int OpenDebugLog()
 {
 	
	struct statfs stbuf;
	struct stat buf;
	unsigned long total_space = 0;
	unsigned long free_space = 0;
	
	if (statfs("/sd", &stbuf) == -1) 
	{
		printf("Unable to get /sd info!\n");
		return -1;
	}
		
	total_space = (stbuf.f_bsize / 256) * (stbuf.f_blocks / 4);
	
	free_space = (stbuf.f_bsize / 256) * (stbuf.f_bavail/ 4);     

	fprintf(stderr, "sd  free_space = %lu\n", free_space);
	if((long)free_space <= 50*1024)
	{
		fprintf(stderr, "sd is not enough space! free_space = %lu\n", free_space);
		if(unlink(DEBUG_LOG_NAME) < 0)
		{
			system("rm -f "DEBUG_LOG_NAME);
		}
	}
	
	if(stat(DEBUG_LOG_NAME, &buf) == 0)
	{
		fprintf(stderr, "debug log size = %ld\n", buf.st_size);
		if(buf.st_size >= 10*1024*1024)
		{
			fprintf(stderr, "debug log is more than 10 M!\n");

			if(unlink(DEBUG_LOG_NAME) >= 0)
			{
				fprintf(stderr, "delete "DEBUG_LOG_NAME" successful!\n");
			}
			else
			{
				fprintf(stderr, "rm "DEBUG_LOG_NAME);
				if(unlink(DEBUG_LOG_NAME) < 0)
				{
					system("rm -f "DEBUG_LOG_NAME);
				}
			}
		}
	}

/*
	RepLogFile = fopen(REPORTSTATION_LOG_NAME, "a+");
	if(NULL != RepLogFile)
	{
		fprintf(stderr, "open reportstation log file %s successful!\n", REPORTSTATION_LOG_NAME);
	}
	else
	{
		fprintf(stderr, "open reportstation log file %s failed!\n", REPORTSTATION_LOG_NAME);
		return -1;
	}
*/
	return 0;
 }

 /*****************************************************************************
函数功能:写调试日志处理函数
输入参数:无
输出参数:无
返回参数:成功返回0
备注信息:
******************************************************************************/
int WriteDebugLog(char *logstr)
 {
 	
 	ADDLOCK(DebugLogMutex);
 	if(NULL == DebugLogFile)
 	{
 		DebugLogFile = fopen(DEBUG_LOG_NAME, "a+");
 	}
	
	if(NULL != DebugLogFile)
	{
		//fprintf(stderr, "open reportstation log file %s successful!\n", REPORTSTATION_LOG_NAME);
		fprintf(DebugLogFile, "%s\n", logstr);
	 	//fflush(RepLogFile);
		fclose(DebugLogFile);
		DebugLogFile = NULL;
	}
	else
	{
		fprintf(stderr, "open log file %s failed!\n", DEBUG_LOG_NAME);
		RELEASELOCK(DebugLogMutex);
		return -1;
	}

	RELEASELOCK(DebugLogMutex);
	return 0;
 }

 /*****************************************************************************
函数功能:关闭调试日志处理函数
输入参数:无
输出参数:无
返回参数:成功返回0
备注信息:
******************************************************************************/
int CloseDebugLog()
 {
  	ADDLOCK(DebugLogMutex);
 	if(NULL != DebugLogFile)
 	{
	 	//fflush(RepLogFile);
		fprintf(stderr, "close rep file = %d\n", fclose(DebugLogFile));
		DebugLogFile = NULL;
		//sync();
 	}
	RELEASELOCK(DebugLogMutex);

	return 0;
 }
 
#endif


