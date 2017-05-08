/******************************************************************************
  File Name     : Video_ISP.cpp
  Version       : 
  Last Modified :
  Description   : 
  Function List :
  History       :
    Modification: Created file

******************************************************************************/


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

#include "mpi_vi.h"
#include "Video_ISP.h"

#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_af.h"

#include "hi_mipi.h"
#include "ModuleFuncInterface.h"
#include "IR_Cut.h"
static HI_BOOL gbIspInited = HI_FALSE;

static pthread_t gs_IspPid = 0;


extern int sensorid;
#if 0
static HI_U16 Gamma_data[257] ={80,115,149,181,212,242,274,310,346,386,431,479,530,583,638,693,748,804,861,918,976,1035,1094,1149,
1203,1253,1302,1347,1392,1432,1472,1507,1542,1582,1622,1659,1696,1730,1765,1797,1829,1859,1890,
1918,1947,1976,2005,2032,2059,2086,2112,2138,2163,2187,2211,2235,2259,2282,2304,2326,2347,2366,
2386,2406,2425,2442,2458,2474,2489,2504,2519,2534,2548,2562,2577,2591,2606,2620,2634,2649,2663,
2678,2692,2706,2719,2733,2746,2760,2774,2787,2801,2814,2828,2842,2855,2869,2882,2896,2910,2923,
2937,2950,2963,2976,2989,3002,3014,3027,3040,3053,3066,3078,3091,3102,3114,3125,3136,3147,3158,
3170,3181,3190,3200,3210,3219,3229,3238,3248,3258,3267,3277,3286,3296,3306,3315,3325,3334,3344,
3354,3363,3373,3382,3392,3402,3411,3419,3427,3435,3443,3451,3459,3467,3474,3480,3486,3493,3499,
3506,3512,3518,3525,3531,3538,3544,3550,3557,3563,3570,3576,3582,3589,3595,3602,3606,3611,3616,
3621,3626,3630,3635,3640,3645,3650,3654,3659,3664,3669,3674,3678,3683,3688,3693,3698,3702,3707,
3712,3717,3722,3726,3731,3736,3741,3746,3750,3755,3760,3765,3770,3774,3779,3784,3789,3794,3798,
3803,3808,3813,3818,3822,3827,3832,3837,3842,3846,3851,3856,3861,3866,3870,3875,3880,3885,3890,
3894,3899,3904,3909,3914,3918,3923,3928,3933,3938,3942,3947,3952,3955,3958,3962,3965,3968,3971,
3974,3978,3981,3984,3987,};
#endif

CVideoISP::CVideoISP()
{

}

CVideoISP::~CVideoISP()
{
	
}


/*****************************************************************************
函数名称:ISP_Run
函数功能:
输入参数:无
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明:外部调用

******************************************************************************/
combo_dev_attr_t MIPI_CMOS3V3_ATTR={INPUT_MODE_CMOS_33V,{}};

int CVideoISP::ISP_Run()
{ 
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ISP_PUB_ATTR_S stPubAttr;
    ALG_LIB_S stLib;
  #if 1
    HI_S32 fd;
    combo_dev_attr_t *pstcomboDevAttr = NULL;

    /* mipi reset unrest */
    fd = open("/dev/hi_mipi", O_RDWR);
    if (fd < 0)
    {
        printf("warning: open hi_mipi dev failed\n");
        return -1;
    }
//printf("=============SAMPLE_COMM_VI_SetMipiAttr enWDRMode: %d\n", pstViConfig->enWDRMode);
 	pstcomboDevAttr = &MIPI_CMOS3V3_ATTR;


  	if (ioctl(fd, HI_MIPI_SET_DEV_ATTR, pstcomboDevAttr))
    {
        printf("set mipi attr failed\n");
        close(fd);
        return HI_FAILURE;
    }
    close(fd);
#endif	
    /* 1. sensor register callback */
    s32Ret = sensor_register_callback();
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: sensor_register_callback failed with %#x!\n", \
               __FUNCTION__, s32Ret);
        return s32Ret;
    }

    /* 2. register hisi ae lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_Register(IspDev, &stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AE_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 3. register hisi awb lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_Register(IspDev, &stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AWB_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }
	
	/* 4. register hisi af lib */
	stLib.s32Id = 0;
	strcpy(stLib.acLibName, HI_AF_LIB_NAME);
	s32Ret = HI_MPI_AF_Register(IspDev, &stLib);
	if (s32Ret != HI_SUCCESS)
	{
	   printf("%s: HI_MPI_AF_Register failed!\n", __FUNCTION__);
	   return s32Ret;
	}

	/* 5. isp mem init */
	s32Ret = HI_MPI_ISP_MemInit(IspDev);
	if (s32Ret != HI_SUCCESS)
	{
	   printf("%s: HI_MPI_ISP_MemInit failed!\n", __FUNCTION__);
	   return s32Ret;
	}

	/* 6. isp set WDR mode */
	ISP_WDR_MODE_S stWdrMode;
	stWdrMode.enWDRMode	= WDR_MODE_NONE;
	s32Ret = HI_MPI_ISP_SetWDRMode(0, &stWdrMode);	 
	if (HI_SUCCESS != s32Ret)
	{
	   printf("start ISP WDR failed!\n");
	   return s32Ret;
	}
	
	/* 7. isp set pub attributes */
	/* note : different sensor, different ISP_PUB_ATTR_S define.
	  if the sensor you used is different, you can change
	  ISP_PUB_ATTR_S definition */
	stPubAttr.enBayer               = BAYER_BGGR;//BAYER_GRBG;//BAYER_GBRG;//BAYER_GRBG;
	stPubAttr.f32FrameRate          = 30;
	stPubAttr.stWndRect.s32X        = 0;
	stPubAttr.stWndRect.s32Y        = 0;
	stPubAttr.stWndRect.u32Width    = 1280;
	stPubAttr.stWndRect.u32Height   = 720;//720;
	printf("    stPubAttr.enBayer               = BAYER_GBRG;//BAYER_GRBG;\n");	
	 
		s32Ret = HI_MPI_ISP_SetPubAttr(IspDev, &stPubAttr);
	if (s32Ret != HI_SUCCESS)
	{
	 printf("%s: HI_MPI_ISP_SetPubAttr failed with %#x!\n", __FUNCTION__, s32Ret);
	 return s32Ret;
	}

	/* 8. isp init */
	s32Ret = HI_MPI_ISP_Init(IspDev);
	if (s32Ret != HI_SUCCESS)
	{
	 	 printf("%s: HI_MPI_ISP_Init failed!\n", __FUNCTION__);
		 return s32Ret;
	}

	gbIspInited = HI_TRUE;
	// return HI_SUCCESS;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 4096 * 1024);
	if (0 != pthread_create(&gs_IspPid, &attr, (void* (*)(void*))HI_MPI_ISP_Run, NULL))
	{
		printf("%s: create isp running thread failed!\n", __FUNCTION__);
		pthread_attr_destroy(&attr);
		return HI_FAILURE;
	}
	usleep(1000);
	pthread_attr_destroy(&attr);
	return HI_SUCCESS;
 
}
int CVideoISP::StopIsp()
{
    /* 11. isp exit and main programme exit                                    */
    printf("Isp will exit!\n");
//    HI_MPI_ISP_Exit();
 //   pthread_join(m_IspPid, 0);

    return 0;
}

int CVideoISP::SetVideoSaturation(unsigned int value)
{

//	HI_S32 s32ret; 
//	s32ret = HI_MPI_ISP_SetSaturation(value); 
//	if (HI_SUCCESS != s32ret) 
//	{ 
//		printf("SetVideoSaturation err:0x%x\n", s32ret); 
//		return s32ret; 
//	}
	return HI_SUCCESS;
}
#if 0
int CVideoISP::SetVideoBrightness(unsigned int value)
{
	if(value >255)
	{
		value = 130;
	}
	ISP_AE_ATTR_S stAEAttr; 
	ISP_EXP_STA_INFO_S stExpStatistic;
	
	HI_MPI_ISP_GetAEAttr(&stAEAttr); 
	stAEAttr.u16ExpTimeMax = 750; 
	//stAEAttr.u16ExpTimeMin = 2; 
	//stAEAttr.u16AGainMax	 = 28; 
	//stAEAttr.u16AGainMin	 = 0; 
	//stAEAttr.u16DGainMax	 = 38; 
	//stAEAttr.u16DGainMin	 = 0; 
	stAEAttr.u8ExpStep	   = 100; 
	stAEAttr.s16ExpTolerance = 4; 
	stAEAttr.u8ExpCompensation = value;
	HI_MPI_ISP_SetAEAttr(&stAEAttr); 
	
	HI_MPI_ISP_GetExpStaInfo(&stExpStatistic);
	stExpStatistic.u8AveLum=value;
	HI_MPI_ISP_SetExpStaInfo(&stExpStatistic);

	return HI_SUCCESS;
}
#endif

int CVideoISP::SetVideoChnAnaLog(int brightness,int contrast,int saturation)
{
	unsigned char flag;
	HI_S32 s32Ret; 
	static VI_CSC_ATTR_S stCscAttr; 
	//printf("======kb brightness=%d; contrast=%d; saturation=%d\n",brightness,contrast,saturation); 
	/* Get attribute for vi CSC attr */ 
	s32Ret = HI_MPI_VI_GetCSCAttr(0, &stCscAttr); 
	if (HI_SUCCESS != s32Ret) 
	{ 
		 printf("Get vi CSC attr err:0x%x\n", s32Ret); 
		 return s32Ret; 
	} 
	//printf("kb need to know:%d %d %d\n",brightness,contrast,saturation);
	
	/* Init CSC attr */ 
	//stCscAttr.enViCscType = VI_CSC_TYPE_709; 
//	

	stCscAttr.u32ContrVal = (100*contrast)/255; //暂时屏蔽对比度
	if(stCscAttr.u32ContrVal>60)
		stCscAttr.u32ContrVal=60;
	stCscAttr.u32LumaVal = (100*brightness)/255;
	stCscAttr.u32SatuVal =  (100*saturation)/255;
	Get_IRCutStatus(flag);
	if(flag==0){
		//
		stCscAttr.u32SatuVal=0;
	}
	printf("########################## u32LumaVal:%d ,u32ContrVal:%d,u32SatuVal:%d\r\n", \
		stCscAttr.u32LumaVal,stCscAttr.u32ContrVal,stCscAttr.u32SatuVal );
	 
	/* Set attribute for vi CSC attr */ 
	s32Ret = HI_MPI_VI_SetCSCAttr(0, &stCscAttr); 
	if (HI_SUCCESS != s32Ret) 
	{ 
		printf("Set vi CSC attr err:0x%x\n", s32Ret); 
		return s32Ret; 
	} 
	 
	return 0;
 
 }


 

 int CVideoISP::SetAntiFlickerAttr(unsigned char enable,unsigned short Frequency)
 {
 #if 0
	HI_S32 s32Ret;
	ISP_ANTIFLICKER_S Antiflicker;

 	s32Ret =  HI_MPI_ISP_GetAntiFlickerAttr(&Antiflicker);
	if (s32Ret !=HI_SUCCESS )
	{
			printf("%s: HI_MPI_ISP_GetAntiFlickerAttr failed with %#x!\n", __FUNCTION__, s32Ret);
		return s32Ret;
	}


	printf("###########Antiflicker.bEnable :: %d\n",Antiflicker.bEnable);
	printf("###########Antiflicker.u8Frequency :: %d\n",Antiflicker.u8Frequency);
	printf("###########Antiflicker.enMode :: %d\n",Antiflicker.enMode);
	if(enable)
	{
		Antiflicker.bEnable = HI_TRUE;
		if(Frequency == 0)/*自动模式*/
		{
			Antiflicker.enMode = ISP_ANTIFLICKER_MODE_1; 
			Antiflicker.u8Frequency =60;
		}	
		else
		{
			Antiflicker.u8Frequency =Frequency; 	
			Antiflicker.enMode = ISP_ANTIFLICKER_MODE_0;//ISP_ANTIFLICKER_MODE_BUTT ;//ISP_ANTIFLICKER_MODE_1;		
		}
	}
	else
	{
	
		Antiflicker.bEnable = HI_FALSE;
		
	}

	s32Ret =  HI_MPI_ISP_SetAntiFlickerAttr(&Antiflicker);
	if (s32Ret !=HI_SUCCESS )
	{
			printf("%s: HI_MPI_ISP_GetAntiFlickerAttr failed with %#x!\n", __FUNCTION__, s32Ret);
		return s32Ret;
	}	


	s32Ret =  HI_MPI_ISP_GetAntiFlickerAttr(&Antiflicker);
	if (s32Ret !=HI_SUCCESS )
	{
			printf("%s: HI_MPI_ISP_GetAntiFlickerAttr failed with %#x!\n", __FUNCTION__, s32Ret);
		return s32Ret;
	}
	printf("####2######Antiflicker.bEnable :: %d\n",Antiflicker.bEnable);
	printf("####2######Antiflicker.u8Frequency :: %d\n",Antiflicker.u8Frequency);
	printf("####2######Antiflicker.enMode :: %d\n",Antiflicker.enMode);
	#endif
	return HI_SUCCESS;
 }
int CVideoISP::H42_ISPseting()
{

	HI_S32 s32Ret;


	ISP_EXPOSURE_ATTR_S  stAEAttr;
	s32Ret =  HI_MPI_ISP_GetExposureAttr(0,&stAEAttr);
	if (s32Ret !=HI_SUCCESS )
	{
		printf("%s: HI_MPI_ISP_GetExposureAttr failed with %#x!\n", __FUNCTION__, s32Ret);
		//return s32Ret;
	}


	stAEAttr.stAuto.stExpTimeRange.u32Max= 0xffffffff;
	stAEAttr.stAuto.stExpTimeRange.u32Min = 0x2;

	stAEAttr.stAuto.stAGainRange.u32Max = 0x1400;
	stAEAttr.stAuto.stDGainRange.u32Max = 0x1000;
	stAEAttr.stAuto.stISPDGainRange.u32Max = 0x1000;
	stAEAttr.stAuto.stSysGainRange.u32Max = 0x2800;

		
	s32Ret =  HI_MPI_ISP_SetExposureAttr(0,&stAEAttr);
	if (s32Ret !=HI_SUCCESS )
	{
		printf("%s: HI_MPI_ISP_SetAEAttr failed with %#x!\n", __FUNCTION__, s32Ret);
		
	}	

#if 1
	printf("---------------H42_ISPseting----------------------------\n");
	printf("stAEAttr.enAEMode = %x\n",stAEAttr.enOpType);
	printf("stExpTimeRange.u32Max = %x\n",stAEAttr.stAuto.stExpTimeRange.u32Max);
	printf("stExpTimeRange.u32Min = %x\n",stAEAttr.stAuto.stExpTimeRange.u32Min);

	printf("stAGainRange.u32Max = %x\n",stAEAttr.stAuto.stAGainRange.u32Max);
	printf("stAGainRange.u32Min = %x\n",stAEAttr.stAuto.stAGainRange.u32Min);
	

	printf("stDGainRange.u16DGainMax = %x\n",stAEAttr.stAuto.stDGainRange.u32Max );
	printf("stDGainRange.u16DGainMin = %x\n",stAEAttr.stAuto.stDGainRange.u32Min);


	printf("stISPDGainRange.u16DGainMax = %x\n",stAEAttr.stAuto.stISPDGainRange.u32Max);
	printf("stSysGainRange.u16DGainMin = %x\n",stAEAttr.stAuto.stSysGainRange.u32Max);


	printf("--------------------------------------------------------\n");
#endif



	

	ISP_WB_ATTR_S	  stAWBAttr;
	s32Ret =  HI_MPI_ISP_GetWBAttr(0,&stAWBAttr);
	if (s32Ret !=HI_SUCCESS )
	{
		printf("%s: HI_MPI_ISP_GetAEAttr failed with %#x!\n", __FUNCTION__, s32Ret);
		//return s32Ret;
	}


	printf("stAWBAttr.stAuto.u8RGStrength = %x\n",stAWBAttr.stAuto.u8RGStrength);
	printf("stAWBAttr.stAuto.u8BGStrength = %x\n",stAWBAttr.stAuto.u8BGStrength);


	stAWBAttr.stAuto.u8RGStrength = 0x80;
	stAWBAttr.stAuto.u8BGStrength = 0x80;
	stAWBAttr.stAuto.au16StaticWB[3]=0x250;
	
	s32Ret =  HI_MPI_ISP_SetWBAttr(0,&stAWBAttr);
	if (s32Ret !=HI_SUCCESS )
	{
		printf("%s: HI_MPI_ISP_SetAEAttr failed with %#x!\n", __FUNCTION__, s32Ret);
		
	}	

	ISP_SATURATION_ATTR_S stSatAttr;

	s32Ret =  HI_MPI_ISP_GetSaturationAttr(0,&stSatAttr);
	if (s32Ret !=HI_SUCCESS )
	{
		printf("%s: HI_MPI_ISP_GetSaturationAttr failed with %#x!\n", __FUNCTION__, s32Ret);
		
	}	
	int index=0;
	stSatAttr.stAuto.au8Sat[index++]=0x80;
	stSatAttr.stAuto.au8Sat[index++]=0x78;
	stSatAttr.stAuto.au8Sat[index++]=0x70;
	stSatAttr.stAuto.au8Sat[index++]=0x68;
	
	stSatAttr.stAuto.au8Sat[index++]=0x60;
	stSatAttr.stAuto.au8Sat[index++]=0x58;
	stSatAttr.stAuto.au8Sat[index++]=0x52;
	stSatAttr.stAuto.au8Sat[index++]=0x50;
	s32Ret =  HI_MPI_ISP_SetSaturationAttr(0,&stSatAttr);

#if 0
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	s32Ret = HI_MPI_VPSS_GetGrpAttr(0, &stVpssGrpAttr);
	if (s32Ret !=HI_SUCCESS )
	{
		printf("%s: HI_MPI_VPSS_GetGrpNRBParam	%#x!\n", __FUNCTION__, s32Ret);
	
	}
	printf("%s: stVpssGrpAttr.bNrEn:%d\n", __FUNCTION__, stVpssGrpAttr.bNrEn);
	stVpssGrpAttr.bNrEn = HI_TRUE;
	s32Ret = HI_MPI_VPSS_SetGrpAttr(0, &stVpssGrpAttr);
	if (s32Ret !=HI_SUCCESS )
	{
		printf("%s: HI_MPI_VPSS_SetGrpAttr	%#x!\n", __FUNCTION__, s32Ret);
	
	}
	VPSS_GRP_NRB_PARAM_S NRBParam;
	HI_MPI_VPSS_GetGrpNRBParam(0,&NRBParam);
	if (s32Ret !=HI_SUCCESS )
	{
		printf("%s: HI_MPI_VPSS_GetGrpNRBParam	%#x!\n", __FUNCTION__, s32Ret);
	
	}	
	printf("%s: HI_MPI_VPSS_GetGrpNRBParam	%#x!\n", __FUNCTION__, s32Ret);
	
	printf("=============================================\n");
	printf("enNRVer = %d\n",NRBParam.enNRVer);
	printf("SBS. = %d\n",NRBParam.stNRBParam_V1.sf[0].SBS );
	printf("SBS. = %d\n",NRBParam.stNRBParam_V1.sf[1].SBS );
	printf("SBS. = %d\n",NRBParam.stNRBParam_V1.sf[2].SBS );
	
	printf("SDS. = %d\n",NRBParam.stNRBParam_V1.sf[0].SDS );
	printf("SDS. = %d\n",NRBParam.stNRBParam_V1.sf[1].SDS );
	printf("SDS. = %d\n",NRBParam.stNRBParam_V1.sf[2].SDS );
	printf("=============================================\n");	
	
	NRBParam.stNRBParam_V1.sf[0].SBS = 18;
	NRBParam.stNRBParam_V1.sf[1].SBS = 14;
	NRBParam.stNRBParam_V1.sf[2].SBS = 0;
	
	NRBParam.stNRBParam_V1.sf[0].SDS= 18;
	NRBParam.stNRBParam_V1.sf[1].SDS = 14;
	NRBParam.stNRBParam_V1.sf[2].SDS = 0;
	NRBParam.enNRVer = VPSS_NR_V1;
	s32Ret=HI_MPI_VPSS_SetGrpNRBParam(0,&NRBParam);
	if (s32Ret !=HI_SUCCESS )
	{
		printf("%s: HI_MPI_VPSS_SetGrpNRBParam	%#x!\n", __FUNCTION__, s32Ret);
	
	}	
	printf("%s: HI_MPI_VPSS_SetGrpNRBParam	%#x!\n", __FUNCTION__, s32Ret); 
#endif
	VPSS_NR_PARAM_U unNrParam = {{0}};
	s32Ret = HI_MPI_VPSS_GetNRParam(0, &unNrParam);
	printf("%s: HI_MPI_VPSS_GetNRParam	%#x!\n", __FUNCTION__, s32Ret);
	
	printf("\t\typk 	 %d\n",  unNrParam.stNRParam_V1.s32YPKStr);
	printf("\t\tysf 	  %d\n", unNrParam.stNRParam_V1.s32YSFStr);
	printf("\t\tytf 	  %d\n", unNrParam.stNRParam_V1.s32YTFStr);
	printf("\t\tytfmax	  %d\n", unNrParam.stNRParam_V1.s32TFStrMax);
	printf("\t\tyss 	  %d\n", unNrParam.stNRParam_V1.s32YSmthStr);
	printf("\t\tysr 	  %d\n", unNrParam.stNRParam_V1.s32YSmthRat);
	printf("\t\tysfdlt	  %d\n", unNrParam.stNRParam_V1.s32YSFStrDlt);
	printf("\t\tytfdlt	  %d\n", unNrParam.stNRParam_V1.s32YTFStrDlt);
	printf("\t\tytfdl	  %d\n", unNrParam.stNRParam_V1.s32YTFStrDl);
	printf("\t\tysfbr	  %d\n", unNrParam.stNRParam_V1.s32YSFBriRat);
	printf("\t\tcsf 	  %d\n", unNrParam.stNRParam_V1.s32CSFStr);
	printf("\t\tctf 	  %d\n", unNrParam.stNRParam_V1.s32CTFstr);
	unNrParam.stNRParam_V1.s32YSFStr = 0x79;
	s32Ret = HI_MPI_VPSS_SetNRParam(0, &unNrParam);
	printf("%s: HI_MPI_VPSS_SetNRParam	%#x!\n", __FUNCTION__, s32Ret); 

	ISP_DP_DYNAMIC_ATTR_S stDPDynamicAttr;
	HI_MPI_ISP_GetDPDynamicAttr(0, &stDPDynamicAttr);
	index = 0;
	stDPDynamicAttr.stAuto.au16Slope[index++]=160;
	stDPDynamicAttr.stAuto.au16Slope[index++]=160;
	stDPDynamicAttr.stAuto.au16Slope[index++]=160;

	stDPDynamicAttr.stAuto.au16Slope[index++]=170;
	stDPDynamicAttr.stAuto.au16Slope[index++]=170;
	stDPDynamicAttr.stAuto.au16Slope[index++]=170;

	stDPDynamicAttr.stAuto.au16Slope[index++]=180;
	stDPDynamicAttr.stAuto.au16Slope[index++]=180;
	stDPDynamicAttr.stAuto.au16Slope[index++]=180;

	stDPDynamicAttr.stAuto.au16Slope[index++]=190;
	stDPDynamicAttr.stAuto.au16Slope[index++]=190;
	stDPDynamicAttr.stAuto.au16Slope[index++]=190;

	stDPDynamicAttr.stAuto.au16Slope[index++]=200;
	stDPDynamicAttr.stAuto.au16Slope[index++]=200;
	stDPDynamicAttr.stAuto.au16Slope[index++]=200;
	stDPDynamicAttr.stAuto.au16Slope[index++]=200;

	HI_MPI_ISP_SetDPDynamicAttr(0, &stDPDynamicAttr);


	return HI_SUCCESS;
}









