/******************************************************************************
  File Name     : Audio.h
  Version       : 
  Last Modified :2013.01.08
  Description   : 音频编解码
  Function List :
  History       :
   Modification: Created file
  by:harvey
******************************************************************************/

#ifndef _AUDIO_H_
#define _AUDIO_H_
#include "CommonDefine.h"
#include "hi_comm_aio.h"
#include "acodec.h"
#include "Video_comm.h"
#include "hi_common.h"
#include "hi_comm_aenc.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_adec.h"
#include "mpi_ao.h"
#include "common.h"
#include "BufferManage.h"

#include "AbnormalVoice.h"


#define MAXUSERNUM     8
#define BUFFERLEN     1024*50
#define NumPerFrm		160

#define ZBAR_SCAN_FILE	  "/app/voice/zbarscan"

#define ZBAR_SCAN_ST_C    "/app/voice/chinese_connecting.wav"
#define ZBAR_SCAN_ST_E    "/app/voice/english_connecting.wav"
#define ZBAR_SCAN_ST_S    "/app/voice/spanish_connecting.wav"



#define ZBAR_SCAN_W_C     "/app/voice/chinese_wait.wav"
#define ZBAR_SCAN_W_E     "/app/voice/english_wait.wav"
#define ZBAR_SCAN_W_S     "/app/voice/spanish_wait.wav"



#define ZBAR_SCAN_SU_C    "/app/voice/chinese_success.wav"
#define ZBAR_SCAN_SU_E    "/app/voice/english_success.wav"
#define ZBAR_SCAN_SU_S    "/app/voice/spanish_success.wav"
#define VOLUMEMUSIC 	   "/app/voice/sound"

#define ACODEC_FILE     "/dev/acodec"


#define GPIO_DEV        "/dev/gpio_dev"
#define GPIO_ADUIO_AMPLIFIER _IO('p',0x9)

typedef enum AUDIO_TYPE
{
    ZBAR_SCAN,
  	WIIF_CONNECT,
  	ZBAR_WIFI_START_CH,
  	ZBAR_WIFI_START_EN,
  	ZBAR_WIFI_START_SP,
  	ZBAR_WIFI_WAIT_CH,
  	ZBAR_WIFI_WAIT_EN,
  	ZBAR_WIFI_WAIT_SP,
  	ZBAR_WIFI_SUCCESS_CH,
  	ZBAR_WIFI_SUCCESS_EN,
  	ZBAR_WIFI_SUCCESS_SP,
  	BUTT,
  	VOLUME_SOUND,
    
} PLAY_TPYE;

typedef struct
{
    bool	bStart;
	bool 	bthreadrun;
    pthread_t stAencPid;
    HI_S32  AeChn;
    HI_S32  AdChn;
    HI_BOOL bSendAdChn;
	class Audio *pAudio;

} AudioEnc_S;
typedef struct
{
	unsigned char Type;/*0 g711 , 1 g726*/
	unsigned int  datalen;
	unsigned long long timestamp;
} AudioHead;



typedef struct
{
	
	unsigned char 			samplerate; 	/*采样率 0:8K ,1:12K,  2: 11.025K, 3:16K ,4:22.050K ,5:24K ,6:32K ,7:48K*/;	
	unsigned char 			audiotype;		/*编码类型0 :g711 ,1:G726*/
	unsigned char 			enBitwidth;		/*位宽0 :8 ,1:16 2:32*/
	unsigned char			inputvolume;		/*输入音量0 --31 */
	unsigned char 			outputvolume;		/*输出音量0 --31*/
	unsigned short			framelen ;		//音频帧大小(80/160/240/320/480/1024/2048)
	char 					reserved[9];		/*保留9，一共16个字节*/
} AudioParm;




class Audio
{
	private:
			AIO_ATTR_S m_stAioAttr;
			AudioEnc_S m_AudioEnc;
			bool	   m_DecodeState;/*0:解码未开始 1:解码正在进行*/
			pthread_mutex_t m_AudioLock;

			unsigned char m_inputvolume;
			unsigned char m_outputvolume;

			
	public:
		Audio();
		~Audio();

		int GetAudioParam(AudioParm *param);
		/*****************************************************************************
		函数名称:InitAudio
		函数功能:初始化音频，主要是设置声道声音，增益等
		输入参数:无
		输出参数:无
		返  回   值:0:成功  -1:失败
		使用说明: 外部调用，在音频编解码前需要调用
		by :harvey
		******************************************************************************/
		int 	InitAudio();

		bool	InitAbnormalVoice();
		void 	AbnormalVoiceProcess(char *buf,int len);
		/*****************************************************************************
		函数名称:StartAudioEncode
		函数功能:开启音频编码，单声道 g711编码
		输入参数:无
		输出参数:无
		返	回	 值:0:成功	-1:失败
		使用说明: 外部调用?		by :harvey
		******************************************************************************/
		int StartAudioEncode(int Audiotype);
		
		/*****************************************************************************
		函数名称:StopAudioEncode
		函数功能:关闭音频编码，
		输入参数:无
		输出参数:无
		返	回	 值:0:成功	-1:失败
		使用说明: 外部调用，
		by :harvey
		******************************************************************************/
		int StopAudioEncode();
		
		/*****************************************************************************
		函数名称:StartAudioDecode
		函数功能:开启音频解码，
		输入参数:无
		输出参数:无
		返	回	 值:0:成功	-1:失败
		使用说明: 外部调用，调用SendAudioStreamToDecode 前需要先开启解码
		by :harvey
		******************************************************************************/
		int StartAudioDecode();
		
		/*****************************************************************************
		函数名称:StopAudioDecode
		函数功能:停止音频解码，
		输入参数:无
		输出参数:无
		返	回	 值:0:成功	-1:失败
		使用说明: 外部调用，StopAudioDecode需要再开启解码后才可以调用
		by :harvey
		******************************************************************************/
		int StopAudioDecode();
		int StartPlayFile(PLAY_TPYE  Type,void *other);
		/*****************************************************************************
		函数名称:SetInPutVolume
		函数功能:设置输入音量
		输入参数:vol (0~31)       15为默认声音vol 越大声音越大
		输出参数:无
		返  回   值:0:成功  -1:失败
		使用说明: 外部调用，
		by :harvey
		******************************************************************************/
		int SetInPutVolume(unsigned int vol);
		
		/*****************************************************************************
		函数名称:SetOutPutVolume
		函数功能:设置输出音量
		输入参数:vol (0~31)       25为默认声音vol 越大声音越大
		输出参数:无
		返  回   值:0:成功  -1:失败
		使用说明: 外部调用，
		by :harvey
		******************************************************************************/
		int SetOutPutVolume(unsigned int vol);
		
		/*****************************************************************************
		函数名称:SendAudioStreamToDecode
		函数功能:向解码器发送音频数据流
		输入参数:无
		输出参数:无
		返	回	 值:0:成功	-1:失败
		使用说明: 外部调用再此函数调用前需要开启音频解码
		by :harvey
		******************************************************************************/
		int SendAudioStreamToDecode(unsigned char *buffer,int len,int block=0);
		
		/*****************************************************************************
		函数名称:AudioEncodeThreadProcess
		函数功能:编码处理线程
		输入参数:parg
		输出参数:无
		返	回	 值:无
		使用说明: 
		by :harvey
		******************************************************************************/
		void AudioEncodeThreadProcess(void *parg);
		void PalyFileThreadProcess(void *parg);


        int AudioOutPutOnOff(unsigned int flag);

		bool AudioGetSpeeker();
		bool AudioReleaseSpeeker();
		void AudioInput(bool cmd);
		int GetAudioAlarm();
		void SetAudioAlarmValue(int value);
		int AudioAlarmSet(int alarm);
	private:
		
		/*****************************************************************************
		函数名称:StartAudioAI
		函数功能:开启输入，单通道chid  --1
		输入参数:无
		输出参数:无
		返	回	 值:无
		使用说明: 内部调用
		by :harvey
		******************************************************************************/
		int StartAudioAI();
		
		/*****************************************************************************
		函数名称:StartAudioAO
		函数功能:开启输出，双声道
		输入参数:无
		输出参数:无
		返	回	 值:无
		使用说明: 内部调用
		by :harvey
		******************************************************************************/
		int StartAudioAO();


	
		
		int AudioExit();
		bool	m_audioinput;
		bool	m_speekfree;
		bool	m_PlayFile;
		bool	m_initAbnormalVoice;
        unsigned int    m_TalkDelay;
		PLAY_TPYE		m_PlayType;
		pthread_mutex_t m_speeker;
		int		m_Audioalarm;

};


#endif

