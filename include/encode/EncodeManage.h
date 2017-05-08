
#ifndef _ENCODE_MANAGE_H
#define _ENCODE_MANAGE_H

#include "ViVo_Api.h"
#include "BufferManage.h"
#include "parametermanage.h"
#include "FrontOsd.h"
#include "McuCommunication.h"
#include "Audio.h"  

#ifdef __cplusplus
extern "C" {
#endif



typedef struct
{
    HI_BOOL bStart;
    HI_S32 s32AoDev;
    HI_S32 s32AoChn;
    pthread_t stAdecPid;
    pthread_t stAdecPid2;
} SAMPLE_ADEC_S;

/*****************************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int GetSysTime(SystemDateTime *pSysTime);

/*****************************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int InitEncodeSystem();




/*****************************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int StartEncodeSystem();




/*****************************************************************************
函数功能: 停止编码系统
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int StopEncodeSystem(int isSync,int isFinish = 0);

int StartSubEncodeSystem(int norm);

/*****************************************************************************
*writed by panjy
****************************************************************************/
int StartVgaEncodeSystem(VENC_MODE_E VencMode, int norm);
int StopVgaEncodeSystem(int norm);



/*****************************************************************************
函数功能:修改前端osd 的通道号标识区域
输入参数:
输出参数:无
返  回   值:无
使用说明:
******************************************************************************/

/*****************************
开始抓拍
*****************************/

typedef struct _snap_pic_arg_
{
	int 				Result[MAX_SNAP_CH_NUM];
	char 			fileName[MAX_SNAP_CH_NUM][100];
	datetime_setting 	 m_datetime;
}snap_pic_arg;


typedef struct 
{
	int 			sendflag;
	int 			Result;
	char 			path[96];/*路径  eg : /tmp*/
	char			FileName[64];/*抓拍输入的文件名称,email_pic.jpg  文件名邮件已经使用*/
	char			FullFileName[128];/*返回的全路径 eg:   /tmp/email_pic.jpg  */
	datetime_setting 	 m_DateTime;
	int			m_SendingPic;

}SnapImageArg;



int SnapOneChannelImage(int ch, SnapImageArg *Snap);







int StartMicTalkAudioEncode();

/*****************************************************************************
函数功能:创建视频遮盖区域
输入参数:ch 为通道号0---channelmax-1 ， rect 为长方形区域
输出参数:无
返  回   值:成功返回0  失败返回 -1
使用说明:
******************************************************************************/
int CreateFullVideoCoverRegion(int ch, RECT_S rect,int area);

/*****************************************************************************
函数功能:删除视频遮盖区域
输入参数:ch 为通道号0---channelmax-1 
输出参数:无
返  回   值:成功返回0  失败返回 -1
使用说明:
******************************************************************************/
int DeleteFullVideoCoverRegion(int ch,int index);

#ifdef  SUBSTREAM_ENC
int StartSubEncodeSystem(int norm);
#endif 


int ResetUserData2IFrameBySecond(int channel,int streamtype, int userid, int seconds) ;


/*****************************************************************************
函数功能:
输入参数:@userid:用户id(1~MAX_BUFUSER_ID )
			  @streamtype 为码流类型0 :720  1:VGA
输出参数:@buffer: 返回帧的起始地址
			   @pFrameInfo:返回此帧的信息
返  回   值:成功返回0，否则返回-1
使用说明:返回失败，请勿使用buffer 指针进行数据的获取
******************************************************************************/

int GetSendNetFrame(int channel,int streamtpye, int userid, unsigned char **buffer, FrameInfo *m_frameinfo);


/*****************************************************************************
函数功能:复位指定消费者用户的读指针信息
输入参数:streamtype 为码流类型0 :720  1:VGA
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/

int ResetUserData2IFrame(int channel,int streamtype, int userid);

/*****************************************************************************
函数功能:强制生成I 帧
输入参数:streamtype 为码流类型0 :720  1:VGA 2:QVGA
				time:0 立即生成，
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int RequestIFrame(int streamtype,int time);
int SetAudioVolumeLevel(int level);

int StartOsd();
int ResetUserData2CurrentIFrame(int channel,int streamtype, int userid);

/*
下面是音频编解码接口，语音对讲时IPC 向平台发送音频为上行，
平台向IPC发送音频为下行。
上行时函数调用步骤:1,StartAudioEncode  2,GetSendNetFrame 3,StopAudioEncode
下行时函数调用步骤:1,StartAudioDecode  2,SendAudioStreamToDecode 3,StopAudioDecode

设置音频输入输出的大小SetAudioVolume
*/
/*****************************************************************************
函数名称:GetAudioParam
函数功能:获取音频参数
输入参数:param 
输出参数:无
返	回	 值:0:成功	-1:失败
使用说明:
******************************************************************************/

int GetAudioParam(AudioParm *param);

/*****************************************************************************
函数名称:StartAudioDecode
函数功能:开始音频解码
输入参数:Audiotype   0:G711  1:G726 
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:
******************************************************************************/
int	StartAudioDecode(int Audiotype);
/*****************************************************************************
函数名称:SendAudioStreamToDecode
函数功能:发送音频数据解码，
输入参数:
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int	SendAudioStreamToDecode(unsigned char *buffer,int len,int block);
/*****************************************************************************
函数名称:StopAudioDecode
函数功能:结束音频解码
输入参数:
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int	StopAudioDecode();
/*****************************************************************************
函数名称:StartAudioEncode
函数功能:开始音频编码
输入参数:Audiotype   0:G711  1:G726 
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int	StartAudioEncode(int Audiotype);

int	GetAudioData(int userid,bool isfirst,unsigned char *buf,int *audiolen);

/*****************************************************************************
函数名称:StopAudioEncode
函数功能:结束音频解码
输入参数:
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int	StopAudioEncode();
/*****************************************************************************
函数名称:SetAudioVolume
函数功能:
输入参数::0 :input  1:output  vol (0~31) 	  25为默认声音vol 越大声音越大
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int	SetAudioVolume(int inoutput,unsigned int vol);
bool AudioGetSpeeker();
bool AudioReleaseSpeeker();

int UpdateCoverlayOsd();


/*****************************************************************************
函数名称:VideoMirrorFlipSet
函数功能:设置图像镜像和翻转
输入参数:mode   4:正常1 翻转2 镜像3 镜像加翻转
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int VideoMirrorFlipSet(int mode);

int VideoMirrorSet(bool set);
int VideoFlipSet(bool set);




/*****************************************************************************
函数名称:VideoBrightnessSet
函数功能:设置图像亮度
输入参数:value   0~255 
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/

int VideoBrightnessSet(int value);
/*****************************************************************************
函数名称:VideoContrastSet
函数功能:设置对比度
输入参数:value   0~255 
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/

int VideoContrastSet(int value);

/*****************************************************************************
函数名称:VideoSaturationSet
函数功能:设置图像饱和度
输入参数:value   0~255 
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/


int VideoSaturationSet(int value);
/*****************************************************************************
函数名称:VideoColourSet
函数功能:设置图像彩色或者黑白
输入参数:value   1:彩色模式2:黑白模式
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/

int VideoColourSet(int value);
/*****************************************************************************
函数名称:VideoPowerSet
函数功能:设置电压 暂时无效
输入参数:value	 1:50HZ 2:60HZ
输出参数:无
返	回	 值:0:成功	-1:失败
使用说明:

******************************************************************************/

int VideoPowerSet(int value);

int SetVideoChnAnaLog(int brightness,int contrast,int saturation);

#ifdef __cplusplus
 };
#endif

#endif

