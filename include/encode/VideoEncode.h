
#ifndef _VIDEO_ENCODE_H_
#define _VIDEO_ENCODE_H_

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
#include "BufferManage.h"
#include "His_Api_define.h"
#include "CommonFunction.h"
#include "Video_ISP.h"
#include "Video_comm.h"
#include "ModuleFuncInterface.h"
#include "EncodeManage.h"
#define MAX_CH_STREAM	3



//编码参数结构体定义
typedef struct {
	unsigned char			norm;		/*0--PAL, 1--NTSC*/	
	unsigned char			resolution;	/*0--720p,1--VGA  2-QVGA*/
	unsigned char			framerate;	/*帧率*/
	unsigned char			VbrOrCbr;	/*1--CBR, 0--VBR*/
	
	unsigned char			quality;		/*画质*/
	unsigned char			gop;		/*I 帧间隔*/
	unsigned char			streamtype;	/*1--主码流，0--子码流*/
		
	int						bitrate;		/*码率*/


}EncodePara_S;

class VideoEncode;
typedef struct _encoderstream_{/*码流*/
	class VideoEncode *pEncoder;	
	class BufferManage	*pBufferManage;
	int 		venc;/*编码通道*/
	int			EncodeStat;/*编码状态1:running 0:stop*/
	int			ThreadStat;/*线程状态1:running 0:stop*/
	pthread_t		pid;
}encoderstream_t;

class VideoEncode{
private:

	 
	pthread_t m_IspPid;

	

public:
	VideoEncode();
	~VideoEncode(){};
	CVideoISP 			*m_pISP;
	encoderstream_t		m_EncoderStream[MAX_CH_STREAM];

	int InitVideoEncoder();
	/*****************************************************************************
	函数名称:InitBuffer
	函数功能:初始化buffer
	输入参数:无
	输出参数:无
	返	回	 值: 0: 成功-1: 失败
	使用说明: 内部部调用，
	
	******************************************************************************/
	int InitBuffer();	

	
	/*****************************************************************************
	函数名称:VideoViConfig
	函数功能:vi输入设置
	输入参数:无
	输出参数:无
	返	回	 值: 0: 成功-1: 失败
	使用说明: 内部部调用，
	
	******************************************************************************/
	int VideoViConfig();
	
	
	/*****************************************************************************
	函数名称:Hi3518SysInit
	函数功能:系统初始
	输入参数:无
	输出参数:无
	返	回	 值: 
	使用说明: 内部部调用，
	
	******************************************************************************/
	int Hi3518SysInit();
	/*****************************************************************************
	函数名称:StartVideoEncoder
	函数功能:初始化视频编码
	输入参数:pEncPara 相见参数结构体
	输出参数:无
	返	回	 值: 0: 成功-1: 失败
	使用说明: 外部调用，
	编码和系统初始化后才可以调用
	
	******************************************************************************/
	int StartVideoEncoder(EncodePara_S *pEncPara);
	int StartStreamThread(int venc);
	int VideoStreamThreadBody(int venc);
	/*****************************************************************************
	函数名称:VideoMirrorFlipSet
	函数功能:图像翻转镜像熟悉的设置
	输入参数:Para 4:正常1 翻转2 镜像3 镜像加翻转
	输出参数:无
	返	回	 值: 0: 成功-1: 失败
	使用说明: 外部调用，
	编码和系统初始化后才可以调用
	
	******************************************************************************/

	int VideoMirrorFlipSet(int Para);

	int VideoMirrorSet(bool set);

	int VideoFlipSet(bool set);

	//停止编码
	int StopVideocEncode();

	int VideoEncodeExit(int VeChn );

	int SensorInit();
	int SetVideoAntiFlickerAttr(bool enable,unsigned short Frequency );

	
};

#endif
