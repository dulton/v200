/******************************************************************************
  File Name     : ViVo_Api.h
  Version       : Initial Draft 1.0
  Last Modified :
  Description   : Function declare of the vi and vo
  Function List :
  History       :

******************************************************************************/
#ifndef _AIAO_API_H_
#define _AIAO_API_H_

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vo.h"
#include "hi_comm_vi.h"
#include "hi_comm_region.h"
#include "hi_comm_venc.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
//#include "mpi_vpp.h"
#include "mpi_venc.h"

 enum AI_PARA_E{
	AI_MODE_8K = 0,
	AI_MODE_16K,
	AI_MODE_32k
};

/*****************************************************************************
函数功能:音频输入配置
输入参数:@AiMode: AI 通道设置模式
			   @AiDevId: AI 设备号
			   @ChlNum: 设置通道数
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int AudioInputConfig(AI_PARA_E AiMode, AUDIO_DEV AiDevId, int ChlNum);

/*****************************************************************************
函数功能:禁止音频输入
输入参数:@AiDevId: AI 设备号
			   @ChlNum: 设置通道数
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int DisableAudioInput(AUDIO_DEV AiDevId, int ChlNum);

/*****************************************************************************
函数功能:音频输出配置
输入参数:@AiMode: AO 通道设置模式
			   @AoDevId: AO 设备号
			   @AoChn: AO 通道号
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int AudioOutputConfig(AI_PARA_E AiMode, AUDIO_DEV AoDevId, AO_CHN AoChn, int BitMode);

/*****************************************************************************
函数功能:禁止音频输出
输入参数:@AoDevId: AO 设备号
			   @AoChn: AO 通道号
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int DisableAudioOutput(AUDIO_DEV AoDevId, AO_CHN AoChn);


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
int Ai2AoHandler(int Aidev, int Aichn, int Aodev, int Aochn);

#endif
