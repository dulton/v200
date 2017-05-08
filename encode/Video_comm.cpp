#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "Video_comm.h"
#include "hi_i2c.h"

#include "hi_mipi.h"

#include "hi_common.h"
//#include "sample_comm.h"

/*OV9732 DC 12bit input 720P@30fps*/
VI_DEV_ATTR_S DEV_ATTR_OV9732_DC_720P_BASE =
{
    /* interface mode */
    VI_MODE_DIGITAL_CAMERA,
    /* multiplex mode */
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    //{0xFFC0000,    0x0},
    {0x3FF0000,    0x0},
    /* progessive or interleaving */
    VI_SCAN_PROGRESSIVE,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, only support yuv*/
    VI_INPUT_DATA_YUYV,

    /* synchronization information */
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {370,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     6,            720,        6,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /* use interior ISP */
    VI_PATH_ISP,
    /* input data type */
    VI_DATA_TYPE_RGB,
    /* Data Reverse */
    HI_FALSE,
    {0, 0, 1280, 720}
};

/*AR0130 DC 12bit输入720P@30fps*/
VI_DEV_ATTR_S DEV_ATTR_AR0130_DC_720P =
{
    /*接口模式*/
    VI_MODE_DIGITAL_CAMERA,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
      {0xFFF0000,    0x0}, //{0xFFF00000,    0x0}, 
    /*逐行or隔行输入*/
    VI_SCAN_PROGRESSIVE,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, 仅支持YUV格式*/
    VI_INPUT_DATA_YUYV,
     
    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,
    
    /*timing信息，对应reg手册的如下配置*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {370,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     6,            720,        6,                 //     6,            720,        6,
    /*vsync1_vhb vsync1_act vsync1_hhb*/ 
     0,            0,            0}
    },    
    /*使用内部ISP*/
    VI_PATH_ISP,
    /*输入数据类型*/
    VI_DATA_TYPE_RGB,
      /* Data Reverse */
    HI_FALSE,
       {0, 0, 1280, 720}//{0, 0, 1280, 720}
};

/*OV9712 DC 10bit输入*/
VI_DEV_ATTR_S DEV_ATTR_OV9712_DC_720P =
/* 典型时序3:7441 BT1120 720P@60fps典型时序 (对接时序: 时序)*/
{
    /*接口模式*/
    VI_MODE_DIGITAL_CAMERA,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFFC00000,    0x0},
    /*逐行or隔行输入*/
    VI_SCAN_PROGRESSIVE,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, 仅支持YUV格式*/
    VI_INPUT_DATA_YUYV,

    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,
    
    /*timing信息，对应reg手册的如下配置*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {408,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     6,            720,        6,
    /*vsync1_vhb vsync1_act vsync1_hhb*/ 
     0,            0,            0}
    },
    /*使用内部ISP*/
    VI_PATH_ISP,
    /*输入数据类型*/
    VI_DATA_TYPE_RGB
};

VI_DEV_ATTR_S DEV_ATTR_HIMAX_1MUX =
{
    /*接口模式*/
    VI_MODE_BT601,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},
    /*逐行or隔行输入*/
    VI_SCAN_PROGRESSIVE,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, 仅支持YUV格式*/
    VI_INPUT_DATA_YUYV,     
    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,
    
    /*timing信息，对应reg手册的如下配置*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/ 
     0,            0,            0}
    },    
  
    VI_PATH_BYPASS,
    /*输入数据类型*/
    VI_DATA_TYPE_YUV
};


#define OV9712_ID	0x9711
#define HM1375_ID	0x75
#define JXH22_ID    0xA022
#define AR0130_ID	0x2402

#define OV9712_ADDR	0x20//0x60
#define JXH22_ADDR  0x20 //0x60 
#define HM1375_ADDR 0x48
#define SC1135_ADDR 0x60    //0x20




struct sensor_set_struct{
	unsigned int ID;
	unsigned int (*fun)(void);
	unsigned int name;
};

unsigned int jxh22_read_id();
unsigned int ov9712_read_id();
unsigned int hm1375_read_id();
unsigned int ar0130_read_id();

#define ADD_SENSOR(id, name) \
	{id, name##_read_id, libsns_##name##_720p,}

struct sensor_set_struct sensor_list[] = {
	ADD_SENSOR(OV9712_ID, ov9712),
	ADD_SENSOR(JXH22_ID,  jxh22),
	ADD_SENSOR(HM1375_ID, hm1375),
	ADD_SENSOR(AR0130_ID, ar0130),
};

int sensor_read_register(int Saddr, int reg, int reg_len, int data_len)
{
//	int ret;
	int fd = -1;
	I2C_DATA_S i2c_data;

	fd = open("/dev/i2c-0", 0);
	if(fd < 0){
		printf("Open /dev/i2c-0 dev error!\n");
    	return -1;
	}

	i2c_data.dev_addr = Saddr;
	i2c_data.reg_addr = reg;
	i2c_data.addr_byte_num = reg_len;
    i2c_data.data_byte_num = data_len;

//	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);

	close(fd);

	return i2c_data.data;
}

void sensor_write_register(int Saddr, int reg, int reg_len, int data_len, int val)
{
//	int ret;
	int fd = -1;
	I2C_DATA_S i2c_data;

	fd = open("/dev/i2c-0", 0);
	if(fd < 0){
		printf("Open /dev/i2c-0 error!\n");
    	close(fd);;
	}

	i2c_data.dev_addr = Saddr;
	i2c_data.reg_addr = reg;
	i2c_data.data = val;
	i2c_data.addr_byte_num = reg_len;
    i2c_data.data_byte_num = data_len;

//	ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

	close(fd);
}

int detect_sensor()
{
	unsigned char num;
	unsigned char i;
	unsigned int id;
	

	num = sizeof(sensor_list)/sizeof(sensor_list[0]);
	printf("sensor list num %d\n", num);

	for(i = 0; i < num; i++){
		id = (sensor_list + i)->fun();
		printf("sensor_list id = 0x%x , ID = 0x%x\n", id, (sensor_list + i)->ID);

		if(id == (sensor_list + i)->ID)
			return (sensor_list + i)->name;

		id = 0;
	}

	printf("id = 0x%x\n\n", id);

	return libsns_ov9712_720p;
}

unsigned int ov9712_read_id()
{
	unsigned int IDL, IDH;

	IDL = sensor_read_register(OV9712_ADDR, 0x0b, 1, 1);
	IDH = sensor_read_register(OV9712_ADDR, 0x0a, 1, 1);

	printf("ov9712 idl = 0x%x idh = 0x%x\n", IDL, IDH);

	return ((IDH << 8) | IDL);
}


unsigned int jxh22_read_id()
{
	unsigned int IDL, IDH;

	IDL = sensor_read_register(JXH22_ADDR, 0x0b, 1, 1);
	IDH = sensor_read_register(JXH22_ADDR, 0x0a, 1, 1);

	printf("JXH22 idl = 0x%x idh = 0x%x\n", IDL, IDH);

	return ((IDH << 8) | IDL);
}

unsigned int hm1375_read_id()
{
	unsigned int ID;

	ID = sensor_read_register(HM1375_ADDR, 0x0002, 2, 1);
	printf("hm1375 id = 0x%x\n", ID);

	return ID;
}

unsigned int ar0130_read_id()
{
	unsigned int ID;

	ID = sensor_read_register(SC1135_ADDR, 0x3000, 2, 2);
	printf("ar0130 id = 0x%x\n", ID);

	return ID;
}


int Comm_VI_StartDev(int sensorid)
{
	HI_S32 s32Ret;
	VI_DEV ViDev =0;
	static VI_DEV_ATTR_S	 stViDevAttr;
	memset(&stViDevAttr,0,sizeof(VI_DEV_ATTR_S));
#if 0
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
#if 0
	switch (sensorid)
	{
		case libsns_ar0130_720p:			
			memcpy(&stViDevAttr,&DEV_ATTR_AR0130_DC_720P,sizeof(stViDevAttr));
			break;
		case libsns_jxh22_720p: 
		case libsns_ov9712_720p:
	//		memcpy(&stViDevAttr,&DEV_ATTR_OV9712_DC_720P,sizeof(stViDevAttr));
	memcpy(&stViDevAttr,&DEV_ATTR_AR0130_DC_720P,sizeof(stViDevAttr));	
			break;	  
		case libsns_hm1375_720p:			
			memcpy(&stViDevAttr,&DEV_ATTR_HIMAX_1MUX,sizeof(stViDevAttr));
			break;

		default:
			break;
			
	}
	#endif
#if 0
	printf("ViDev:%d \n enIntfMode:%d\n enWorkMode:%d \n au32CompMask0:%08x \n au32CompMask1 %08x\n",ViDev,\
	 stViDevAttr.enIntfMode, stViDevAttr.enWorkMode,stViDevAttr.au32CompMask[0],stViDevAttr.au32CompMask[1]);
	printf("enScanMode:%d \n s32AdChnId0:%d \n s32AdChnId1:%d \n s32AdChnId2:%d \n s32AdChnId3%d \n",\
	 stViDevAttr.enScanMode, stViDevAttr.s32AdChnId[0],stViDevAttr.s32AdChnId[1],stViDevAttr.s32AdChnId[2],stViDevAttr.s32AdChnId[3]);
	printf("enDataSeq:%d \n enDataPath:%d \n enInputDataType:%d \n ",\
	 stViDevAttr.enDataSeq,  stViDevAttr.enDataPath,stViDevAttr.enInputDataType);
#endif
       memcpy(&stViDevAttr,&DEV_ATTR_OV9732_DC_720P_BASE,sizeof(stViDevAttr));  
	s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
	if (s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_VI_SetDevAttr failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
#if 1
	HI_S32 s32IspDev = 0;
	ISP_WDR_MODE_S stWdrMode;

	s32Ret = HI_MPI_ISP_GetWDRMode(s32IspDev, &stWdrMode);
	if (s32Ret != HI_SUCCESS)
	{
	   printf("HI_MPI_ISP_GetWDRMode failed with %#x!\n", s32Ret);
	   return HI_FAILURE;
	}
	VI_WDR_ATTR_S stWdrAttr;
	HI_MPI_VI_GetWDRAttr(ViDev, &stWdrAttr);
	stWdrAttr.enWDRMode = stWdrMode.enWDRMode;
	stWdrAttr.bCompress = HI_FALSE;

	s32Ret = HI_MPI_VI_SetWDRAttr(ViDev, &stWdrAttr);
	if (s32Ret)
	{
	   printf("HI_MPI_VI_SetWDRAttr failed with %#x!\n", s32Ret);
	   return HI_FAILURE;
	}

#endif
	s32Ret = HI_MPI_VI_EnableDev(ViDev);
	if (s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_VI_EnableDev failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


int Comm_GetPicSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S *pstSize)
{
    switch (enPicSize)
    {
        case PIC_QCIF:
            pstSize->u32Width = 176;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?144:120;
            break;
        case PIC_CIF:
            pstSize->u32Width = 352;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?288:240;
            break;
        case PIC_D1:
            pstSize->u32Width = 720;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case PIC_960H:
            pstSize->u32Width = 960;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;			
        case PIC_2CIF:
            pstSize->u32Width = 360;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case PIC_QVGA:    /* 320 * 240 */
            pstSize->u32Width = 320;
            pstSize->u32Height = 240;
            break;
        case PIC_VGA:     /* 640 * 480 */
            pstSize->u32Width = 640;
            pstSize->u32Height = 480;
            break;
        case PIC_XGA:     /* 1024 * 768 */
            pstSize->u32Width = 1024;
            pstSize->u32Height = 768;
            break;
        case PIC_SXGA:    /* 1400 * 1050 */
            pstSize->u32Width = 1400;
            pstSize->u32Height = 1050;
            break;
        case PIC_UXGA:    /* 1600 * 1200 */
            pstSize->u32Width = 1600;
            pstSize->u32Height = 1200;
            break;
        case PIC_QXGA:    /* 2048 * 1536 */
            pstSize->u32Width = 2048;
            pstSize->u32Height = 1536;
            break;
        case PIC_WVGA:    /* 854 * 480 */
            pstSize->u32Width = 854;
            pstSize->u32Height = 480;
            break;
        case PIC_WSXGA:   /* 1680 * 1050 */
            pstSize->u32Width = 1680;
            pstSize->u32Height = 1050;
            break;
        case PIC_WUXGA:   /* 1920 * 1200 */
            pstSize->u32Width = 1920;
            pstSize->u32Height = 1200;
            break;
        case PIC_WQXGA:   /* 2560 * 1600 */
            pstSize->u32Width = 2560;
            pstSize->u32Height = 1600;
            break;
        case PIC_HD720:   /* 1280 * 720 */
            pstSize->u32Width = 1280;
            pstSize->u32Height = 720;
            break;
        case PIC_HD1080:  /* 1920 * 1080 */
            pstSize->u32Width = 1920;
            pstSize->u32Height = 1080;
            break;
        default:
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

int  VideoVencStart(VENC_GRP VencGrp,VENC_CHN VencChn,VIDEO_NORM_E enNorm, PIC_SIZE_E enSize, VIDEO_RC_E enRcMode,unsigned int	framerate,unsigned int bitrate,unsigned int Gop)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_H264_S stH264Attr;
    VENC_ATTR_H264_CBR_S    stH264Cbr;
    VENC_ATTR_H264_VBR_S    stH264Vbr;
//    VENC_ATTR_H264_FIXQP_S  stH264FixQp;
    SIZE_S stPicSize;

    s32Ret = Comm_GetPicSize(enNorm, enSize, &stPicSize);
     if (HI_SUCCESS != s32Ret)
    {
        printf("Get picture size failed!\n");
        return HI_FAILURE;
    }
    /******************************************
     step 1: Greate Venc Group
    ******************************************/
    #if 0
    s32Ret = HI_MPI_VENC_CreateGroup(VencGrp);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_CreateGroup[%d] failed with %#x!\n",\
                 VencGrp, s32Ret);
        return HI_FAILURE;
    }
  #endif
    /******************************************
     step 2:  Create Venc Channel
    ******************************************/
    stVencChnAttr.stVeAttr.enType = PT_H264;

    stH264Attr.u32MaxPicWidth = stPicSize.u32Width;
    stH264Attr.u32MaxPicHeight = stPicSize.u32Height;
    stH264Attr.u32PicWidth = stPicSize.u32Width;/*the picture width*/
    stH264Attr.u32PicHeight = stPicSize.u32Height;/*the picture height*/
    stH264Attr.u32BufSize  = stPicSize.u32Width * stPicSize.u32Height ;//* 2;/*stream buffer size*/
    stH264Attr.u32Profile  = 0;/*0: baseline; 1:MP; 2:HP   ? */
    stH264Attr.bByFrame = HI_TRUE;/*get stream mode is slice mode or frame mode?*/
//    stH264Attr.bField = HI_FALSE;  /* surpport frame code only for hi3516, bfield = HI_FALSE */
//    stH264Attr.bMainStream = HI_TRUE; /* surpport main stream only for hi3516, bMainStream = HI_TRUE */
//    stH264Attr.u32Priority = 0; /*channels precedence level. invalidate for hi3516*/
//    stH264Attr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame. Invalidate for hi3516*/
   stH264Attr.u32BFrameNum = 0;/* 0: not support B frame; >=1: number of B frames */
   stH264Attr.u32RefNum = 1;/* 0: default; number of refrence frame*/
	memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));

    if(RC_CBR == enRcMode)
    {
        stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        stH264Cbr.u32Gop            = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;//framerate;//Gop;
        stH264Cbr.u32StatTime       = 1; /* stream rate statics time(s) */
  //      stH264Cbr.u32ViFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* input (vi) frame rate */
//        stH264Cbr.u32SrcFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
//       stH264Cbr.fr32DstFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
 //    stH264Cbr.fr32TargetFrmRate = framerate;/* target frame rate */
 		stH264Cbr.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;//framerate;/* target frame rate */
 		stH264Cbr.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;//framerate;/* target frame rate */
		stH264Cbr.u32BitRate = bitrate;		        
        stH264Cbr.u32FluctuateLevel = 0; /* average bit rate */
        memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
    }
    else if (RC_FIXQP == enRcMode) 
    {
  //      stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
   //     stH264FixQp.u32Gop = Gop;
   //     stH264FixQp.u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
   //     stH264FixQp.fr32TargetFrmRate = framerate;
   //     stH264FixQp.u32IQp = 20;
   //     stH264FixQp.u32PQp = 23;
   //     memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
    }
    else if (RC_VBR == enRcMode) 
    {
        stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
        stH264Vbr.u32Gop =Gop;// (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
        stH264Vbr.u32StatTime = 1;
        stH264Vbr.u32SrcFrmRate =(VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;

        stH264Vbr.fr32DstFrmRate = framerate;
        stH264Vbr.u32MinQp = 32;//32;
        stH264Vbr.u32MaxQp = 48;//40;

		stH264Vbr.u32MaxBitRate = bitrate;	
 
        memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
    }
    else
    {
        return HI_FAILURE;
    }
        

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                VencChn, s32Ret);
        return s32Ret;
    }

    /******************************************
     step 3:  Regist Venc Channel to VencGrp
    ******************************************/
   #if 0 
    s32Ret = HI_MPI_VENC_RegisterChn(VencGrp, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_RegisterChn faild with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
 #endif
    /******************************************
     step 4:  Start Recv Venc Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;

}
int VideoVencStop(VENC_GRP VencGrp,VENC_CHN VencChn)
{
    HI_S32 s32Ret;

    /******************************************
     step 1:  Stop Recv Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        //return HI_FAILURE;
    }

    /******************************************
     step 2:  UnRegist Venc Channel
    ******************************************/
   #if 0
	s32Ret = HI_MPI_VENC_UnRegisterChn(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_UnRegisterChn vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
       // return HI_FAILURE;
    }
  #endif
    /******************************************
     step 3:  Distroy Venc Channel
    ******************************************/
    s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
       // return HI_FAILURE;
    }

    /******************************************
     step 4:  Distroy Venc Group
    ******************************************/
    #if 0
    s32Ret = HI_MPI_VENC_DestroyGroup(VencGrp);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_DestroyGroup group[%d] failed with %#x!\n",\
               VencGrp, s32Ret);
        //return HI_FAILURE;
    }
    #endif
    return HI_SUCCESS;
}

int VideoVencBindVpss(VENC_CHN VeChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
	printf("=========VeChn|VpssGrp|VpssChn===================[%d,%d,%d]",VeChn,VpssGrp,VpssChn);
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;


    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("652HI_MPI_SYS_Bind failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}
int VideoVencUnBindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_GROUP;
    stDestChn.s32DevId = GrpChn;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_SYS_UnBind failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

HI_S32 VideoVpssStartGroup(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    HI_S32 s32Ret;
    VPSS_GRP_PARAM_S stVpssParam;
    
    if (VpssGrp < 0 || VpssGrp > VPSS_MAX_GRP_NUM)
    {
        printf("VpssGrp%d is out of rang. \n", VpssGrp);
        return HI_FAILURE;
    }

    if (HI_NULL == pstVpssGrpAttr)
    {
        printf("null ptr,line%d. \n", __LINE__);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    /*** set vpss param ***/
    s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VPSS_GetGrpParam failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
 //   stVpssParam.u32MotionThresh = 0;
     stVpssParam.s32MotionLimen = 0;
    s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VPSS_SetGrpParam failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

int VideoVpssEnableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, 
                                                  VPSS_CHN_ATTR_S *pstVpssChnAttr,
                                                  VPSS_CHN_MODE_S *pstVpssChnMode,
                                                  VPSS_EXT_CHN_ATTR_S *pstVpssExtChnAttr)
{
    HI_S32 s32Ret;

    if (VpssGrp < 0 || VpssGrp > VPSS_MAX_GRP_NUM)
    {
        printf("VpssGrp%d is out of rang[0,%d]. \n", VpssGrp, VPSS_MAX_GRP_NUM);
        return HI_FAILURE;
    }

    if (VpssChn < 0 || VpssChn > VPSS_MAX_CHN_NUM)
    {
        printf("VpssChn%d is out of rang[0,%d]. \n", VpssChn, VPSS_MAX_CHN_NUM);
        return HI_FAILURE;
    }

    if (HI_NULL == pstVpssChnAttr && HI_NULL == pstVpssExtChnAttr)
    {
        printf("null ptr,line%d. \n", __LINE__);
        return HI_FAILURE;
    }

    if (VpssChn < VPSS_MAX_PHY_CHN_NUM)
    {
        s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, pstVpssChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }
    else
    {
        s32Ret = HI_MPI_VPSS_SetExtChnAttr(VpssGrp, VpssChn, pstVpssExtChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            printf("%s failed with %#x\n", __FUNCTION__, s32Ret);
            return HI_FAILURE;
        }
    }
    
    if (VpssChn < VPSS_MAX_PHY_CHN_NUM && HI_NULL != pstVpssChnMode)
    {
        s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, pstVpssChnMode);
        if (s32Ret != HI_SUCCESS)
        {
            printf("%s failed with %#x\n", __FUNCTION__, s32Ret);
            return HI_FAILURE;
        }     
    }
    
    s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
int VideoVIBindVpss()
{
    HI_S32 s32Ret;    
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
        printf("HI_MPI_SYS_Bind failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }        

    return HI_SUCCESS;
}
int VideoVIBindVenc(VENC_GRP VencGrp)
{
    HI_S32 s32Ret;    
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;//HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 1;//0;

    stDestChn.enModId =HI_ID_VENC;//HI_ID_GROUP;
    stDestChn.s32DevId = 0;//VencGrp;
    stDestChn.s32ChnId =1;// 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_SYS_Bind failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }        

    return HI_SUCCESS;
}

int VideoVIBindUnVenc(VENC_GRP VencGrp)
{
    HI_S32 s32Ret;    
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDestChn.enModId = HI_ID_GROUP;
    stDestChn.s32DevId = VencGrp;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_SYS_Bind failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }        

    return HI_SUCCESS;
}


