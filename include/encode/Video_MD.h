
#ifndef __VIDEO_MD_H__
#define __VIDEO_MD_H__

#include "common.h"
#include "hi_type.h"
#include "hi_comm_vda.h"
#include "mpi_vda.h"
#include "Audio.h"
#include "HDalg.h"


#include "ModuleFuncInterface.h"
#include "parametermanage.h"

#define MAXOBJNUM		32

#define VIDEOINPUT_MAX_CHANNEL_NUM		CHANNEL_MAX
#define WIDTH_NUM	40   //80
#define HEIGHT_NUM  30   //45
#define MAX_MACROCELL_NUM   WIDTH_NUM*HEIGHT_NUM	/* 80*45 or 40*30 */
#define USER_WIDTH_NUM  20  //10
#define USER_HEIGHT_NUM  15  //10
#define MAX_USERSET_AREA	USER_WIDTH_NUM*USER_HEIGHT_NUM

#define MDTION_ALARM_DEALY_TIME		65
#define MD_VALID_NAME_LEN   	(HI_U8)16
#define MD_MACROCELL_WIDTH      (HI_U8)16
#define MD_MACROCELL_HEIGHT     (HI_U8)16

typedef int 	(*Connect_Wifi)(char *ssid ,char *password,int lang);/*连接wifi 回调*/
typedef int 	(*Get_Wifi_Status)(void);/*获取wifi连接状态，成功true,失败false*/
int ZMD_StartQRScan(bool isfirst,Connect_Wifi ConnectWifiCB,Get_Wifi_Status GetWifiStatusCB );/*启动QR扫描*/
int Get_Md_Status();


typedef struct hiMD_AREA_CFG_S
{
	HI_S32 chno; /* 通道号*/
	HI_BOOL enable_flag; /* 用户指定是否使能该区域进行移动侦测*/
	HI_BOOL run_flag; /*该区域正在进行移动侦测标志*/
	HI_S32 Macro_threshold; /* 宏块阈值 */
	HI_S32 Macro_ratio; /* 区域中报警宏块比例 */
	HI_S32 FrameInterval; /* 帧间隔，每隔几帧做一次移动侦测处理 */
	RECT_S m_Area;
	HI_S8 userMask[MAX_USERSET_AREA];
	HI_S8 mask[MAX_MACROCELL_NUM]; /* 单个移动侦测区域对应的宏块号 */
}MD_AREA_CFG_S;

//其中，结构体MD_STATUS_S结构定义如下。
typedef struct hiMD_STATUS_S
{
HI_U8 u8Mask[MAX_MACROCELL_NUM]; /* 移动侦测宏块侦测标识 */
HI_U8 u8MDValue[MAX_MACROCELL_NUM]; /* SAD，取值0～100表示 */
} MD_STATUS_S;
typedef struct _MdInfo_
{
	MD_AREA_CFG_S		MDPara;
	int 				MdAlarm;	
	bool				MdThreadRun;	
	bool 				mdThreadexit;
}Md;
typedef struct _MdObjRegion_
{
	unsigned short		Left;
	unsigned short		Top;
	unsigned short		Right;
	unsigned short		Bottom;
	unsigned int 		area;
}MdobjRegion;




class MD_HANDLE
{

private :

	
	
	
	static  MD_HANDLE		*m_pInstance;	
	


	Md					m_Mdpara;
	MdobjRegion			m_Objregion[MAXOBJNUM];
	unsigned char 		m_RelMdData[MAX_MACROCELL_NUM];
	unsigned int		m_objnum;	
	int					m_Real_threshold;
	int 				m_sentive;
	int 				m_web_upload;
	pthread_t			m_pid;
	pthread_mutex_t 	m_mdlock;









	/*****************************************************************************
	函数功能:分析区域是否产生移动侦测
	输入参数:pstVdaData 分析数据源，内部使用
	输出参数:无
	返  回   值:0:成功  -1:失败
	使用说明: 
	******************************************************************************/
	int AnalyseMDObj(VDA_DATA_S *pstVdaData);
	/*****************************************************************************
	函数功能:分析宏块判断是否产生移动侦测
	输入参数:pstVdaData 分析数据源，内部使用
	输出参数:无
	返  回   值:0:成功  -1:失败
	使用说明: 
	******************************************************************************/
	int AnalyseMDSad (VDA_DATA_S *pstVdaData);
	int MDAlarmSet(bool alarm);
	int MDRecordSet();

public:

	MD_HANDLE();
	~MD_HANDLE();
	static  MD_HANDLE*  Instance();
	/*****************************************************************************
	函数功能:初始化md参数
	输入参数:
	
	输出参数:无
	返	回	 值:无
	使用说明:
	******************************************************************************/
	void InitMDConfig();
	/*****************************************************************************
	函数功能:设置md参数
	输入参数:uMask --- 要检测的区域掩码
				  sentive----灵敏度0~4
				  ch---通道号 暂时没有使用，
	
	输出参数:无
	返	回	 值:无
	使用说明: 
	******************************************************************************/
	void MotionDetectionUserCfg(int ch, unsigned char *uMask, int sentive);
	/*****************************************************************************
	函数功能:获取md状态
	输入参数:				 
				  ch---通道号 暂时没有使用，
	
	输出参数:0 :没有报警  1:报警
	返	回	 值:无
	使用说明: 
	******************************************************************************/

	int GetVideoMdStatus(int ch);
	int GetRealMdStatus();
	int GetVideoObjStatus(MdobjRegion **pMdregion);
	/*****************************************************************************
	函数功能:启动移动侦测
	输入参数:	
	输出参数:无
	返	回	 值:0:成功	-1:失败
	使用说明: 
	******************************************************************************/

	int StartMotionDetection();
	/*****************************************************************************
	函数功能:
	输入参数:	
	输出参数:无
	返	回	 值:无
	使用说明: 
	******************************************************************************/

	void ResetMotionArea(int ch);
	/*****************************************************************************
	函数功能:停止移动侦测
	输入参数:	
	输出参数:无
	返	回	 值:0:成功	-1:失败
	使用说明: 
	******************************************************************************/
	int StopMdProcess();

	
	/*****************************************************************************
	函数功能:移动侦测处理线程
	输入参数:	
	输出参数:无
	返	回	 值:0:成功	-1:失败
	使用说明: 
	******************************************************************************/
	int MDThreadBody();
	


	int MOTION_CHECK( HumanDetHandle  phdHandle );
	int ALARM_PRO();
	volatile int md_snap_time;
	int		MD_STATUS;
	long 	MD_interval;
	void Set_Md_Flag(int flag);
	int Get_Md_Flag();
	
};

#endif 

