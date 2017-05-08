#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
//#include "netserver.h"
#include "AudioManage.h"
#include "ModuleFuncInterface.h"


typedef struct _DECODE_ARGUMENT_
{
	char  fileName[150];
	int AoDevId;		//ao输出设备ID
	int DecCh;		//解码通道
	int AoCh;		//Ao输出通道
	int startTd;		//线程运行开关
	int Runing;		//线程正在运行的标志
	pthread_t ThreadId;		//解码线程ID
}DECODE_ARG;





void *DecodeAlarmvoice(void *arg)
{


	return NULL;
}


int StopPlayAlarmVoice()
{


	return HI_SUCCESS;
}


//return :1:正在播放，0:已经停止播放
int PlayAlarmStatus()
{


	return HI_SUCCESS;
}

int StartPlayAlarmVoice(MUSICINDEX index)
{


	return HI_SUCCESS;
}






void *AudioTalkDecodeProcess(void *Arg)
{


	return NULL;
}


int StartAudioTalkDecode()
{


	return HI_SUCCESS;
}



