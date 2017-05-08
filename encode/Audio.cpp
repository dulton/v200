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
#include <dlfcn.h>		

extern int startupdate;

#include "Audio.h"
#include "Video_MD.h"
#include "ModuleFuncInterface.h"

#define AUDIO_TIME
extern BufferManage 		*pBufferManage[MAX_REC_CHANNEL];
extern volatile int md_continue;
ZMD_AbHandle m_en_handle =NULL;
int (*dl_ZMD_AbnormalVoice_G711_process)(ZMD_AbHandle handle,short* tmpBuffer,int bufferLen,double *db_ret);

void *PlayFileThead(void *parg)
{
	printf("function: %s threadid %d  ,line:%d\n",__FUNCTION__, (unsigned)pthread_self(),__LINE__);
	Audio *ptr = (Audio *)parg;	
	ptr->PalyFileThreadProcess(NULL);
	return NULL;

}
void *AudioEncodecThread(void *parg)
{
	printf("function: %s threadid %d  ,line:%d\n",__FUNCTION__, (unsigned)pthread_self(),__LINE__);
	AudioEnc_S *ptr = (AudioEnc_S*)parg;
	ptr->pAudio->AudioEncodeThreadProcess(parg);
	return NULL;
}

Audio::Audio()
{
	memset(&m_stAioAttr,0x0,sizeof(AIO_ATTR_S));
	memset(&m_AudioEnc,0x0,sizeof(AudioEnc_S));
	m_DecodeState = 0;

	
    m_stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    m_stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    m_stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    m_stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    m_stAioAttr.u32EXFlag      = 0;
    m_stAioAttr.u32FrmNum      = 30;
    m_stAioAttr.u32PtNumPerFrm = NumPerFrm;
    m_stAioAttr.u32ChnCnt      = 1;
    m_stAioAttr.u32ClkSel      = 0;


	
	
	m_audioinput = true;
	m_inputvolume = 31;
	m_outputvolume = 20;
	m_speekfree = true;
	m_PlayFile = false;
	m_initAbnormalVoice =false;
	m_PlayType = BUTT;
    m_TalkDelay =0;
	m_Audioalarm = -1;
	pthread_mutex_init(&m_speeker, NULL);

		
}
Audio::~Audio()
{

}
/*****************************************************************************
函数名称:InitAudio
函数功能:初始化音频，主要是设置声道声音，增益等
输入参数:无
输出参数:无
返  回   值:0:成功  -1:失败
使用说明: 外部调用，在音频编解码前需要调用

******************************************************************************/

int Audio::InitAudio()
{
	HI_S32 fdAcodec = -1;
	unsigned int i2s_fs_sel = 0;		
	AUDIO_SAMPLE_RATE_E enSample = AUDIO_SAMPLE_RATE_8000;		
	AudioOutPutOnOff(0);
	fdAcodec = open(ACODEC_FILE,O_RDWR);
	printf("%s: InitAudioInitAudio,%s\n", __FUNCTION__, ACODEC_FILE);
	if (fdAcodec < 0)
	{
		printf("%s: can't open acodec,%s\n", __FUNCTION__, ACODEC_FILE);
		close(fdAcodec);
		return HI_FAILURE;	   
	}
	
	if(ioctl(fdAcodec, ACODEC_SOFT_RESET_CTRL))
	{
		printf("Reset audio codec error\n");
		close(fdAcodec);
		return HI_FAILURE;
	}

	if ((AUDIO_SAMPLE_RATE_8000 == enSample)
		|| (AUDIO_SAMPLE_RATE_11025 == enSample)
		|| (AUDIO_SAMPLE_RATE_12000 == enSample))
	{
		i2s_fs_sel = 0x18;
	}
	else if ((AUDIO_SAMPLE_RATE_16000 == enSample)
		|| (AUDIO_SAMPLE_RATE_22050 == enSample)
		|| (AUDIO_SAMPLE_RATE_24000 == enSample))
	{
		i2s_fs_sel = 0x19;
	}
	else if ((AUDIO_SAMPLE_RATE_32000 == enSample)
		|| (AUDIO_SAMPLE_RATE_44100 == enSample)
		|| (AUDIO_SAMPLE_RATE_48000 == enSample))
	{
		i2s_fs_sel = 0x1a;
	}
	else 
	{
		printf("%s: not support enSample:%d\n", __FUNCTION__, enSample);
		close(fdAcodec);
		return HI_FAILURE;
	}
	
	if (ioctl(fdAcodec, ACODEC_SET_I2S1_FS, &i2s_fs_sel))
	{
		printf("%s: set acodec sample rate failed\n", __FUNCTION__);
		close(fdAcodec);
		return HI_FAILURE;
	}

	//select IN or IN_Difference
	ACODEC_MIXER_E input_mode = ACODEC_MIXER_IN;
	
	if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &input_mode)) 
	{
		printf("%s: select acodec input_mode failed\n", __FUNCTION__);
		
	}

	close(fdAcodec); 
	// 暂时屏蔽异常声音检测
	//m_initAbnormalVoice = InitAbnormalVoice();
	printf("=============== m_initAbnormalVoice:%d\n",m_initAbnormalVoice);
	return HI_SUCCESS;
}
bool	Audio::InitAbnormalVoice()
{
/*********************************************************/
	

	char *error;
	void *handle;
	void *(*dl_ZMD_AbnormalVoice_init)();

	handle = dlopen ("/app/voice/libAbnormalVoice.so", RTLD_LAZY|RTLD_GLOBAL);
	if (!handle)	
	{		 
		fprintf (stderr, "%s\n", dlerror());		
		return false;			  
	}

	dl_ZMD_AbnormalVoice_init = (void *(*)())dlsym(handle, "ZMD_AbnormalVoice_init");	  
	if ((error = dlerror()) != NULL) 
	{		 
		fprintf (stderr, " %s\n", error);		 
		return false;	  
	}
	m_en_handle =(void*)(dl_ZMD_AbnormalVoice_init());


	dl_ZMD_AbnormalVoice_G711_process = (int (*)(ZMD_AbHandle handle,short* tmpBuffer,int bufferLen,double *db_ret))dlsym(handle, "ZMD_AbnormalVoice_G711_process");	  
	if ((error = dlerror()) != NULL) 
	{		 
		fprintf (stderr, " %s\n", error);		 
		return false;	  
	}
	return true;
	/*********************************************************/	
}
/*****************************************************************************
函数名称:SetInPutVolume
函数功能:设置输入音量
输入参数:vol (0~31)       15为默认声音vol 越大声音越大
输出参数:无
返  回   值:0:成功  -1:失败
使用说明: 外部调用，
by :harvey

******************************************************************************/

int Audio::SetInPutVolume(unsigned int vol)
{
	return 0;
	m_inputvolume = vol;
	HI_S32 fdAcodec = -1;
	unsigned int volume = 0;
	if(vol >31)
	{
		vol = 31;
	}
	volume = vol;
	fdAcodec = open(ACODEC_FILE,O_RDWR);
	if(fdAcodec <0 )
	{
		close(fdAcodec);
		return -1;
	}
	if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICL, &volume))
	{
		//printf("%s: set acodec micin volume failed\n", __FUNCTION__);
		close(fdAcodec);
		return -1;
	}
	if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICR, &volume))
	{
		//printf("%s: set acodec micin volume failed\n", __FUNCTION__);
		close(fdAcodec);
		return -1;
	}
	printf("SetInPutVolume :%d\n",vol);

	close(fdAcodec);
	return 0;

}
/*****************************************************************************
函数名称:SetOutPutVolume
函数功能:设置输出音量
输入参数:vol (0~31) 	  25为默认声音vol 越大声音越大
输出参数:无
返	回	 值:0:成功	-1:失败
使用说明: 外部调用，

******************************************************************************/

int Audio::SetOutPutVolume(unsigned int vol)
{
	//return 0;
	HI_S32 fdAcodec = -1;
	int iVol=6;
	ACODEC_VOL_CTRL VolCtl;
	memset(&VolCtl,0x0,sizeof(ACODEC_VOL_CTRL));
	m_outputvolume = vol;
	if(vol > 31)
	{
		vol = 31;
	}	
	VolCtl.vol_ctrl = 31-vol;
	//VolCtl.vol_ctrl=10;
	printf("vol_ctrl=%d\n",VolCtl.vol_ctrl);
	VolCtl.vol_ctrl_mute =0;
	fdAcodec = open(ACODEC_FILE,O_RDWR);
	if(fdAcodec <0 )
	{
		printf("%s: SetOutPutVolume can't open acodec,%s\n", __FUNCTION__, ACODEC_FILE);
		close(fdAcodec);
		return -1;
	}	
	/*
	if (ioctl(fdAcodec, ACODEC_SET_OUTPUT_VOL, &iVol))
	{
		printf("%s: set acodec micin volume failed4\n", __FUNCTION__);
		close(fdAcodec);
		return HI_FAILURE;
	}
	*/
	if (ioctl(fdAcodec, ACODEC_SET_DACL_VOL, &VolCtl))
	{
		printf("%s: set acodec micin volume failed4\n", __FUNCTION__);
		close(fdAcodec);
		return HI_FAILURE;
	}
	
	if (ioctl(fdAcodec, ACODEC_SET_DACR_VOL, &VolCtl))
	{
		printf("%s: set acodec micin volume failed4\n", __FUNCTION__);
		close(fdAcodec);
		return HI_FAILURE;
	}
	printf("SetOutPutVolume :%d\n",vol);

	close(fdAcodec);
	return 0;

}
/*****************************************************************************
函数名称:StartAudioAI
函数功能:开启输入，单通道chid  --1
输入参数:无
输出参数:无
返	回	 值:无
使用说明: 内部调用
by :harvey

******************************************************************************/

int Audio::StartAudioAI()
{
	
	HI_S32  s32Ret;
	AUDIO_DEV AiDevId = 0;	
	AI_CHN AiChn = 0;
	s32Ret = HI_MPI_AI_SetPubAttr(AiDevId, &m_stAioAttr);
	if (s32Ret)
	{
		printf("%s: HI_MPI_AI_SetPubAttr(%d) failed with %#x\n", __FUNCTION__, AiDevId, s32Ret);
		return HI_FAILURE;
	}
	if (HI_MPI_AI_Enable(AiDevId))
	{
		printf("%s: HI_MPI_AI_Enable(%d) failed with %#x\n", __FUNCTION__, AiDevId, s32Ret);
		return HI_FAILURE;
	}				 

	if (HI_MPI_AI_EnableChn(AiDevId, AiChn))
	{
		printf("%s: HI_MPI_AI_EnableChn(%d,%d) failed with %#x\n", __FUNCTION__,\
				AiDevId, AiChn, s32Ret);
		return HI_FAILURE;
	}
	HI_MPI_AI_EnableChn(AiDevId, 1);
	return HI_SUCCESS;
}
/*****************************************************************************
函数名称:StartAudioAO
函数功能:开启输出，双声道
输入参数:无
输出参数:无
返	回	 值:无
使用说明: 内部调用

******************************************************************************/

int Audio::StartAudioAO()
{
	
	HI_S32 s32Ret;
	AUDIO_DEV AoDevId =0;
	AO_CHN AoChn=0;
	ADEC_CHN AdChn=0 ;
	AIO_ATTR_S AioAttr;
	memset(&AioAttr,0x0,sizeof(AIO_ATTR_S));
	memcpy(&AioAttr,&m_stAioAttr,sizeof(AIO_ATTR_S));
	AioAttr.u32PtNumPerFrm = NumPerFrm;
	AioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	
//	AIO_ATTR_S *pstAioAttr = &AioAttr;
//	AIO_ATTR_S *pstAioAttr = &m_stAioAttr;
	s32Ret = HI_MPI_AO_SetPubAttr(AoDevId, &AioAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("%s: HI_MPI_AO_SetPubAttr(%d) failed with %#x!\n", __FUNCTION__, \
			   AoDevId,s32Ret);
		return HI_FAILURE;
	}
	s32Ret = HI_MPI_AO_Enable(AoDevId);
	if(HI_SUCCESS != s32Ret)
	{
		printf("%s: HI_MPI_AO_Enable(%d) failed with %#x!\n", __FUNCTION__, \
			   AoDevId, s32Ret);
		return HI_FAILURE;
	}
    MPP_CHN_S stSrcChn,stDestChn;
	AdChn = 0;
	for(unsigned int i=0;i< 1;i++)
	{
		AoChn = i;		
		stSrcChn.enModId = HI_ID_ADEC;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = AdChn;
		stDestChn.enModId = HI_ID_AO;
		stDestChn.s32DevId = AoDevId;
		stDestChn.s32ChnId = AoChn;  
		s32Ret = HI_MPI_AO_EnableChn(AoDevId, AoChn);
		if(HI_SUCCESS != s32Ret)
		{
			printf("%s: HI_MPI_AO_EnableChn(%d) failed with %#x!\n", __FUNCTION__,\
				   AoChn, s32Ret);
			return HI_FAILURE;
		}
		s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn); 
		if(HI_SUCCESS != s32Ret)
		{
			printf("line %d: HI_MPI_SYS_Bind(%d) failed with %#x!\n", __LINE__,\
				   AoChn, s32Ret);
			return HI_FAILURE;
		}
	}
	


	return HI_SUCCESS;
}


/*****************************************************************************
函数名称:StartAudioEncode
函数功能:开启音频编码，单声道 g711编码
输入参数:无
输出参数:无
返	回	 值:0:成功	-1:失败
使用说明: 外部调用，
by :harvey
******************************************************************************/

int Audio::StartAudioEncode(int Audiotype)
{
	
	HI_S32 s32Ret=0;
	AENC_CHN_ATTR_S stAencAttr;
	AENC_ATTR_G711_S stAencG711;	
	memset(&stAencG711,0x0,sizeof(AENC_ATTR_G711_S));
	memset(&stAencAttr,0x0,sizeof(AENC_CHN_ATTR_S));
	
	AI_CHN AiChn =0;
	AENC_CHN AeChn=0;	
	/* set AENC chn attr */

	stAencAttr.enType = PT_G711A;	
	stAencAttr.u32BufSize = 30;		
	stAencAttr.u32PtNumPerFrm = NumPerFrm;
	stAencAttr.pValue		= &stAencG711;		
	
	if(HI_SUCCESS != StartAudioAI())
	{
		printf("%s: StartAudioAI(%d) failed with %#x!\n", __FUNCTION__,
			   AeChn, s32Ret);
		return HI_FAILURE;	
	}
	
	/* create aenc chn*/
	s32Ret = HI_MPI_AENC_CreateChn(AeChn, &stAencAttr);
	if (s32Ret != HI_SUCCESS)
	{
		printf("%d: HI_MPI_AENC_CreateChn(%d) failed with %#x!\n", __LINE__, AeChn, s32Ret);
		return HI_FAILURE;
	}		 
    MPP_CHN_S stSrcChn,stDestChn;
    stSrcChn.enModId = HI_ID_AI;
    stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = AiChn;
    stDestChn.enModId = HI_ID_AENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = AeChn;    
    s32Ret =HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		printf("%s: HI_MPI_SYS_Bindn(%d) failed with %#x!\n", __FUNCTION__,
			   AeChn, s32Ret);
		return HI_FAILURE;
	}

	printf("Ai(%d,%d) bind to AencChn:%d ok!\n",0 , AiChn, AeChn);


	m_AudioEnc.pAudio = this;
	m_AudioEnc.AdChn = 0;
	m_AudioEnc.AeChn =0;
	m_AudioEnc.bSendAdChn =HI_TRUE;
	m_AudioEnc.bStart =true;
	
	/* 
	s32Ret = HI_MPI_AI_EnableAec(0, 0,0,0); 
	if(HI_SUCCESS != s32Ret) 
	{ 
	 	 printf("enable aec err:0x%x\n", s32Ret); 	 
	}
	*/

	
	pthread_create(&(m_AudioEnc.stAencPid ), NULL, AudioEncodecThread, &m_AudioEnc);

	pthread_t PlayFilepid;
	pthread_create(&PlayFilepid, NULL, PlayFileThead,this);
	StartAudioDecode();
	return HI_SUCCESS;
}
/*****************************************************************************
函数名称:StopAudioEncode
函数功能:关闭音频编码，
输入参数:无
输出参数:无
返	回	 值:0:成功	-1:失败
使用说明: 外部调用，与StartAudioEncode成对使用

******************************************************************************/

int Audio::StopAudioEncode()
{
	HI_S32 s32Ret;
	AUDIO_DEV AiDev =0;
	AI_CHN AiChn =0;
	AENC_CHN AeChn = 0;
    m_AudioEnc.bStart = false;
	if(m_AudioEnc.bthreadrun == true)
	{
		 pthread_join(m_AudioEnc.stAencPid, NULL);
		 sleep(1);
		 m_AudioEnc.bthreadrun = false;
		 
	}
    MPP_CHN_S stSrcChn,stDestChn;

    stSrcChn.enModId = HI_ID_AI;
    stSrcChn.s32DevId = AiDev;
    stSrcChn.s32ChnId = AiChn;
    stDestChn.enModId = HI_ID_AENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = AeChn;    
      

    s32Ret =HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		printf("%s: HI_MPI_SYS_UnBind(%d) failed with %#x!\n", __FUNCTION__,
			   AeChn, s32Ret);
		//return HI_FAILURE;
	}	
	HI_MPI_AENC_DestroyChn(AeChn);
	if (s32Ret != HI_SUCCESS)
	{
		printf("%s: HI_MPI_AENC_DestroyChn(%d) failed with %#x!\n", __FUNCTION__,
			   AeChn, s32Ret);
		//return HI_FAILURE;
	}
    HI_MPI_AI_DisableChn(AiDev, AiChn); 
	if (s32Ret != HI_SUCCESS)
	{
		printf("%s: HI_MPI_AI_DisableChn(%d) failed with %#x!\n", __FUNCTION__,
			   AiChn, s32Ret);
		//return HI_FAILURE;
	}
    HI_MPI_AI_Disable(AiDev);
	if (s32Ret != HI_SUCCESS)
	{
		printf("%s: HI_MPI_AI_Disable(%d) failed with %#x!\n", __FUNCTION__,
			   AiDev, s32Ret);
		//return HI_FAILURE;
	}

	return HI_SUCCESS;
}
int Audio::GetAudioParam(AudioParm *param)
{
	param->samplerate = 0;
	param->audiotype = 0;
	param->enBitwidth = 1;
	param->inputvolume = m_inputvolume;
	param->inputvolume = m_outputvolume;
	param->framelen    = 160;
	return 0;
}

/*****************************************************************************
函数名称:StartAudioDecode
函数功能:开启音频解码，
输入参数:无
输出参数:无
返	回	 值:0:成功	-1:失败
使用说明: 外部调用，调用SendAudioStreamToDecode 前需要先开启解码
by :harvey

******************************************************************************/

int Audio::StartAudioDecode()
{
	
	HI_S32 s32Ret;
	ADEC_CHN_ATTR_S stAdecAttr;
	ADEC_CHN AdChn =0;	
	stAdecAttr.enType = PT_G711A;
	stAdecAttr.u32BufSize = 20;
	stAdecAttr.enMode = ADEC_MODE_PACK ;	

	ADEC_ATTR_G711_S stAdecG711;
	stAdecAttr.pValue = &stAdecG711;	
	
	/* create adec chn*/
	s32Ret = HI_MPI_ADEC_CreateChn(AdChn, &stAdecAttr);
	if (s32Ret)
	{
		printf("%s: HI_MPI_ADEC_CreateChn(%d) failed with %#x!\n", __FUNCTION__,\
			   AdChn,s32Ret);
		return s32Ret;
	}
	
	s32Ret = StartAudioAO();
	if(HI_SUCCESS != s32Ret)
	{
		printf("%s: StartAudioAO failed with %#x!\n", __FUNCTION__, s32Ret);
		return HI_FAILURE;
	}
	/*
	s32Ret = HI_MPI_AI_EnableAec(0, 0,0,0); 
	if(HI_SUCCESS != s32Ret) 
	{ 
	 	 printf("enable aec err:0x%x\n", s32Ret); 	 
	}
	*/
	m_DecodeState = 1;
	printf("StartAudioDecode Success\n");
	return 0;
}
/*****************************************************************************
函数名称:StopAudioDecode
函数功能:停止音频解码，
输入参数:无
输出参数:无
返	回	 值:0:成功	-1:失败
使用说明: 外部调用，StopAudioDecode需要再开启解码后才可以调用

******************************************************************************/

int Audio::StopAudioDecode()
{
	HI_S32 s32Ret;
	AUDIO_DEV AoDevId = 0;
	AO_CHN AoChn =0;
	ADEC_CHN AdChn = 0 ;

	for(unsigned int i=0;i< 1;i++)
	{
		AoChn = i;
		s32Ret = HI_MPI_AO_DisableChn(AoDevId, AoChn);
		if (s32Ret)
		{
			printf("%s: HI_MPI_AO_DisableChn(%d) failed with %#x!\n", __FUNCTION__,AoChn,s32Ret);
			
		}
	}
	s32Ret = HI_MPI_AO_Disable(AoDevId);
	if (s32Ret)
	{
		printf("%s: HI_MPI_AO_Disable(%d) failed with %#x!\n", __FUNCTION__,\
			   AoDevId,s32Ret);
		
	}

	s32Ret =  HI_MPI_ADEC_DestroyChn(AdChn);
	if (s32Ret)
	{
		printf("%s: HI_MPI_ADEC_CreateChn(%d) failed with %#x!\n", __FUNCTION__,\
			   AdChn,s32Ret);		
	}
	
	MPP_CHN_S stSrcChn,stDestChn;
	AdChn =0;
	for(unsigned int i=0;i< 1;i++)
	{
		AoChn = i;
		stSrcChn.enModId = HI_ID_ADEC;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = AdChn;
		stDestChn.enModId = HI_ID_AO;
		stDestChn.s32DevId = AoDevId;
		stDestChn.s32ChnId = AoChn;  
		s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn); 
		if(HI_SUCCESS != s32Ret)
		{
			printf("line %d: HI_MPI_SYS_Bind(%d) failed with %#x!\n", __LINE__,\
				   AoChn, s32Ret);			
		}
	}    

	m_DecodeState = 0;
	printf("StopAudioDecode Success\n");
	return 0;
}
int Audio::StartPlayFile(PLAY_TPYE  Type,void *other)
{
	m_PlayFile = true;
	m_PlayType = Type;
	printf("======================================StartPlayFile m_PlayType:%d\n",m_PlayType);
	return 0;
}

/*****************************************************************************
函数名称:SendAudioStreamToDecode
函数功能:向解码器发送音频数据流
输入参数:无
输出参数:无
返	回	 值:0:成功	-1:失败
使用说明: 外部调用再此函数调用前需要开启音频解码

******************************************************************************/

int Audio::SendAudioStreamToDecode(unsigned char *buffer,int len,int block)
{
	if((buffer == NULL)||(len>NumPerFrm+4))
	{
		printf("len is error ,need:%d\n",NumPerFrm+4);
		return -1;
	}
	HI_S32 s32Ret=0;
	AUDIO_STREAM_S stAudioStream;    
	stAudioStream.u32Len = len;
	stAudioStream.pStream = buffer;
	//printf("SendAudioStreamToDecode len:%d,%02x,%02x,%02x,%02x,\n",len,buffer[0],buffer[1],buffer[2],buffer[3]);
	//HI_FALSE
	s32Ret = HI_MPI_ADEC_SendStream(0, &stAudioStream, (HI_BOOL)block);
	//s32Ret = HI_MPI_ADEC_SendStream(0, &stAudioStream,HI_FALSE);
	if (s32Ret != HI_SUCCESS)
		printf("SendAudioStreamToDecode  failed len:%d,%02x,%02x,%02x,%02x,\n",len,buffer[0],buffer[1],buffer[2],buffer[3]);
   	m_TalkDelay =20;
	if (s32Ret)
	{
		if((s32Ret&0xf)==0xF)		
		{			
			printf("%s: HI_MPI_ADEC_SendStream failed with %#x!\n", __FUNCTION__, s32Ret);			
			printf("%s: HI_MPI_ADEC_ClearChnBuf ######## with %#x!,%02x\n", __FUNCTION__, s32Ret,(s32Ret&0xf));			
			s32Ret=HI_MPI_AO_ClearChnBuf(0,0);			
			HI_MPI_ADEC_ClearChnBuf(0);							
			HI_MPI_AO_ClearChnBuf(0,0);		
		}				
		return -1;
	}
	
	return 0;

}
static int cnt=0;
void Audio::PalyFileThreadProcess(void *parg)
{
	int fd = -1;
	unsigned char decodebuf[164]={0x0};
	decodebuf[0]=0x0;
	decodebuf[1]=0x01;
	decodebuf[2]=0x50;
	decodebuf[3]=0x00;
	int readlen =0;
	unsigned char tmpvolume=0;
	while(1)
	{
		if((m_PlayFile == false)||(m_PlayType == BUTT))
		{
			usleep(500*1000);
			continue;
		}
		if(fd < 0)
		{
			if(m_PlayType == ZBAR_SCAN)
			{
				fd = open(ZBAR_SCAN_FILE, O_RDONLY);
			}
			else if(m_PlayType == WIIF_CONNECT)
			{
				fd = open(ZBAR_SCAN_FILE, O_RDONLY);
			}
			else if(m_PlayType == ZBAR_WIFI_START_CH)
			{
				fd = open(ZBAR_SCAN_ST_C,O_RDONLY);
			}
			else if(m_PlayType == ZBAR_WIFI_START_EN)
			{
				fd = open(ZBAR_SCAN_ST_E,O_RDONLY);
			}
			else if(m_PlayType == ZBAR_WIFI_START_SP)
			{
				fd = open(ZBAR_SCAN_ST_S,O_RDONLY);
			}
			else if(m_PlayType == ZBAR_WIFI_WAIT_CH)
			{
				fd = open(ZBAR_SCAN_W_C,O_RDONLY);
			}
			else if(m_PlayType == ZBAR_WIFI_WAIT_EN)
			{
				fd = open(ZBAR_SCAN_W_E,O_RDONLY);
			}
			else if(m_PlayType == ZBAR_WIFI_WAIT_SP)
			{
				fd = open(ZBAR_SCAN_W_S,O_RDONLY);
			}

			else if(m_PlayType == ZBAR_WIFI_SUCCESS_CH)
			{
				fd = open(ZBAR_SCAN_SU_C,O_RDONLY);
			}
			else if(m_PlayType == ZBAR_WIFI_SUCCESS_EN)
			{
				fd = open(ZBAR_SCAN_SU_E,O_RDONLY);
			}
			else if(m_PlayType == VOLUME_SOUND)
			{				
				fd = open(VOLUMEMUSIC,O_RDONLY);
			}			
			else
			{
				m_PlayFile = false;
				m_PlayType = BUTT;
				continue;
			}
			
			if (fd < 0)
			{
				printf("Open (%s) error!!!!!\n", ZBAR_SCAN_FILE);
				sleep(1);
				continue;
			}
			AudioOutPutOnOff(1);
			
			tmpvolume = m_outputvolume;
			//SetOutPutVolume(25);
		}
		int decodecount  = 0;
		while(1)
		{			
			readlen= read(fd,decodebuf+4,160);
			if(readlen<=0)
			{
				m_PlayFile = false;
				m_PlayType = BUTT;
				close(fd);
				cnt=0;
				fd = -1;
				sleep(2);
				AudioOutPutOnOff(0);				
				//SetOutPutVolume(tmpvolume);
				break;
			}			
			SendAudioStreamToDecode(decodebuf,164,1);			
			//cnt++;
			if(decodecount++>20)
			{				
				sleep(1);	
				decodecount =0;
			}
		}
		sleep(1);

	}
	
}

/*****************************************************************************
函数名称:AudioEncodeThreadProcess
函数功能:编码处理线程
输入参数:parg
输出参数:无
返	回	 值:无
使用说明: 

******************************************************************************/

void Audio::AudioEncodeThreadProcess(void *parg)
{
	HI_S32 s32Ret;
	HI_S32 AencFd;
	AudioEnc_S *pstAencCtl = (AudioEnc_S *)parg;
	pthread_detach(pthread_self());/*退出线程释放资源*/
	AUDIO_STREAM_S stStream;
	fd_set read_fds;
	struct timeval TimeoutVal;
	
	FD_ZERO(&read_fds);    
	AencFd = HI_MPI_AENC_GetFd(pstAencCtl->AeChn);
	FD_SET(AencFd, &read_fds);
	m_AudioEnc.bthreadrun = true;
    bool    talk = false;
	
	while (pstAencCtl->bStart)
	{	
		if(startupdate == 0)
		{
			printf("thread exit file:%s line:%d pid:%d\n",__FILE__,__LINE__,(unsigned)pthread_self());
			AudioExit();
			pthread_exit(0);
		}
		TimeoutVal.tv_sec = 1;
		TimeoutVal.tv_usec = 0;
		FD_ZERO(&read_fds);
		FD_SET(AencFd,&read_fds);
		
		s32Ret = select(AencFd+1, &read_fds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0) 
		{
			sleep(5);
			continue;			
		}
		else if (0 == s32Ret) 
		{
			printf("%s: get aenc stream select time out\n", __FUNCTION__);
			continue;
		}
		
		if (FD_ISSET(AencFd, &read_fds))
		{
			/* get stream from aenc chn */
			s32Ret = HI_MPI_AENC_GetStream(pstAencCtl->AeChn, &stStream, HI_FALSE);
			if (HI_SUCCESS != s32Ret )
			{
				printf("%s: HI_MPI_AENC_GetStream(%d), failed with %#x!\n",\
					   __FUNCTION__, pstAencCtl->AeChn, s32Ret);
				pstAencCtl->bStart = false;
				continue ;
			}
			if(false == GetDeviceStatus())
			{
				HI_MPI_AENC_ReleaseStream(pstAencCtl->AeChn, &stStream);
				continue;
			}
            if(m_TalkDelay>0)
            {
                   m_TalkDelay--;
                   talk = true;
            }
            else
            {
                   talk = false;
            }
			if(m_audioinput == true)
			{
				/*音频帧存放在720P(pBufferManage[0])中*/
				if(pBufferManage[0] != NULL)
				{
					pBufferManage[0]->PutOneAFrameToBuffer((void *)(&stStream),talk);
				}	
				if(pBufferManage[1] != NULL)
				{
					pBufferManage[1]->PutOneAFrameToBuffer((void *)(&stStream),talk);
				}
				if(pBufferManage[2] != NULL)
				{
					pBufferManage[2]->PutOneAFrameToBuffer((void *)(&stStream),talk);
				} 
			}
			if(m_audioinput&&m_initAbnormalVoice)
			{
				AbnormalVoiceProcess((char *)(stStream.pStream),stStream.u32Len);
			}

			/* finally you must release the stream */
			HI_MPI_AENC_ReleaseStream(pstAencCtl->AeChn, &stStream);
		}	 
	}	
	pstAencCtl->bStart = false;
	m_AudioEnc.bthreadrun = false;
	return ;
}


int Audio::AudioExit()
{

	StopAudioEncode();
	StopAudioDecode();
	return -1;
}
//音频输出硬件开关 flag: 1 开、0 关


int Audio::AudioOutPutOnOff(unsigned int flag)
{
    int fd = 0, ret = 0;
    printf("-------------------------AudioOutPutOnOff flag:%d\n",flag);
    fd = open(GPIO_DEV, O_RDONLY);
	if(fd<0)
	{
		printf("open gpio erro!\n");
		return -1;
	}

    ret = ioctl(fd, GPIO_ADUIO_AMPLIFIER, &flag);
	if(ret < 0)
	{
		printf("ioctl erro\n");
        close(fd);
        return -1;
	}

    close(fd);
    return 0;
}

bool Audio::AudioGetSpeeker()
{	
	pthread_mutex_lock(&m_speeker);
	bool ret = false;
	if(m_speekfree == true)
	{
		ret = true;
		m_speekfree = false;
	}
	else
	{
		ret = false;
		
	}		
	pthread_mutex_unlock(&m_speeker);
	printf("-----------------------AudioGetSpeeker :%d\n",ret);
	return ret;
}
bool Audio::AudioReleaseSpeeker()
{
	pthread_mutex_lock(&m_speeker);
	m_speekfree = true;
	pthread_mutex_unlock(&m_speeker);
	printf("-----------------------AudioReleaseSpeeker\n");
	return true;
}
void Audio::AudioInput(bool cmd)
{
	m_audioinput = cmd;
}
void Audio::AbnormalVoiceProcess(char *buf,int len)
{
	/*********************************************************************/
	static int continuetime = 0;
	double db_ret = 0;
	int ret = (*dl_ZMD_AbnormalVoice_G711_process)(m_en_handle, (short*)buf, len,&db_ret);
	////-1:normal     0:abnormal voice     1:baby cry
	switch (ret)
	{
		case ZMD_TRAIN_VOICE:
			//printf("训练中\n");
			break;
		case ZMD_ABNORMAL_VOICE:
			
			//printf("-----------------------ABNORMAL_VOICE!!!!\n");
			
			continuetime=0;
			//BroadcastAlarms();
			AudioAlarmSet(0);
			break;
		case ZMD_NORMAL_VOICE:
			continuetime++;
			if(continuetime>50)
			{
				AudioAlarmSet(-1);
				m_Audioalarm = -1;
			}
			
			//printf("正常声音\n");
			break;
		case ZMD_BABYCRY_VOICE:
			printf("#########################baby cry!!!!\n");
			//m_Audioalarm = 1;
			AudioAlarmSet(1);
			continuetime=0;
			
			break;
		default:
			break;
		}

/**************************************************************************/
}
int Audio::GetAudioAlarm()
{

	printf("----------------------------------------audio alarm:%d !!!\n",m_Audioalarm);
	return m_Audioalarm;
}
void Audio::SetAudioAlarmValue(int value)
{
	m_Audioalarm=value;
	printf("----------------------------------------set audio alarm:%d !!!\n",m_Audioalarm);	
	return;
}

int Audio::AudioAlarmSet(int alarm)
{
	/* 判断报警类型 */
	if(alarm != 0)
	{
		return -1;
	}

	static time_t begin;
	time_t now = time(NULL);
	int alarm_interal=900;/*默认是15min*/

	/*判断音频报警开关*/
	if(GetWebScheduleSwitch(2) != 1)	
		return -1;	

	alarm_interal=GetAlarmInterval();
	
	//printf("\n now time[%ld] begin[%ld] alarm_interal[%d]    now - begin[%ld]########\r\n", now,begin,alarm_interal,now - begin);

	if((now - begin) >= alarm_interal)
	{
		printf("############### audio alarm ####################\r\n");
		/*设置异常声音告警-1 normal   0 abnormal  1 baycry*/
		m_Audioalarm=0;

		/*音频告警上报*/
		BroadcastAlarmsEx(P2P_ALARM_AUDIO_EXCEPTION, 0);
	}
	begin = now;
	
	return 0;



	
}



