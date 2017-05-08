/******************************************************************************
  File Name     : ViVo_Api.cpp
  Version       : Initial Draft
  Last Modified :
  Description   : the functions of vi and vi inplement  
  Function List :
  History       :

******************************************************************************/

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

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

#include "His_Api_define.h"
#include "AiAo_Api.h"
#include "CommonDefine.h"


/*****************************************************************************
函数功能:音频输入配置
输入参数:@AiMode: AI 通道设置模式
			   @AiDevId: AI 设备号
			   @ChlNum: 设置通道数
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int AudioInputConfig(AI_PARA_E AiMode, AUDIO_DEV AiDevId, int ChlNum)
{
	AIO_ATTR_S stAttr;

	switch(AiMode)
	{
		case AI_MODE_8K:
			if(1/*ChlNum > 4*/)
			{
				stAttr.enBitwidth = AUDIO_BIT_WIDTH_8;/*音频采样精度（从模式下，此参数必须和codec 的采样精度匹配)*/
			}
			else 
			{
				stAttr.enBitwidth = AUDIO_BIT_WIDTH_16;//音频采样精度（从模式下，此参数必须和codec 的采样精度匹配)
			}
			
			stAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;//音频采样率（从模式下，此参数不起作用）
			stAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;//单声
			stAttr.enWorkmode = AIO_MODE_I2S_SLAVE;
			stAttr.u32EXFlag = 0;//8bit 到16bit 扩展标志（8bit 精度时有效）1：扩展
			stAttr.u32FrmNum = 40;//缓存帧数目
			stAttr.u32PtNumPerFrm = 160;//每帧的采样点个数
			break;
		case AI_MODE_16K:

			{
				stAttr.enBitwidth = AUDIO_BIT_WIDTH_16;//音频采样精度（从模式下，此参数必须和codec 的采样精度匹配)
			}
			
			stAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;//音频采样率（从模式下，此参数不起作用）
			stAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;//单声
			stAttr.enWorkmode = AIO_MODE_I2S_SLAVE;
			stAttr.u32EXFlag = 0;//8bit 到16bit 扩展标志（8bit 精度时有效）1：扩展
			stAttr.u32FrmNum = 40;//缓存帧数目
			stAttr.u32PtNumPerFrm = 160;//每帧的采样点个数
			break;
			
		case AI_MODE_32k:
		default:
			return HI_FAILURE;
	}
	
	if(HI_MPI_AI_SetPubAttr(AiDevId, &stAttr) !=0)
	{
        	DEBUG_INFO("set ai %d attr err\n", AiDevId);
		return HI_FAILURE;	
	}
	
	if(HI_MPI_AI_Enable(AiDevId) != 0)/* enable ai device*/
	{
        	DEBUG_INFO("HI_MPI_AI_Enable err\n");
		return HI_FAILURE;
	}
	
	for(int i = 0;i < ChlNum; i ++)//aichn 8
	{
		if(HI_MPI_AI_EnableChn(AiDevId, i) != 0)
		{
	        	DEBUG_INFO("HI_MPI_AI_EnableChn err\n");
			return HI_FAILURE;	
		}		
	}
	
	//DEBUG_INFO(("StartupAudioInputDevice(%d)*********************ok\n",AiDevId));
	return HI_SUCCESS;
}

/*****************************************************************************
函数功能:禁止音频输入
输入参数:@AiDevId: AI 设备号
			   @ChlNum: 设置通道数
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int DisableAudioInput(AUDIO_DEV AiDevId, int ChlNum)
{	
	for(int i = 0;i < ChlNum; i ++)//aichn 8
	{
		if(HI_MPI_AI_DisableChn(AiDevId, i) != 0)
		{
	        	DEBUG_INFO("HI_MPI_AI_DisableChn err\n");
			return HI_FAILURE;	
		}		
	}
	
	HI_MPI_AI_Disable(AiDevId);

	return HI_SUCCESS;
}

/*****************************************************************************
函数功能:音频输出配置
输入参数:@AiMode: AO 通道设置模式
			   @AoDevId: AO 设备号
			   @AoChn: AO 通道号
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int AudioOutputConfig(AI_PARA_E AiMode, AUDIO_DEV AoDevId, AO_CHN AoChn, int BitMode)
{
	AIO_ATTR_S stAttr;
	HI_S32 s32ret;

	switch(AiMode)
	{
		case AI_MODE_8K:
			if(BitMode > 0)
				stAttr.enBitwidth = AUDIO_BIT_WIDTH_16;//音频采样精度（从模式下，此参数必须和codec 的采样精度匹配)
			else 
				stAttr.enBitwidth = AUDIO_BIT_WIDTH_8;
			
			stAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;//音频采样率（从模式下，此参数不起作用）
			stAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;//单声
			stAttr.enWorkmode = AIO_MODE_I2S_MASTER;
			stAttr.u32EXFlag = 0;//8bit 到16bit 扩展标志（8bit 精度时有效）1：扩展
			stAttr.u32FrmNum = 10;//缓存帧数目
			stAttr.u32PtNumPerFrm =160;//每帧的采样点个数
			break;
		case AI_MODE_16K:
			stAttr.enBitwidth = AUDIO_BIT_WIDTH_16;//音频采样精度（从模式下，此参数必须和codec 的采样精度匹配)
			stAttr.enSamplerate = AUDIO_SAMPLE_RATE_16000;//音频采样率（从模式下，此参数不起作用）
			stAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;//单声
			stAttr.enWorkmode = AIO_MODE_I2S_SLAVE;
			stAttr.u32EXFlag = 0;//8bit 到16bit 扩展标志（8bit 精度时有效）1：扩展
			stAttr.u32FrmNum = 30;//10;//缓存帧数目
			stAttr.u32PtNumPerFrm = 320;//160;//每帧的采样点个数
			break;
		case AI_MODE_32k:
		default:
			return HI_FAILURE;
	}
	
	/* set ao public attr*/
	s32ret = HI_MPI_AO_SetPubAttr(AoDevId, &stAttr);
	if(HI_SUCCESS != s32ret)
	{
		DEBUG_INFO("set ao %d attr err:0x%x\n", AoDevId,s32ret);
		return HI_FAILURE;
	}
	
	/* enable ao device*/
	s32ret = HI_MPI_AO_Enable(AoDevId);
	if(HI_SUCCESS != s32ret)
	{
		DEBUG_INFO("enable ao dev %d err:0x%x\n", AoDevId, s32ret);
		return HI_FAILURE;
	}

	/* enable ao channel*/
	s32ret = HI_MPI_AO_EnableChn(AoDevId, AoChn);
	if(HI_SUCCESS != s32ret)
	{
		DEBUG_INFO("enable ao dev %d err:0x%x\n", AoDevId, s32ret);
		return HI_FAILURE;
	}

#ifdef Y_BOARD    
	/* enable ao channel*/
	s32ret = HI_MPI_AO_EnableChn(AoDevId, 2);
	if(HI_SUCCESS != s32ret)
	{
		DEBUG_INFO("enable ao dev %d err:0x%x\n", AoDevId, s32ret);
		return HI_FAILURE;
	}
#endif
	return HI_SUCCESS;
}

/*****************************************************************************
函数功能:禁止音频输出
输入参数:@AoDevId: AO 设备号
			   @AoChn: AO 通道号
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int DisableAudioOutput(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	HI_S32 s32Ret;
	
	s32Ret = HI_MPI_AO_DisableChn(AoDevId, AoChn);
	if(HI_SUCCESS != s32Ret)
	{
		DEBUG_INFO("disable ao channel %d err:0x%x\n", AoDevId, s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_AO_Disable(AoDevId);
	if(HI_SUCCESS != s32Ret)
	{
		DEBUG_INFO("disable ao dev %d err:0x%x\n", AoDevId, s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


/*****************************************************************************
函数功能:音频换回输出处理
输入参数:@AoDevId: AO 设备号
			   @AoChn: AO 通道号
			   @Aidev:AI 设备号
			   @Aichn:AI 通道号
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int Ai2AoHandler(int Aidev, int Aichn, int Aodev, int Aochn)
{
	HI_S32 s32ret;
	AUDIO_FRAME_S stFrame;

	/* get audio frame form ai chn */
	s32ret = HI_MPI_AI_GetFrame(Aidev, Aichn,&stFrame,NULL,HI_IO_BLOCK);
	if(HI_SUCCESS != s32ret)
	{   
		printf("get ai frame err:0x%x ai(%d,%d)\n",s32ret,Aidev, Aichn);       
		return HI_FAILURE;
	}

	printf("AiAo_proc:get ai frame ok %d wth : %d  md : %d  len :%d  \n" , stFrame.u32Seq, stFrame.enBitwidth, stFrame.enSoundmode, stFrame.u32Len);
	
	/* send audio frme to ao */
	s32ret = HI_MPI_AO_SendFrame(Aodev, Aochn, &stFrame, HI_IO_BLOCK);
	if (HI_SUCCESS != s32ret)
	{   
		printf("ao send frame err:0x%x\n",s32ret);      
		return HI_FAILURE;
	}
		
    return HI_SUCCESS;
}

