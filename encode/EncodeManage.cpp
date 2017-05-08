
#include <time.h>

#include "DebugPrint.h"
#include "EncodeManage.h"
#include "VideoEncode.h"    
#include "FrontOsd.h"
#include "AiAo_Api.h"
#include "ModuleFuncInterface.h"
//#include "AVenc_Api.h"
#include "CommonFunction.h"

#include "JpegSnap.h"	//抓拍头
#include "common.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include "FrontOsd.h"
#include "Video_MD.h"
#include "GpioApi.h"
#include "IR_Cut.h"



#ifdef __cplusplus
extern "C" {
#endif


//编码模式1--CBR, 0--VBR
#define ENCODE_MODE				0  

enum{
	ENCODE_STOP = 0,
	ENCODE_STOPING,
	ENCODE_STARTING,
	ENCODE_RUNING,
};



VideoEncode  		*pVideoEncode[MAX_REC_CHANNEL];
BufferManage 		*pBufferManage[MAX_CH_STREAM];
FrontOsd			*pFrontOsdObj;
VideoEncode 		*pVideo;
Audio 		*paudio = NULL;
extern PARAMETER_MANAGE*	g_cParaManage;
extern int startupdate;
extern DeviceConfigInfo 	ConfigInfo;

char			picturemode;
extern int	FrameRate;
extern bool	Night;


/**************************
创建抓拍通道
**************************/
bool CreateSnapCh()
{
	JpegSnap_S *pJpep = JpegSnap_S::Instanse();
	if(pJpep != NULL)
	{
	//	pJpep->CreateSnapChn(VIDEO_ENCODING_MODE_NTSC,PIC_QVGA);
		pJpep->CreateSnapChn(VIDEO_ENCODING_MODE_AUTO,PIC_QVGA);
		 
	}
	
	return true;
}

/***************************
销毁抓拍通道
***************************/
bool DestroySnapCh()
{

	JpegSnap_S *pJpep = JpegSnap_S::Instanse();
	
	pJpep->DestroySnapCh();
	
	return true;
}



int SnapOneChannelImage(int ch, SnapImageArg *Snap)
{
	
	JpegSnap_S *pJpep = JpegSnap_S::Instanse();

	if((NULL == pJpep) || (NULL == Snap) || (ch < 0) || (ch >=MAX_SNAP_CH_NUM)||(strlen(Snap->FileName)==0))
	{
		return -1;
	}		
	if(startupdate == 0)
	{
		printf("handleEmail thread exit file:%s line:%d pid:%d	\n",__FILE__,__LINE__,getpid());
		pthread_exit(0);
	}

	sprintf(Snap->FullFileName, "/tmp/%s", Snap->FileName);
	strcpy(Snap->path, "/tmp");
	
	unlink(Snap->FullFileName); 
	
	FILE *fp = NULL;	
	fp	= fopen(Snap->FullFileName, "w+b"); 	
	if(fp != NULL)
	{
		Snap->Result = pJpep->JpegSnapProcess((void*)fp);	
	}
	else
	{
		printf("open %s  failed\n",Snap->FullFileName);
		perror("open snap pic");		
		return -1;
	}
	
	fflush(fp);

	if(fp  != NULL)
	{
		fclose(fp );
	}
	
	fp	= NULL;
	

	return 0;

}







/*****************************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int GetSysTime(SystemDateTime *pSysTime)
{
	GlobalGetSystemTime((datetime_setting*)pSysTime);
	return 0;
}

int StartVideo()
{
	//获取编码参数
	CAMERA_PARA  encode_para;	
	GetEncodePara(&encode_para);
	int framerate = 0;	
	
	/* 720P */
	EncodePara_S EncPara;
	framerate = encode_para.m_ChannelPara[0].m_uFrameRate;
	if((framerate == 0)||(framerate >= 20))
	{
		framerate = 18;
	}	
	if(Night)
		FrameRate  = framerate/2;/*夜视模式*/
	else
		FrameRate  = framerate;
	EncPara.norm = 1;
	EncPara.resolution = 0;
	EncPara.framerate = framerate;
	EncPara.VbrOrCbr = encode_para.m_ChannelPara[0].m_uEncType;
	EncPara.quality = encode_para.m_ChannelPara[0].m_uQuality;
	//EncPara.gop = 10;
	EncPara.streamtype = 1;		
	
	EncPara.bitrate = (1024*2)/(EncPara.quality+1);	/*最大比特率*/	
	
	pVideo->StartVideoEncoder(&EncPara);	
#if 0	
	/* VGA */
	
	framerate = encode_para.m_ChannelPara[0].m_uSubFrameRate;
	if((framerate == 0)||(framerate > 25))
	{
		framerate = 25;
	}
	EncPara.norm = 1;
	EncPara.resolution = 2;
	EncPara.framerate = 12;
	EncPara.VbrOrCbr = 0;	
	EncPara.quality = encode_para.m_ChannelPara[0].m_uSubQuality;
	EncPara.bitrate = (320*2)/(EncPara.quality+1); 		/*最大比特率*/		
	pVideo->StartVideoEncoder(&EncPara);
//#endif
#endif	

	/* QVGA */

	framerate = encode_para.m_ChannelPara[0].m_uSubFrameRate;
	if((framerate == 0)||(framerate >= 15))
	{
		framerate = 10;
	}
	EncPara.norm = 1;
	EncPara.resolution = 2;
	EncPara.framerate = framerate;
	EncPara.VbrOrCbr = encode_para.m_ChannelPara[0].m_uSubEncType;	
	EncPara.quality = encode_para.m_ChannelPara[0].m_uSubQuality;
	EncPara.bitrate =(320*2)/(EncPara.quality+1); 		/*最大比特率*/		
	pVideo->StartVideoEncoder(&EncPara);




	
	
	CreateSnapCh();
	SnapImageArg  Snap;
	memset(&Snap,0x0,sizeof(SnapImageArg));
	sprintf(Snap.FileName, "%s", "snaponvif.jpg");
	int snap_ret = -1;
	snap_ret  = SnapOneChannelImage(0, &Snap);
	if(snap_ret == 0)
	{
		printf("path:%s\r\n", Snap.FullFileName);
	}

	return 0;
}

int StartAudio()
{

	SetAudioVolume(0,28);
	SetAudioVolume(1,28);
	
	return StartAudioEncode(0);
	
}

int StartMd()
{

	MD_HANDLE  *VideoMd = NULL;
	VideoMd = MD_HANDLE::Instance();
	if(VideoMd == NULL)
	{
		return -1;
	}
	return VideoMd->StartMotionDetection();
	
	

}



int StartOsd()
{
	if(pFrontOsdObj == NULL)
	{
		return -1;
	}	
	//获取编码参数
	CAMERA_PARA  encode_para;	
	GetEncodePara(&encode_para);
	
/****************show time *****************************/
	COMMON_PARA  CommPara;
	g_cParaManage->GetSysParameter(SYSCOMMON_SET, &CommPara);
	pFrontOsdObj->m_TimeMode = CommPara.m_uTimeMode;
	if(encode_para.m_ChannelPara[0].m_TimeSwitch == 1)
	{
		pFrontOsdObj->ShowVideoOsd_Time(NULL);
	}

/****************show channel name********************* */
	if(encode_para.m_ChannelPara[0].m_TltleSwitch == 1)
	{
		VideoOsd_S OsdPara;
		memset(&OsdPara,0x0,sizeof(VideoOsd_S));
		//sprintf(OsdPara.chnname,"%s","IPC");
		strncpy(OsdPara.chnname, encode_para.m_ChannelPara[0].m_Title,16);
		OsdPara.x = 0;
		OsdPara.y = 0;
		pFrontOsdObj->ShowVideoOsd_Channel(&OsdPara);
	}

	pFrontOsdObj->StartOsdProcess();



/*********CoverRegion*************************/
	UpdateCoverlayOsd();


	
	return 0;

}

/*****************************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int InitEncodeSystem()
{

     printf("InitEncodeSystem296\n");
/**********video********/
	pVideo = new VideoEncode();
	if(pVideo == NULL)
	{
		
		exit(1);
	}
	unsigned short 	PowerFreq = 60;
	bool			enable = true;
	CAMERASENSOR_PARA	  sensor;
	g_cParaManage->GetSysParameter(SENSOR_SET,&sensor);
	if(sensor.m_PowerFreq == 1)
	{   printf("=3091\n");
		PowerFreq = 50;
		enable =true;
	}
	else if(sensor.m_PowerFreq == 2)
	{  printf("=3141\n");
		PowerFreq = 60;
		enable =true;
	}
	else if(sensor.m_PowerFreq == 3)
	{ printf("=3191\n");
		enable =true;
		PowerFreq = 0;
	}
	else
	{
		enable = false;
	}
	web_sync_param_t  sync_data;
	PubGetSysParameter(WEB_SET, &sync_data);
	Set_NightSwtich(sync_data.nightvision_switch);
	
	pVideoEncode[0] = pVideo;
	pVideo->InitVideoEncoder();	
		
	pVideo->SetVideoAntiFlickerAttr(enable,PowerFreq);


	pVideo->VideoMirrorFlipSet(sensor.m_picMode);
	picturemode = sensor.m_picMode;
	CAMERA_ANALOG  lchaPara;	
	g_cParaManage->GetSysParameter(SYSANALOG_SET,&lchaPara);
	printf("InitEncodeSystem339\n");   

	SetVideoChnAnaLog(
		lchaPara.m_Channels[0].m_nBrightness,
		lchaPara.m_Channels[0].m_nContrast,
		lchaPara.m_Channels[0].m_nSaturation);	
	
      printf("InitEncodeSystem346\n");

/**********Audio********/

	paudio = new Audio();
	if(paudio == NULL)
	{
		exit(1);
	}
	paudio->InitAudio();
	StartAudio();

/**********osd********/
	pFrontOsdObj = new FrontOsd(&GetSysTime);
	if(pFrontOsdObj == NULL)
	{
		exit(1);
	}
	pFrontOsdObj->VideoOsdInit();
/**********md********/	
	InitSystemMD(); 		
	StartMd();
	
	StartEncodeSystem();

	return 0;
}
  



/*****************************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int StartEncodeSystem()
{
	StartVideo();
	
	StartOsd();
	
	return 0;
}

/*****************************************************************************
函数功能: 停止编码系统
输入参数:IsFinish表示进入关机流程里
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int StopEncodeSystem(int isSync,int isFinish)
{
	/*停止编码顺序不可改变*/
	


	/*stop osd*/
	pFrontOsdObj->StopOsdProcess();

	for(int i=0;i<4;i++)
	{
		pFrontOsdObj->DeleteCoverLayerRegion(i);
	}
	/*stop video encode*/
	pVideo->StopVideocEncode();		

	
	return 0;
}



int CreateFullVideoCoverRegion(int ch, RECT_S rect,int area)
{
	pFrontOsdObj->DeleteCoverLayerRegion(area);
	return pFrontOsdObj->CreateCoverLayerRegion(rect,area);
	

}

int DeleteFullVideoCoverRegion(int ch,int index)
{
	
	return pFrontOsdObj->DeleteCoverLayerRegion(index);
}
int ResetUserData2IFrameBySecond(int channel,int streamtype, int userid, int seconds) 
{ 

	int retval= S_FAILURE; 

	if(pBufferManage[streamtype] != NULL) 
	{ 
		retval = pBufferManage[streamtype]->StartGetFrame(userid, 2, (void *)&seconds); 
	} 
	return retval; 

}


int StartGetFrame4Record(int channel,int streamtype, int userid) 
{ 
	//int pretime = 1;
	//return pBufferManage[streamtype]->StartGetFrame(userid, 2, (void *)&pretime);	
	return pBufferManage[streamtype]->ResetUserInfo(userid);
}

/*****************************************************************************
函数功能:
输入参数:@userid:用户id(1~MAX_BUFUSER_ID )
			  @streamtype 为码流类型0 :720  1:VGA 2:QVGA
输出参数:@buffer: 返回帧的起始地址
			   @pFrameInfo:返回此帧的信息
返  回   值:成功返回0，否则返回-1
使用说明:返回失败，请勿使用buffer 指针进行数据的获取
******************************************************************************/
int GetSendNetFrame(int channel,int streamtype, int userid, unsigned char **buffer, FrameInfo *m_frameinfo)
{
	
	int retval= S_FAILURE;
	

	if((userid <1) ||(startupdate == 0)|| (userid >= MAX_BUFUSER_ID))
	{
		//printf(" invalid user id  %d \n", userid);
		return S_FAILURE;
	}
	if(( streamtype >= MAX_CH_STREAM)||( streamtype <0 ))
	{
		return S_FAILURE;
	}
	if(pBufferManage[streamtype] != NULL)
	{
		 retval = pBufferManage[streamtype]->GetOneFrameFromBuffer(userid, buffer, m_frameinfo);
	}
    if(m_frameinfo->Flag == 3 && m_frameinfo->talk)
    {
      //  printf("drop audio.....\n");
        return -2;
    }
	#ifdef ADJUST_STREAM
	if((pBufferManage[streamtype]->m_FrameBufferUser[userid].diffpos > FrameDiffPos)&&(retval !=-1))/*读数据指针比写数据达到一个阈值，丢帧*/
	{
		/*开始丢帧，但经过测试，连接开始时丢帧会花屏，
		故在前n(100)次下不丢帧。
		*/
		pBufferManage[streamtype]->m_FrameBufferUser[userid].throwframcount ++;
		if(pBufferManage[streamtype]->m_FrameBufferUser[userid].throwframcount < 100)
		{
			return 0;
		}
		VideoFrameHeader *ptr =(VideoFrameHeader *)(*buffer);

		//printf("type:%d,%d\n",ptr->m_FrameType,ptr->m_nVHeaderFlag);
		if(m_frameinfo->Flag == P_FRAME)
		{

			
			if(ptr->m_FrameType == BASE_PSLICE_REFBYENHANCE||\
				ptr->m_FrameType == ENHANCE_PSLICE_NOTFORREF)
			{
				/*丢弃非关键帧*/
				printf("throwfram type:%d,diffpos:%d\n",ptr->m_FrameType,pBufferManage[streamtype]->m_FrameBufferUser[userid].diffpos);
				return -2;/*返回值为-2，表示读和写指针差值已经很大，网络需继续
							取数据，底层做跳帧处理*/
			}
		
		}
	}
	#endif
	return retval;
	
}
/*****************************************************************************
函数功能:复位指定消费者用户的读指针信息
输入参数:streamtype 为码流类型0 :720  1:VGA 2:QVGA
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int ResetUserData2IFrame(int channel,int streamtype, int userid)
{

	int retval= S_FAILURE;

	if((userid <1)||(startupdate == 0) || (userid >= MAX_BUFUSER_ID))
	{
		//printf(" invalid user id %d \n", userid);
		return S_FAILURE;
	}
	if(( streamtype >= MAX_CH_STREAM)||( streamtype <0 ))
	{
		return S_FAILURE;
	}
	if(streamtype==0)
	{
		RequestIFrame(streamtype,0);
	}
	else if(streamtype==1)
	{		
		RequestIFrame(1,0);
	}
	else if(streamtype==2){
		RequestIFrame(1,0);
	}
	usleep(200000);

	if(pBufferManage[streamtype] != NULL)
	{
		retval = pBufferManage[streamtype]->ResetUserInfo(userid);
	}


	return retval;

}
/*****************************************************************************
函数功能:强制生成I帧
输入参数:streamtype 为码流类型0 :720  1:VGA 2:QVGA
				time: 0,立即生成，
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/

int RequestIFrame (int streamtype,int time)
{	
	 HI_MPI_VENC_RequestIDR (streamtype,HI_TRUE);
	 return 0;
	
}
/*
下面是音频编解码接口，语音对讲时IPC 向平台发送音频为上行，
平台向IPC发送音频为下行。
上行时函数调用步骤:1,StartAudioEncode  2,GetSendNetFrame 3,StopAudioEncode
下行时函数调用步骤:1,StartAudioDecode  2,SendAudioStreamToDecode 3,StopAudioDecode

设置音频输入输出的大小SetAudioVolume
*/

int GetAudioParam(AudioParm *param)
{
	if((param == NULL)||(paudio == NULL))
	{
		return -1;
	}
	return paudio->GetAudioParam(param);
}

/*****************************************************************************
函数名称:StartAudioDecode
函数功能:开始音频解码
输入参数:Audiotype   0:G711  1:G726 
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:
******************************************************************************/
int	StartAudioDecode(int Audiotype)
{
	if(paudio != NULL)
	{
		//return paudio->StartAudioDecode();/*解码暂时只支持g711*/
		//AudioControl(1);
		printf(".....StartAudioDecode.......\n");
        return paudio->AudioOutPutOnOff(1);		
	}
	return 0;
}
/*****************************************************************************
函数名称:SendAudioStreamToDecode
函数功能:发送音频数据解码，
输入参数:
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int	SendAudioStreamToDecode(unsigned char *buffer,int len,int block)
{
	if((paudio != NULL)&&(buffer != NULL))
	{
		return paudio->SendAudioStreamToDecode(buffer,len,block);
	}
	return -1;
}

/*****************************************************************************
函数名称:StopAudioDecode
函数功能:结束音频解码
输入参数:
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int	StopAudioDecode()
{
	if(paudio != NULL)
	{
		//return paudio->StopAudioDecode();
		printf("......StopAudioDecode.......\n");
        return paudio->AudioOutPutOnOff(0);
	}
	return 0;
}

/*****************************************************************************
函数名称:StartAudioEncode
函数功能:开始音频编码
输入参数:Audiotype   0:G711  1:G726 
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int	StartAudioEncode(int Audiotype)
{
	if(paudio != NULL)
	{
		return paudio->StartAudioEncode(Audiotype);
	}
	return -1;
}
/*****************************************************************************
函数名称:GetAudioData
函数功能:从音频buffer中取一帧数据
输入参数:userid 用户id 最大10，isfirst 是否是第一次获取数据，buf 数据audiolen 数据长度
输出参数:无
返	回	 值:0:成功	-1:失败
使用说明: isfirst 每次开始对讲时第一次为true，后续为false

******************************************************************************/

int	GetAudioData(int userid,bool isfirst,unsigned char *buf,int *audiolen)
{
	if(paudio != NULL)
	{
		//return paudio->GetAudioFromBuffer(userid,isfirst,buf,audiolen);
	}
	return -1;
}

/*****************************************************************************
函数名称:StopAudioEncode
函数功能:结束音频解码
输入参数:
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int	StopAudioEncode()
{
	if(paudio != NULL)
	{
		return paudio->StopAudioEncode();
	}
	return -1;
}

/*****************************************************************************
函数名称:SetAudioVolume
函数功能:
输入参数::0 :input  1:output  vol (0~31) 	  25为默认声音vol 越大声音越大
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int	SetAudioVolume(int inoutput,unsigned int vol)
{
	if(paudio != NULL)
	{
		
		if(inoutput == 0)
		{
			return paudio->SetInPutVolume(vol);
		}
		else if(inoutput == 1)
		{
			return paudio->SetOutPutVolume(vol);
		}		 
		
	}
	return -1;
}
static int currentVol=26;
int SetAudioVolumeLevel(int level){
	//22-31 default:26
	int vol;
	if(level<0 ||level>100){
		return -1;
	}
	vol=level/10+22;
	if(vol!=currentVol){
		printf("tell kb the volume:%d\n",vol);
		if(paudio != NULL)
		{
			currentVol=vol;			
			paudio->SetOutPutVolume(vol);	
			paudio->StartPlayFile(VOLUME_SOUND,NULL);
			return 0;
		}
	}
	else{
		printf("the voice is the same\n");
		return 0;
	}
	printf("seting failure;paudio is null\n");
	return -1;
}

bool AudioGetSpeeker()
{
	
	if(paudio == NULL)
	{
		return false;
	}
	return paudio->AudioGetSpeeker();

}
bool AudioReleaseSpeeker()
{
	
	if(paudio == NULL)
	{
		return false;
	}
	return paudio->AudioReleaseSpeeker();

}

/*****************************************************************************
函数功能:查找BufferManage里已经存在的最新的I帧，不生成新I帧
输入参数:streamtype 为码流类型0 :720  1:VGA 2:QVGA
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
******************************************************************************/
int ResetUserData2CurrentIFrame(int channel,int streamtype, int userid)
{	
	int retval= S_FAILURE;

	if((userid <1)||(startupdate == 0) || (userid >= MAX_BUFUSER_ID))
	{
		return S_FAILURE;
	}
	if(( streamtype >= MAX_CH_STREAM)||( streamtype <0 ))
	{
		return S_FAILURE;
	}
	printf("try to find current i frame 0 :720  1:VGA 2:QVGA, streamtype:%d\n",streamtype);
	if(pBufferManage[streamtype] != NULL)
	{
		retval = pBufferManage[streamtype]->ResetUserInfo(userid);
	}
	return retval;
}

int UpdateCoverlayOsd()
{
	int i =0;
	VIDEOOSDINSERT		CoverSet;
	PubGetSysParameter(SYSOSDINSERT_SET, (void*)&CoverSet);
	RECT_S rect;
	int valid = -1;
	for(int k=0;k<4;k++)
	{
		if(k == 0)
		{
			rect.s32X = CoverSet.m_CoverLay[i].m_u16X;
			rect.s32Y = CoverSet.m_CoverLay[i].m_u16Y;
			rect.u32Width = CoverSet.m_CoverLay[i].m_u16Width;
			rect.u32Height = CoverSet.m_CoverLay[i].m_u16Height;
			valid = CoverSet.m_CoverLay[i].m_u8OverValid;
		}
		else if(k == 1)
		{
			rect.s32X = CoverSet.m_CoverLay2[i].m_u16X;
			rect.s32Y = CoverSet.m_CoverLay2[i].m_u16Y;
			rect.u32Width = CoverSet.m_CoverLay2[i].m_u16Width;
			rect.u32Height = CoverSet.m_CoverLay2[i].m_u16Height;
			valid = CoverSet.m_CoverLay2[i].m_u8OverValid;
		}
		else if(k == 2)
		{
			rect.s32X = CoverSet.m_CoverLay3[i].m_u16X;
			rect.s32Y = CoverSet.m_CoverLay3[i].m_u16Y;
			rect.u32Width = CoverSet.m_CoverLay3[i].m_u16Width;
			rect.u32Height = CoverSet.m_CoverLay3[i].m_u16Height;
			valid = CoverSet.m_CoverLay3[i].m_u8OverValid;
		}
		else if(k == 3)
		{
			rect.s32X = CoverSet.m_CoverLay4[i].m_u16X;
			rect.s32Y = CoverSet.m_CoverLay4[i].m_u16Y;
			rect.u32Width = CoverSet.m_CoverLay4[i].m_u16Width;
			rect.u32Height = CoverSet.m_CoverLay4[i].m_u16Height;
			valid = CoverSet.m_CoverLay4[i].m_u8OverValid;
		}

		if(valid)
		{
			printf("x:%d,y:%d,w:%d,h:%d,x+w:%d,y+h:%d\n",rect.s32X,rect.s32Y,rect.u32Width,rect.u32Height,\
				rect.s32X+rect.u32Width,rect.s32Y+rect.u32Height);
		
			int tmp_x = rect.s32X + rect.u32Width;
			int tmp_y = rect.s32Y + rect.u32Height;
#if defined (PT_IPC)
			if(!(CONFIG_FLIP&ConfigInfo.SupportInfo))

#elif defined (V74_NORMAL_IPC)
			if(1)
#else
			if(CONFIG_FLIP&ConfigInfo.SupportInfo)
#endif




			{
				if(picturemode == 1)
				{
					
				}
				else if(picturemode == 2)
				{
					if(tmp_y>480)
					{
						rect.s32Y = 0;
					}
					else
					{
						rect.s32Y = 480-tmp_y;
					}				
				}
				else if(picturemode == 3)
				{
					if(tmp_x>640)
					{
						rect.s32X = 0;
					}
					else
					{
						rect.s32X = 640-tmp_x;
					}

					
				}
				else if(picturemode == 4)
				{
					if(tmp_y>480)
					{
						rect.s32Y = 0;
					}
					else
					{
						rect.s32Y = 480-tmp_y;
					}	
					if(tmp_x>640)
					{
						rect.s32X = 0;
					}
					else
					{
						rect.s32X = 640-tmp_x;
					}

				}	
			}
			else
			{
				if(picturemode == 4)
				{
					
				}
				else if(picturemode == 1)
				{
					if(tmp_y>480)
					{
						rect.s32Y = 0;
					}
					else
					{
						rect.s32Y = 480-tmp_y;
					}				
				}
				else if(picturemode == 2)
				{
					if(tmp_x>640)
					{
						rect.s32X = 0;
					}
					else
					{
						rect.s32X = 640-tmp_x;
					}

					
				}
				else if(picturemode == 3)
				{
					if(tmp_y>480)
					{
						rect.s32Y = 0;
					}
					else
					{
						rect.s32Y = 480-tmp_y;
					}	
					if(tmp_x>640)
					{
						rect.s32X = 0;
					}
					else
					{
						rect.s32X = 640-tmp_x;
					}

				}		
			}
			CreateFullVideoCoverRegion(i, rect,k);
		}
	}	
	return 0;

}








/*****************************************************************************
函数名称:VideoMirrorFlipSet
函数功能:设置图像镜像和翻转
输入参数:mode   4:正常1 翻转2 镜像3 镜像加翻转
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/

int VideoMirrorFlipSet(int mode)
{
	printf("VideoMirrorFlipSet mode:%d\n",mode);
	if(picturemode != mode)
	{
		pVideo->VideoMirrorFlipSet(mode);
		picturemode = mode;
		UpdateCoverlayOsd();
	}

	return 0;
}
int VideoMirrorSet(bool set)
{
	pVideo->VideoMirrorSet(set);
	return 0;
}
int VideoFlipSet(bool set)
{
	pVideo->VideoFlipSet(set);
	return 0;
}
/*****************************************************************************
函数名称:VideoSaturationSet
函数功能:设置图像饱和度
输入参数:value   0~255 
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/

int VideoSaturationSet(int value)
{

	//return pVideo->m_pISP->SetVideoSaturation((unsigned int)value);
	return 0;
}
/*****************************************************************************
函数名称:VideoColourSet
函数功能:设置图像彩色或者黑白
输入参数:value   1:彩色模式2:黑白模式
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/

int VideoColourSet(int value)
{	
	if(value == 1)
	{
		
		return pVideo->m_pISP->SetVideoSaturation(139);
	}
	else
	{
		return pVideo->m_pISP->SetVideoSaturation(0);
	}
	return -1;
}

/*****************************************************************************
函数名称:VideoBrightnessSet
函数功能:设置图像亮度
输入参数:value   0~255 
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int VideoBrightnessSet(int value)
{

	//return pVideo->m_pISP->SetVideoBrightness((unsigned int)value);
	return 0;
}
/*****************************************************************************
函数名称:VideoContrastSet
函数功能:设置对比度
输入参数:value   0~255 
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/
int VideoContrastSet(int value)
{

	//return pVideo->m_pISP->SetVideoContrast((unsigned int)value);
	return 0;
}

/*****************************************************************************
函数名称:VideoPowerSet
函数功能:设置电压 暂时无效
输入参数:value   1:50HZ 2:60HZ
输出参数:无
返  回   值:0:成功  -1:失败
使用说明:

******************************************************************************/

int VideoPowerSet(int value)
{
	unsigned short 	PowerFreq = 60;
	bool			enable = true;	
	if(value == 1)
	{
		PowerFreq = 50;
		enable =true;
	}
	else if(value == 2)
	{
		PowerFreq = 60;
		enable =true;
	}
	else if(value== 3)
	{
		enable =true;
		PowerFreq = 0;
	}
	else
	{
		enable = false;
	}
	pVideo->SetVideoAntiFlickerAttr(enable,PowerFreq);

	return  0;
}

int SetVideoChnAnaLog(int brightness,int contrast,int saturation)
{
	
	printf("file=%s,func=%s,line=%d b=%d c=%d s=%d\n",__FILE__,__FUNCTION__,__LINE__,brightness,contrast,saturation);
	return  pVideo->m_pISP->SetVideoChnAnaLog(brightness,contrast,saturation);
}



#ifdef __cplusplus
 };
#endif

