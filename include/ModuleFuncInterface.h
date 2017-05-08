
#ifndef __MODULE_FUNC_INTERFACE_H__

#define __MODULE_FUNC_INTERFACE_H__

#include <signal.h>
#include <sys/types.h>

#include "BlockDeviceManage.h"
#include "parametermanage.h"
#include "upgrademodule.h"
#include "BufferManage.h"   
#include "TdFeedDog.h"
#include "hi_type.h"
#include "VideoADCtrl.h"
#include "McuCommunication.h"
#include "AdapterMessage.h"

#include "interfacedef.h"
#define NTP_SERVER "ntpusa.meshare.com"
#include "wificonfig.h" 
#include "p2p_interface.h"
#include "p2p_alarm.h"
#define plog(fmt, ...)  printf("[%u]%s:" fmt,__LINE__,__FUNCTION__,## __VA_ARGS__)

#define plogfn()  printf("[%u]%s[%lu]\n", __LINE__, __FUNCTION__, pthread_self())
#define plogt(fmt, ...)  syslog(LOG_USER|LOG_INFO, "(%u %s) " fmt,__LINE__,__FUNCTION__,## __VA_ARGS__)

#define SUPPORT_WATCHDOG

#ifdef SUPPORT_WATCHDOG
#define REGISTER_DOG()	 \
							CTrdDog tdDog; \
							tdDog.RegisterDog();

#define DECLARE_DOG		static CTrdDog tdDog

#define REGISTER_DOG_EX()	tdDog.RegisterDog()

#define DEBUG_WATCH_DOG(x) 	//fprintf(stderr, "%s: ptread_t = %d\n", x, tdDog.m_trdId)

#define FEED_DOG()	tdDog.EatFood()

#define UNREGISTER_DOG()	tdDog.DeregisterDog()
#else
#define REGISTER_DOG()	

#define DECLARE_DOG		

#define REGISTER_DOG_EX()	

#define DEBUG_WATCH_DOG(x)	

#define FEED_DOG()

#define UNREGISTER_DOG()

#endif


void SignalRegister();


void  GlobalGetSystemTime(datetime_setting *datetime);

void GetEncodePara(CAMERA_PARA *para);




void PubSaveSystemParameter(SYSTEM_PARAMETER *para);

void RestoreDefault(void *para);

SYSTEM_PARAMETER *PubGetParameterPtr();

void PubReadSystemParameter(SYSTEM_PARAMETER *para);




int GetMotionInfo(int ch);
int GetRealTimeMotion();


void  RestartEncodeSystem();

void PubSyncSytemTime( datetime_setting datetime);
void WaitForSystemTime();
void BackupRealTime();

void VerifySystemTime(datetime_setting  *psystime);

unsigned int strtime2sec(char *str);


int  InitSysParameter();


void StartSystemEncode();
int PubGetSysParameter(int type, void * para);
int PubSetSysParameter(int type, void * para);

int PubGetSinglePara(int type, void * value);

int GetSystemTime(SystemDateTime *pSysTime);


/*****************************************************************************
函数功能: 初始化移动侦测
输入参数:无
输出参数:无
返  回   值:无
使用说明:
******************************************************************************/
void InitSystemMD();
void MotionDetectionSet(int sensitive);

void StopSystemMD();



/*****************************************************************************
函数功能: 设置每个通道的移动侦测参数
输入参数:无
输出参数:无
返  回   值:无
使用说明:
******************************************************************************/
void SetMotionDetetionPara(int ch, MD_SETUP MD, int Sensetive);




void SetRecordCtrlThread(int status);


int GetSoftWareVersion(TYPE_DEVICE_INFO *version);

unsigned char CaculateWeek(unsigned char day, unsigned char month , unsigned char year);

void RebootSystem();
void SystemResourceExit();



int SetVideoFlipMirror(int mode);

int Setdhcpandmac(int flag);

int GetDiffByTzIdx(unsigned char m_idx_tzname,int *pm_diff_timezone); /*获取时区与UTC秒数差距*/
int InitDeviceConfigInfo();
int UpdateDeviceConfigInfo(void *data);

bool GetSpeeker();
bool ReleaseSpeeker();
/*
cmd:false 关闭，true:开启
return :false:failed  true:success
*/
bool AudioInputCtrl(bool cmd);
/*
return :false:off  true:on
*/

bool GetAudioInputStatus();


/*
录像控制
chn :			通道
type:			录像类型0 普通录像1 报警录像
recodeswitch:		录像开关true:开false:关
controltype:		0: 设置 1:获取
返回值:		-1 失败  0 成功
*/

int RecodeControl(unsigned int chn, char type, bool *recodeswitch , char controltype);

/*add by hayson begin 2013.12.26*/
/*返回无线网卡名字*/
const char* get_wifi_name();
/*返回有线网卡名字*/
const char* get_local_name();
/*返回MAC和二维码配置文件路径*/
const char* get_mac_id_file();
/*返回IE插件版本*/
const char* get_ie_version();
const char*  get_tools_path();

typedef enum
{
	NET_WORK_CARD_LOCAL = 0, /*只支持有线网卡*/
	NET_WORK_CARD_WIFI,		/*只支持无线网卡*/
	NET_WORK_CARD_LOCAL_AND_WIFI, /*支持双网卡*/
}NET_WORK_CARD;

/*返回支持网卡状态*/
NET_WORK_CARD get_network_support();
/*add by hayson end 2013.12.26*/

/* add by mike 2014-01-09*/
const char* get_app_version();

const char* get_uboot_version();

const char* get_kernel_version();

const char* get_fs_version();

/*add by hayson 204.4.16*/
/*最高分辨率支持*/
typedef enum
{
	HD_720P = 0, 
	HD_1080P,
	HD_VGA,
}NET_HD_REOLUTION;
/*返回设备最高支持的分辨率*/
NET_HD_REOLUTION get_hd_resolution();

int get_preset_length();
int Crtl_ptzPara(int cmd,STRUCT_SET_PTZ_REQUEST *pReq);

void NightMode(bool night);
const char* GetConfigDir();



int GetDeviceExpendCapacity();
unsigned long long GetDeviceSupplyCapacity();
int OnSetTimezoneByMeshare(int timezone_index, int timezone_offset);

void RestartNtpClient(int timezone_offset);
int GetAbnormalNoiseDetect();
void ClearAbnormalNoise();

//int ZMD_SYS_FCNTL_NOCLONE(int fd);

void BlockWaitForAlarms();

struct p2p_broadcast_alarm_t
{
	int alarm_type;
	int channel;
	void* alarm_info;
};

void BroadcastAlarmsEx(P2pAlarmType alarm_type, int chl, void *alarm_info=NULL);

int  P2pGetOneAlarm(p2p_broadcast_alarm_t * alarm);

/*
* @fuction:写入缺省配置信息
* @hayson 2015.1.27 
* return: 0 成功 -1 失败 
*/ 
int SysSetDefaultConfig(DevDefaultConfig* config); 
/* 
* @fuction:读取缺省配置信息 
* return: 0 成功 -1 失败 
*/ 
int SysGetDefaultConfig(DevDefaultConfig* config); 


int SysSetNvrNetStatus(int route) ;
void DeviceSwtich();



#define 	WEB_SYNC_DEVICE_SCHEDULE	(1<<0)
#define 	WEB_SYNC_ALAEM_SCHEDULE 	(1<<1)
int WebSensitivity_to_Local(int websensitive);


int webserver_get_device(web_sync_param_t* sync_data);
int webserver_set_device(web_sync_param_t* sync_data);
int GetWebScheduleSwitch(int type);
int SysGetNvrNetStatus() ;
int  Set_Net_Level(int level);
int  Get_Vi_Rate(VENC_ATTR_H264_VBR_S   	*vBr);
int  Set_Vi_Rate(VENC_ATTR_H264_VBR_S   	*vBr);

int StartLowBiteStream();

#if 1
typedef struct _device_function_t 
{
	int md;				/*0:不支持， 1:支持抓5张图上传  2:支持单次抓图上传 */	
	int audio;			/* 0:不支持 1:支持声音检测 */
}device_function_t;

	

int meshare_get_fuction(device_function_t * paramter);

int p2p_get_motion(int chl);
#endif

/*第一次MD标志位*/
int Get_Md_Flag();

int GetDeviceType();
int  FindRecordFile(FindFileType *findType,RecordFileName *RecordFile, int mode, int searchmax);

//@date: '2014-07-16'

//@channel: channel number
//@info: malloc memory inside
//@num: file number
//return 0 on success
//if success and num is not 0, user must free info

int SearchRecordFileByDate( const char* date,
							 int channel, 
							 p2p_find_playback_file_t** info,
							 int *num );

/* 	
	!!!注意!!! 

以下两个修改被动设备列表的函数，每次操作都是覆盖服务器或者设备本地的被动设备列表里的全部内容
即使只是想修改列表中的单个被动设备的信息，也要提交全部被动设备信息   	
*/
/* 当本地的被动设备主动变化时，如 */
int SetSubDeviceListToWeb(int num, const sub_device_info *sd_list);

/* 当手机修改被动设备时，平台库调用此函数 */
int SetSubDeviceListToLocal(int num, const sub_device_info *sd_list);

int Web_Sync_MirrorFilp_Value(int mode);
bool SetIPCMirrorFilp(int comid);

int WebSetIPCMirrorFilp(int cmdid);

int GetAlarmInterval();
int UpdateConfigFileAD(int min,int max);
/* 
* @fuction:读取设备开关状态 
* return: 0 关闭 1 开启 
*/ 
int GetDeviceSwitchStatus();
int  CheckRecordTime();
/* 启动p2p模块 */
void P2P_Start();

int  NtpIfEnable();

void MeshareNtpCallback(time_t seconds);
int SysTime2Time(SystemDateTime *pSysTime);
int ForceIdrInsertion(int channel,int streamtype);
int GetOneFrame4Record(int channel,int streamtype, int userid, unsigned char **buffer, FrameInfo *m_frameinfo);
int ZD_DEBUG_Printf(const char *fmt, ...);
/*
InitRecordSystemBlockManage API
*/
int InitBlockDevice();
int GetBlockDeviceNum();
int GetBlockDeviceInfo(int type, BlockDevInfo_S * Info);
int MountBlockDevice(BLK_DEV_ID enBlkDevId);
int UmountBlockDevice(BLK_DEV_ID enBlkDevId);
int  FormatBlockDevice(BLK_DEV_ID enBlkDevId);
int GetMountInfo();
/**
record API
**/
int InitRecordManage();
int StartRecordSystem();
int StopRecordSystem();
void SetRecSchedule(GROUPRECORDTASK *pRecSchedule);
int  FindRecordFile(FindFileType *findType,RecordFileName *RecordFile,REC_SEARCH enMode);
int  HaveRecordFile(struct tm time);
void SetMdAlarmLink(unsigned int u32MdAlarmLink,RECORD_LEVEL enRecordLevel);
void SetExAlarmLink(unsigned int u32ExAlarmLink,RECORD_LEVEL enRecordLevel);
void SetMwAlarmLink(unsigned int u32MwAlarmLink,RECORD_LEVEL enRecordLevel);
void SetSD433AlarmLink(unsigned int u32SD433AlarmLink,RECORD_LEVEL enRecordLevel);
void SetAudioAlarmLink(unsigned int u32AudioAlarmLink,RECORD_LEVEL enRecordLevel);
int getRecordOffset(const char* path, unsigned int start_time, unsigned int *pOffset);
void InitDeviceAesKey();

char *GetAesKey();
int AesEncrypt(unsigned char *input, unsigned char *output);
int SetMdRegion(P2P_MD_REGION_CHANNEL* region);
int GetMdRegion(P2P_MD_REGION_CHANNEL* region);
void MdRegionCtrlCallback(int op_type, p2p_md_region_t* reg);

int GetOSPQueueSendQuenceMsgID();
bool GetDeviceStatus();
#endif 

