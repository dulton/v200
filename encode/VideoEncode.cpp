


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
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dlfcn.h>
#include "VideoEncode.h"

#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_af.h"


char SensorTable[8][32]={\
	{"/lib/libsns_sc1135.so"},//{"/lib/libsns_ov9712.so"},
	{"/lib/libsns_soih22.so"},
	{"/lib/libsns_hm1375_720p.so"},
	{"/lib/libsns_ar0130_720p.so"},
	{""},
	{""},
	{""},
	{""}	
};
int sensorid=libsns_ov9712_720p;
extern BufferManage 		*pBufferManage[MAX_REC_CHANNEL];
extern int	    startupdate;
extern int	NotFeedDog;
extern int RequestIF;
extern 	bool		mdalarmupload ;

int		talk_w_ptr=0;
char	*TalkEncodeBufferPool=NULL;
int		NeedRebootEncode = 0;
extern DeviceConfigInfo 	ConfigInfo;


#define savestream
#ifdef savestream
int SaveStreamToFile(FILE *fpH264File, VENC_STREAM_S *pstStream)
{
    HI_U32 i;

    if((fpH264File == NULL)||(pstStream == NULL ))
    {
    	return -1;
    }
	#if 0
    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        fwrite(pstStream->pstPack[i].pu8Addr[0],
               pstStream->pstPack[i].u32Len[0], 1, fpH264File);

        fflush(fpH264File);

        if (pstStream->pstPack[i].u32Len[1] > 0)
        {
            fwrite(pstStream->pstPack[i].pu8Addr[1],
                   pstStream->pstPack[i].u32Len[1], 1, fpH264File);

            fflush(fpH264File);
        }
    }
  #else
      for (i = 0; i < pstStream->u32PackCount; i++)
    {
        fwrite(pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,
               pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset, 1, fpH264File);

        fflush(fpH264File);
    }
  #endif 
	return 0;
}

#endif


void *VideoEncoderThreadEntry(void *para)
{
	
	encoderstream_t *pStream = (encoderstream_t *)para;
	printf("function: %s threadid %d  ,line:%d\n",__FUNCTION__, (unsigned)pthread_self(),__LINE__);	
	pStream->pEncoder->VideoStreamThreadBody(pStream->venc);
	return NULL;
}
/*****************************************************************************
函数名称:VideoEncode
函数功能:
输入参数:无
输出参数:无
返	回	 值: 
使用说明: 内部部调用，

******************************************************************************/

VideoEncode::VideoEncode()
{

	
	memset(&m_EncoderStream,0x0,MAX_CH_STREAM*sizeof(encoderstream_t));
	
	m_pISP = new CVideoISP();
}

/*****************************************************************************
函数名称:Hi3518SysInit
函数功能:系统初始
输入参数:无
输出参数:无
返	回	 值:  0: 成功-1: 失败
使用说明: 内部部调用，

******************************************************************************/

int VideoEncode::Hi3518SysInit()
{
#if 0
	MPP_SYS_CONF_S stSysConf = {0};
	VB_CONF_S stVbConf ={0};
	int  retval = -1;
	HI_S32 ret = 0;
	HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

	HI_U32 u32BlkSize;
	stVbConf.u32MaxPoolCnt = 96;
    /*video buffer*/   
    u32BlkSize = 1280*720*2;
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 10;

    u32BlkSize =320*240*2;
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 4;


	ret = HI_MPI_VB_SetConf(&stVbConf);
	if( ret != 0)
	{
		printf("InitPlatform HI_MPI_VB_SetConf failed %0x\n",ret);
    	return HI_FAILURE;
	}
	ret = HI_MPI_VB_Init();
	if( ret != 0)
	{
		printf("InitPlatform HI_MPI_VB_Init failed %0x\n",ret);
    	return HI_FAILURE;
	}
		
	stSysConf.u32AlignWidth = 64;
	if (HI_MPI_SYS_SetConf(&stSysConf) != 0)
	{
		KEY_INFO(("InitPlatform HI_MPI_SYS_SetConf failed\n"));
    	return HI_FAILURE;
	}
		
	 if ((retval = HI_MPI_SYS_Init()) != 0)
	{
		KEY_INFO(("InitPlatform HI_MPI_SYS_Init failed \n"));
		printf(" failure code : %x \n", retval);
    	return HI_FAILURE;
	}
	MPP_VERSION_S pstVersion;
	if(HI_MPI_SYS_GetVersion(&pstVersion) != 0)
	{
		printf("Get Version Failure  \n");
	}
	printf("MPP Version  %s  \n", pstVersion.aVersion);
	#endif
	return HI_SUCCESS;
	
}
int VideoEncode::SensorInit()
{
	
	
	#if 0
	sensor_init();
	sensor_mode_set(0);
	sensor_register_callback();
	sensorid = 0;
	#else
	sensorid= detect_sensor();
	printf("sensor id ############################:%d\n",sensorid);
	if((sensorid>8)||(sensorid<0))
	{
		return -1;
	}
	void *handle;	 
	void (*dl_sensor_init)();	
	char *error;
	printf("+++++++++++++++++++++sensorid:%d,%s\n",sensorid,SensorTable[sensorid]);
	handle = dlopen (SensorTable[sensorid], RTLD_LAZY|RTLD_GLOBAL);
	if (!handle)	
	{		 
		fprintf (stderr, "dlopen %s %s\n",SensorTable[sensorid], dlerror());		
		return -1;			  
	}
	
	dl_sensor_init = (void (*)())dlsym(handle, "sensor_init");	  
	if ((error = dlerror()) != NULL) 
	{		 
		fprintf (stderr, " sensor_init %s\n", error);		 
		return -1;	  
	}
	(*dl_sensor_init)();
	

	
	int (*dl_sensor_mode_set)(unsigned char mode);
	dl_sensor_mode_set = (int (*)(unsigned char ))dlsym(handle, "sensor_mode_set");    
	if ((error = dlerror()) != NULL) 
	{		 
		fprintf (stderr, " sensor_mode_set %s\n", error);		 
		return -1;	  
	}
	(*dl_sensor_mode_set)(0);
	int (*dl_sensor_register_callback)();
	dl_sensor_register_callback = (int (*)())dlsym(handle, "sensor_register_callback");   
	if ((error = dlerror()) != NULL) 
	{		 
		fprintf (stderr, "sensor_register_callback %s\n", error);		
		return -1;	  
	}
	(*dl_sensor_register_callback)();
	#endif
	return 0;

}

/*****************************************************************************
函数名称:VideoViConfig
函数功能:vi输入设置
输入参数:无
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明: 内部部调用，

******************************************************************************/
int VideoEncode::VideoViConfig()
{return 0;}

/*****************************************************************************
函数名称:InitBuffer
函数功能:初始化buffer
输入参数:无
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明: 内部部调用，

******************************************************************************/

int VideoEncode::InitBuffer()
{
	m_EncoderStream[0].pBufferManage = NULL;
	m_EncoderStream[1].pBufferManage = NULL;
	pBufferManage[0] = new BufferManage(&GetSysTime, 0);
	pBufferManage[1] = new BufferManage(&GetSysTime, 0);	
	pBufferManage[2] = new BufferManage(&GetSysTime, 0);
	if((pBufferManage[0] == NULL )||(pBufferManage[1]==NULL)||(pBufferManage[2]==NULL))
	{
		return -1;
	}	
	if(pBufferManage[0]->CreateBufferPool(RES_720P, 1, 0) == -1)
	{	
		return -1;
	}
	if(pBufferManage[1]->CreateBufferPool(RES_QVGA, 1, 0) == -1)
	{	
		return -1;
	}
	if(pBufferManage[2]->CreateBufferPool(RES_QVGA, 1, 0) == -1)
	{	
		return -1;
	}	
	m_EncoderStream[0].pBufferManage = pBufferManage[0];
	m_EncoderStream[1].pBufferManage = pBufferManage[1];	
	m_EncoderStream[2].pBufferManage = pBufferManage[2];
	return 0;
}



/*****************************************************************************
函数名称:InitVideoEncoder
函数功能:初始化视频编码
输入参数:无
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明: 内部调用，

******************************************************************************/

int VideoEncode::InitVideoEncoder()
{	
	 HI_S32 s32Ret = HI_FAILURE;
	/******************************************
	 step  1: init sys variable 
	******************************************/

	HI_U32 u32BlkSize;
	VB_CONF_S stVbConf;
	memset(&stVbConf,0,sizeof(VB_CONF_S));
	stVbConf.u32MaxPoolCnt = 96;//96;
	/*video buffer*/   
	u32BlkSize = 1280*720*2;//u32BlkSize = 1280*720*2;
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = 10;//10;
#if 1
	u32BlkSize =320*240*2;
	stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt = 4;
    u32BlkSize = 320*240*2;
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 4;	
#endif
	/******************************************
	 step 2: mpp system init. 
	******************************************/

	HI_MPI_SYS_Exit();
	HI_MPI_VB_Exit();



	s32Ret = HI_MPI_VB_SetConf(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VB_SetConf ffailed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VB_Init();
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VB_Init failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	MPP_SYS_CONF_S stSysConf = {0};
	stSysConf.u32AlignWidth = 64;//16;
	s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_SYS_SetConf failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_SYS_Init();
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_SYS_Init failed with %#x!\n", s32Ret);
//		return HI_FAILURE;
	}
#if 1	
	MPP_VERSION_S pstVersion;
	if(HI_MPI_SYS_GetVersion(&pstVersion) != 0)
	{
		printf("Get Version Failure  \n");
	}
	printf("MPP Version  %s  \n", pstVersion.aVersion);
#endif
	/******************************************
	 step 3: start vi dev & chn to capture
	******************************************/

/***********************************************/

	sensorid=libsns_jxh22_720p;//detect_sensor();
	printf("sensor id ############################:%d\n",sensorid);

/***********************************************/
	m_pISP->ISP_Run();
	Comm_VI_StartDev(sensorid);
		
	

	
	VI_CHN_ATTR_S stChnAttr;
	ROTATE_E enRotate = ROTATE_NONE;
	/* step  5: config & start vicap dev */
  //  memcpy(&stChnAttr.stCapRect, pstCapRect, sizeof(RECT_S));
	stChnAttr.stCapRect.s32X=0;
	stChnAttr.stCapRect.s32Y=0;
	stChnAttr.stCapRect.u32Width=1280;
	stChnAttr.stCapRect.u32Height=720;//720;
	
	
	stChnAttr.enCapSel = VI_CAPSEL_BOTH;
	/* to show scale. this is a sample only, we want to show dist_size = D1 only */
	stChnAttr.stDestSize.u32Width = 1280;
	stChnAttr.stDestSize.u32Height =720;//720;
	stChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;   /* sp420 or sp422 */

	stChnAttr.bMirror = HI_FALSE;
	stChnAttr.bFlip = HI_FALSE;

//	stChnAttr.bChromaResample = HI_FALSE;
//	stChnAttr.s32SrcFrameRate = 25;
//	stChnAttr.s32FrameRate = 20;

      stChnAttr.s32SrcFrameRate = -1;
      stChnAttr.s32DstFrameRate = -1;
	stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
	
	s32Ret = HI_MPI_VI_SetChnAttr(0, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		printf("631failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	if(ROTATE_NONE != enRotate)
	{
		s32Ret = HI_MPI_VI_SetRotate(0, enRotate);
		if (s32Ret != HI_SUCCESS)
		{
			printf("HI_MPI_VI_SetRotate failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
	}
	
	s32Ret = HI_MPI_VI_EnableChn(0);
	if (s32Ret != HI_SUCCESS)
	{
		printf("648failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}


	VPSS_GRP_ATTR_S stVpssGrpAttr;
	stVpssGrpAttr.u32MaxW = 1280;
	stVpssGrpAttr.u32MaxH = 720;//720;
//	stVpssGrpAttr.bDrEn = HI_FALSE;
//	stVpssGrpAttr.bDbEn = HI_FALSE;
//	stVpssGrpAttr.bIeEn = HI_TRUE;
//	stVpssGrpAttr.bNrEn = HI_TRUE;
//	stVpssGrpAttr.bHistEn = HI_TRUE;
//	stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
	stVpssGrpAttr.bIeEn = HI_FALSE;
	stVpssGrpAttr.bNrEn = HI_TRUE;
	stVpssGrpAttr.bHistEn = HI_FALSE;
	stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;

	stVpssGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	VPSS_GRP VpssGrp =0;
//	VPSS_GRP_PARAM_S stVpssParam;
	s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != HI_SUCCESS)
	{
	  printf("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
	  return HI_FAILURE;
	}
#if 0	
	/*** set vpss param ***/
	s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
	if (s32Ret != HI_SUCCESS)
	{
	  printf("failed with %#x!\n", s32Ret);
	  return HI_FAILURE;
	}

	stVpssParam.u32MotionThresh = 0;

	s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
	if (s32Ret != HI_SUCCESS)
	{
	  printf("failed with %#x!\n", s32Ret);
	  return HI_FAILURE;
	}
#else
 /*   VPSS_NR_PARAM_U unNrParam = {{0}};
    s32Ret = HI_MPI_VPSS_GetNRParam(VpssGrp, &unNrParam);
	if (s32Ret != HI_SUCCESS)
		{
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
		}
	s32Ret = HI_MPI_VPSS_SetNRParam(VpssGrp, &unNrParam);
	if (s32Ret != HI_SUCCESS)
		{
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
		}
*/

#endif
	s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != HI_SUCCESS)
	{
	  printf("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
	  return HI_FAILURE;
	}

	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	stSrcChn.enModId = HI_ID_VIU;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;
	
	stDestChn.enModId = HI_ID_VPSS;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = 0;
	
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}


	VPSS_CHN_ATTR_S stVpssChnAttr;
	int VpssChn =0; 
	memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
//	stVpssChnAttr.bFrameEn = HI_FALSE;
//	stVpssChnAttr.bSpEn    = HI_TRUE;	 
//	s32Ret = VideoVpssEnableChn(VpssGrp, VpssChn, &stVpssChnAttr, HI_NULL, HI_NULL);
	
	VpssChn = 0;	
	VPSS_CHN_MODE_S stVpssChnMode;
	stVpssChnMode.enChnMode 	= VPSS_CHN_MODE_USER;
	stVpssChnMode.bDouble		= HI_FALSE;
	stVpssChnMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stVpssChnMode.u32Width		= 1280;//320;
	stVpssChnMode.u32Height 	= 720;//720;//240;
	stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;//COMPRESS_MODE_SEG;

	  stVpssChnAttr.s32SrcFrameRate = -1;
	  stVpssChnAttr.s32DstFrameRate = -1;
	
	s32Ret = VideoVpssEnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
	if (HI_SUCCESS != s32Ret)
	{
		printf("VideoVpssEnableChn failed!\n"); 
	
	}
    VpssChn = 1;
    stVpssChnMode.enChnMode       = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble         = HI_FALSE;
    stVpssChnMode.enPixelFormat   = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVpssChnMode.u32Width        = 640;
    stVpssChnMode.u32Height       = 480;
    stVpssChnMode.enCompressMode  = COMPRESS_MODE_SEG;
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;	
    s32Ret = VideoVpssEnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
	if (HI_SUCCESS != s32Ret)
	{
		printf("VideoVpssEnableChn failed! %x\n",s32Ret); 
	
	}

#if 1
	VpssChn = 2;
	stVpssChnMode.enChnMode 	  = VPSS_CHN_MODE_USER;
	stVpssChnMode.bDouble		  = HI_FALSE;
	stVpssChnMode.enPixelFormat   = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stVpssChnMode.u32Width		  = 320;
	stVpssChnMode.u32Height 	  = 240;
	stVpssChnMode.enCompressMode  = COMPRESS_MODE_SEG;
	stVpssChnAttr.s32SrcFrameRate = -1;
	stVpssChnAttr.s32DstFrameRate = -1; 
	s32Ret = VideoVpssEnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
	if (HI_SUCCESS != s32Ret)
	{
		printf("VideoVpssEnableChn failed! %x\n",s32Ret); 
	
	}

#endif
	if(InitBuffer()== -1)
	{
		return -1;
	}
	if(m_pISP!=NULL)
		m_pISP->H42_ISPseting();
	return HI_SUCCESS;


}


/*****************************************************************************
函数名称:StartVideoEncoder
函数功能:初始化视频编码
输入参数:pEncPara 相见参数结构体
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明: 外部调用，
编码和系统初始化后才可以调用

******************************************************************************/

int VideoEncode::StartVideoEncoder(EncodePara_S *pEncPara)
{
	if(pEncPara == NULL)
	{
		return -1;
	}
	HI_S32 s32Ret;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = 0;
	VENC_GRP VencGrp = 0;
	VENC_CHN VencChn = 0;  
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC; 
	PIC_SIZE_E enSize = PIC_HD720;
	VIDEO_RC_E enRcMode = RC_VBR;
	HI_U32	framerate = 0;
	HI_U32 	bitrate = 0 ;



	if(pEncPara->resolution == 0)/*720 P*/
	{
	    VpssChn = 0;
	    VencGrp = 0;
	    VencChn = 0;
		enSize = PIC_HD720;	
		if((pEncPara->bitrate>1024*10)||(pEncPara->bitrate<100))
		{
			pEncPara->bitrate =1024*2;//1024;
		}
		#ifdef V200_NORMAL_IPC
			pEncPara->bitrate=800;
		#endif

	}
	#if 0
	else if(pEncPara->resolution == 1)/*VGA*/
	{
	    VpssChn = 1;
	    VencGrp = 1;
	    VencChn = 1;
		enSize = PIC_VGA;
		if((pEncPara->bitrate>640*10)||(pEncPara->bitrate<100))
		{
			pEncPara->bitrate =640;
		}
	}	
	#endif
	else if(pEncPara->resolution == 2)/*QVGA*/
	{
	    VpssChn = 1;
	    VencGrp = 0;
	    VencChn = 1;
		enSize = PIC_VGA;
		if((pEncPara->bitrate>320*10)||(pEncPara->bitrate<50))
		{
			pEncPara->bitrate =320;
		}
	}	




	if(pEncPara->VbrOrCbr == 0 )/*1--CBR, 0--VBR*/
	{
		enRcMode = RC_VBR;
	}
	else if(pEncPara->VbrOrCbr == 1 )
	{
		enRcMode = RC_CBR;
		pEncPara->bitrate =pEncPara->bitrate;
	}
	if( pEncPara->framerate<6)
	 	pEncPara->bitrate =400;
	if(pEncPara->framerate>25)
	{
		framerate = 25;
	}
	else if(pEncPara->framerate<5)
	{	
		framerate = 5;
	}
	else
	{
	
		framerate = pEncPara->framerate;
	}
	//framerate = (pEncPara->framerate)>30?30:pEncPara->framerate;
	bitrate = pEncPara->bitrate;
	enRcMode = RC_VBR;

	unsigned int gop =5*framerate;
	#ifdef ADJUST_STREAM
	gop =IframeGop*framerate;
	#endif
	
	printf("---------------------------\n");

	printf("resolution:%d\n",pEncPara->resolution);
	printf("framerate:%d\n",framerate);
	printf("VbrOrCbr:%d\n",pEncPara->VbrOrCbr);
	printf("bitrate:%d\n",bitrate);
	printf("gop:%d\n",gop);
	printf("---------------------------\n");
	

    s32Ret = VideoVencStart(VencGrp, VencChn, enNorm,enSize, enRcMode,framerate,bitrate,gop);
    if (HI_SUCCESS != s32Ret)
    {
        printf("875Start Venc failed!\n");        
    }

    s32Ret = VideoVencBindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("881Start Venc failed!\n");
       
    }
	StartStreamThread(VencChn);

	

	return 0;
}
int VideoEncode::StopVideocEncode()
{
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;    
    VENC_GRP VencGrp;
    VENC_CHN VencChn;

	for(unsigned int i=0;i<MAX_CH_STREAM;i++)
	{
		if((m_EncoderStream[i].EncodeStat)||(m_EncoderStream[i].ThreadStat))
		{
			pthread_join(m_EncoderStream[i].pid, NULL);
			m_EncoderStream[i].pid = 0;
		}
		m_EncoderStream[i].EncodeStat = 0;
		m_EncoderStream[i].ThreadStat = 0;
		VpssChn = i;
		VencGrp = i;   
		VencChn = i;
		VpssGrp = i;
		VideoVencUnBindVpss(VencGrp, VpssGrp, VpssChn);
		VideoVencStop(VencGrp,VencChn);

	}	
	return 0;
}

int VideoEncode::StartStreamThread(int venc)
{
	
	if(venc >= MAX_CH_STREAM)
	{
		printf("StartStreamThread %s ,%d Failed\n ",__FUNCTION__,__LINE__);
		return -1;
	}
	m_EncoderStream[venc].pEncoder = this;
	m_EncoderStream[venc].venc= venc;	
	if(pthread_create(&m_EncoderStream[venc].pid, NULL, VideoEncoderThreadEntry, (void *)(&m_EncoderStream[venc])) == 0)//创建线程
	{
		printf("StartVideoEncoder %d thread success!\n",venc);
	}	
	return 0;
}

int VideoEncode::VideoStreamThreadBody(int venc)
{	
#if 1
        HI_S32 i=venc;
     VENC_CHN_ATTR_S stVencChnAttr;
 //   SAMPLE_VENC_GETSTREAM_PARA_S *pstPara;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set  read_fds;
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    //HI_CHAR aszFileName[VENC_MAX_CHN_NUM][64];
    //FILE *pFile[VENC_MAX_CHN_NUM];
   // char szFilePostfix[10];
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
 //   VENC_CHN VencChn;
  //  PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];
    s32Ret = HI_MPI_VENC_GetChnAttr(0, &stVencChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VENC_GetChnAttr chn0 failed with %#x!\n",s32Ret);
            return 0;
        }
		#if 0
        enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;
           strcpy(szFilePostfix, ".h264");
           sprintf(aszFileName[i], "stream_chn%d%s", i, szFilePostfix);
        pFile[i] = fopen(aszFileName[i], "wb");
        if (!pFile[i])
        {
            printf("open file[%s] failed!\n",aszFileName[i]);
            return NULL;
        }
		#endif
        /* Set Venc Fd. */
        VencFd[i] = HI_MPI_VENC_GetFd(i);
        if (VencFd[i] < 0)
        {
            printf("HI_MPI_VENC_GetFd failed with %#x!\n",  VencFd[i]);
            return NULL;
        }
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }
    while (1)
    {
        FD_ZERO(&read_fds);
      
       FD_SET(VencFd[i], &read_fds);
   

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            printf("select failed!\n");
            break;
        }
		else if (s32Ret == 0)
        {
            printf("get venc %d stream time out!!!\n",i);
            continue;
        }
        else
        {
           
            if (FD_ISSET(VencFd[i], &read_fds))
                {
                   
                    memset(&stStream, 0, sizeof(stStream));
                    s32Ret = HI_MPI_VENC_Query(i, &stStat);
                    if (HI_SUCCESS != s32Ret)
                    {
                        printf("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
                        break;
                    }
			if(0 == stStat.u32CurPacks)
					{
						 printf("NOTE: Current  frame is NULL!\n");
						  continue;
					}
			 stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        printf("malloc stream pack failed!\n");
                        break;
                    }
					 stStream.u32PackCount = stStat.u32CurPacks;
                    s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                       printf("HI_MPI_VENC_GetStream failed with %#x!\n",s32Ret);
                        break;
                    }
					if(GetDeviceStatus())
					{
						if(venc==1)
						{
						   m_EncoderStream[1].pBufferManage->PutOneVFrameToBuffer((void *)(&stStream));
						   m_EncoderStream[2].pBufferManage->PutOneVFrameToBuffer((void *)(&stStream));
						   
						}
						else
						{
						   m_EncoderStream[venc].pBufferManage->PutOneVFrameToBuffer((void *)(&stStream));
						}
					}

           	        // 
        	       //  SaveStreamToFile(pFile[i],&stStream);
					 s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }
		  			 free(stStream.pstPack);
                    stStream.pstPack = NULL;
            	}
		}
    	}
		
	// fclose(pFile[i]);
	
					
#endif
#if 0
  fd_set read_fds;
	int VencFd;
	int s32ret = -1;
	
	VENC_CHN VeChn =venc;
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stVStream;
	pthread_detach(pthread_self());/*退出线程释放资源*/
	VENC_PACK_S *pPack = NULL;
	unsigned int packCount=10;
	printf("\n\nGetAVStreamInOneChn Start!! venc:%d\n\n",venc);
	
#ifdef ADJUST_STREAM
	
	printf(">>>>>>>>>>>>>>>>> SetH264eRefMode:%08x\n",HI_MPI_VENC_SetH264eRefMode(venc,H264E_REF_MODE_4X));
#endif

	pPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S)*10);
	if (pPack == NULL) 
	{
		printf("\n\nGet_AVStreamInOneChn malloc err:::::::::::::::::::\n");
		return -1;
	}	
	
	stVStream.pstPack = pPack;
	m_EncoderStream[venc].EncodeStat = 1;
	m_EncoderStream[venc].ThreadStat = 1;

	struct timeval TimeOut; 
	
	#ifdef 0//savestream
//	int count =0;	
	char h264name[32]={0x0};
	sprintf(h264name, "stream_chn%d.h264",venc);
	FILE *pFile = fopen(h264name, "wb");
	#endif	
	while(m_EncoderStream[venc].EncodeStat)
	{
		
		if(startupdate == 0)
		{
			printf("VideoStreamThreadBody thread exit file:%s line:%d pid:%d\n",__FILE__,__LINE__,(unsigned)pthread_self());
			free(pPack);
			VideoEncodeExit(VeChn);
			pthread_exit(0);
		}

		TimeOut.tv_sec = 2;
		TimeOut.tv_usec = 0;
		//printf("m_EncoderStream[venc].EncodeStat:%d\n",m_EncoderStream[venc].EncodeStat);
		FD_ZERO(&read_fds);		
		VencFd = HI_MPI_VENC_GetFd(VeChn);		
		if(VencFd < 0)
		{
			printf("VideoStreamThreadBody HI_MPI_VENC_GetFd main stream failed\n");                   
			break;
		}
		FD_SET(VencFd, &read_fds);

		s32ret = select(VencFd+1,&read_fds,NULL,NULL,&TimeOut);			
		if(s32ret > 0)
		{
			goto READ_STREAM;
		}
		
		else if(s32ret < 0)            
		{
			printf("EncoderThreadBody select error(%d)\n", s32ret);
			break;
		}
		else
		{
			printf("select venc :%d time out!!!\n",venc);
			continue;
		}
READ_STREAM:	
		if(FD_ISSET(VencFd, &read_fds))
		{
			
			s32ret = HI_MPI_VENC_Query(VeChn, &stStat);
			if (s32ret) 
			{
				
				printf("GetVideoStream HI_MPI_VENC_Query err:0x%x\n",s32ret);
				continue;
			}			
			if(stStat.u32CurPacks > packCount)
			{
				free(pPack);
				pPack = NULL;
				pPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S)*stStat.u32CurPacks);
				if (!pPack) 
				{
					printf("\n\nGet_AVStreamInOneChn malloc err:::::::::::::::::::\n");
					break;
				}
				packCount = stStat.u32CurPacks;
			}			
			stVStream.pstPack = pPack;
			stVStream.u32PackCount = stStat.u32CurPacks;
			if (!stVStream.pstPack) 
			{
				
				printf("GetVideoStream malloc err:\n");
				break;
			}			
			s32ret = HI_MPI_VENC_GetStream(VeChn, &stVStream, HI_IO_BLOCK);			
			if (s32ret) 
			{
				printf("GetVideoStream HI_MPI_VENC_GetStream err:0x%x\n",s32ret);
			}
			#ifdef savestream
			SaveStreamToFile(pFile,&stVStream);
			#endif
			#ifdef ADJUST_STREAM
			if(mdalarmupload == false&&s32ret == HI_SUCCESS)
			{
				if(stVStream.stH264Info.enRefType == ENHANCE_PSLICE_NOTFORREF)
				{				
					HI_MPI_VENC_ReleaseStream(VeChn,&stVStream);
					continue;	
				}						
			}
			#endif
			
			if(m_EncoderStream[venc].pBufferManage != NULL)
			{	
				if (s32ret == HI_SUCCESS)
				{
					m_EncoderStream[venc].pBufferManage->PutOneVFrameToBuffer((void *)(&stVStream));
				}
				if((venc==0)&&m_EncoderStream[1].pBufferManage != NULL)
				{
					m_EncoderStream[1].pBufferManage->PutOneVFrameToBuffer((void *)(&stVStream));
				}
			}
		 	if (s32ret == HI_SUCCESS)
			{
				HI_MPI_VENC_ReleaseStream(VeChn,&stVStream);
			}
			
			

		}
		
	}
	
	if(pPack != NULL)
	{
		free(pPack);
	}
	#ifdef savestream
	fclose(pFile);
	#endif
	m_EncoderStream[venc].EncodeStat = 0;
	m_EncoderStream[venc].ThreadStat = 0;
	printf("编码%d线程退出:%d!!!!\n",venc,m_EncoderStream[venc].EncodeStat);
#endif	
	return 0;
}
/*****************************************************************************
函数名称:VideoMirrorFlipSet
函数功能:图像翻转镜像熟悉的设置
输入参数:Para 4:正常1 翻转2 镜像3 镜像加翻转
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明: 外部调用，
编码和系统初始化后才可以调用

******************************************************************************/

int VideoEncode::VideoMirrorFlipSet(int Para)
{

	HI_S32 s32ret; 
	VPSS_CHN_ATTR_S  stChnAttr;




	HI_BOOL bMirror =HI_FALSE;            /*mirror enable*/
    HI_BOOL bFlip =HI_FALSE;              /*flip   enable*/

	
	if(CONFIG_FLIP&ConfigInfo.SupportInfo)
	{
		if(Para == 1)
		{
			bMirror = HI_FALSE;
			bFlip = HI_FALSE;
		}
		else if(Para == 4)
		{
			bMirror = HI_TRUE;
			bFlip = HI_TRUE;

		}
		else if(Para == 3)
		{
			bMirror = HI_TRUE;
			bFlip = HI_FALSE;

		}
		else if(Para == 2)
		{
			bMirror = HI_FALSE;
			bFlip = HI_TRUE;

		}

	}
	else
	{
		if(Para == 4)
		{
			bMirror = HI_FALSE;
			bFlip = HI_FALSE;
		}
		else if(Para == 3)
		{
			bMirror = HI_TRUE;
			bFlip = HI_TRUE;

		}
		else if(Para == 2)
		{
			bMirror = HI_TRUE;
			bFlip = HI_FALSE;

		}
		else if(Para == 1)
		{
			bMirror = HI_FALSE;
			bFlip = HI_TRUE;

		}

	}	
	
	int i=0;
	for(i=0;i<3;i++)
	{
		memset(&stChnAttr,0,sizeof(VPSS_CHN_ATTR_S));

		HI_MPI_VPSS_GetChnAttr(0,i,&stChnAttr);

		stChnAttr.bFlip = bFlip;
		stChnAttr.bMirror = bMirror;
		printf("======HI_MPI_VPSS_SetChnAttr[%d,%d,%d]\n",Para,bMirror,bFlip);
		HI_MPI_VPSS_SetChnAttr(0,i, &stChnAttr);
	}	
	return HI_SUCCESS;
	
}

int VideoEncode::VideoMirrorSet(bool set)
{

	HI_S32 s32ret; 
	VI_CHN ViChn = 0; 
	VI_CHN_ATTR_S  stChnAttr; 
	s32ret = HI_MPI_VI_GetChnAttr(ViChn, &stChnAttr); 
	if (HI_SUCCESS != s32ret) 
	{ 
		printf("get vi chn attr err:0x%x\n", s32ret); 
		return s32ret; 
	} 
	if(set)
		stChnAttr.bMirror = HI_TRUE;
	else
		stChnAttr.bMirror = HI_FALSE;
//	stChnAttr.bMirror = HI_FALSE;

	s32ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr); 
	if (HI_SUCCESS != s32ret) 
	{ 
		printf("set vi chn attr err:0x%x\n", s32ret); 
		return s32ret; 
	} 	
	return HI_SUCCESS;
	
}
int VideoEncode::VideoFlipSet(bool set)
{

	HI_S32 s32ret; 
	VI_CHN ViChn = 0; 
	VI_CHN_ATTR_S  stChnAttr; 
	s32ret = HI_MPI_VI_GetChnAttr(ViChn, &stChnAttr); 
	if (HI_SUCCESS != s32ret) 
	{ 
		printf("get vi chn attr err:0x%x\n", s32ret); 
		return s32ret; 
	} 
	if(set)
		stChnAttr.bFlip = HI_TRUE;
	else
		//stChnAttr.bMirror = HI_FALSE;
		stChnAttr.bFlip = HI_FALSE;

	s32ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr); 
	if (HI_SUCCESS != s32ret) 
	{ 
		printf("set vi chn attr err:0x%x\n", s32ret); 
		return s32ret; 
	} 	
	return HI_SUCCESS;
	
}

int VideoEncode::VideoEncodeExit(int VeChn )
{
	printf("start VideoEncodeExit!!!\n");
	if(0==pBufferManage[VeChn]->DestroyBufferPool())
	{
		printf("DestroyBufferPool VeChn:%d success \n",VeChn);
	}	
	VideoVencUnBindVpss(VeChn, VeChn, VeChn);
	VideoVencStop(VeChn,VeChn);
	return 0;
}

int VideoEncode::SetVideoAntiFlickerAttr(bool enable,unsigned short Frequency )
{
	if(m_pISP)
	{
		m_pISP->SetAntiFlickerAttr(enable,Frequency);
		return 0;
	}
		
	return -1;
}











