#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "Video_MD.h"
#include "zbar.h"




#define   Alarm_Num    800
//#define   HISI_MD

#define			ALARM_INTVAL_TIME		5
MD_HANDLE  *MD_HANDLE::m_pInstance = NULL;	
extern int startupdate;
volatile  bool		mdalarmupload = false;
static 	int			mdalarm_interval = 900;
int Md_Flag_Upload = 0;

volatile int md_continue = 0;
Get_Wifi_Status  getwifistatus  = NULL;
HumanDetHandle  phd = NULL;
extern PARAMETER_MANAGE*	g_cParaManage;

//#define  md_debug
/*
@brief 输入一帧yuv用于qrcode解码
@param yuvData yuv数据缓冲区
@param imWidth   图像的宽度
@param height  图像的高度
@param imHeight  解码出来的字符串内容
@param qrBufferLen 字符串缓冲区的长度
@return >0 表示解码成功 =0 表示没找到数据  <0 表示内部可能出错
*/
int decodeQrcode(unsigned char* yuvData,int imWidth,int imHeight, char* qrData,int qrBufferLen)
{
	#if 0
	//clean the memory
	memset(qrData,0,qrBufferLen);
	//create a reader
    zbar_image_scanner_t *scanner = zbar_image_scanner_create(); 
     // configure the reader ,cloase all,open qrcode only
    zbar_image_scanner_set_config(scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 0);
    zbar_image_scanner_set_config(scanner, ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

    // obtain image data 
    int width = 0, height = 0;  
    const void *raw = NULL;  

    width = imWidth;
    height = imHeight;
    raw = (const void*)yuvData;

    //wrap image data
    zbar_image_t *image = zbar_image_create();  
    zbar_image_set_format(image, *(int*)"Y800");  
    zbar_image_set_size(image, width, height);  
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);

 	//scan the image for barcodes
    int n = zbar_scan_image(scanner, image); //n == 0 is failed  
    if(n>0)
    {
	    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
	    for(; symbol; symbol = zbar_symbol_next(symbol))
	    {
		    int strlength=0;
		    const char *codeData = zbar_symbol_get_data(symbol);
		    strlength = strlen(codeData);
		    if(strlength+1 < qrBufferLen)
		    {
				strcpy(qrData,codeData);
				break;//we just need the first one    
		    }
		    else
		    {
			    n=-1;//memory is not enough
		    }
	    }
    }

    if(scanner)zbar_image_scanner_destroy(scanner);
    //if(image)zbar_image_destroy(image); this may caused failed.......
    return n;
	#endif
	return 0;
}
int DumpYUVDataToDecode(char *result)
{
	 VIDEO_FRAME_INFO_S stFrame;
	 HI_CHAR *pUserPageAddr[2];
	 char * pVBufVirt_Y;
	 HI_U32 phy_addr,size;
	VI_CHN ViChn = 0;
	//char result[128] = {0x0};
    if (HI_MPI_VI_SetFrameDepth(ViChn, 1))
    {
        printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn);
        return -1;
    }
    usleep(5000);
    if (HI_MPI_VI_GetFrame(ViChn, &stFrame,2000))
    {
        printf("HI_MPI_VI_GetFrame err, vi chn %d \n", ViChn);
        return -1;
    }
	
	size = (stFrame.stVFrame.u32Stride[0])*(stFrame.stVFrame.u32Height)*3/2;
	phy_addr =  stFrame.stVFrame.u32PhyAddr[0];
    //printf("phy_addr:%x, size:%d\n", phy_addr, size);
    pUserPageAddr[0] = (HI_CHAR *) HI_MPI_SYS_Mmap(phy_addr, size);
    if (NULL == pUserPageAddr[0])
    {
    	printf("HI_MPI_SYS_Mmap err, vi chn %d \n", ViChn);
        return -1;
    }
   // printf("stride: %d,%d\n",pVBuf->u32Stride[0],pVBuf->u32Stride[1] );
	printf("stride: %d,%d\n",stFrame.stVFrame.u32Stride[0],stFrame.stVFrame.u32Stride[1] );

	pVBufVirt_Y = pUserPageAddr[0];
	int ret = decodeQrcode((unsigned char *)pVBufVirt_Y,1280,720, result,128);
	printf("ret:%d,-------result:%s\n",ret,result);
	HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
	HI_MPI_VI_ReleaseFrame(ViChn, &stFrame);

   // for(int h=0; h<stFrame.stVFrame.u32Height; h++)
   // {
   //     pMemContent = pVBufVirt_Y + h*pVBuf->u32Stride[0];
       // fwrite(pMemContent, pVBuf->u32Width, 1, pfd);
   // }
	if(ret>0)
		return 0;

	return -1;
	
}

Connect_Wifi  connetwifi;
//Get_Wifi_Status  getwifistatus;
bool		  isfirst=false;
bool		  QR_Scan_Thread_run = false;
bool 	CheckString(unsigned char *buf)
{
	int len = strlen((const char*)buf);
	if(len>63)
		return false;
	for(int i=0;i<len;i++)
	{
		if((buf[i]=='"')||(buf[i]==',')||(buf[i]==0xd)||(buf[i]==0xa))
		{
			buf[i] =0;
			return true;
		}
	}
	return true;
}
void *QR_Scan_Thread(void *para)
{
	printf("##### QR_Scan_Thread success\n");
	int scan_conut=0;
	char scanbuf[128]={0x0};
	pthread_detach(pthread_self());/*退出线程释放资源*/
	while(1)
	{
		if(getwifistatus!=NULL&&0== getwifistatus())/*wifi正在连接等待*/
		{
			
			sleep(1);
			continue;
		}

		if(getwifistatus!=NULL&&1== getwifistatus())/*wifi连接成功直接返回*/
		{
		
			break;
		}
		if(mdalarmupload)/*发生移动侦测，有手机放在摄像头扫描*/
		{
			scan_conut = 50;/*发生移动侦测扫描扫描50次*/
		}
		
		if(scan_conut>0)
		{	
			scan_conut--;			
		}
		else		/*发生移动侦测扫描40次退出*/
		{
			sleep(1);
			continue;
		}

		memset(scanbuf,0x0,sizeof(scanbuf));
		if(0 == DumpYUVDataToDecode(scanbuf))/*开启扫描*/
		{
			//int len = strlen(scanbuf);
			char ssid[32]={0x0};
			char passwd[64]={0x0};	
			int i=0;
			for(i=0;i<128;i++)
			{
				//printf("%02x ",scanbuf[i]);
				if(scanbuf[i] == 0x0a)
				{
					break;
				}						
			}
			if(i<128)
			{
				memcpy(ssid,scanbuf,i);
				strcpy(passwd,&scanbuf[i+1]);	

				if(CheckString((unsigned char *)ssid)==false) continue;
				if(CheckString((unsigned char *)passwd)==false) continue;			
				if(connetwifi)connetwifi(ssid,passwd,-1);/*连接wifi*/
					sleep(15);/*等待连接*/
			}
		}
		usleep(200*1000);

		
	}
	printf("success connect wifi exit QRScan\n");
	return NULL;
}

int ZMD_StartQRScan(bool isfirst,Connect_Wifi ConnectWifiCB,Get_Wifi_Status GetWifiStatusCB )
{
	
	return -1;
	connetwifi = ConnectWifiCB;
	getwifistatus = GetWifiStatusCB;
	pthread_t			pid;
	if(pthread_create(&pid, NULL, QR_Scan_Thread, NULL) < 0)//创建线程
	{
		printf("QR_Scan_Thread failed \n");
		return -1;
	}
	
	return 0;	
}

void *MDThreadEntry(void *para)
{
	
	MD_HANDLE *pMd = (MD_HANDLE *)para;
	printf("function: %s threadid %d  ,line:%d\n",__FUNCTION__, (unsigned)pthread_self(),__LINE__);
	pMd->MDThreadBody();
	return NULL;
}
int Get_Md_Status()
{
	MD_HANDLE	*pMDClass = NULL;
	pMDClass = MD_HANDLE::Instance();
	
	return pMDClass->MD_STATUS;
}

int CheckTimerTable(unsigned char chl)
{
	unsigned int CurTime = 0;
	unsigned char i = 0;
	unsigned int STime = 0, ETime = 0;
	unsigned char Week = 0;
	CAMERA_MD  MdSet;
	datetime_setting m_datetime;
	PubGetSysParameter(SYSMOTION_SET, (void *)&MdSet); 
	if(MdSet.m_Channel[0].m_uMDSwitch == 0)
	{
		//mdalarmupload = false;
		return 0;
	}
	GlobalGetSystemTime(&m_datetime);
	Week = m_datetime.week&0x07;
	CurTime = m_datetime.hour*60+m_datetime.minute;
	for(i = 0; i < 4; i++)
	{
		STime = MdSet.m_Channel[chl].m_TimeTblSet[Week].m_TBLSection[i].m_u16StartTime;
		ETime = MdSet.m_Channel[chl].m_TimeTblSet[Week].m_TBLSection[i].m_u16EndTime;
		if((CurTime >= STime) && (CurTime <= ETime) && (STime != ETime) && (MdSet.m_Channel[chl].m_TimeTblSet[Week].m_TBLSection[i].m_u8Valid))
		{
			if(MdSet.m_Channel[0].m_uAalarmOutMode&0x40)
			{
				//mdalarmupload = true;
				//printf("^^^^^^^^^^^^^^mdalarmupload^^^^^^^^^^^^^^^^\n");
			}
			else
			{
				//mdalarmupload = false;
			}
			return 1;
		}
		
	}
	//mdalarmupload = false;
	return 0;
}


MD_HANDLE::MD_HANDLE()
{
	md_snap_time = -1;
	m_objnum = 0;
	memset(&m_Objregion,0x0,sizeof(m_Objregion));	
	memset(&m_Mdpara,0x0,sizeof(Md));
	memset(m_RelMdData,0x0,sizeof(m_RelMdData));
	
	m_pid			= 0; 
	m_Mdpara.MdAlarm = 0;
	m_Mdpara.MdThreadRun = false;
	m_Mdpara.mdThreadexit = false;
	
	pthread_mutex_init(&m_mdlock, NULL);
	MD_STATUS = 0;
	MD_interval = 0;
	
}

MD_HANDLE::~MD_HANDLE()
{


}

MD_HANDLE* MD_HANDLE::Instance()
{
	if(m_pInstance == NULL)
	{
		m_pInstance = new MD_HANDLE();
	}

	return m_pInstance;
	
}


/*****************************************************************************
函数功能:初始化md参数
输入参数:

输出参数:无
返  回   值:无
使用说明:
******************************************************************************/

void MD_HANDLE::InitMDConfig()
{

	return ;
}


void MD_HANDLE::ResetMotionArea(int ch)
{
	return ;
}
/*****************************************************************************
函数功能:设置md参数
输入参数:uMask --- 要检测的区域掩码
			  sentive----灵敏度0~4
			  ch---通道号 暂时没有使用，

输出参数:无
返  回   值:无
使用说明: 
******************************************************************************/

void MD_HANDLE::MotionDetectionUserCfg(int ch, unsigned char *uMask, int sentive)
{
	if(uMask == NULL)
	{
		return ;
	}	
	int  PIC_MAX_USERSET_AREA = 0;
	int  PIC_WIDTH_NUM=0;
	int  PIC_USER_WIDTH_NUM=0;
	int	 MacroCount =0;
	int i,j;

	pthread_mutex_lock(&m_mdlock);
	m_Mdpara.MdAlarm =0 ;	
	
	PIC_MAX_USERSET_AREA = 10*10;
	PIC_WIDTH_NUM = 40;
	PIC_USER_WIDTH_NUM = 10;
	//m_MaxMdBlock = 40*30;
	memset(m_Mdpara.MDPara.mask,0x0,sizeof(m_Mdpara.MDPara.mask));
	for(i = 0;i < PIC_MAX_USERSET_AREA;i++)
	{
		if(uMask[i] == 1)
		{
			for(j = 0;j < 3;j++)
			{
				m_Mdpara.MDPara.mask[(PIC_WIDTH_NUM * 3) * (i/PIC_USER_WIDTH_NUM) + (i%PIC_USER_WIDTH_NUM) *4 + PIC_WIDTH_NUM *j] = 1;
				m_Mdpara.MDPara.mask[(PIC_WIDTH_NUM * 3) * (i/PIC_USER_WIDTH_NUM) + (i%PIC_USER_WIDTH_NUM) *4 + PIC_WIDTH_NUM *j + 1] = 1;
				m_Mdpara.MDPara.mask[(PIC_WIDTH_NUM * 3) * (i/PIC_USER_WIDTH_NUM) + (i%PIC_USER_WIDTH_NUM) *4 + PIC_WIDTH_NUM *j + 2] = 1;
				m_Mdpara.MDPara.mask[(PIC_WIDTH_NUM * 3) * (i/PIC_USER_WIDTH_NUM) + (i%PIC_USER_WIDTH_NUM) *4 + PIC_WIDTH_NUM *j + 3] = 1;
				
			}
			MacroCount+=12;
		}
	}
	#if 0
	int m=0;
	int n=0;
	for(m = 0;m < 40*30;m++)
	{
		n++;
		printf("%d ",m_Mdpara.MDPara.mask[m]);
		if(n%40 == 0)
			printf("\r\n");
	}
	#endif
	m_sentive = sentive;
	int baseratio = 0;
	if(sentive==0)
	{
		baseratio = 4;
		m_Mdpara.MDPara.Macro_threshold = 6;
	}
	else if(sentive==1)
	{
		baseratio = 8;
		m_Mdpara.MDPara.Macro_threshold = 10;    //8
	}
	else if(sentive==2)
	{
		baseratio = 10;
		m_Mdpara.MDPara.Macro_threshold = 12;   //15
	}
	else if(sentive==3)
	{
		baseratio = 15;
		m_Mdpara.MDPara.Macro_threshold = 20;
	}
	else 
	{
		baseratio = 10;
		m_Mdpara.MDPara.Macro_threshold = 10;
	}
	
	m_Mdpara.MDPara.Macro_ratio = baseratio ;
	m_Real_threshold = m_Mdpara.MDPara.Macro_threshold;

	#if 0
	printf("@@@@@@@@@@@@@@@@@@@@@  sentive:%d,Macro_ratio:%d Macro_threshold:%d \n",sentive,m_Mdpara.MDPara.Macro_ratio,m_Mdpara.MDPara.Macro_threshold );
	for(int m = 0 ; m < MAX_MACROCELL_NUM;m++)
	{
		if(m%40==0)printf("\n");
		printf("%d ",m_MDPara.mask[m]);
		
	}
	#endif
	pthread_mutex_unlock(&m_mdlock);
	return ;

}
/*****************************************************************************
函数功能:获取md状态
输入参数:
			 
			  ch---通道号 暂时没有使用，

输出参数:0 :没有报警  1:报警
返  回   值:无
使用说明: 
******************************************************************************/

int MD_HANDLE::GetVideoMdStatus(int ch)
{
	return m_Mdpara.MdAlarm;
}
int MD_HANDLE::GetRealMdStatus()
{
	if(mdalarmupload)
		return 1;
	else 
		return 0;
}


int MD_HANDLE::GetVideoObjStatus(MdobjRegion **pMdregion)
{
	*pMdregion =  m_Objregion;
	return m_objnum;
}




/*****************************************************************************
函数功能:启动移动侦测
输入参数:
输出参数:无
返  回   值:0:成功  -1:失败
使用说明: 
******************************************************************************/


int MD_HANDLE::StartMotionDetection()
{
#ifdef HISI_MD
   HI_S32 s32Ret = HI_SUCCESS;
   VDA_CHN VdaChn = 0;
   VDA_CHN_ATTR_S stVdaChnAttr;
   MPP_CHN_S stSrcChn, stDestChn;	
   
   /* step 1 create vda channel */
   stVdaChnAttr.enWorkMode = VDA_WORK_MODE_MD;
   stVdaChnAttr.u32Width   = 640;
   stVdaChnAttr.u32Height  = 480;

   stVdaChnAttr.unAttr.stMdAttr.enVdaAlg	  = VDA_ALG_REF;
   stVdaChnAttr.unAttr.stMdAttr.enMbSize	  = VDA_MB_16PIXEL;
   stVdaChnAttr.unAttr.stMdAttr.enMbSadBits   = VDA_MB_SAD_8BIT;
   stVdaChnAttr.unAttr.stMdAttr.enRefMode	  = VDA_REF_MODE_DYNAMIC;
   stVdaChnAttr.unAttr.stMdAttr.u32MdBufNum   = 5;
   stVdaChnAttr.unAttr.stMdAttr.u32VdaIntvl   = 10;	
   stVdaChnAttr.unAttr.stMdAttr.u32BgUpSrcWgt = 128;
   stVdaChnAttr.unAttr.stMdAttr.u32SadTh	  = 100;
   stVdaChnAttr.unAttr.stMdAttr.u32ObjNumMax  = MAXOBJNUM;

   s32Ret = HI_MPI_VDA_CreateChn(VdaChn, &stVdaChnAttr);
   if(s32Ret != HI_SUCCESS)
   {
	   printf("HI_MPI_VDA_CreateChn failed with %#x!\n", s32Ret);
	   return s32Ret;
   }

   /* step 2: vda chn bind vi chn */
   stSrcChn.enModId = HI_ID_VIU;
   stSrcChn.s32ChnId = 0;
   stSrcChn.s32DevId = 0;

   stDestChn.enModId = HI_ID_VDA;
   stDestChn.s32ChnId = VdaChn;
   stDestChn.s32DevId = 0;

   s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
   if(s32Ret != HI_SUCCESS)
   {
	   printf("HI_MPI_SYS_Bind failed with %#x!\n", s32Ret);
	   return s32Ret;
   }

   /* step 3: vda chn start recv picture */
   s32Ret = HI_MPI_VDA_StartRecvPic(VdaChn);
   if(s32Ret != HI_SUCCESS)
   {
	   printf("HI_MPI_VDA_StartRecvPic failed with %#x!\n", s32Ret);
	   return s32Ret;
   }
  #endif
	if(m_Mdpara.MdThreadRun == false )
	{
		m_Mdpara.MdThreadRun= true;
		
		if(pthread_create(&m_pid, NULL, MDThreadEntry, (void *)(this)) < 0)//创建线程
		{
			printf("MDThreadEntry failed \n");
			m_Mdpara.MdThreadRun= false;
			m_Mdpara.mdThreadexit= true;
		}
	}
	
	return HI_SUCCESS;
}
/*****************************************************************************
函数功能:停止移动侦测
输入参数:	
输出参数:无
返	回	 值:0:成功	-1:失败
使用说明: 
******************************************************************************/

int MD_HANDLE::StopMdProcess()
{
	return 0;
}


int MD_HANDLE::ALARM_PRO()
{
	int retval = MD_STATUS;
	static int	GetParmTimeTick = 0;
	int alarm_interal=900;/*默认是15min*/
	/*
	 *retval = 0,画面基本无变化，不需要录像
	 *retval = 1,画面有变化，需要录像，但不需要上报移动侦测
	 *retval = 3,需要录像且上报移动侦测
	*/	
	//if(retval!=0)
	//	printf("========================MD_STATUS:%d\n",MD_STATUS);
	if (3 == retval)
	{
		/* 填充视频流头的移动侦测BIT位*/
		mdalarmupload = true;
		/* ie状态标志位 */
		m_Mdpara.MdAlarm =1;			
		//printf("========================MD_STATUS:%d\n",MD_STATUS);
		time_t now_time = time(NULL);

		/*移动侦测告警开关*/
		if(0 == GetWebScheduleSwitch(1))
		{
			return 0;
		}	
		alarm_interal=GetAlarmInterval();

		/*检测是否在报警间隔内，报警间隔时间为65s*/
		//if(now_time - MD_interval >= MDTION_ALARM_DEALY_TIME)
		//printf("====================diff:%ld,interval:%d\n",now_time - MD_interval,mdalarm_interval);
		if(now_time - MD_interval >= alarm_interal)
		{
			//printf("############### md_alarm_upload ######################\r\n");
			
			/* P2P状态标志位 */
			Set_Md_Flag(1);		

			/* 通知移动侦测告警抓图上传 */
			BroadcastAlarmsEx(P2P_ALARM_MD, 0);;
		}
		else
		{
			/* 清除移动状态 */
			Set_Md_Flag(0);	
		}
		SetMdAlarmLink(1,RECORD_LEVEL3);
		MD_interval = now_time;
	}
	else if(1 == retval)
	{
		/* 清除移动状态 */
		Set_Md_Flag(0);
		m_Mdpara.MdAlarm =0;
		
		/* 填充视频流头的移动侦测BIT位*/
		mdalarmupload = true;
		/* 智能录像状态 */
		MDRecordSet();
	}
	else if(0 == retval)
	{
		/* 清除移动状态 */
		Set_Md_Flag(0);
		m_Mdpara.MdAlarm = 0;
		mdalarmupload = false;
		SetMdAlarmLink(0,RECORD_LEVEL3);
	}
	if(GetParmTimeTick++>200)
	{
		GetParmTimeTick = 0;
		web_sync_param_t sync_data;
		PubGetSysParameter(WEB_SET, (void *)&sync_data);
		mdalarm_interval = sync_data.alarm_interval;
		m_sentive =WebSensitivity_to_Local( sync_data.sensitivity);
		//printf("---------------------mdalarm_interval:%d,m_sentive:%d\n",mdalarm_interval,m_sentive);

	}
	return 0;
}
int MD_HANDLE::MOTION_CHECK( HumanDetHandle  phdHandle )
{
//	int retval = -1;
	char * pVBufVirt_Y;

	VIDEO_FRAME_INFO_S stFrame;
	HI_CHAR *pUserPageAddr[2];
	HI_U32 phy_addr,size;
	VI_CHN ViChn = 0;
	int ret = -1;
	
	/* 获取一帧原始YUV数据 */

	//Ret=HI_MPI_VPSS_GetChnFrame(0,0,&stFrameInfo,1000);
	if((ret = HI_MPI_VPSS_GetChnFrame(0,0, &stFrame,2000)) != 0)
	{
		printf("HI_MPI_VI_GetFrame err, vi chn %d ,ret:%x\n", ViChn,ret);
		return -1;
	}	

	/* 移动侦测区域初始化(暂为全局检测，不可修改)*/
	//size = (stFrame.stVFrame.u32Stride[0])*(stFrame.stVFrame.u32Height)*3/2;
	size = (stFrame.stVFrame.u32Stride[0]) * (stFrame.stVFrame.u32Height) * 3 / 2;

	
	phy_addr =	stFrame.stVFrame.u32PhyAddr[0];
	/* hisi 系统映射初始化*/
	pUserPageAddr[0] = (HI_CHAR *) HI_MPI_SYS_Mmap(phy_addr, size);
	if (NULL == pUserPageAddr[0])
	{
	   printf("HI_MPI_SYS_Mmap err, vi chn %d \n", ViChn);
	  // HI_MPI_VI_ReleaseFrame(ViChn, &stFrame);
	   HI_MPI_VPSS_ReleaseChnFrame(0, 0, &stFrame);
	   return -1;
	}
	pVBufVirt_Y = pUserPageAddr[0];

	/* 移动侦测算法检测*/
	MD_STATUS = UpdateAndHD(phdHandle, (uchar*)pVBufVirt_Y, 1280, 720, 1280, m_sentive );
	//if(MD_STATUS==3)printf("===========================%d\n",MD_STATUS);
	ALARM_PRO();
	
	/* 释放资源 */
	HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
	//HI_MPI_VI_ReleaseFrame(ViChn, &stFrame);
	HI_MPI_VPSS_ReleaseChnFrame(0, 0, &stFrame);
	return 0;
}
int MD_HANDLE::MDThreadBody()
{
	
	printf(" MDThreadBody TID:%lu !!!!\n", pthread_self());

	pthread_detach(pthread_self());/*退出线程释放资源*/

	HI_U32 depth = 0;

	HI_U32 u32OnlineMode;
	HI_MPI_VPSS_SetDepth(0,0,2);
	HI_MPI_VPSS_GetDepth(0,0,&depth);
    HI_MPI_SYS_GetViVpssMode(&u32OnlineMode);

	/* MD 初始化建模 */
	phd = CreateBackModel();
	P2P_MD_REGION_CHANNEL mdset;
	memset(&mdset,0x0,sizeof(P2P_MD_REGION_CHANNEL));
	if(g_cParaManage)g_cParaManage->GetSysParameter(MD_SET,&mdset);
	SetROI(phd,1280*mdset.x,720*mdset.y,1280*mdset.width,720*mdset.height,0);
	while(m_Mdpara.MdThreadRun)
	{
		if(GetDeviceStatus()==false)
		{
			sleep(5);
			continue;
		}
		MOTION_CHECK(phd);
		usleep(30000);
	}
	
	/* 释放建模背景资源 */
	ReleaseBackModel( &phd );
	


	
	return 0;	
}

/*****************************************************************************
函数功能:分析区域是否产生移动侦测
输入参数:pstVdaData 分析数据源，内部使用
输出参数:无
返  回   值:0:成功  -1:失败
使用说明: 分析结果按区域面积的大小从大到小排序
******************************************************************************/

int MD_HANDLE::AnalyseMDObj(VDA_DATA_S *pstVdaData)
{
	VDA_OBJ_S *pstVdaObj;
	HI_U32 i,j,unchange;
	 
	if (HI_TRUE != pstVdaData->unData.stMdData.bObjValid)
	{
		printf("bMbObjValid = FALSE.\n");
		return HI_SUCCESS;
	}
	if( pstVdaData->unData.stMdData.stObjData.u32ObjNum < 3)
	{
		m_objnum = 0;
		return HI_SUCCESS;
	}
	#if 0
	printf("ObjNum=%d, IndexOfMaxObj=%d, SizeOfMaxObj=%d, SizeOfTotalObj=%d\n", \
				   pstVdaData->unData.stMdData.stObjData.u32ObjNum, \
			 pstVdaData->unData.stMdData.stObjData.u32IndexOfMaxObj, \
			 pstVdaData->unData.stMdData.stObjData.u32SizeOfMaxObj,\
			 pstVdaData->unData.stMdData.stObjData.u32SizeOfTotalObj);	
	#endif
	for (i=0; i<pstVdaData->unData.stMdData.stObjData.u32ObjNum; i++)
	{
		pstVdaObj = pstVdaData->unData.stMdData.stObjData.pstAddr + i;
		if(i == MAXOBJNUM)
		{
			m_objnum = 0;
			return HI_FAILURE;
		}
		m_Objregion[i].Left		= pstVdaObj->u16Left;
		m_Objregion[i].Top 		= pstVdaObj->u16Top;
		m_Objregion[i].Right 	= pstVdaObj->u16Right;
		m_Objregion[i].Bottom 	= pstVdaObj->u16Bottom;
		m_Objregion[i].area  	= (pstVdaObj->u16Right-pstVdaObj->u16Left)*(pstVdaObj->u16Bottom-pstVdaObj->u16Top);
		#if 0
		printf( "[%d]\t left=%d, top=%d, right=%d, bottom=%d\n", i, \
			  pstVdaObj->u16Left, pstVdaObj->u16Top, \
			  pstVdaObj->u16Right, pstVdaObj->u16Bottom);
		#endif
	}	
	m_objnum = pstVdaData->unData.stMdData.stObjData.u32ObjNum;

	MdobjRegion tmp;
	for (i=1; i<m_objnum; i++)
	{
		unchange = 1;
		for (j=0; j<m_objnum-1-i; j++)
		{
			if (m_Objregion[j].area < m_Objregion[j+1].area)
			{
				 unchange = 0;			
				 
				tmp.area= m_Objregion[j].area;
				tmp.Left= m_Objregion[j].Left;
				tmp.Top= m_Objregion[j].Top;
				tmp.Right= m_Objregion[j].Right;
				tmp.Bottom= m_Objregion[j].Bottom;
				
				m_Objregion[j].area= m_Objregion[j+1].area;
				m_Objregion[j].Left= m_Objregion[j+1].Left;
				m_Objregion[j].Top= m_Objregion[j+1].Top;
				m_Objregion[j].Right= m_Objregion[j+1].Right;
				m_Objregion[j].Bottom= m_Objregion[j+1].Bottom;

				m_Objregion[j+1].area = tmp.area;
				m_Objregion[j+1].Left = tmp.Left;
				m_Objregion[j+1].Top = tmp.Top;
				m_Objregion[j+1].Right = tmp.Right;
				m_Objregion[j+1].Bottom = tmp.Bottom;

			}
		}
		if (unchange == 1) //不再有变化，无需再排
		{
			break;
		}
	
	}
	
	return HI_SUCCESS;
}


/*****************************************************************************
函数功能:分析宏块判断是否产生移动侦测
输入参数:pstVdaData 分析数据源，内部使用
输出参数:无
返  回   值:0:成功  -1:失败
使用说明: 
******************************************************************************/

int MD_HANDLE::AnalyseMDSad(VDA_DATA_S *pstVdaData)
{

	HI_U32 i, j,k;
	k =0;
	HI_VOID *pAddr;	
	
	if (HI_TRUE != pstVdaData->unData.stMdData.bMbSadValid)
	{
		printf("bMbSadValid = FALSE.\n");
		return HI_SUCCESS;
	}
	memset(m_RelMdData,0x0,sizeof(m_RelMdData));
	for(i=0; i<pstVdaData->u32MbHeight; i++)//30
	{
		pAddr = (HI_VOID *)((HI_U32)pstVdaData->unData.stMdData.stMbSadData.pAddr
					+ i * pstVdaData->unData.stMdData.stMbSadData.u32Stride);
	
		for(j=0; j<pstVdaData->u32MbWidth; j++)//40
		{
			HI_U8  *pu8Addr;
			HI_U16 *pu16Addr;
		
			if(VDA_MB_SAD_8BIT == pstVdaData->unData.stMdData.stMbSadData.enMbSadBits)
			{
				pu8Addr = (HI_U8 *)pAddr + j;
				m_RelMdData[k++] = *pu8Addr;
			}
			else
			{
				pu16Addr = (HI_U16 *)pAddr + j;

				//printf("%04d ",*pu16Addr);
			}
			
		}
		
		//printf("\n");
	}	

	int AlarmBlock = 0;
	int SumBlock =0;
	int SumMd =0;
	int diff =0;
	int avg=0;
	pthread_mutex_lock(&m_mdlock);
	for(int i=0; i<MAX_MACROCELL_NUM; i++)
	{
		if(1 == m_Mdpara.MDPara.mask[i])
		{
			SumBlock++;
			if(m_RelMdData[i]>m_Real_threshold)
			{
				AlarmBlock++;
			}
			
		}
		SumMd+=m_RelMdData[i];
	}	
	avg = SumMd/1200;
	for(int i=0; i<MAX_MACROCELL_NUM; i++)
	{
		if(m_RelMdData[i]>avg)
			diff+=m_RelMdData[i]-avg;
		else
			diff+=avg-m_RelMdData[i];
	}
	static unsigned int count =0;
	if(AlarmBlock > Alarm_Num)
	{
		count = 0;
	}
	//printf("\ncount:%d,BlockSum:%d AlarmBlock:%d ===%d\n",count,SumBlock,AlarmBlock,m_Mdpara.MdAlarm );

	if(count++>15&&(AlarmBlock>(SumBlock*(m_Mdpara.MDPara.Macro_ratio))/100))/*3s让切换稳定*/
	{
		mdalarmupload = true;
		//sleep(10);
		if( 1==CheckTimerTable(0))
		{
			
			//m_Mdpara.MdAlarm = 1;
			MDAlarmSet(true);
		}
		else
		{
			//m_Mdpara.MdAlarm = 0;
			MDAlarmSet(false);
		}

		
		//printf("\n\n\n\n\n\nBlockSum:%d AlarmBlock:%d ===%d\n",SumBlock,AlarmBlock,m_Mdpara.MdAlarm );
	}
	else
	{
		//m_Mdpara.MdAlarm  = 0;
		MDAlarmSet(false);
		mdalarmupload = false;
	}


	if((diff<2000)&&(avg>3))
	{
		if(avg<6)
		{
			m_Real_threshold =  30;   //25
		}
		else if(avg<8)
		{
			m_Real_threshold =  35;   //30
		}
		else if(avg<10)
		{
			m_Real_threshold =  40;   //35
		}
		else if(avg<12) 
		{
			m_Real_threshold =  45;   //40
		}
		else 
		{
			m_Real_threshold =  50;   //45
		}

	}
	else if(avg<5)
	{
		m_Real_threshold = m_Mdpara.MDPara.Macro_threshold;
	}

	pthread_mutex_unlock(&m_mdlock);

	return 0;
}

int MD_HANDLE::Get_Md_Flag()
{
	return md_snap_time;
}

void MD_HANDLE::Set_Md_Flag(int flag)
{
	md_snap_time = flag;
}

int MD_HANDLE::MDAlarmSet(bool alarm)
{
#if 0
	static time_t  timebak =  time(NULL);
	static int waittime = 0;
	int  difftime = abs(time(NULL)-timebak);	
	/*移动侦测结束后，预留5s*/
	if(waittime>0)
	{	
		//printf("-----alarm wainttime:%d\n",waittime);
		waittime--;
		return 0;
	}
	
	if((Md_Flag_Upload == 0)&&(true == alarm))
	{
		/*移动侦测告警开关*/
		if(0 == GetWebScheduleSwitch(1))
		{
			return 0;
		}	
		printf("MDAlarmSet start\n");
		Md_Flag_Upload = 1;
		md_continue = 1;
		m_Mdpara.MdAlarm =1;
		timebak =  time(NULL);
		//printf("alarm = %d,MdAlarm= %d,################################start md upload!",alarm,m_Mdpara.MdAlarm);
		/*确定是否上报，通知网络库抓拍*/
		Set_Md_Flag(1);	
		BroadcastAlarms();
	}	
	
	if((Md_Flag_Upload == 1)&&(m_Mdpara.MdAlarm == 1)&&(false == alarm)&&difftime>=ALARM_DEALY_TIME )
	{
		//int wait_count =0;
		Md_Flag_Upload = 0;
		m_Mdpara.MdAlarm = 0;
		timebak =  time(NULL);
		Set_Md_Flag(0);
		md_continue = 0;
		printf("alarm = %d,MdAlarm= %d,############################### alarm stop\n",alarm,m_Mdpara.MdAlarm);
		waittime = ALARM_INTVAL_TIME*3;
		return 0;
	}
	if(true == alarm)
	{
		Md_Flag_Upload = 1;
		m_Mdpara.MdAlarm =1;
		timebak =  time(NULL);
		md_continue = 1;
		Set_Md_Flag(0);
		printf("alarm = %d,MdAlarm= %d,-----alarm continue\n",alarm,m_Mdpara.MdAlarm);
		//return 0;
	}
	
	return 0;
#else

	static time_t begin;
	time_t now = time(NULL);

	/*检测是否在报警间隔内，报警间隔时间为65s*/
	if(now - begin >= MDTION_ALARM_DEALY_TIME)
	{
		if(true == alarm)
		{
			begin = now;
			Md_Flag_Upload = 0;
			
		}
		else
		{
			/*移动侦测是否在持续时间内，反馈给声音，确定是异常声音是否上报*/
			md_continue = 0;
			Set_Md_Flag(0);
			printf("#################--alarm stop--##############\r\n");
			//waittime = ALARM_INTVAL_TIME*3;
			return 0;
		}	
	}	
	else
	{
		if(Md_Flag_Upload == 1)
		{
			if(true == alarm)
			{
				printf("############--alarm continue--##########\r\n ");
				m_Mdpara.MdAlarm =1;
				md_continue = 1;
				Set_Md_Flag(0);
				begin = time(NULL);
			}
		}
		return 0;
	}	

	if(Md_Flag_Upload == 0)
	{
		if(true == alarm)
		{
			printf("############################### MDAlarmSet start\r\n");
			Md_Flag_Upload = 1;
			md_continue = 1;
			m_Mdpara.MdAlarm =1;
			/*通知是否为第一次，决定是否上报*/
			Set_Md_Flag(1);
			/*移动侦测告警开关*/
			if(0 == GetWebScheduleSwitch(1))
			{
				return 0;
			}	
			BroadcastAlarmsEx(P2P_ALARM_MD, 0);;
		}
		else
		{
			return 0;
		}	
	}	
		
	return 0;

#endif
}

int MD_HANDLE::MDRecordSet()
{
	m_Mdpara.MdAlarm = 1;
	return 0;
}

