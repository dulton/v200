#ifndef _AUDIO_MANAGE_H_
#define _AUDIO_MANAGE_H_
#include "AVdec_Api.h"

typedef enum _EN_MUSIC_INDEX_
{
	SD_NOT_EXIST = 0,
	SD_LOAD_SUCC,
	TEST_AUDIO,
	TEST_TALKAUDIO,
	MUSIC_MAX_NUM
}MUSICINDEX;

struct MUSIC_NAME
{
	MUSICINDEX 	code;
	char *description;
};


int StartPlayAlarmVoice(MUSICINDEX index);
int StopPlayAlarmVoice();
int PlayAlarmStatus();
int StartMicTalkDecode();
int StartAudioTalkDecode();


#endif//_AUDIO_MANAGE_H_

