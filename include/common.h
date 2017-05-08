
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
//#include "memwatch.h"
//IE控件版本号
#define CAB_VERSION		"1,1,6,49"
//#define 	__DEBUG_ON__						// 打印信息开关

#define  SUBSTREAM_ENC

#ifdef 	SUBSTREAM_ENC
#define	SENDSUBSTREAM_ENC
#endif		
#define	SUBSTREAMBITRATE		32
#define	SUBSTREAMWIDE			176
#define	SUBSTREAMHIGHT			144
#define	SUBSTREAMFRAMERATE		5
//#define   VOICE_MIC_TALK_FUNC		// 语音对讲

/*add by harvey */
//#ifdef ZMDISSBVS23EM

/*版本为动态ip OPEN_DHCP =1 否则为0*/
#define		OPEN_DHCP			1
#define		HARDWAREVERSION		"799990092"
#define		IPC720P

#ifdef AR9271_IPC
#define		DEVICENAME			"ZH-IXB1D-WAC"	
#define		SOFTWAREVERSION		"V7.1.0.8"
#elif defined NORMAL_IPC
#define		DEVICENAME			"ZH-IXB1D-WAC"
#define		SOFTWAREVERSION		"V7.8.0.91"
#elif defined PT_IPC
//#define		DEVICENAME			"ZH-IXB1D-WAC"
//#define		SOFTWAREVERSION		"V7.2.0.31001"
#define		DEVICENAME			"SD-H2911"
#define		SOFTWAREVERSION		"V8.0.0.11001"
#elif defined MINI_IPC
#define		DEVICENAME			"ZH-IXB1D-WAC"
#define		SOFTWAREVERSION		"V7.3.0.1"
#elif defined NEW_NORMAL_IPC
#define		DEVICENAME			"ZH-IXB1D-WAC"
#define		SOFTWAREVERSION		"V7.9.0.91"
#elif defined V74_NORMAL_IPC
#define		DEVICENAME			"ZH-IXB1D-WAC"
#define		SOFTWAREVERSION		"V7.4.0.18"
#elif defined SPOE_IPC
#define		DEVICENAME			"ZP-IBH23-S"
#define		SOFTWAREVERSION		"V7.0.0.07001"
#elif defined H42SPOE_IPC
#define		DEVICENAME			"ZM-SS71D001-S"
#define		SOFTWAREVERSION		"V7.7.0.05003"
#elif defined H42PT_IPC
#define		DEVICENAME			"ZM-SS71D001-S"
#define		SOFTWAREVERSION		"V7.13.0.15002"


#elif defined V200_NORMAL_IPC
#define		DEVICENAME			"SD-H2902"
#define		SOFTWAREVERSION		"V8.0.0.22"

#elif defined V200_SPOE_IPC
#define		DEVICENAME			"SD-H2903"
#define		SOFTWAREVERSION		"V8.1.0.18"
#endif






#ifdef V200
#define		UBOOTVERSION		"V8.0.0.0"
#define		KERNELVERSION		"V8.0.0.0"
#define		ROOTFSVERSION		"V8.0.0.0"
#else
#define		UBOOTVERSION		"V7.0.0.0"
#define		KERNELVERSION		"V7.0.0.0"
#define		ROOTFSVERSION		"V7.0.0.0"
#endif



#define 	S_FAILURE			-1
#define	S_SUCCESS			0
     
#define 	S_HDD_FULL			-2			

#define VGA_Contrast						98
#define VGA_Hue							252
#define VGA_Brightness					192
#define VGA_Saturation					128




#define	DVR_1				1

//#define TMW_RERSION					1

#ifdef TMW_RERSION
#define GROUNDCOLOR	 				0xa94a
#define FILEUNLOCKCOLOR 				0xdef7
#define FILELOCKCOLOR 					0x825f

#define STATICCOLOR	 					0xdef7
#define OUTLINECOLOR	 				0x825f
#else
#define GROUNDCOLOR 					0xa529
#define FILEUNLOCKCOLOR 				0xdef7
#define FILELOCKCOLOR 					0x825f

#define STATICCOLOR	 					0xdef7
#define OUTLINECOLOR	 				0x825f
#endif



#define	CHANNEL     			16
#define	ALARMOUTPORTNUM	8

#define		RELEASE_APP			1
//#define	ENGLISH_DVR			1

/*只需在makefile或者make 命令编译时制定机型即可打开特定宏定义by harvey*/

//#define	IPC720P
//#define 	IPCVGA

#ifdef DVR_1
#define 	MAX_REC_CHANNEL		1
#define 	MAX_VIDEO_CHANNEL	1	//App支持的最大视频通道数

#ifdef IPC720P
#define 	MAX_OSD_CHANNEL		4 //3  //2			
#else
//modify by arvin 20121120
#define 	MAX_OSD_CHANNEL		4//2  //2			
#endif
#endif

#ifdef DVR_4
#define 	MAX_REC_CHANNEL		4
#define 	MAX_OSD_CHANNEL		8			
#endif

#ifdef DVR_8
#define 	MAX_REC_CHANNEL		8
#define 	MAX_OSD_CHANNEL		16			
#endif

#define		SUB_ENCODE_CHANNEL	MAX_OSD_CHANNEL

#define 	MAX_CHN				MAX_REC_CHANNEL

#define   	CHANNEL_NUM			MAX_REC_CHANNEL

#define		CHANNEL_MAX			MAX_REC_CHANNEL

#define 	MAX_ENCODE_CHANNEL	MAX_REC_CHANNEL

#define 	MAX_SNAP_CH_NUM		MAX_REC_CHANNEL
//#define 	HIS_DEMB			// 开发板调试模式		

#define 	AUDIOENC_ADPCM

#define	Def_Hue						128
#define	Def_Contrast				128
#define	Def_Saturation				128


#define	Def_Brightness 				128 //0x4



#define	Type_Hue				0
#define	Type_Contrast			1
#define	Type_Saturation			2
#define	Type_Brightness 		3


//版本信息
//*8-15 bits保留
#define		REL_SPECIAL_COM		0x00000000
//*16-23 bits 代表设备类型00:IPC CMOS VGA 01:IPC CMOS 720P 02:IPC CMOS 1080P 03 IPC CCD 04:DVR 05:NVR
#ifdef HI1080P_IPC
#define		REL_VERSION_COM		0x00020000
#else
#define     REL_VERSION_COM		0x00010000
#endif

//#ifdef IPCVGA
//#define		SOFTWAREVERSION	"V.1.2.02"
//#define		REL_VERSION_COM		0x00010000
//#define		DEVICENAME			"ZMD-ISV-BFS23NM"	//枪机
//#define		DEVICENAME			"ZMD-ISV-OAS13NM"	//方块机
//added by panjy
#define VGA_STREAM_ENC 1
#define VGA_CHANNEL_COUNT 1
#define VGA_CHANNEL 2
#define SUPPORT_SD_CARD 1
//end
//#endif
//#ifdef IPC720P
//#define		SOFTWAREVERSION	"V.1.0.10"
//#define		REL_VERSION_COM		0x00010000
//#define     	DEVICENAME  "ZMD-ISS-DSK36NM"				//半球
//#define		DEVICENAME			"ZMD-ISS-BFS23NM"	//枪机
//#define		DEVICENAME			"ZMD-ISS-OAS13NM"	//方块机
//added by panjy
#define VGA_STREAM_ENC 1
#define VGA_CHANNEL_COUNT 1
#define VGA_CHANNEL 2
#define SUPPORT_SD_CARD 1
//end
//#endif







//*24-31表示芯片类型
/*IPC */
#define CHIP_TYPE_HI3507           0x50000000 /*3507芯片*/
#define CHIP_TYPE_HI3518_C          0x57000000 /*3518C 芯片*/
#define CHIP_TYPE_HI3518            0x58000000 /*3518 芯片*/
#define CHIP_TYPE_HI3516_C          0x59000000  /*3516C 芯片*/
#define  CHIP_TYPE_TI368            0x60000000 /*dm368*/
#define  CHIP_TYPE_TI365            0x61000000 /*dm365*/
#define CHIP_TYPE_HI3518C_MINI_IPC  0x6F000000  /*3518C 芯片  MINI_IPC*/
#define CHIP_TYPE_HI3518E            0x62000000 /*3518E 芯片*/

 
/*NVR/DVR */
#define     CHIP_TYPE_HI3515        0x52000000
#define     CHIP_TYPE_HI3520        0x54000000
#define     CHIP_TYPE_HI3531        0x56000000
#define     CHIP_TYPE_HI3520_D      0x5A000000
#define     CHIP_TYPE_HI3521        0x5B000000
#define     CHIP_TYPE_HI3515_A      0x5D000000
#define     CHIP_TYPE_HI3535        0x5E000000

#ifdef HI3518C_IPC
#define		DEV_TYPE_INFO		CHIP_TYPE_HI3518C_MINI_IPC+REL_VERSION_COM+REL_SPECIAL_COM+MAX_REC_CHANNEL
#elif defined HI1080P_IPC
#define     DEV_TYPE_INFO		CHIP_TYPE_HI3516_C+REL_VERSION_COM+REL_SPECIAL_COM+MAX_REC_CHANNEL
#else
#define     DEV_TYPE_INFO		CHIP_TYPE_HI3518E+REL_VERSION_COM+REL_SPECIAL_COM+MAX_REC_CHANNEL
#endif

#define		VIDEONUM			MAX_REC_CHANNEL
#define		ALARMINNUM		0
#define		ALARMOUTNUM		0

//#ifdef IPCVGA
#define		SUPPORTSTORE		0
//#endif
//#ifdef IPC720P
//#define		SUPPORTSTORE		1
//#endif
#define		SUPPORTWIFI		1

#if(MAX_REC_CHANNEL==8)
#define 	CHANNEL_MASK			0xff
#else 
#define 	CHANNEL_MASK			0x0f
#endif 

#define 	HDD_NUM				2

#undef 	PAL
#undef 	NTSC
#define NTSC 					1
#define PAL 					0

#define INVALID_WEEKDAY				8


#define	HDD_BASEPATH				"/hdd00"  // 后面的00 代表硬盘编号	

//#define ADTYPE_TW2815	1


#define MT9D131

#if defined (__DEBUG_ON__)
#define		DPRINTF(x...)			fprintf(stderr, x)
#else
#define		DPRINTF(x...)
#endif 

#define	H264_FLAG									0x34363248

#define	OSD_HEIGHT									576
#define	OSD_WIDTH									720

#define  FIND_FILE_MAX								5

#define SHUTCUTSEARCHMAX							9

#define  FIND_LOG_MAX								5

#define DATETIME_SEARCH							0
#define VEHICLENUM_SEARCH							1
#define DRIVERNAME_SEARCH							2
#define EVENTNAME_SEARCH							3

#define SUPER_PASSWORD							888888

#define SYSTEMLOGFILENAME							"/hdd00/p01/systemlog.log"


#define PARTITION_MAXNUM							4


#define LASTDAY_RECORDFILE							"/var/lastday.bin"
#define LASTDAY_MAGIC							0x20081222

#define SD_DEVICE_NAME					"/dev/mmcblk0p1"

#define SD_DEVICE_BLOCKNAME			"/dev/mmcblk0"

#define SD_DEVICE_EXSIT					"/sys/block/mmcblk0/mmcblk0p1"

#define	USB_BUS_DIR_1					"/sys/bus/usb/devices/1-1"

#define	USB_BUS_DIR_2					"/sys/bus/usb/devices/2-1"

#define	USB_BLK_DIR					"/sys/block/"

#define USB_MOUNT_PIONT				"/usb0/p01/"

#define USB2_MOUNT_PIONT				"/usb1/p01/"

#define SD_MOUNT_POINT					"/sd/p01/"

#define HDD_MOUNT_PIONT				(char*)("/hdd")

#define MAX_ALARM_ZONE				16

typedef enum
{
	VIDEO_NTSC	= 0x00,
	VIDEO_PAL	= 0x01
}VIDEO_TVSYSTEM;

typedef enum
{
	PLAYACIOTN_NULL = 0,
	PLAYACTION_START,
	PLAYACTION_PAUSE,
	PLAYACTION_STOP,
}PLAYACTION_TYPE;

typedef enum
{
	THREAD_STOP = 0,
	THREAD_START,
	THREAD_PAUSE,
	THREAD_CONTINUE,

}THREAD_ACT_TYPE;


	

#define		BUFFER_ONE_BLOCK_SIZE			0x8000

typedef  int  BOOL;


typedef struct 
{
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char week;	
	unsigned char reserved;
} datetime_setting;

typedef struct 
{
	char m_filename[128];

}FILE_UNIT;

typedef struct
{
	int m_file_num; // 文件的个数
	FILE_UNIT	m_file_array[10];
}PlayFile_List;

typedef enum
{
	NULL_INFO_TYPE = 0,
	GPS_INFO_TYPE,
	TRAIN_INFO_TYPE,

}VIDEOATTACHINFOTYPE;

typedef struct
{
	unsigned int		m_headerflag; // 帧标识，0X99DB
	unsigned int 		m_framelen;  // 帧的长度
	unsigned int		m_infomode;// 信息帧类型 00 代表没有> 0标识有帧内容
	unsigned int 		m_reserved;
}INFOFRAMEHEADER;



 typedef struct 
 {
	unsigned	int				m_TargetBitRate;	/*目标码率*/
	unsigned int				m_MaxBitRate;		/*最大码率*/
	unsigned char				m_FrameRate;		/*帧率*/
	unsigned char				m_BaseQp;			
	unsigned char				m_MaxQp;
	unsigned char				m_reserved;
	
}VideoQualityConfig;
 

typedef struct
{
	unsigned int		m_uMagic;
	datetime_setting  	m_sDateTime;
	unsigned int		m_nHddId;
	unsigned int		m_reserved[16];
}SysLastRecordStatus;

typedef struct
{
	char 		m_devicename[32];
	char		m_blkdevicename[32];
	char		m_mountpoint[32];
	char		m_sysblkname[32];
	int 		m_isFulldev;	// 是否是Full speed设备
	int			m_exist;		// 是否已经存在
	int 		m_mounted;		// 是否已经加载上
}STORAGEINFO;


typedef struct {
	int 		m_nDiskId;				/*disk id*/
	int 		m_nPartitionId;			/*disk partition id*/
	char 	m_cMountPoint[64];		/*disk mount destination path*/
	char  	m_cDeviceName[64];		/*disk node path*/
}PARTITION_INFO;


/********************************************************
	硬盘信息结构体，各个域的含义如下:
	1)diskid:硬盘ID，对应的是插槽的编号
	2)partitionnum:该硬盘上分区的个数
	3)partitioninfo:该硬盘的分区信息
********************************************************/
typedef struct
{
	int 					m_nDiskId;				/*disk id*/
	int 					m_nPartitionNum;		/*disk partitions' num*/
	PARTITION_INFO		m_partitioninfo[PARTITION_MAXNUM];
}BlockDevInfo;


typedef struct
{
	unsigned int 			m_u8Exist;  // 0: 不存在 ， 1 存在但是没有加载上，2，存在并加载上文件系统
	unsigned long			m_u32Capacity;  // 以k为单位
	unsigned long			m_u32FreeSpace; // 以k 为单位
	unsigned char			m_cDevName[16];
}BlockDevInfo_S;

typedef struct
{
	unsigned char  m_u8AlarmNum;  //报警代号
	unsigned char	 m_u8FangNum;   // 防区号
	unsigned char  m_u8AStatus[7];  //  警情
	unsigned char  m_u8CmdType; //命令类型
	unsigned char  m_u8AZoneNum;  //分区号
	unsigned char  m_u8AlarmCH;
	unsigned int	m_u32Random;	//关联报警信息图片及录像
}AlarmStatusTmp;

typedef struct
{
	unsigned char			m_u8Valid; // 0 :  已经被取走， 1 存在
	unsigned char	 		m_u8RecType;  // 具体意义值见枚举类 enum  RECORD_KIND
	unsigned int			m_u8Time;  // 录像时间
	char					m_cFilename[128];
	AlarmStatusTmp		AlarmStaus;
	
}RecordFileInfo_S;

typedef enum
{
	PLAY_FF = 0,
	PLAY_BB,
	PLAY_SLOW,
	PLAY_FRAME,
	PLAY_PAUSE,
	PLAYNORMAL,
	END_PLAY,
	CLEAN_PLAYFLAG,
	
} PLAYSTATUS_TYPE;



typedef enum
{
	SYSSET_NULL = 0,
	SYSNET_SET,
	SYSSECURITY_SET,
	SYSRECTASK_SET,
	SYSMACHINE_SET,
	SYSCAMERA_SET,
	SYSANALOG_SET,
	SYSALARM_SET,
	SYSSENSOR_SET,
	SYSPOWER_SET,
	SYSCOMMON_SET,
	SYSBLIND_SET,
	SYSMOTION_SET,
	SYSREC00TASK_SET,
	SYSREC01TASK_SET,
	SYSREC02TASK_SET,
	SYSREC03TASK_SET,
	SYSREC04TASK_SET,
	SYSREC05TASK_SET,
	SYSREC06TASK_SET,
	SYSREC07TASK_SET,
	SYSRECSCHEDULE_SET,

	SYSPCDIR_SET,
	SYSALARMIN_SET,
	SYSMAINETANCE_SET,   // 系统维护
	SYSDISPLAY_SET,
	SYSUSERPREMIT_SET,
	SYSEXCEPT_SET,
	SYSOSDINSERT_SET,
	SYSRUNINFO_SET,
	SYSPTZ_SET,
	SYSALARMZONE_SET,
	SYSDEFSCHEDULE_SET,
	SYSPTZLINK_SET,
	PANEL_SET,
	MEGAEYE_SET,
	MEGAEYE_ECHOSET,
	SYS3G_SET,
	SYS3G_DIAL_SET,
	AUTOSWITCH_SET,
	PELCOCMD_SET,
	PICTIMER_SET,
	VOPIC_SET,
	NETDECODER_SET,
	ALARMPORT_SET,
	VIDEOLOSS_SET,
	SENSOR_SET,
	EXTEND_SET,
	WEB_SET,
	MD_SET
}SYSPARATYPE;


typedef enum
{
	SINGLESET_NULL = 0,
	TIMEINSERT_SET,
	


}SINGLESYSPARATYPE;


typedef enum
{
	LOCAL_REC_ID = 0,
	CLIENT_TRANS_ID,
	
	//MAX_BUFUSER_ID = 17,
	MAX_BUFUSER_ID = MAX_REC_CHANNEL*16*2,
}BufferUserID;

typedef enum
{
  	NONE_WH_MODE = 0,
  	WEEK_WH_MODE,
  	DATE_WH_MODE,

	MAX_WH_MODE
	
}MAINETANCEMODE;

typedef enum
{
	OUTSIDE_BUFANG = 0,
	INROOM_BUFANG,
	DROP_BUFANG,
	RELEASE_ALARM,



	MAX_ALARMACTION,
}ALARMACTION_E;


typedef enum
{

	ALARM_DELAY = 1, // 延迟
	BEYOND_REGION,  // 越界
	ALARM_THEFT,  //盗警
	ALARM_FULLDAY, // 全天报警
	ALARM_EMERGENT, //紧急报警
	ALARM_FIRE,   // 火警
	ALAMR_CHAI, // 拆动报警

	ALARMTYPE_MAX = 16,
	
}ALARM_CODE_E ;

typedef enum
{

	RP_CHEFANG = 8, // 撤防
	RP_OUTBUFANG, //外出布防 
	RP_PARTBUFANG, //部分布防
	RP_AC_POWEROFF, //交流掉电

	RP_VOL_LOW,  //探测器低压
	RP_DETECTORLOSS, //探测器丢失
	RP_ACVOL_RECOVER, //交流恢复
	RP_TELLINE_RECOVER, // 电话线故障恢复
	RP_SET_MOD,  // 编程改动
	RP_SYSVOL_LOW,  // 系统低压
	RP_SYSVOL_RECOVER, // 系统电源恢复
	RP_MANUAL_TEST,  // 手动测试
	RP_TIMER_TEEST,  // 定时测试
	RP_FIREALARM_RECOVER, // 火警恢复
	RP_TELLIEN_FAULT,  // 电话线故障
	RP_COMM_FAULT,  // 通讯故障
	RP_COMM_RECOVER,  // 通讯故障
	RP_INVALID_CHEFANG, // 无效撤防
	RP_MADECINE,  //医疗

	RP_MAX_TYPE = 32,
}ALARM_REPORT_E;

typedef enum WIFI_Connect_Status
{
	WPA_DISCONNECTED = 0, //8188wifi 状态
	WPA_INTERFACE_DISABLED,
	WPA_INACTIVE,
	WPA_SCANNING,
	WPA_AUTHENTICATING,
	WPA_ASSOCIATING,
	WPA_ASSOCIATED,
	WPA_4WAY_HANDSHAKE,
	WPA_GROUP_HANDSHAKE,
	WPA_COMPLETED,
	WIFI_PASSWD_ERROR,
	MONITOR_WIFI,
	WPA_IDLE,
	UNCONNECTED,//down 9271wifi 状态
	CONNECTING,
	CONNECTED,
	WIFI_ERROR,
	WIFI_MONITOR, 
	SYS_RESET//共有状态 

}WIFI_Connect_Status;


typedef struct
{
	WIFI_Connect_Status *wifi_state;
	unsigned int Conn_SAP;
}LED_Get_Wifi_Status;


typedef struct
{
	unsigned char       UbootVersion[16];
	unsigned char       KernelVersion[16];
	unsigned char       RootfsVersion[16];
	unsigned char       AppVersion[16];
	unsigned char       DeviceName[32];
	unsigned char		HardWareVersion[32];
	unsigned char		CreatVersionDate[16];
	unsigned int		CRC32;
	unsigned int		SupportInfo;
	unsigned char		Channel;
	unsigned char		Resolution;
	unsigned char 		Brand;/*品牌，0：zmodo，1：funlex 2:中性*/
	unsigned char		AD_Direction;
	unsigned short		IrCut_AdMax;
	unsigned short		IrCut_AdMin;	
	unsigned char		Res[92];
	
}DeviceConfigInfo;
/*
0:加密
1:onvif
2:音频
3:对讲
4:wiif
5:pt
6:zoom
7:录像
8:点对点
9:翻转
10.mini NVR route
11. WPS----->此bit位修改为是否支持有线
*/
#define 	CONFIG_ENC			(1<<0)
#define 	CONFIG_ONVIF 		(1<<1)
#define 	CONFIG_AUDIO 		(1<<2)
#define 	CONFIG_INTERCOM 	(1<<3)
#define 	CONFIG_WIFI 		(1<<4)
#define 	CONFIG_PT 			(1<<5)
#define 	CONFIG_ZOOM 		(1<<6)
#define 	CONFIG_RECODE 		(1<<7)
#define 	CONFIG_P2P 			(1<<8)
#define  	CONFIG_FLIP  		(1<<9)
#define  	CONFIG_ROUTE		(1<<10)
#define  	CONFIG_LOCALNET		(1<<11)
#define 	CONFIG_SPOE_1_T_2 (1<< 13) /*spoe 一拖二*/
#define 	CONFIG_MININVR_IPC (1<< 14)/*与MININVR 配套的IPC*/
/*DNS 路径*/
#define DNS_CFG_FILE	(char *)"/tmp/resolv.conf"
/*mac和二维码路径*/

#define MAC_ADDR_FILE	"/config/devicemac"
#define DEVICE_CONFIG_BIN "/config/DeviceConfig.bin"
#define DEV_DEFAULT_FILE "/config/deviceDefaultConfig"

#define TOOLS_ROOT_PATH  "/app"
#define DEVICE_AES_KEY	"/config/key"

#define WEBSEVER_SCHEDULE_MAX_NUM		10 /* 平台最大支持时间段个数*/

#ifdef NEW_MPP
#define H264_VBR  VENC_RC_MODE_H264VBRv2    
#define H264_CBR  VENC_RC_MODE_H264CBRv2
#else
#define H264_VBR  VENC_RC_MODE_H264VBR    
#define H264_CBR  VENC_RC_MODE_H264CBR

#endif

#endif 


