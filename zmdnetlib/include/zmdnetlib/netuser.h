

#ifndef _JIANGHM_NET_USER_HEADER_2343298u7324423
#define _JIANGHM_NET_USER_HEADER_2343298u7324423

#include "interfacedef.h"
#include "tcplibdef.h"

#ifdef SUPPORT_WIFI
#include "wificonfig.h"
#endif
#include "zspadapter.h"


#ifdef APP3531
#define MAX_UPDATEFILE_LEN		24*1024*1024
#endif
#ifdef DM368
#define MAX_UPDATEFILE_LEN		16*1024*1024
#endif

#define UPDATEFILE_LEN			1024*1024


#define UPDATEFILE "/tmp/UpdateFile"

#define GET_HEADER_LEN( type )		(sizeof(type) - sizeof( Cmd_Header ) )

/***********************************************/

enum ZmdProtoType
{
	ProtoUnknown = -1,
	ProtoZSP = 0,
	ProtoP2P = 1
};
class CNetUser
{
public:
	CNetUser( ) ;
	~CNetUser( ) ;

public:

	//初始化用户，将用户的信息全部置为默认值
	void ResetUser( ) ;
	
	//======================================
	//用户事件处理函数
	void onClose() ;
	bool onReceive( char* data , int len , int& used ) ;
	bool onIdle() ;

protected:

	bool onPacketProcesser( char* data , int len ) ;
	
	//---------------------------------------------------------
	//消息处理函数
	bool onDeviceType( char* data , int len ) ;
	bool onRequestVideoLiving( char* data , int len ) ;

	bool onStopVideoLiving( char* data , int len ) ;

	bool onCmdFormatDisk( char* data , int len ) ;
	
	bool onGetDeviceMac( char* data , int len ) ;
	bool onSetDeviceMac( char* data , int len ) ;

	bool onTalkOn( char* data , int len ) ;
	bool onTalOff( char* data , int len ) ;

	bool onGetTalkSetting( char* data , int len ) ;
	bool onSetTalkSetting( char* data , int len ) ;
	bool onAlarmUpdate( char* data , int len ) ;
	bool onTalkData( char* data , int len ) ;

	bool onReadDevParam( char* data , int len ) ;
	bool onSetDevParam( char* data , int len ) ;
	bool onSetPtz( char* data , int len ) ;

	bool onSetReboot( char* data , int len ) ;

	bool onRequestUpdate( char* data , int len ) ;
	bool onCmdNpt( char* data , int len ) ;

	bool onRequestResotre( char* data , int len ) ;
	bool on3GCtrlCmd( char* data , int len ) ;

	bool on3GRepower( char* data , int len ) ;
	bool onGet3GRepower( char* data , int len ) ;
	bool onGet3GInfo( char* data , int len ) ;

	bool onReadLog( char* data , int len ) ;
	
	bool onReadDevInfo( char* data , int len ) ;
	bool onSearchPlayList( char* data , int len ) ;
	bool searchPlaybackListInDates( char *data , int len );

	bool onPlaybackPlay( char* data , int len ) ;
	bool onPlaybackStop( char* data , int len ) ;

	bool onReqLogin( char* data , int len ) ;
	bool onResetChnAna( char* data , int len ) ;

	bool onSetChnAnaLog( char* data , int len ) ;
	bool onGetChnAnaLog( char* data , int len ) ;

	bool onGetWifiAP( char* data , int len ) ;
	bool onSetWifiConnect( char* data , int len ) ;

	bool onGetWifiStatus( char* data , int len ) ;
	bool onSetPicInfo( char* data , int len ) ;

	bool onSetVgaPicInfo( char* data , int len );
	bool onSet720PPicInfo( char* data , int len ) ;

	bool onNvrPort( char* data , int len ) ;

	bool onReadVideoCoderParam( char* data , int len ) ;

	//===========================================
	//水稻育苗新增协议
	bool onGetSnapshot( char* data , int len ) ;
	bool onGetSensorAlarmInfo( char* data , int len ) ;
	bool onSetSensorAlarmInfo( char* data , int len ) ;

	//===========================================
	//预置点消息
	bool onGetPreset( char* data , int len ) ;
	
	//
	bool onSetVideoCode( char* data , int len ) ;

	//==========================================
	//设置和获取设备id
	bool onGetDeviceID( char* data , int len ) ;
	bool onSetDeviceID( char* data , int len ) ;

	bool onSetAudioSwitch( char* data , int len ) ;
	bool onGetAudioParm( char* data , int len );


	bool onSetWifiLED( char* data , int len );
	bool onGetWifiLED( char* data , int len );	
	//=======================================
	//强制I 帧
	//add hayson 2013.8.6
	bool onSetIframe(char* data , int len);
	//烧录设备型号配置文件 
	//add hayson 2013.11.26
	bool onSetDevConfig(char* data , int len);

	//add by mike,2013-12-19,增加处理远程升级相关的信令
	//检测软件更新信息
	bool onCheckAppUpdateInfo( char* data, int len );
	
	//开始进行远程自动升级
	bool onStartRemoteUpdate( char* data, int len );
	
	//获取远程升级进行的状态
	bool onGetRemoteUpdateStat( char* data, int len );
	
	//取消远程自动升级
	bool onCancelRemoteUpdate( char* data, int len );
	
	//暂停远程自动升级
	bool onPauseRemoteUpdate( char* data, int len );
	
	//恢复远程自动升级
	bool onResumeRemoteUpdate( char* data, int len );

	//获取时间格式
	bool onGetDateFormat(char* data, int len);

	//设置时间格式
	bool onSetDateFormat(char* data, int len);

	//获取sd卡信息
	bool onGetDiskInfo( char* data, int len);

	//获取设备版本信息
	bool onGetVersion(char* data, int len);
	// 设置wifi NVR 码流模式
	bool onWifiNVRMode(char *data, int len);

	// 设置ipc 码率
	bool onChangeStreamRate(char *data, int len);

	bool onGetViRate(char *data, int len);
	
	bool onSetViRate(char *data, int len);
	
	//获取视频加密密钥
	bool onGetVideoKey(char *data, int len);

	//设置和获取移动侦测区域
	bool onSetMdRegion(char *data, int len);
	bool onGetMdRegion(char *data, int len);

	//电池转动测试
	bool onSetMotorTest(char *data, int len);
	//复位设置测试
	bool onResetTest(char *data, int len);
	//夜视设置测试
	bool onNightVisionTest(char *data, int len);

	

protected:
	//======================================
	// 以下是内部功能函数

	//询问系统，是否能进行该请求的视频直播
	int		RequestStartVideoLiving( int command , int channel ) ;
	//打开视屏直播
	int		StartVideoLiving( int command , int channel ) ;
	
	//=======================================
	//发送直播数据
	bool	SendLivingData(  ) ;

	//=======================================
	//发送录像数据
	bool	SendRecordPlay( ) ;

	//=======================================
	//发送录像文件
	bool	SendRecordFile( );

	//=======================================
	//发送录音数据
	bool	SendAudioData( ) ;
	

	//NTP对时
	int netSetTimezone( char* data , int len );
	int netGetTimezone( char* data , int len );

	int		CheckWifiSaveParaMeter(TYPE_WIFI_LOGIN  wifilogin) ;
	
public:

	void*			m_sessionHandle ;
	int				m_mediaCmd ;		//记录用户请求媒体数据的信令号(CMD_START_VIDEO,CMD_START_SUBVIDEO,CMD_START_720P)
	int				m_Channel ;			//记录用户请求的通道id
	int				m_ChannelType ;		//记录用户请求的码流类型( VGA,QVGA,720P)

	//用户ID,全局为一个用户标识
	int				m_nUserID ;
	//用户申请的媒体通道id.
	int				m_mediaSid ;
	//用户申请的报警通道id
	int				m_alarmID ;

	//
	int				m_bSendAudio ;


	//直播标志
	int				m_bStartSendMedia ;

	//对讲标志
	int				m_bStartTalk ;

	//点播标志.录像播放
	int				m_bStartSendRecord ;	//0:未播放,1:播放,2:下载
	PLAYBACK_ITEM	m_RecordItem ;			//
	FILE*			m_RecordFp ;
	unsigned int	m_nSendPos ;				//上次发送的文件位置
	unsigned int	m_timeTick ;			//计时器

	unsigned int	m_remoteIP ;

	int				m_bUpdateFile ;
	char			m_FrameBuf[TCP_RECV_BUF_SIZE] ;

	int				m_bufType ;

	int				m_ActiveTime ;

	/* 记录网络状况, 一分钟统计一次 */
	int				m_NetSpeed; // 当前网速
	struct timeval	m_StartCalcTime; // 开始计算时间
	int 			m_SendTime; // send 函数累计耗费时间
	int				m_BytesSend; // 发送的字节数

	int				m_protoType;//标志是zsp协议还是 p2p协议, 0 是zsp 1是 p2p
};

int  Set_NightSwtich(unsigned char status);
#endif

