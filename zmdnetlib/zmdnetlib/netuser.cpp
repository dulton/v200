#include "netuser.h"
#include "netserver.h"
#include "mediamgr.h"
#include "zmdcrypt.h"
#include "zmdnetlib.h"
#include "tcplib.h"
#include "zmdntpclient.h"
#include "common.h"
#include "ModuleFuncInterface.h"
#include "cspara.h"
#include "ptz.h"
#include "upgrademodule.h"
#include "logmanage.h"

#include "ov7725.h"
#include "Ds1337Api.h"
#include "EncodeManage.h"
#include "remoteupdate.h"
#include "RecordManage.h"
#include "BlockDeviceManage.h"

#ifdef SUPPORT_WIFI
#include "wificonfig.h"
#endif
#include <vector>

//=================================================
//一些扩展定义,由外部实现，而网络层需要调用的功能

extern struct wpa_ctrl			*ctrl_conn;
extern int  GetWifiMac(unsigned char *mac);
extern int	startupdate ;
extern PARAMETER_MANAGE		*g_cParaManage ;
extern DeviceConfigInfo 	ConfigInfo;
#ifdef DM368
extern "C" int SnAewb_UiSetBrtness(int rsiBrtness);
extern "C" int SnAewb_UiSetContrast(int rsiContrast);
extern "C" int SnAewb_UiSetSatDgr(int rsiSatDgr);
extern "C" void AnalogParaSet(ANALOG_CHANNEL analog_para);
#endif
extern struct wpa_ctrl			*ctrl_conn;
extern int  GetWifiMac(unsigned char *mac);
extern int	startupdate ;
extern PARAMETER_MANAGE		*g_cParaManage ;
extern DeviceConfigInfo 	ConfigInfo;


CNetUser::CNetUser( )
{
	ResetUser() ;
	m_ActiveTime = time( NULL ) ;
	m_protoType = -1;
}

CNetUser::~CNetUser( )
{

}

void CNetUser::ResetUser( )
{
	m_bStartTalk = false ; 
	GetNetServerObj()->RequestTalkOff( m_nUserID ) ;

	m_bStartSendMedia = 0 ;
	m_mediaSid = INVALID_USERID ;
	m_mediaCmd = 0 ;
	m_sessionHandle = 0 ;
	m_Channel = 0 ;
	m_nUserID = INVALID_USERID ;
	m_alarmID = INVALID_USERID ;
	m_remoteIP = 0 ;
	m_bSendAudio = 0 ;

	m_RecordFp = 0 ;
	memset( &m_RecordItem , 0 , sizeof( m_RecordItem ) ) ;
	m_bStartSendRecord= 0 ;

	m_ActiveTime = time( NULL ) ;
	m_protoType = ProtoUnknown;

}

void CNetUser::onClose()
{
	//=====================================
	//关闭用户
	m_sessionHandle = 0 ;
	m_bStartSendMedia = 0 ;
	//如果正在接收视频，关闭媒体会话
    if( INVALID_USERID != m_mediaSid )
    {
		GetMediaMgr()->freeMediaSession( m_Channel , m_ChannelType , m_mediaSid ) ;
		m_mediaSid = INVALID_SESSION ;
		m_bStartSendMedia = 0 ;
	}

    if( INVALID_USERID != m_alarmID )
    {
		GetNetServerObj()->freeAlarmSession( m_alarmID ) ;
		m_alarmID = INVALID_USERID ;
	}
	
	//释放工作会话
	GetNetServerObj()->freeWorkSession( m_nUserID ) ;
	
#ifdef SUPPORT_AUDIO 
    if( m_bStartTalk )
    {
		StopAudioDecode() ;
		GetNetServerObj()->RequestTalkOff( m_nUserID ) ;
		m_bStartTalk = false ;	
	}
#endif
	if(m_protoType == ProtoP2P)
		//m_p2pAnalizer.HandleClose();
	m_protoType = ProtoUnknown;
	ResetUser( ) ;
}

bool CNetUser::onReceive( char *data , int len , int &used )
{
    //从数据流中拆分数据包
    char *start = (char *)data ;
    char *end = (char *)data + len ;

	int packetSize = 0 ;

    while( start + sizeof( Cmd_Header ) <= end )
    {
        Cmd_Header *pHeader = (Cmd_Header *)start ;
		if(pHeader->head == 0xaaaa5555)
		{
			m_protoType = ProtoZSP;
		}
		else if(pHeader->head == 0x9F55FFFF)
		{			
			m_ActiveTime = time(NULL);
			if(m_protoType != ProtoP2P)
			{
				//m_p2pAnalizer.InitAdapter(GetSessionSock(m_sessionHandle));
				m_protoType = ProtoP2P;
			}
			//used = m_p2pAnalizer.HandleCmd(start, end - start);
			if(used < 0)
				return false;
			else
				return true;
		}
		
        packetSize = sizeof( Cmd_Header ) ;
        packetSize += pHeader->length ;

		//由于部分信令会超出接收缓冲区，所以我们在这预处理
		//查看是否是升级信令
		switch( pHeader->commd )
		{
		case CMD_S_UPDATE :
			onRequestUpdate( data , len ) ;
			used = len ;
			//关闭连接
			return false ;
			break ;
		}
		
		if( packetSize > TCP_RECV_BUF_SIZE )
			return false ;

        if( end - start  < packetSize )
        {
            //剩余数据不是一个完整的包，标志已经处理的数据大小
            used = start - data ;
            return true ;
        }
        if( !onPacketProcesser( start , packetSize ) )
            return false ;
        start += packetSize ;
    }
    //标志已经处理的数据大小
    used = start - data;
    return true ;
}

bool CNetUser::onIdle()
{
	int now = time( NULL ) ;
	//是否需要发送媒体数据
    if( m_bStartSendMedia )
    {
		m_ActiveTime = now ;
		return SendLivingData() ;
	}
	
	//检查数据
    if( now - m_ActiveTime > 20 )
    {
		ERR( "session time out!!\r\n" ) ;
		StopTcpSession( m_sessionHandle ) ;
	}
	// 
	if(m_protoType == ProtoP2P)
	{
		m_ActiveTime = now ;
		//while( m_p2pAnalizer.HandleIdle() >0){}
	}

	return true ;	
}

//-----------------------------------------------
//数据包命令解析
bool CNetUser::onPacketProcesser( char *data , int len )
{
	bool bRet = true ;
	bool isNeedClose = false;

	m_ActiveTime = time( NULL ) ;

	Cmd_Header* pHeader = (Cmd_Header*)data ;
	
	/*此处统一检查包头长度与实际接收数据是否匹配
	数据长度不等于实际接收数据包的长度*/
    if( pHeader->length + sizeof( Cmd_Header ) != len )
    {
        ERR( "******pHeader->length error!!!!\r\n*********" ) ;
		return false ;
	}
	/*此处添加解析出来的数据包处理函数。
	若是处理数据包出现错误或者处理完数据包后需要断开连接
	处理函数返回false即可*/

	if(pHeader->commd != CMD_PING && pHeader->commd !=  CMD_TALK_DATA)
		//NDB("Client Command:0x%4x\r\n", pHeader->commd);
	
		printf("Client Command:0x%4x\r\n", pHeader->commd);
	switch(pHeader->commd )
	{
	case CMD_FORMATHDD:					/*处理格式化磁盘信*/
		onCmdFormatDisk( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_G_MAC:						/*获取MAC 地址*/
		onGetDeviceMac( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_S_MAC:						/*设置MAC 地址*/
		onSetDeviceMac( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_S_DEVICEID:				/*设置设备二维码ID信令*/
		onSetDeviceID( data , len ) ;
		isNeedClose = true;
		break ;
	case CMD_G_DEVICEID :				/*读取设备二维码ID 信令*/
		onGetDeviceID( data , len ) ;
		break ;
	case CMD_S_DEVCONFIG:				/*烧录设备型号配置文件 */
		onSetDevConfig( data , len ) ;
		break;
	case CMD_DECIVE_TYPE:				/*用户登录*/
		bRet = onDeviceType( data , len ) ;		
		break;
	case CMD_START_720P: 
	case CMD_START_VIDEO:
	case CMD_START_SUBVIDEO:			/*请求直播视频，3条信令分别请求不同类型的直播*/
		bRet = onRequestVideoLiving( data , len ) ;
		break;
	case CMD_STOP_VIDEO:
		onStopVideoLiving( data , len );/*停止视频*/
		isNeedClose = true;
		break;
	case CMD_AUDIO_ON:					/*音频开启*/
		break;
	case CMD_AUDIO_OFF:					/*音频关闭*/
		break;
	case CMD_TALK_ON:					/*请求对讲通道*/
		onTalkOn( data , len ) ;	
		break;
	case CMD_TALK_OFF:					/* 停止对讲*/
		onTalOff( data , len ) ;
		break;
	case CMD_TALK_DATA:					/*请求对接数据*/
		onTalkData( data , len ) ;
		break;
	case CMD_ALARM_UPDATE:				/*  报警处理命令*/
		onAlarmUpdate( data , len ) ;
		break;
	case CMD_R_DEV_PARA:				/* 读取设备参数*/
		onReadDevParam( data , len ) ;
		break;
	case CMD_S_DEV_PARA:				/* 设置设备参数*/
		onSetDevParam( data , len ) ;
		break;
	case CMD_S_PTZ:						/*PTZ云台控制*/
		onSetPtz( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_S_REBOOT:					/*重启设备*/
		onSetReboot( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_S_UPDATE:					/* 升级设备*/
		onRequestUpdate( data , len ) ;		
		break;
	case CMD_S_NTP:						/*手动校时*/
		bRet = onCmdNpt( data , len ) ;		
		break;
	case CMD_SET_TIMEZONE:				/*设置NTP信息*/
		netSetTimezone(data,len);
		break;
	case CMD_GET_TIMEZONE:				/*获取NTP信息*/
		netGetTimezone(data,len);
		break;
	case CMD_S_RESTORE:					/*恢复出厂设置*/
		onRequestResotre( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_S_3GCTL:					/* 3G模块控制信令(暂未使用)*/
		on3GCtrlCmd( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_S_3G_REPOWER:				/* 3G模块断电信令(暂未使用)*/
		on3GRepower( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_R_3G_INFO:					/*读3G连接状态和型号(暂未使用)*/
		onGet3GInfo( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_R_LOG:						/*查询日志*/
		onReadLog( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_R_DEV_INFO:				/*查询设备信息*/
		onReadDevInfo( data , len ) ;		
		break;
	case CMD_R_SEARCH_PLAYLIST:			/* 检索回放列表*/
		onSearchPlayList( data , len ) ;
		break;
	case CMD_PLAYBACK_PLAY:				/*回放打开*/
		bRet = onPlaybackPlay( data , len ) ;
		break;
	case CMD_PLAYBACK_STOP:				/*回放停止*/
		onPlaybackStop( data , len ) ;
		break;
	case CMD_REQ_LOGIN:					/* 登陆请求命令*/
		onReqLogin( data , len ) ;	
		break;
	case CMD_S_CHN_ANA:					/* 恢复单通道默认颜色*/
		onResetChnAna( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_S_CHN_ANALOG:				/*3A设置*/
		onSetChnAnaLog( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_G_CHN_ANALOG:				/*3A获取*/
		onGetChnAnaLog( data , len ) ;
		isNeedClose = true;
		break;
	#ifdef SUPPORT_WIFI
	case CMD_G_WIFI_AP:					/*获取wifi ap扫描列表*/
		onGetWifiAP( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_S_WIFI_CONNECT:			/*连接WIFI 热点*/
		onSetWifiConnect( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_G_WIFI_STATUS:				/*获取WIFI 状态*/
		onGetWifiStatus( data , len ) ;
		break;
	case CMD_SET_WIFILED:				/*设置WIFI指示灯(3518)*/
		onSetWifiLED( data , len ) ;
		break ;
	case CMD_GET_WIFILED:				/*读取WIFI指示灯(3518)*/
		onGetWifiLED( data , len ) ;
		break ;
	#endif
	case CMD_S_PIC_NORMAL:				/*图像设置信，令镜像翻转等*/
	case CMD_S_PIC_FLIP:
	case CMD_S_PIC_MIRRORON:
	case CMD_S_PIC_FLIPMIRROR:
	case CMD_S_PIC_COLORON:		
	case CMD_S_PIC_COLOROFF:	
	case CMD_S_SENSORFREQ_50:
	case CMD_S_SENSORFREQ_60:
	case CMD_S_SENSORFREQ_OUT:
	case CMD_S_SENSORFREQ_AUTO:
		onSetPicInfo( data , len ) ;
		isNeedClose = true;
		break;
	case CMD_NVR_PORT:					/*NVR报警通知端口注册(已废弃该方式)*/
		bRet = onNvrPort( data , len ) ;
		break ;
	case CMD_R_VIDEO_CODE_PARA :		/*获取设备视频编码信息*/
		bRet = onReadVideoCoderParam( data , len ) ;
		isNeedClose = true;
		break ;
	case CMD_G_SNAPSHOT :				/*抓拍*/
		onGetSnapshot( data , len ) ;
		isNeedClose = true;
		break ;
	case CMD_S_SENSOR_ALARMINFO :		/*设置传感器信息(水稻育苗项目)*/
		onSetSensorAlarmInfo( data,len);
		isNeedClose = true;
		break ;
	case CMD_G_SENSOR_ALARMINFO :		/*传感器状态获取(水稻育苗项目)*/
		onGetSensorAlarmInfo( data , len ) ;
		break ;	
	case CMD_S_GET_PRESET_POINT :		/*获取预置点信息*/
		onGetPreset( data , len );
		isNeedClose = true;
		break ;
	case CMD_SET_VIDEO_CODE :			/*设置视频编码参数*/
		onSetVideoCode( data , len ) ;
		break ;
	case CMD_G_TALK_SETTING :			/*获取设备音频设置*/
		onGetTalkSetting( data , len ) ;
		break ;
	case CMD_S_TALK_SETTING :			/*设置设备音频*/
		onSetTalkSetting( data , len ) ;
		break ;
	case CMD_SET_AUDIOSWITCH :			/*设置音频参数(未使用)*/
		onSetAudioSwitch( data , len ) ;
		break;
	case CMD_GET_AUDIOPARM:				/*获取音频参数*/
		onGetAudioParm( data , len ) ;
		break ;
	case CMD_CODE_I_FRAME:    			/*强制I 帧请求*/
		onSetIframe(data, len);
		break;

	/** add by mike, 2013-12-19, 增加远程升级相关接口*/
	case CMD_CHECK_SW_UPDATE_INFO:		/* 检测软件版本更新信息*/
		onCheckAppUpdateInfo(data, len);
		isNeedClose = true;		
		break;

	case CMD_START_REMOTE_UPDATE:		/* 开始自动升级*/
		onStartRemoteUpdate(data, len);
		isNeedClose = true;
		break;

	case CMD_GET_REMOTE_UPDATE_STAT:	/* 获取自动升级状态*/
		onGetRemoteUpdateStat(data, len);
		isNeedClose = true;
		break;

	case CMD_CANCEL_REMOTE_UPDATE:		/* 取消自动升级*/
		onCancelRemoteUpdate(data, len);
		isNeedClose = true;
		break;

	case CMD_PAUSE_REMOTE_UPDATE:		/* 暂停自动升级 */
		onPauseRemoteUpdate(data, len);
		isNeedClose = true;
		break;

	case CMD_RESUME_REMOTE_UPDATE:		/* 恢复自动升级*/
		onResumeRemoteUpdate(data, len);
		isNeedClose = true;
		break;
		
	case CMD_GET_DATE_FORMAT:			/* 获取时间格式 */
		onGetDateFormat(data, len);
		isNeedClose = true;
		break;
	case CMD_SET_DATE_FORMAT:			/* 设置时间格式 */
		onSetDateFormat(data, len);
		isNeedClose = true;
		break;
	case CMD_R_DEV_DISK:				/* 获取设备录像信息接口*/
		onGetDiskInfo(data, len);
		break;
	case CMD_GET_IPC_VERSION:			/* 获取设备版本号*/
		onGetVersion(data, len);
		break;
	case CMD_S_WIFINVR_NOTIFY:			/* 设置wifi NVR 码流模式 */
		onWifiNVRMode(data, len);
		break;
	case CMD_S_CHANGE_IPC_STREAM_RATE:	/* 改变IPC码流 */
		onChangeStreamRate(data, len);
		break;
	case CMD_G_IPC_VENC_ATTR:
		onGetViRate(data, len);
		break;
	case CMD_S_IPC_VENC_ATTR:
		onSetViRate(data, len);
		break;
	case CMD_G_VIDEO_KEY:
		onGetVideoKey(data, len); /* 获取设备视频加密密钥 */
		break;
	case CMD_G_SEARCH_RECORD_DAY:
		searchPlaybackListInDates(data, len);
		break;
	case 0xfefe:
		{
			FILE* file = fopen("/config/p2p_test_config.h", "rb");
			Cmd_Header head={0};
			if(file)
			{
				char buf[1024]="";
				fread(buf, 1, 1024, file);
				fclose(file);				

				head.length = strlen(buf);
				head.commd  = 0xfefe;				
				
				TcpBlockSend( m_sessionHandle , (char *)&head , sizeof( head ) ) ;
				TcpBlockSend( m_sessionHandle , buf, head.length ) ;
			}else
			{
				head.length = 0;
				head.commd  = 0xfefe;
				TcpBlockSend( m_sessionHandle , (char *)&head , sizeof( head ) ) ;
			}
		}
		break;
		//set p2p server info
	case 0xfeff:
		{
			FILE* file = fopen("/config/p2p_test_config.h", "wb");
			if(file)
			{
				fwrite(data+sizeof(Cmd_Header), 1, len-sizeof(Cmd_Header), file);
				fclose(file);

				Cmd_Header head={0};;

				head.length = 0;
				head.commd  = 0xfeff;				
				
				TcpBlockSend( m_sessionHandle , (char *)&head , sizeof( head ) ) ;
			}
		}
		break;
	case CMD_NOTIFY_IPC_NETSTATUS:
		printf("========================CMD_NOTIFY_IPC_NETSTATUS\n");
		break;
	case CMD_S_MD_REGIN:
		onSetMdRegion(data, len); /* 设置移动侦测区域 */
		break;
	case CMD_G_MD_REGIN:
		onGetMdRegion(data, len); /* 获取移动侦测区域 */
		break;
	case CMD_S_MOTOR_TEST: 
		onSetMotorTest(data, len);/*外设模块检测工具控制电机转动*/
		break;
/*
	case CMD_S_RESTORE: 
		onResetTest(data, len);
		break;	
*/
	case CMD_S_NIGHT_SWITCH_TEST: 
		onNightVisionTest(data, len);/*外设模块检测工具控制复位*/
		break;	
	default :
		ERR( "CNetUser recv unknow message =0x%x!!\r\n" , pHeader->commd ) ;
		isNeedClose = true;
		break ;
	}
	
	if(isNeedClose)
		return false;//close socket
		
	return bRet ;
}

//----------------------------------------------
//用户登录请求
bool CNetUser::onDeviceType( char *data , int len )
{
    char userName[USER_NAME_LEN + 1];
    char userPwd[USER_PASS_LEN + 1];

    STRUCT_DEVICE_TYPE_REQUEST *pReq = ( STRUCT_DEVICE_TYPE_REQUEST *) data ;

    //拷贝用户名密码，并对数据长度进行保护
    memcpy(&userName , pReq->name , USER_NAME_LEN);
    userName[USER_NAME_LEN] = 0 ;
    memcpy(&userPwd , pReq->pwd , USER_PASS_LEN);
    userPwd[USER_PASS_LEN] = 0 ;

	//printf( "&&&&&&&*****user login name = %s || pwd = %s\r\n" , userName ,  userPwd ) ;
	
	//用户名为空，直接断链接
    if(strcmp("", userName) == 0)
    {
		return false ;
	}

	int echoCode = 0;
	int permit = 0;
	
	echoCode = GetNetServerObj()->VerifyUser( userName , userPwd , permit ) ;
    if( echoCode != 0 )
    {
        ZmdDeCrypt( userName , (char *)CRYPT_KEY ) ;
        ZmdDeCrypt( userPwd , (char *)CRYPT_KEY ) ;
		//printf( "user login name = %s || pwd = %s\r\n" , userName ,  userPwd ) ;
		echoCode = GetNetServerObj()->VerifyUser( userName , userPwd , permit ) ;
	}

	//组回应包
	STRUCT_DEVIDE_TYPE_ECHO resp ;
	memcpy( &resp.header , data , sizeof( Cmd_Header ) ) ;

	GetSoftWareVersion( &resp.deviceInfo );

	//memcpy( &resp.deviceInfo , &version , sizeof( version ) ) ;
	memcpy( &resp.echoCode , &echoCode , sizeof( echoCode )) ;
	memcpy( &resp.permit , &permit , sizeof( permit ) ) ;

	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;
	return true ;
}

//-------------------------------------------
//处理直播请求信令
bool CNetUser::onRequestVideoLiving( char *data , int len )
{
	NDB( "onRequestVideoLiving() len = %d \r\n" , len ) ;
	STRUCT_START_LIVING* pReq = (STRUCT_START_LIVING*)data ;

	if( sizeof( STRUCT_START_LIVING ) - sizeof(Cmd_Header) !=  pReq->header.length )
		return false ;
	
	int echo = 0 ;
	//检查请求
	m_mediaCmd = pReq->header.commd ;
	m_Channel = pReq->header.channel ;

	//设置用户请求的媒体数据类型
	switch(m_mediaCmd )
	{
	case CMD_START_VIDEO :
		m_ChannelType = VGA_CHN_TYPE ;
		break ;
	case CMD_START_SUBVIDEO :
		m_ChannelType = QVGA_CHN_TYPE ;
		break ;
	case CMD_START_720P :
		m_ChannelType = D720P_CHN_TYPE ;
		break ;
	default :
		echo = -1 ;
		break ;
	}
	if (0 == GetDeviceSwitchStatus()) 
	{ 
		echo = -5; // 设备关闭 
	} 

	//请求开始直播视频
	if( !echo )
		echo = RequestStartVideoLiving( m_mediaCmd , m_Channel ) ;

	STRUCT_START_LIVING_ECHO resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.result = echo ;
	resp.header.length = GET_HEADER_LEN( STRUCT_START_LIVING_ECHO ) ;
	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	///成功则启动视频直播
    if( !echo )
    {
		StartVideoLiving( m_mediaCmd , m_Channel ) ;
	}
	
	unsigned char* ip = (unsigned char*)&m_remoteIP ;
//	NDB( "onRequestVideoLiving() channel = %d channelType=%d , m_mediaSid=%d , echo = %d ip=%d.%d.%d.%d\r\n" , m_Channel , m_ChannelType , m_mediaSid , echo ,
//		ip[0] , ip[1] , ip[2] , ip[3] ) ;

	return (!echo) ;
}

//====================================
//向系统请求直播
int	CNetUser::RequestStartVideoLiving( int command , int channel )
{
	NDB("RequestStartVideoLiving=%d,%d\r\n" ,GetNetServerObj()->GetTcpUserCount() , MAX_STREAM_USER ) ;
	if( GetNetServerObj()->GetTcpUserCount() >= MAX_STREAM_USER )
	{
		ERR( "max tcp link!!! tcp count=%d max=%d\r\n" , GetNetServerObj()->GetTcpUserCount() , MAX_STREAM_USER ) ;
		return -1 ;
	}

	//获取一个空闲媒体会话
	int mediaid = GetMediaMgr()->getUnuseMediaSession( m_Channel , m_ChannelType ) ;
	if( INVALID_SESSION == mediaid )
	{
	    ERR( "RequestStartVideoLiving() getUnuseMediaSession() return -1 \r\n" ) ;
	    return -1 ;
	}

	m_mediaSid = mediaid ;

	int streamtype = 0;
	switch(command )
	{
		//是否支持720P
		case CMD_START_VIDEO :
			streamtype = 1;		
			break ;
		case CMD_START_SUBVIDEO:
			streamtype = 2;
			break ;
		case CMD_START_720P :
			streamtype = 0; 
			break ;
		default :
			break ;
	}
	ResetUserData2IFrame(0,streamtype , m_mediaSid ) ;

	return 0 ;
}


//==============================================
//开始直播
int	CNetUser::StartVideoLiving( int command , int channel )
{
	m_bStartSendMedia = true ;
	//数据将会在onIdle()中发送
	return 0 ;
}

//===============================================
//处理停止直播请求信令
bool CNetUser::onStopVideoLiving( char *data , int len )
{
    NDB( "onStopVideoLiving() len = %d \r\n" , len ) ;

	//返回false通知底层关闭连接
	m_bStartSendMedia = false ;
	//断开连接
	return false ;
}

//=============================================
//处理格式化磁盘信令
bool CNetUser::onCmdFormatDisk( char *data , int len )
{
	NDB( "onCmdFormatDisk() len = %d \r\n" , len ) ;
	
	return false ;
}

//==========================================
//处理获取设备mac地址信令
bool CNetUser::onGetDeviceMac( char *data , int len )
{
	NDB( "onGetDeviceMac() len = %d \r\n" , len ) ;

    STRUCT_GET_MAC_ECHO resp ;
    NETWORK_PARA		netset;

    //获取网络配置
    g_cParaManage->GetSysParameter(SYSNET_SET, &netset);

    memcpy( &resp.header , data , sizeof(Cmd_Header) );
    memcpy( &resp.macAddr , netset.m_Eth0Config.m_uMac , 6 );

    //mac地址 6字节长度
    resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;

    return true ;
}

//===================================================
//处理设置设备mac地址信令
bool CNetUser::onSetDeviceMac( char *data , int len )
{
    STRUCT_SET_MAC_REQUEST *pReq = (STRUCT_SET_MAC_REQUEST *)data ;
    char szMac[8] ;
    memcpy( szMac , pReq->macAddr , 6 ) ;
    szMac[6] = 0 ;
    int echo = -1 ;

    NDB( "onSetDeviceMac() len = %d , mac = %2x:%2x:%2x:%2x:%2x:%2x\r\n" , len , szMac[0] ,
            szMac[1] , szMac[2] , szMac[3] , szMac[4] , szMac[5] ) ;

	if(szMac[0]%2 == 1)/*mac 地址首位不能为奇数否则设备无法通信*/
		szMac[0]+=1;
	
	//设置mac地址
	int fd = -1;
	int retval = 0 ;
	echo = -1 ;

	char cmd[64] = {0};
	sprintf(cmd, "rm %s", MAC_ID_FILE);
    system(cmd);
    fd = open(MAC_ID_FILE,  O_CREAT | O_RDWR, 0777);
	
	//写入mac地址
	if( fd >= 0 )
	{
		retval = write(fd , szMac , 6);
		if( retval > 0 )
		{
			echo = 0 ;
			fsync(fd);
		}
		close(fd);
		fd = -1;
	}

	NETWORK_PARA	netset;
	//读取设备网络配置
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
	memcpy( netset.m_Eth0Config.m_uMac, szMac , 6 );
	//写配置文件
	g_cParaManage->SetSystemParameter(SYSNET_SET,&netset);
	//重新设置网卡属性
	
	GetNetModule()->SetLoaclNetMac();
	
	//发送回应
	STRUCT_SET_MAC_ECHO resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.echo = echo ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;

	return true ;
}

//==========================================================
//处理语音对讲打开信令
bool CNetUser::onTalkOn( char *data , int len )
{
	NDB( "onTalkOn() len = %d \r\n" , len ) ;

	STRUCT_TALK_ON_REQUEST* pReq = ( STRUCT_TALK_ON_REQUEST *)data ;

	STRUCT_TALK_ON_ECHO resp ;
	memset( &resp , 0 , sizeof( resp ) ) ;

	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

	resp.talkFlag = 1 ;

	do
	{
		if( !GetNetServerObj()->RequestTalkOn( m_nUserID ) )
			break ;

#ifdef SUPPORT_AUDIO
		//打开通知音频解码,0 is g711
		if( StartAudioDecode( 0 ) != 0 )
		{
			resp.talkFlag = 2 ;
			break ;
		}

		AudioParm adParam ;
		memset( &adParam , 0 , sizeof( adParam ) ) ;

		GetAudioParam( &adParam ) ;
		
		resp.audioParam.audioType = adParam.audiotype ;
		resp.audioParam.enBitwidth = adParam.enBitwidth ; 
		resp.audioParam.sampleRate = adParam.samplerate ;
		resp.audioParam.recordVolume = adParam.inputvolume ;
		resp.audioParam.speakVolume = adParam.outputvolume ;
		resp.audioParam.framelen	= adParam.framelen;
#endif
		m_bStartTalk = true ;
		resp.talkFlag = 0 ;

    }
    while( 0 ) ;


	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
	
	return true ;
}

//===================================================
//处理语音对讲关闭信令
bool CNetUser::onTalOff( char *data , int len )
{
	STRUCT_TALK_OFF_REQUEST* pReq = (STRUCT_TALK_OFF_REQUEST*)data ;
	
	GetNetServerObj()->RequestTalkOff( m_nUserID ) ;
#ifdef SUPPORT_AUDIO
	StopAudioDecode() ;
#endif
	m_bStartTalk = false ;

	return false  ;
}

//===================================================
//
bool CNetUser::onAlarmUpdate( char *data , int len )
{
    NDB( "onAlarmUpdate() len = %d \r\n" , len ) ;

	return true ;
}

//===================================================
//处理语音对讲数据
bool CNetUser::onTalkData( char *data , int len )
{
    STRUCT_TALK_DATA *pReq = (STRUCT_TALK_DATA *)data ;
    if( pReq->header.length != len - sizeof( Cmd_Header ))
    {
        //数据长度有误
        return false ;
    }

#ifdef SUPPORT_AUDIO
    SendAudioStreamToDecode( (unsigned char *)pReq->audioData , pReq->header.length,0) ;
#endif

	return true ;
}

//===================================================
//处理读取设备参数信令
bool CNetUser::onReadDevParam( char *data , int len )
{
	NDB( "onReadDevParam() len = %d \r\n" , len ) ;

    STRUCT_READ_DEV_PARAM_REQUEST *pReq = (STRUCT_READ_DEV_PARAM_REQUEST *)data ;

    STRUCT_READ_DEV_PARAM_ECHO resp ;

	memset( &resp , 0 , sizeof( resp ) ) ;
	//系统参数为空
	if( !g_cParaManage->m_syspara )
		return false ;
			

	if(get_network_support()!=NET_WORK_CARD_LOCAL)
	{
		unsigned char *p = g_cParaManage->m_syspara->m_NetWork.m_WifiConfig.WifiAddrMode.m_uMac;
		//如果系统启动时wifi模块未加载，重新给参数赋值一次。
		if( !*p )
		{
		   
		    GetWifiMac(g_cParaManage->m_syspara->m_NetWork.m_WifiConfig.WifiAddrMode.m_uMac);
			printf("\nwifi mac addr reload...\n");
		    
		}
	}
	//发送回应
	memcpy( &resp.param , g_cParaManage->m_syspara , sizeof(SYSTEM_PARAMETER));
	
	printf("=============================sizeof(SYSTEM_PARAMETER),%d\n",sizeof(SYSTEM_PARAMETER));
	//加密用户信息
    ZmdEnCrypt( resp.param.m_NetWork.m_CenterNet.passwd , (char *)CRYPT_KEY ) ;
    ZmdEnCrypt( resp.param.m_NetWork.m_PppoeSet.m_s8UserName , (char *)CRYPT_KEY ) ;
    ZmdEnCrypt( resp.param.m_NetWork.m_PppoeSet.m_s32Passwd, (char *)CRYPT_KEY ) ;
    ZmdEnCrypt( resp.param.m_NetWork.m_DomainConfig.m_s8Name , (char *)CRYPT_KEY ) ;
    ZmdEnCrypt( resp.param.m_NetWork.m_DomainConfig.m_s32Passwd  , (char *)CRYPT_KEY ) ;

    ZmdEnCrypt( resp.param.m_NetWork.m_email.m_account , (char *)CRYPT_KEY ) ;
    ZmdEnCrypt( resp.param.m_NetWork.m_email.m_password , (char *)CRYPT_KEY ) ;

    ZmdEnCrypt( resp.param.m_NetWork.m_ftp.m_account , (char *)CRYPT_KEY ) ;
    ZmdEnCrypt( resp.param.m_NetWork.m_ftp.m_password , (char *)CRYPT_KEY ) ;

    int index ;
    for( index = 0 ; index < 16 ; index ++ )
    {
        if( strlen( resp.param.m_Users.m_UserSet[index].m_cUserName ) > 0 )
        {
            ZmdEnCrypt( resp.param.m_Users.m_UserSet[index].m_cUserName , (char *)CRYPT_KEY ) ;
        }

        if( strlen( resp.param.m_Users.m_UserSet[index].m_s32Passwd ) > 0 )
        {
			ZmdEnCrypt( resp.param.m_Users.m_UserSet[index].m_s32Passwd , (char*)CRYPT_KEY ) ;
		}
	}

	memcpy(&resp.header ,  &pReq->header , sizeof( pReq->header ) );
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	return true ;
}

//===================================================
//设置系统参数信令
bool CNetUser::onSetDevParam( char *data , int len )
{
	NDB( "onSetDevParam() len = %d \r\n" , len ) ;

    STRUCT_SET_DEV_PARAM_REQUEST *pReq = (STRUCT_SET_DEV_PARAM_REQUEST *)data ;
    SYSTEM_PARAMETER localPara, csPara;

    int	echocmd = 0;
    int needreboot1 = 0;
    int index ;

    memset(&localPara, 0, sizeof(SYSTEM_PARAMETER));
    memset(&csPara, 0, sizeof(SYSTEM_PARAMETER));
    memcpy(&csPara, &pReq->param , sizeof( SYSTEM_PARAMETER ) );

    //解密用户信息
    ZmdDeCrypt( csPara.m_NetWork.m_CenterNet.passwd , (char *)CRYPT_KEY ) ;
    ZmdDeCrypt( csPara.m_NetWork.m_PppoeSet.m_s8UserName , (char *)CRYPT_KEY ) ;
    ZmdDeCrypt( csPara.m_NetWork.m_PppoeSet.m_s32Passwd, (char *)CRYPT_KEY ) ;
    ZmdDeCrypt( csPara.m_NetWork.m_DomainConfig.m_s8Name , (char *)CRYPT_KEY ) ;
    ZmdDeCrypt( csPara.m_NetWork.m_DomainConfig.m_s32Passwd  , (char *)CRYPT_KEY ) ;

    ZmdDeCrypt( csPara.m_NetWork.m_email.m_account , (char *)CRYPT_KEY ) ;
    ZmdDeCrypt( csPara.m_NetWork.m_email.m_password , (char *)CRYPT_KEY ) ;

    ZmdDeCrypt( csPara.m_NetWork.m_ftp.m_account , (char *)CRYPT_KEY ) ;
    ZmdDeCrypt( csPara.m_NetWork.m_ftp.m_password , (char *)CRYPT_KEY ) ;

    for( index = 0 ; index < 16 ; index ++ )
    {
        if( strlen( csPara.m_Users.m_UserSet[index].m_cUserName ) > 0 )
        {
            ZmdDeCrypt( csPara.m_Users.m_UserSet[index].m_cUserName , (char *)CRYPT_KEY ) ;
        }

        if( strlen( csPara.m_Users.m_UserSet[index].m_s32Passwd ) > 0 )
        {
            ZmdDeCrypt( csPara.m_Users.m_UserSet[index].m_s32Passwd , (char *)CRYPT_KEY ) ;
        }
    }

    needreboot1 = 0;

    echocmd = 0; // CMD_S_DEV_PARA
    //发送回应
    STRUCT_SET_DEV_PARAM_ECHO resp ;
    memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) );
    resp.echo = echocmd ;
    resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;

	//======================================
	//开始设置参数，并检查是否需要重启
	if( csSetNetWorkPara(&localPara, &csPara) > 0 )
		needreboot1 |= 0x0001;
	
	if( csSetMachinePara(&localPara, &csPara) > 0)
		needreboot1 |= 0x0004;

	if( csSetCameraPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x0008;

	if( csSetAnalogPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x0010;

	if( csSetComParaPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x0040;

	if( csSetCamBlindPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x0080;

	if( csSetCamMdPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x0100;

	if( csSetRecSchedPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x0200;

	if( csSetPcDirPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x0400;

	if( csSetMainetancePara(&localPara, &csPara) > 0)
		needreboot1 |= 0x1000;

	if( csSetDisplayPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x2000;

	if( csSetUsersPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x4000;
	
	if( csSetSysExceptPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x8000;

	if( csSetOsdInsertPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x010000;

	if( csSetPTZPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x020000;

	if( csSetZoneGroupPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x080000;
	
	if( csSetDefSchedPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x100000;

	if( csSetPtzLinkPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x200000;
	if( csSet3GPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x800000;

	if( csSetPELCOPara(&localPara, &csPara) > 0)
		needreboot1 |= 0x02000000;
	csSetCamVideoLossPara(&localPara, &csPara);
	csSetCamAlarmPortPara(&localPara, &csPara);
	
	if(needreboot1)
	{
		NDB("cs need reboot: %x\n", needreboot1);
		sleep(3);
		RebootSystem();
	}

	return true ;
}

//==============================================
// 7 :
//===================================================
//云台控制信令
bool CNetUser::onSetPtz( char *data , int len )
{
	if(ConfigInfo.SupportInfo&(CONFIG_PT) || ConfigInfo.SupportInfo&(CONFIG_ZOOM))
	{
	    STRUCT_SET_PTZ_REQUEST *pReq = (STRUCT_SET_PTZ_REQUEST *)data ;
		Crtl_ptzPara(pReq->cmd,pReq);
	}
	return true ;
}

//===================================================
//系统重启信令
bool CNetUser::onSetReboot( char *data , int len )
{
	NDB( "onSetReboot() len = %d \r\n" , len ) ;

    STRUCT_SET_REBOOT_REQUEST *pReq = (STRUCT_SET_REBOOT_REQUEST *)data ;

    //验证校验字
    if( pReq->headFlag1 != 0x55555555 || pReq->headFlag2 != 0xaaaaaaaa )
        return false ;

    //发送回应
    STRUCT_SET_REBOOT_ECHO resp ;
    memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
    resp.echo = 0 ;
    resp.header.length = sizeof( resp ) - sizeof( Cmd_Header )  ;

    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;

    NDB("========== need reboot from C/S ===============\n");

	sleep(3);
	RebootSystem();

	return true ;
}

//===================================================
//系统升级信令
bool CNetUser::onRequestUpdate( char *data , int len )
{
	STRUCT_SET_UPDATE_REQUEST* pReq = (STRUCT_SET_UPDATE_REQUEST*)data ;
	UPGRADECHECKSTRUCT UpdateFileHeader;
	
	NDB("onRequestUpdate fileLength:%d  len:%d\r\n", pReq->fileLength, len);
		
	unsigned char* fileBuf = 0 ;
	int echo = 0 ; /*回应字段*/
	int updateFileHeaderCount = 0;
	int nRecved = 0 ;
	/*******************************/
	/*  echo
	/*	0 --- 升级成功
	/*	1 --- 申请内存失败!
	/*	2 --- 文件版本不匹配，不能升级
	/*	3 --- 文件接收出错
	/*	4 --- 文件校验不正确!
	/*	5 --- 文件版本相同
	/********************************/
	unlink(UPDATEFILE);
	do
	{
	    /*检查校验，不对，关闭连接*/
	    if( pReq->checkFlag != 0x5555aaaa )
	    {
			ERR("check flag error !!!!!\r\n" ) ;
			return false ;
		}

		//设置升级标志，阻止其他程序
		GetNetServerObj()->SetUpdating(true);	

		fileBuf = (unsigned char*)malloc(UPDATEFILE_LEN) ;
	    if(!fileBuf)
	    {
			echo = 1 ;
			ERR("Faild to malloc update buf!\r\n");
			break;
		}
		
			
		/*减去消息头，剩下的是升级文件数据*/
		int nGetFileCount = len - sizeof( STRUCT_SET_UPDATE_REQUEST);
	
		if(nGetFileCount < 0)
		{
			echo = 3;
			break;
		}
		
		memset(fileBuf, 0x0, UPDATEFILE_LEN);
		memcpy(fileBuf, data + sizeof( STRUCT_SET_UPDATE_REQUEST), nGetFileCount), 
		/*保证先收到升级文件头 UPGRADECHECKSTRUCT*/
		nRecved = TcpBlockRecv(m_sessionHandle, (char*)fileBuf + nGetFileCount, sizeof(UPGRADECHECKSTRUCT), 1000) ;
		if(nRecved < 0)
		{
			echo = 3 ;
			ERR("Recv UpdateFile error!\r\n");
			break;
		}
		
		nGetFileCount += nRecved;	

		memset(&UpdateFileHeader, 0x0, sizeof(UPGRADECHECKSTRUCT));
		
		memcpy(&UpdateFileHeader, (UPGRADECHECKSTRUCT *)fileBuf, sizeof(UPGRADECHECKSTRUCT));
		

		int ret = 0;
		if((ret = CheckUpdateVersion(&UpdateFileHeader)) < 0)
		{
			if(ret == -1)
				echo = 2;
			else if(ret == -2)
				echo = 5;
			
			ERR("Faild to  CheckUpdateVersion! echo:%d\r\n\r\n", ret);	
		}

		if( nGetFileCount > 0 )
		{
			/*保存到文件 UPDATEFILE*/
			

		    FILE* fp;
			int Ret;
			if((fp = fopen(UPDATEFILE, "w+")) == NULL)
			{
				ERR("open file UPDATEFILE faild\r\n");
			}
			fseek(fp, 0, SEEK_SET);
			Ret = fwrite(fileBuf + sizeof(UPGRADECHECKSTRUCT), nGetFileCount - sizeof(UPGRADECHECKSTRUCT), 1, fp);
			fclose(fp);
		}
		
		int nRecved = 0 ;
		int nLeft = pReq->fileLength - nGetFileCount ;
			
		//还有数据，继续接收
	   while(nLeft > 0)
	    {
	    	memset(fileBuf, 0x0, UPDATEFILE_LEN);
			//在此处阻塞底层网络，不允许底层网络接收数据，继续接收剩余的升级文件数据
			int recvLen;
			recvLen = UPDATEFILE_LEN;
			if(nLeft < recvLen)
			{
				recvLen = nLeft;
			}
			
			nRecved = TcpBlockRecv(m_sessionHandle, (char*)fileBuf, recvLen, 20*1000) ;
	
			if(nRecved < 0)
			{
				echo = 3 ;
				ERR("Recv UpdateFile error!\r\n");
				break;
			}
	     
			nLeft -= nRecved ;
		
		    FILE* fp;
			int Ret;
			if((fp = fopen(UPDATEFILE, "a+")) == NULL)
			{
				ERR("open file UPDATEFILE faild\r\n");
			}
			fseek(fp, 0, SEEK_END);
			Ret = fwrite(fileBuf, 1, nRecved, fp);
			fclose(fp);
		}


	   	/**************************/
		/*			MD5校验	  */

		FILE* fp = NULL;
	
 		fp = fopen(UPDATEFILE, "rb");
		if(fp != NULL)
		{
			if(CheckUpdateFileMD5(&UpdateFileHeader, fp) < 0)
			{
				echo = 4;
				ERR("faild to check update file Md5!\r\n");
			}
		}
		else
			echo = 4;

		fclose(fp);
		
		/**************************/
		
	}while(0);

	if(fileBuf)
    {
		free(fileBuf);	
		fileBuf = 0;
	}

	NDB("Update result echo:%d\r\n", echo);
	/*发送升级结果*/
	STRUCT_SET_UPDATE_ECHO resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.header.length = sizeof( STRUCT_SET_UPDATE_ECHO ) - sizeof( Cmd_Header ) ;
	resp.echo = echo ;
	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) );
	sleep(1);
	
	if(echo == 0)
	{
		/*进行升级*/
		UpdateToFlash(UpdateFileHeader.m_filetype);
	}
	else
	{
		GetNetServerObj()->SetUpdating( false ) ;
		startupdate = 1 ;
	}
	return true;
}

//===================================================
//系统设置时间信令
bool CNetUser::onCmdNpt( char *data , int len )
{
	NDB( "onCmdNpt() len = %d \r\n" , len ) ;
	
	STRUCT_SET_NTP_REQUEST* pReq = (STRUCT_SET_NTP_REQUEST*)data ;

	int	echo = -1 ;
	if(ntpclient_is_running())
	{
		ntpclient_stop();
	}
	//检查时间合理性
	do{
		//只能设置到2037年
		if( pReq->datetime.year > 36 )
			break ;
		
		if( pReq->datetime.month == 0 || pReq->datetime.month > 12 )
			break ;

		if( pReq->datetime.day ==0 || pReq->datetime.day > 31 )
			break ;

		if( pReq->datetime.hour > 23 )
			break ;

		if( pReq->datetime.minute > 59 )
			break ;

		if( pReq->datetime.second > 59 )
			break ;
		
		//数据均正常
		echo = 0 ;
    }
    while( 0 ) ;
	
	STRUCT_SET_NTP_ECHO resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.echo = echo ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	rtc_time_t  datetime;
	//赋值
	datetime.year = pReq->datetime.year ;
	datetime.month = pReq->datetime.month ;
	datetime.date = pReq->datetime.day ;
	datetime.hour = pReq->datetime.hour ;
	datetime.minute = pReq->datetime.minute ;
	datetime.second = pReq->datetime.second ;
	datetime.weekday = 0;

	//修正时间
	PubSyncSytemTime( pReq->datetime );
	DS_RTC_SetTime(&datetime);
	DS_RTC_GetTime(&datetime);
	return true ;
}

//==========================================
//恢复出厂设置
bool CNetUser::onRequestResotre( char *data , int len )
{
	NDB( "onRequestResotre() len = %d \r\n" , len ) ;

    STRUCT_SET_RESTORE_REQUEST *pReq = (STRUCT_SET_RESTORE_REQUEST *)data ;

    if( pReq->headFlag1 != 0x55555555 || pReq->headFlag2 != 0xaaaaaaaa )
        return false ;

    STRUCT_SET_RESTORE_ECHO resp ;
    memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
    resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
    resp.echo = 0 ;

    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;

    sleep(3);
    RestoreDefault((SYSTEM_PARAMETER *)PubGetParameterPtr());					

    return true ;
}

//============================================
// 3G模块控制信令
bool CNetUser::on3GCtrlCmd( char *data , int len )
{
#if 0

#ifdef DEBUG_NETSERVER
	printf( "on3GCtrlCmd() len = %d \r\n" , len ) ;
#endif

	STRUCT_SET_3GCTL_REQUEST* pReq = (STRUCT_SET_3GCTL_REQUEST*)data ;

	STRUCT_SET_3GCTL_ECHO resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
	resp.echo = 0 ;

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

#ifndef RUN_LITE
	if( pReq->status )
		g3manst=G3MANUP;
	else
		g3manst=G3MANDOWN;
	
#endif
	#endif
	return true ;
}

//===================================================
//3G模块重启信令
bool CNetUser::on3GRepower( char *data , int len )
{
	NDB( "on3GRepower() len = %d \r\n" , len ) ;
	return true ;
}

//===================================================
//获取3G模块重启状态
bool CNetUser::onGet3GRepower( char *data , int len )
{
	NDB( "onGet3GRepower() len = %d \r\n" , len ) ;
	return true ;
}

//===================================================
//获取3G模块信息
bool CNetUser::onGet3GInfo( char *data , int len )
{
#if 0

#ifdef DEBUG_NETSERVER
	printf( "onGet3GInfo() len = %d \r\n" , len ) ;
#endif


	STRUCT_READ_3GINFO_REQUEST* pReq = (STRUCT_READ_3GINFO_REQUEST*)data ;
	if( 0xaaaa5555 != pReq->l3gtype )
		return false ;

#ifndef RUN_LITE

	STRUCT_READ_3GINFO_ECHO resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header )  ;
	resp.lg3type = g3cfg.g3card; // 型号
	resp.lg3linkstate =  g3tt; // 连接状态

	//获取ip
    if( G3STATUP == resp.lg3linkstate )
    {
		strncpy(resp.lip_buff , g3ip[0] ,15);
		resp.lip_buff[sizeof(resp.lip_buff)-1] = 0 ;
	}
    else
    {
		memset( resp.lip_buff , 0 , sizeof( resp.lip_buff ) ) ;
	}

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

#endif
#endif	
	return true ;
}

//===================================================
//读取设备日志
bool CNetUser::onReadLog( char *data , int len )
{
#if 0
	NDB( "onReadLog() len = %d \r\n" , len ) ;
	
	STRUCT_READ_LOG_REQUEST* pReq = (STRUCT_READ_LOG_REQUEST*)data ;
	
	//=========================================
	//检查数据有效性
	if((pReq->logFind.m_u8Month == 0)||(pReq->logFind.m_u8Month > 12)) 
		return false ;
	if((pReq->logFind.m_u8Day == 0)||(pReq->logFind.m_u8Day >31)) 
		return false ;
	
	int maxcnt = 0; 
	int csLogNum = 0;
	
	pReq->logFind.m_u8SearchType = 0; // 初始加载第一页
	
	//
	if(pReq->logFind.m_u8LogType == 0)
		pReq->logFind.m_u8LogType = 1; // 1--报警日志 2--系统日志

	STRUCT_READ_LOG_ECHO resp ;
	
    do
    {
		//查询日志,函数会填充resp.logItem字段
		//csLogNum = ObtainSomeUserLog(&pReq->logFind , 10 , &resp.logItem ); // 注意，此函数对 FILE_NEXT 是循环搜索

		if(csLogNum < 1) 
			resp.logItem.m_u8ListNum = 0;
		
		//发送回应包
		memcpy(&resp.header , &pReq->header , sizeof( Cmd_Header ) );
		resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

		int nSend = TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
		if( nSend != sizeof( resp ))
			return false ;
		
		//准备下次的搜索
		pReq->logFind.m_u8SearchType = 0xff; 
		maxcnt += resp.logItem.m_u8ListNum; // m_u8SearchType的处理改变会影响到搜索，目的避开循环搜索
    }
    while( (resp.logItem.m_u8ListNum == 10) && (maxcnt < 10000) );
#endif
	return true ;
}

//===================================================
//读取设备信息
bool CNetUser::onReadDevInfo( char *data , int len )
{
	NDB( "onReadDevInfo() len = %d \r\n" , len ) ;
		
	STRUCT_READ_DEVINFO_REQUEST* pReq = (STRUCT_READ_DEVINFO_REQUEST*)data ;

	STRUCT_READ_DEVINFO_ECHO resp ;

	memset( &resp , 0 , sizeof( resp ) ) ;

	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

	GetSoftWareVersion( &resp.devInfo );
	GetBlockDeviceInfo( 0 , &resp.storageDev[0] );
	GetBlockDeviceInfo( 1 , &resp.storageDev[1] );	
	
	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
		
	return true ;
}

//===================================================
//请求搜索录像文件列表
bool CNetUser::onSearchPlayList( char *data , int len )
{
	printf( "onSearchPlayList() len = %d \r\n" , len ) ;

	STRUCT_READ_PLAYLIST_REQUEST* pReq = (STRUCT_READ_PLAYLIST_REQUEST*)data ;
	
	int result, mode, maxcnt, nSend ;

	mode = 0; maxcnt = 0;

	printf("onSearchPlayList: RecordType = %d\n", pReq->findType.RecordType);

	switch(pReq->findType.RecordType)
	{
		case 1: pReq->findType.RecordType = 3; break;
		case 2: pReq->findType.RecordType = 5; break;
		case 3: pReq->findType.RecordType = 1; break;
		case 4: pReq->findType.RecordType = 2; break;
		default : pReq->findType.RecordType = 0; break;
	}; // 类型转换

	STRUCT_READ_PLAYLIST_ECHO resp ;
	printf("Sort_File_list [%d]-[%d]-[%d]\r\n",pReq->findType.time.tm_mday,pReq->findType.time.tm_mon,pReq->findType.time.tm_year + 2000);
	//Sort_File_list(pReq->findType.time.tm_mday,pReq->findType.time.tm_mon,pReq->findType.time.tm_year + 2000,false);		 //按时间顺序,或逆序重新排列recfilelist录像记录

	do{
		result = FindRecordFile(&pReq->findType, &resp.result , mode, 10); // 注意，此函数对 FILE_NEXT 是循环搜索
		
		if(result !=0) 
			resp.result.fileNb = 0;
		printf("playlist num:%d, filenum:%d\n", resp.result.fileNb, resp.result.fileNb);

		memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header )) ;
		resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
		
		nSend = TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
		//数据不正常，关闭连接
		if( nSend != sizeof( resp ) ){
			return false ;
		}
		mode = 0xff; 
		maxcnt += resp.result.fileNb; // FindRecordFile的mode处理改变会影响到搜索，目的避开循环搜索

	}while( (resp.result.fileNb==10) && (maxcnt<10000) );

	memcpy(&resp.header,&pReq->header ,sizeof(Cmd_Header));
	resp.header.length = 0;

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	
	//通知底层关闭连接
	return false ;

}


bool CNetUser::searchPlaybackListInDates( char *data , int len )
{
	STRUCT_GET_RECORD_DAY_REQUEST* req = (STRUCT_GET_RECORD_DAY_REQUEST*)data;

	printf("len:%d, %d\n", len, sizeof(STRUCT_GET_RECORD_DAY_REQUEST));
	if(len < sizeof(STRUCT_GET_RECORD_DAY_REQUEST))
	{
		printf("searchPlaybackListInDates len < sizeof(STRUCT_GET_RECORD_DAY_REQUEST)!!!\n");
		return false;
	}
	req->day_start.tm_year += (2000-1900);
	req->day_start.tm_mon -= 1;
	req->day_end.tm_year += (2000-1900);
	req->day_end.tm_mon -= 1;
	time_t t1 = mktime(&req->day_start);
	time_t t2 = mktime(&req->day_end);

	std::vector<struct tm> v;
	
	do
	{
		struct tm *t, tm_tmp;
		t = localtime_r(&t1, &tm_tmp);

		//printf("%d-%d-%d\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday);
		t1+=3600*24;
		t->tm_year -= 100;
		t->tm_mon += 1;
		if(HaveRecordFile(*t))
		{
			printf("put %d-%d-%d\n", t->tm_year+2000, t->tm_mon, t->tm_mday);
			v.push_back(*t);
		}

	} while(t1 < t2 + 3600*24);

	Cmd_Header		header;

	memcpy(&header, data, sizeof(header));
	header.length = v.size()*sizeof(struct tm);
	TcpBlockSend( m_sessionHandle , (char*)&header , sizeof( header ) ) ;
	if(v.size())
		TcpBlockSend( m_sessionHandle , (char*)&v[0] , header.length ) ;
	return false;
}


//===================================================
//播放录像文件
bool CNetUser::onPlaybackPlay( char *data , int len )
{
	NDB( "onPlaybackPlay() len = %d \r\n" , len ) ;

    STRUCT_PLAYBACK_REQUEST *pReq = (STRUCT_PLAYBACK_REQUEST *)data ;
    int namelen = strlen( pReq->playItem.d_name ) ;
    int echo = 0 ;

	int recid = INVALID_SESSION ;

    do
    {
		if (0 == GetDeviceSwitchStatus()) 
		{ 
			echo = 7; // 设备关闭 
			break; 
		}    
		//验证文件名长度
        if( namelen < 11 || namelen >= 96 )
        {
			echo = 1 ;
			break ;
		}

		//系统是否在升级
		//if( startupdate == 1 ){
        if( GetNetServerObj()->IsUpdating() )
        {
			echo = 1 ;
			break ;
		}

		//是否能够获取到有效的录像会话id
		recid = GetMediaMgr()->getUnuseMediaSession( m_Channel , RECORD_CHN_TYPE ) ;
        if( recid == INVALID_SESSION )
        {
			echo = 1 ;
			break ;
		}
		
	//	printf("download file = %s\r\n" , pReq->playItem.d_name ) ;
		//文件是否存在或者有读权限
		m_RecordFp = fopen( pReq->playItem.d_name , "rb" ) ;
        if( !m_RecordFp )
        {
			echo = 1;
			break ;
		}
		
		memcpy( &m_RecordItem , &pReq->playItem , sizeof( pReq->playItem ) ) ;
		//开始发送录像数据
		m_bStartSendRecord = 1 ;
		echo = 0 ;
		
	}
	while( 0 );
	
	STRUCT_PLAYBACK_ECHO resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

	resp.echo = echo ;
	printf("=======================resp.echo:%d\n",resp.echo);
	int nRet = TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
    if( nRet != sizeof( resp ) )
    {
		if( m_RecordFp )
			fclose( m_RecordFp ) ;
		
		if( INVALID_SESSION != recid )
			GetMediaMgr()->freeMediaSession( m_Channel , RECORD_CHN_TYPE , recid ) ;
		return false ;
	}
	//请求失败则返回
    if( echo ) 											
    {
		if( m_RecordFp )
			fclose( m_RecordFp ) ;

		if( INVALID_SESSION != recid )
			GetMediaMgr()->freeMediaSession( m_Channel , RECORD_CHN_TYPE , recid ) ;

		return false ;
	}

	//成功则开始发送数据
	if( 0 == m_RecordItem.p_mode )
		SendRecordPlay( ) ;
	else
		SendRecordFile( ) ;

	if( m_RecordFp )
		fclose( m_RecordFp ) ;


	if( INVALID_SESSION != recid )
		GetMediaMgr()->freeMediaSession( m_Channel , RECORD_CHN_TYPE , recid ) ;

	return false ;
}

//===================================================
//停止播放录像文件
bool CNetUser::onPlaybackStop( char *data , int len )
{
	NDB( "onPlaybackStop() len = %d \r\n" , len ) ;

	return true ;
}

//===================================================
//请求登陆
bool CNetUser::onReqLogin( char *data , int len )
{
	NDB( "onReqLogin() len = %d \r\n" , len ) ;

	STRUCT_REQ_LOGIN_REQUEST* pReq = (STRUCT_REQ_LOGIN_REQUEST*)data ;

	char userName[USER_NAME_LEN+1];
	char userPwd[USER_PASS_LEN+1];

	//拷贝用户名密码，并对数据长度进行保护
	memcpy(&userName , pReq->name , USER_NAME_LEN);
	userName[USER_NAME_LEN] = 0 ;
	memcpy(&userPwd , pReq->pwd , USER_PASS_LEN);
	userPwd[USER_PASS_LEN] = 0 ;
	
	//用户名为空，直接断链接
    if(strcmp("", userName) == 0)
    {
		return false ;
	}

	int echoCode = 0;
	int permit = 0;

	echoCode = GetNetServerObj()->VerifyUser( userName , userPwd , permit ) ;

    if( echoCode )
    {
		ZmdDeCrypt( userName , (char*)CRYPT_KEY ) ;
		ZmdDeCrypt( userPwd , (char*)CRYPT_KEY ) ;
		echoCode = GetNetServerObj()->VerifyUser( userName , userPwd , permit ) ;
	}

	//组回应包
	STRUCT_REQ_LOGIN_ECHO resp ;
	memcpy( &resp.header , data , sizeof( Cmd_Header ) ) ;
	memcpy( &resp.echo , &echoCode , sizeof( echoCode )) ;
	memcpy( &resp.permit , &permit , sizeof( permit ) ) ;

	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	return true ;
}

//===================================================
//通道摄像头参数回复默认设置
bool CNetUser::onResetChnAna( char *data , int len )
{
	NDB( "onResetChnAna() len = %d \r\n" , len ) ;

    STRUCT_RESET_CHNCOLOR_REQUEST *pReq = (STRUCT_RESET_CHNCOLOR_REQUEST *)data ;
    int channel = pReq->header.channel ;

    //不处理
    if( channel < 0 || channel >= IPC_CLIENT_COUNT )
    {
		ERR( "onResetChnAna() error !!!channel = %d\r\n" , pReq->header.channel ) ;
        return true ;
    }

	STRUCT_RESET_CHNCOLOR_RESPONSE resp ;
	CAMERA_ANALOG chnSet ;

	memset(&chnSet, 0, sizeof(CAMERA_ANALOG));
	g_cParaManage->GetSysParameter( SYSANALOG_SET,&chnSet ); 

	chnSet.m_Channels[channel].m_nBrightness = Def_Brightness;
	chnSet.m_Channels[channel].m_nContrast = Def_Contrast;
	chnSet.m_Channels[channel].m_nHue = Def_Hue;
	chnSet.m_Channels[channel].m_nSaturation = Def_Saturation;


#ifdef DM368		
	Crtl_ResetDefCamPara(channel,&chnSet);
#endif
	g_cParaManage->SetSystemParameter(SYSANALOG_SET,&chnSet);
	//赋值给回复信令
	memcpy(&resp.chnSet , &chnSet.m_Channels[channel] , sizeof( ANALOG_CHANNEL ) ) ;



#if (defined APP3518) 
	SetVideoChnAnaLog(
		chnSet.m_Channels[channel].m_nBrightness,
		chnSet.m_Channels[channel].m_nContrast,
		chnSet.m_Channels[channel].m_nSaturation);

#endif
	
	memcpy( &resp.header , &pReq->header , sizeof(Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
	resp.echo = 0 ;

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

#ifdef APP3511
	#ifdef IPCVGA
		Setov7725Reg(DC_SET_BRIGHT,chnSet.m_Channels[channel].m_nBrightness);
		Setov7725Reg(DC_SET_CONTRACT,chnSet.m_Channels[channel].m_nContrast);
		Setov7725Reg(DC_SET_SATURATION,chnSet.m_Channels[channel].m_nSaturation);
	#endif
	
	#ifdef IPC720P
		NDB(" default m_nBrightness, %s, %d\n", __FILE__, __LINE__);
		SetMt9d131Reg(MT_DC_SET_BRIGHT,chnSet.m_Channels[channel].m_nBrightness);
	#endif
#endif

	return true ;
}
//===================================================
//设置通道摄像头参数
bool CNetUser::onSetChnAnaLog( char *data , int len )
{
	NDB( "onSetChnAnaLog() len = %d \r\n" , len ) ;

    STRUCT_SET_CHNANALOG_REQUEST *pReq = (STRUCT_SET_CHNANALOG_REQUEST *)data ;
    int channel = pReq->header.channel ;	
	//不处理,但是也不断连接
	if( channel < 0 || channel >= IPC_CLIENT_COUNT )
	{
		ERR( "onSetChnAnaLog() error !!!channel = %d\r\n" , pReq->header.channel ) ;
		return true ;
	}

	ANALOG_CHANNEL lchAna;
	CAMERA_ANALOG  lchaPara;
	
	//亮度大于250时有闪烁现象
	//if( pReq->chnSet.m_nBrightness > 230 )
	//	pReq->chnSet.m_nBrightness = 230 ;
	#ifdef DM368	
    AnalogParaSet(pReq->chnSet);
	#endif
	
	g_cParaManage->GetSysParameter(SYSANALOG_SET,&lchaPara); 

	if(lchaPara.m_Channels[channel].m_nBrightness != pReq->chnSet.m_nBrightness)
	{
		lchaPara.m_Channels[channel].m_nBrightness = pReq->chnSet.m_nBrightness;
#ifdef APP3511
		#ifdef IPCVGA		
		Setov7725Reg(DC_SET_BRIGHT,lchaPara.m_Channels[channel].m_nBrightness);
		#endif
		#ifdef IPC720P
		
		NDB("bright %d, %s, %d r\n", lchaPara.m_Channels[channel].m_nBrightness, __FILE__, __LINE__);
		SetMt9d131Reg(MT_DC_SET_BRIGHT,lchaPara.m_Channels[channel].m_nBrightness);
		#endif
#endif

#if (defined APP3518) 	
			SetVideoChnAnaLog(
			pReq->chnSet.m_nBrightness,
			pReq->chnSet.m_nContrast,
			pReq->chnSet.m_nSaturation);
#endif
	}

	
	if(lchaPara.m_Channels[channel].m_nContrast != pReq->chnSet.m_nContrast)
	{
		lchaPara.m_Channels[channel].m_nContrast = pReq->chnSet.m_nContrast ;
#ifdef APP3511
		#ifdef IPCVGA
		Setov7725Reg(DC_SET_CONTRACT,lchaPara.m_Channels[channel].m_nContrast);
		#endif
#endif
#if (defined APP3518)
				SetVideoChnAnaLog(
				pReq->chnSet.m_nBrightness,
				pReq->chnSet.m_nContrast,
				pReq->chnSet.m_nSaturation);
#endif
	}
	if(lchaPara.m_Channels[channel].m_nHue != pReq->chnSet.m_nHue)
	{
		lchaPara.m_Channels[channel].m_nHue = pReq->chnSet.m_nHue;
	}
	if(lchaPara.m_Channels[channel].m_nSaturation != pReq->chnSet.m_nSaturation)
	{
		lchaPara.m_Channels[channel].m_nSaturation = pReq->chnSet.m_nSaturation;
#ifdef APP3511
		#ifdef IPCVGA
				Setov7725Reg(DC_SET_SATURATION,lchaPara.m_Channels[channel].m_nSaturation);
		#endif
#endif

#if (defined APP3518) 
	SetVideoChnAnaLog(
	pReq->chnSet.m_nBrightness,
	pReq->chnSet.m_nContrast,
	pReq->chnSet.m_nSaturation);
#endif
	}
	printf("%d%d%d\n",pReq->chnSet.m_nBrightness,pReq->chnSet.m_nContrast,pReq->chnSet.m_nSaturation);
	//printf("kb need to know:%d%d%d\n",lchaPara.m_Channels[channel].m_nBrightness,lchaPara.m_Channels[channel].m_nContrast,lchaPara.m_Channels[channel].m_nSaturation);
	g_cParaManage->SetSystemParameter(SYSANALOG_SET,&lchaPara);
	
	return true ;
}
//===================================================
//获取通道摄像头参数
bool CNetUser::onGetChnAnaLog( char *data , int len )
{
	NDB( "onGetChnAnaLog() len = %d \r\n" , len ) ;

    STRUCT_GET_CHNANALOG_REQUEST *pReq = (STRUCT_GET_CHNANALOG_REQUEST *)data ;
    int channel = pReq->header.channel ;

	//不处理,但是也不断连接
    if( channel < 0 || channel >= IPC_CLIENT_COUNT )
    {
		ERR( "onGetChnAnaLog() error !!!channel = %d\r\n" , pReq->header.channel ) ;
		return true ;
	}

	STRUCT_GET_CHNANALOG_ECHO resp ;
	CAMERA_ANALOG  lchaPara;

	g_cParaManage->GetSysParameter(SYSANALOG_SET,&lchaPara); 
	// 发送ACK
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ));
	resp.header.length =sizeof(resp ) - sizeof( Cmd_Header );
	memcpy( &resp.chnSet , &lchaPara.m_Channels[channel] , sizeof( ANALOG_CHANNEL ) ) ;
	resp.echo = 0 ;
	
	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
	
	
	return true ;
}
//===================================================
//获取wifi ap列表
bool CNetUser::onGetWifiAP( char *data , int len )
{
	#ifdef SUPPORT_WIFI
	STRUCT_GET_WIFIAP_REQUEST* pReq = (STRUCT_GET_WIFIAP_REQUEST*)data ;
	char sendBuf[8192]={0x0} ;
	STRUCT_GET_WIFIAP_ECHO* pResp = (STRUCT_GET_WIFIAP_ECHO*)sendBuf ;
	
	char*	pCursor = sendBuf + sizeof( STRUCT_GET_WIFIAP_ECHO	) ;
	int 	buflen = sizeof( sendBuf ) - sizeof( STRUCT_GET_WIFIAP_ECHO  ) ;
	unsigned int	apCount = 0 ;

	GetAP_Lists(sendBuf,&apCount);
	
	memcpy( &pResp->header , &pReq->header , sizeof( Cmd_Header ) ) ;
	pResp->header.length = apCount * sizeof( TYPE_WIFI_LOGIN ) ;
	buflen = pResp->header.length + sizeof( Cmd_Header ) ;
	
    NDB("sizeof( TYPE_WIFI_LOGIN )=[%d]\n",sizeof( TYPE_WIFI_LOGIN ));
	NDB("sendBuf:apcount=%d\n",apCount);
	NDB("sendBuf:pResp->header.length =%d\n",pResp->header.length);
	NDB("sendBuf:buflen=%d\n",buflen);
		
	TcpBlockSend( m_sessionHandle , sendBuf , buflen ) ;
	#endif
	
	return true ;
}

//===================================================
//设置所要连接的wifi
bool CNetUser::onSetWifiConnect( char *data , int len )
{
	#ifdef SUPPORT_WIFI
    STRUCT_SET_WIFICONNECT_REQUEST *pReq = (STRUCT_SET_WIFICONNECT_REQUEST *)data ;
	STRUCT_SET_WIFICONNECT_ECHO resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

	TYPE_WIFI_LOGIN wifilogin;
	int	echocmd = -1;//-1--未连接；0--已连接
	
	memset(&wifilogin,0x0,sizeof(TYPE_WIFI_LOGIN));
	memcpy(&wifilogin,&pReq->wifiAP,sizeof(TYPE_WIFI_LOGIN));
	//解密
	ZmdDeCrypt( wifilogin.Passwd , (char*)CRYPT_KEY ) ;
	NDB(" ------- %s  ------ \n",__FUNCTION__);
	NDB("ivan:%s wifilogin.RouteDeviceName ::%s\n",__FUNCTION__,wifilogin.RouteDeviceName);	
	NDB("ivan:%s wifilogin.Passwd ::%s\n",__FUNCTION__,wifilogin.Passwd);
	NDB("ivan:%s wifilogin.EncryptionProtocol ::%d\n",__FUNCTION__,wifilogin.EncryptionProtocol);

	echocmd = StartConnectWifi(&wifilogin);
	
		
RESP:
	resp.echo = echocmd ; 
	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof(resp ) ) ;
	
	#endif
	return true ;
}

//===================================================
//获取wife连接状态
bool CNetUser::onGetWifiStatus( char *data , int len )
{
	#ifdef SUPPORT_WIFI
    STRUCT_GET_WIFISTATUS_REQUEST *pReq = (STRUCT_GET_WIFISTATUS_REQUEST *)data ;
    STRUCT_GET_WIFISTATUS_ECHO resp ;

	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
	
	//pReq->wifiAP.BackupFlag = 0x01 ;
	
	int res = GetWIFI_Status( &pReq->wifiAP );
	if(!res)
	{
		
	}
	memcpy(&resp.wifiAP , &pReq->wifiAP, sizeof(TYPE_WIFI_LOGIN));

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof(resp ) ) ;
	#endif
	
	return true ;
}

//===================================================
//设置图像参数
bool CNetUser::onSetPicInfo( char *data , int len )
{
	NDB( "onSetPicInfo() len = %d \r\n" , len ) ;

    STRUCT_SET_PIC_REQUEST *pReq = (STRUCT_SET_PIC_REQUEST *)data ;

#ifdef IPC720P
	onSet720PPicInfo( data , len ) ;
#else
	onSetVgaPicInfo( data , len ) ;
#endif
	return true ;
}

//===================================================
//若是VGA编译模式，设置VGA设备图像参数
bool CNetUser::onSetVgaPicInfo( char *data , int len )
{
	STRUCT_SET_PIC_REQUEST* pReq = (STRUCT_SET_PIC_REQUEST*)data ;

	NDB( "onSetVgaPicInfo() len = %d \r\n" , len ) ;
	

	return true ;
}
//===================================================
//若是720p编译模式，设置720p图像模式
bool CNetUser::onSet720PPicInfo( char *data , int len )
{
    STRUCT_SET_PIC_REQUEST *pReq = (STRUCT_SET_PIC_REQUEST *)data ;

    NDB( "onSet720PPicInfo() len = %d commd=== %04x\r\n" , len , pReq->header.commd) ;

	#ifdef DM368
	 Crtl_CamPara(pReq);
	 return true ;
	#endif
	
	Web_Sync_MirrorFilp_Value(pReq->header.commd);
	return SetIPCMirrorFilp( pReq->header.commd);

}

//==========================================
// nvr通知报警监听端口
bool CNetUser::onNvrPort( char *data , int len )
{
	STRUCT_NVR_PORT_REQUEST* pReq = (STRUCT_NVR_PORT_REQUEST*)data ;
	
	m_alarmID = GetNetServerObj()->getUnuseAlarmSession( m_nUserID , m_remoteIP , htons(pReq->alarmListenPort) , pReq->devType ) ;

    return true ;
}

//============================================
//读取视频编码信息
bool CNetUser::onReadVideoCoderParam( char *data , int len )
{
	NDB(" onReadVideoCoderParam() \r\n" ) ;
	STRUCT_READ_VIDEOPARAM_REQUEST* pReq = (STRUCT_READ_VIDEOPARAM_REQUEST*)data ;
	
	STRUCT_READ_VIDEOPARAM_ECHO resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header )  ;

	memcpy( &resp.param , &(g_cParaManage->m_syspara->m_Camera),sizeof(CAMERA_PARA));

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
	
	return true ;
}

bool CNetUser::onGetPreset( char *data , int len )
{
	if(ConfigInfo.SupportInfo&(CONFIG_PT) || ConfigInfo.SupportInfo&(CONFIG_ZOOM))
	{
		int readLen = 0 ;						
		char readbuf[514] = {0x0};					
		STRUCT_GET_PRESET_REQUEST *pReq = (STRUCT_GET_PRESET_REQUEST *)data ;

		STRUCT_GET_PRESET_ECHO resp ;
		memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
		resp.header.length = 0 ; 

		memset( resp.presetData , 0 , 514 );			

		if(GetPTZResetPoint(readbuf, &readLen) != -1)			
		{
			if((readLen<=514)&&(readLen>0))
			{
				memcpy( resp.presetData , readbuf , readLen ) ; //xxx个字节预置点信息+1字节停留时间+1字节滑竿速度
				resp.header.length = readLen;
			}
			else
			{
				printf("(%s|%d):read data error:%d\n",__func__ ,__LINE__,readLen);
			}
		}	
		// 不发整个结构体, 只发有预置点信息的buffer
		TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof(Cmd_Header)+readLen ) ;
	}
	return true ;
}

//===============================================
//获取快照
bool CNetUser::onGetSnapshot( char *data , int len )
{
	return false;
}

//===============================================
//获取温湿度传感器信息
bool CNetUser::onGetSensorAlarmInfo( char* data , int len )
{
	NDB( "onGetSensorAlarmInfo\r\n" ) ;
	
	STRUCT_GET_SENSORALARMINFO_REQUEST* pReq = (STRUCT_GET_SENSORALARMINFO_REQUEST*)data ;

	STRUCT_GET_SENSORALARMINFO_ECHO resp ;
	memset( &resp , 0 , sizeof( resp ) ) ;
	//水稻育苗项目定义2个传感器
	STRUCT_SENSOR_CURINFO sensor[3] ;
	memset( &sensor , 0 , sizeof( sensor ) ) ;

	int echo = 0 ;

    do
    {
		int index = 0 ;
		sensor[index].sensorIndex = index ;
		index = 1 ;
		sensor[index].sensorIndex = index ;
		index = 2 ;
		sensor[index].sensorIndex = index ;
		resp.sensorCount = 3 ;

		/*
		if( !GetSensorAlarmInfo( resp.minTem , resp.maxTem , resp.minHum , resp.maxHum , resp.sampleRate )){
			echo = 1 ;
			break ;
		}
		//获取第一个传感器信息
		
		sensor[index].sensorIndex = index ;
		if( !GetSensorCurrentValue( index , sensor[index].curTem , sensor[index].curHum ) ){
			echo = 2 ;
			break ;
		}
		
		//获取第二个传感器信息
		index = 1 ;
		sensor[index].sensorIndex = index ;
		if( !GetSensorCurrentValue( index , sensor[index].curTem , sensor[index].curHum ) ){
			echo = 2 ;
			break ;
		}
		*/
    }
    while( 0 ) ;
	
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) );
	resp.echo = echo ;

	//是否出错
    if( echo )
    {
		resp.header.length = sizeof( resp ) - sizeof(Cmd_Header) ;
		//net_send( net_attr->s_fd , (unsigned char*)&resp , sizeof( resp ) ) ;
		TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
	}
    else
    {
		resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) + sizeof( STRUCT_SENSOR_CURINFO ) * 3 ;
		//net_send( net_attr->s_fd , (unsigned char*)&resp , sizeof( resp ) ) ;
		TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
		TcpBlockSend( m_sessionHandle , (char*)&sensor , sizeof( STRUCT_SENSOR_CURINFO )*3  ) ;
	}

	return true ;
}

//===============================================
//设置传感器信息
bool CNetUser::onSetSensorAlarmInfo( char* data , int len )
{

	NDB( "onSetSensorAlarmInfo\r\n" ) ;
	
	//设置信息
	STRUCT_SET_SENSORALARMINFO_REQUEST* pReq = (STRUCT_SET_SENSORALARMINFO_REQUEST*)data ;
	
	printf( "%f , %f , %f , %f , %d \r\n" , pReq->temperature_min , pReq->temperature_max , 
		pReq->humidity_min , pReq->humidity_max , pReq->sampleRate ) ;
	
	//调用系统功能，设置报警
	int echo = 0 ;
	//echo = SetAlarmInfo( pReq->temperature_min , pReq->temperature_max , 
	//	pReq->humidity_min , pReq->humidity_max , pReq->sampleRate ) ;
	
	//发送回应
	STRUCT_SET_SENSORALARMINFO_ECHO resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
	
	resp.echo = echo ;

	//net_send( net_attr->s_fd , (unsigned char*)&resp , sizeof( resp ) ) ;
	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	return true ;
}

//=======================================
//发送直播数据
bool CNetUser::SendLivingData(  )
{
    unsigned char *data = 0 ;
    FrameInfo info ;
    int datalen = 0 ;
    
    do
    {
		int getRet = 0 ;
		int streamtpye = 0;
		switch(m_mediaCmd )
		{
			case CMD_START_VIDEO :
				streamtpye = 1 ; 
				break ;
			case CMD_START_SUBVIDEO :
				streamtpye = 2 ; 
				break ;
			case CMD_START_720P :
				streamtpye = 0 ;
				break ;
			default :
				m_Channel = 0;
				break ;
		}
		getRet = GetSendNetFrame(m_Channel, streamtpye, m_mediaSid , &data , &info ) ;
		if(getRet == -2)/*返回值-2 表示需要跳帧*/
			continue;

		if( getRet!= -1 && data && info.FrmLength > 0 )
		{
			if( info.FrmLength >= TCP_RECV_BUF_SIZE )
				return true ;
			
			//如果不允许发送音频,则跳过音频帧
			
			#ifdef SUPPORT_AUDIO
			if( 0 == m_bSendAudio )
			{
					if( info.Flag == 3 )
					continue ;
			}																
			#endif

			memcpy( m_FrameBuf , data , info.FrmLength ) ;
			
			//write_data_to_file(m_FrameBuf, info.FrmLength);
			int nSend = TcpBlockSend( m_sessionHandle , (char *)m_FrameBuf , info.FrmLength ) ;
			//发送失败,返回false,通知底层
            if( nSend <= 0 )
            {
				printf( "TcpBlockSend m_nUserID= %d nSend = %d , info.FrmLength = %d\r\n" , m_nUserID , nSend , info.FrmLength ) ;
				//
				return false ;
			}
		}
        else
        {
			break ;
		}

    }
    while( data );
	
	return true ;
}

extern int ChangeClientOffset(char* FileName, 
	unsigned int Startime, 
	unsigned int Endtime, 
	unsigned int* FilePos, 
	unsigned int* FileLen);

//=======================================
//发送录像数据
bool CNetUser::SendRecordPlay( )
{
	unsigned long  dataoffset = 0;
	char buffer[1440*2];
	int  length; 
	unsigned int FileLen = 0;
	const int	FileTopLen = 512;
	VideoFileInfo	filetop;
	
	//m_nSendPos = m_RecordItem.p_offset;// 偏移量
	
	//更录像模块所添加的新接口
#if 1
	ChangeClientOffset( m_RecordItem.d_name, 
						m_RecordItem.start_time, 
						m_RecordItem.end_time,
						&m_nSendPos,
						&FileLen );
#else
	m_nSendPos = FileTopLen;
#endif
	fseek(m_RecordFp , 0, SEEK_SET);

	int nRet = fread( &filetop , 1 , FileTopLen , m_RecordFp );
	if( nRet != FileTopLen )
		return false ;

	printf("m_RecordItem.start_time:%d\nm_RecordItem.p_offset:%d\n",
		m_RecordItem.start_time, m_RecordItem.p_offset);
	fseek(m_RecordFp,0, SEEK_END);
	FileLen = ftell(m_RecordFp);
	fseek(m_RecordFp,0, SEEK_SET);

	filetop.m_MovieOffset = 0;
	//发送文件头
	nRet = TcpBlockSend( m_sessionHandle , (char*)&filetop, FileTopLen );
	if( nRet != FileTopLen )
		return false ;
	
	dataoffset = m_nSendPos+m_RecordItem.p_offset;

	printf("seek %d bytes\n", dataoffset);
	fseek(m_RecordFp, dataoffset, SEEK_SET);
	unsigned int n = 0;

	while( !feof(m_RecordFp) )
	{	
		//如果正在升级
		if( GetNetServerObj()->IsUpdating() ){
			StopTcpSession( m_sessionHandle ) ;
			return false ;
		}
		
		n++;

		length = fread(buffer, 1, 1440*2, m_RecordFp);
		
		if( length <= 0 ){
			printf( "fread() failed ret = %d\r\n" ) ;
			return false ;
		}

		nRet = TcpBlockSend( m_sessionHandle , buffer , length );
		if( nRet != length ){
			printf( "TcpBlockSend() failed ret = %d\r\n" ) ;
			return false ;
		}
		
		//=======================================================
		//避免发送太快，IE控件会崩溃，每4块数据，释放一下cpu占用
		if( n%3 == 0 ){
			RELEASE_CPU( 3 ) ;
		}
	}

	return true ;
}


//=======================================
//发送录像文件
bool CNetUser::SendRecordFile( )
{
	int nFileLen = 0 ;
	char buffer[2048*2];
	int nLeft = 0 ;	//剩余数据
	int length = 0 ;
	int nDataCount = 0 ;
	int nRet = 0 ;
	
	m_nSendPos = m_RecordItem.p_offset ;// 偏移量

	fseek(m_RecordFp , 0, SEEK_END );
	nFileLen = ftell( m_RecordFp );

	if( m_nSendPos >= nFileLen )
		return false ;

	nLeft = nFileLen - m_nSendPos ;
	
	NDB("SendRecordFile() m_nSendPos = %d\r\n" , m_nSendPos ) ; 

	//偏移到指定位置
	fseek( m_RecordFp , m_nSendPos , SEEK_SET ) ;

	while( nLeft > 0 )
	{   
		//正在升级，拒绝连接
        if( GetNetServerObj()->IsUpdating() )
        {
			StopTcpSession( m_sessionHandle ) ;
			return false ;
		}

	    nDataCount++;

		if( nLeft > 1440*2 )
			length = fread(buffer, 1, 1440*2, m_RecordFp);
		else
			length = fread(buffer, 1, nLeft , m_RecordFp);
		
		if( length > 0 )
		{
			nRet = TcpBlockSend( m_sessionHandle , buffer , length );
            if( nRet != length )
            {
				ERR("TcpBlockSend ret =%d,length=%d , send failed .\r\n" , nRet , length ) ;
				return false ;
			}

			nLeft -= length ;

			//=======================================================
			//避免发送太快，IE控件会崩溃，每4块数据，释放一下cpu占用
		if( nDataCount % 3 == 0 )
            {
				RELEASE_CPU( 3 ) ;
			}

		}
        else
        {
			ERR( "read file failed !!!\r\n" ) ;
			break ;
		}
	}
	
	
	return true ;
}

int CNetUser::CheckWifiSaveParaMeter(TYPE_WIFI_LOGIN  wifilogin)
{
	SYSTEM_PARAMETER     m_SysParameter;
	int retflag = -1;
	
	g_cParaManage->LoadParameterFromFile(&m_SysParameter);  //取得保存的参数
	
	if (strncmp(&wifilogin.RouteDeviceName[0],&m_SysParameter.m_NetWork.m_WifiConfig.LoginWifiDev.RouteDeviceName[0],32) == 0)
	{
		if (strncmp(&wifilogin.Passwd[0],&m_SysParameter.m_NetWork.m_WifiConfig.LoginWifiDev.Passwd[0],32) == 0)
		{
			if (wifilogin.EncryptionProtocol  ==  m_SysParameter.m_NetWork.m_WifiConfig.LoginWifiDev.EncryptionProtocol)
			{
				retflag = 0;
			}
			else
			{
				retflag = -1;
			}
		}
		else 
		{
			retflag = -1;
		}
	}
	else
	{
		retflag = -1;
	}
	return retflag;
}

bool CNetUser::onSetVideoCode( char* data , int len )
{
	NDB( "onSetVideoCode\r\n" ) ;

	STRUCT_SET_VIDEO_CODE_REQUEST* pReq = (STRUCT_SET_VIDEO_CODE_REQUEST*)data ;
	
	CAMERA_PARA  localPara ;
	
	STRUCT_SET_VIDEO_CODE_ECHO resp ;
	int	echocmd = 0;

	//if(len <= sizeof(STRUCT_SET_VIDEO_CODE_REQUEST)) 
	//	return false ;

	memset(&localPara,0,sizeof(CAMERA_PARA));

	// 发送ACK
	echocmd = 0; // CMD_S_DEV_PARA
	memcpy(&resp.header , &pReq->header , sizeof( Cmd_Header ));
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
	resp.echo = 0 ;

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	if( csSetCameraVideoCodePara(&localPara, &pReq->param ) > 0)
	{
		NDB("=======set video code end!=======\n");
	}

	return true ;
}

bool CNetUser::onGetDeviceID( char* data , int len )
{
	STRUCT_GET_DEVICEID_REQUEST* pReq = (STRUCT_GET_DEVICEID_REQUEST*)data ;
	STRUCT_GET_DEVICEID_ECHO resp ;
	NET_WORK_CARD CardID;

	NETWORK_PARA netset;
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
	
	memcpy(resp.deviceID , netset.m_CenterNet.deviceid ,15 );
	
	CardID = get_network_support();
	
	if(CardID == NET_WORK_CARD_WIFI)
	{
		#ifdef SUPPORT_WIFI
			GetWifiMac((unsigned char *)resp.mac);
		#endif
	}
	else
	{
		memcpy( resp.mac , netset.m_Eth0Config.m_uMac , 6) ;
	}
	
	NDB( "onGetDeviceID = %s\r\n" , resp.deviceID ) ;
	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header));
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header )  ;
	
	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
	
	return true ;
}
//==================================================
//设置设备id
bool CNetUser::onSetDeviceID( char *data , int len )
{
    STRUCT_SET_DEVICEID_REQUEST *pReq = (STRUCT_SET_DEVICEID_REQUEST *)data ;
    STRUCT_SET_DEVICEID_ECHO resp ;

 	NETWORK_PARA netset;

    if( len < 10 )
        return 0 ;

    //设置mac地址
    int fd = -1;
    int echo = 0;
    int retval = 0;
    g_cParaManage->GetSysParameter(SYSNET_SET, &netset);
    memcpy( netset.m_Eth0Config.m_uMac , pReq->mac , 6 ) ;
    memcpy( netset.m_CenterNet.deviceid  , pReq->deviceID  , 15 );
	
    char cmd[64] = {0};
	sprintf(cmd, "rm %s", MAC_ID_FILE);
    system(cmd);
    fd = open(MAC_ID_FILE,  O_CREAT | O_RDWR, 0777);
	if(fd < 0)
	{
		ERR(" open deviceid failure \n");
		echo = -1;
	}
	
	retval = write(fd, pReq->mac , 6 ) ;
	if(retval <= 0)
	{
        ERR(" write MAC_ADDR_FILE file failure  \n");
		echo = -1;
	}

	//写入设备id
	retval = write( fd , pReq->deviceID , 15  );
	if(retval <= 0)
	{
        ERR(" write MAC_ADDR_FILE file failure  \n");
		echo = -1;
	}
	fsync(fd);
	close(fd);
	fd = -1;
	g_cParaManage->SetSystemParameter(SYSNET_SET,&netset);
	GetNetModule()->SetLoaclNetMac();

    memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header)) ;
    resp.header.length = sizeof(resp) - sizeof( Cmd_Header ) ;
    resp.echo = echo ;

    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;

	NET_WORK_CARD CardID;
	CardID = get_network_support();
	
	if(CardID == NET_WORK_CARD_WIFI)
	{
		
		if(pReq->mac != NULL && get_network_support()!=NET_WORK_CARD_LOCAL)
			SetWifiMac((unsigned char *)pReq->mac);
		
		
	}
	
	return true ;
}

bool CNetUser::onGetTalkSetting( char *data , int len )
{
	STRUCT_GET_TALK_SETTING_REQUEST* pReq = (STRUCT_GET_TALK_SETTING_REQUEST*)data ;
	STRUCT_GET_TALK_SETTING_ECHO resp ;

	memset( &resp , 0 , sizeof( resp ) ) ;
	memcpy( &resp.header , &pReq->header , sizeof(Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

#ifdef SUPPORT_AUDIO
	AudioParm adParam ;
	memset( &adParam , 0 , sizeof( adParam ) ) ;

	GetAudioParam( &adParam ) ;
	
	resp.audioParam.audioType = adParam.audiotype ;
	resp.audioParam.enBitwidth = adParam.enBitwidth ; 
	resp.audioParam.sampleRate = adParam.samplerate ;
	resp.audioParam.recordVolume = adParam.inputvolume ;
	resp.audioParam.speakVolume = adParam.outputvolume ;
	resp.audioParam.framelen = adParam.framelen ;
#endif

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	return true ;
}

bool CNetUser::onSetTalkSetting( char* data , int len )
{
	STRUCT_SET_TALK_SETTING_REQUEST* pReq = (STRUCT_SET_TALK_SETTING_REQUEST*)data ;
	STRUCT_SET_TALK_SETTING_ECHO resp ;

	memset( &resp , 0 , sizeof( resp ) ) ;
	memcpy( &resp.header , &pReq->header , sizeof(Cmd_Header ) ) ;
	resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
	
	resp.echo = -1 ;
	
    if( m_bStartTalk )
    {
		NDB( "mic volume = %d , spk volume = %d\r\n" , pReq->micVol , pReq->spkVol) ;
		resp.echo = 0 ;
	}
    else
    {
		ERR( "no right !!mic volume = %d , spk volume = %d\r\n" , pReq->micVol , pReq->spkVol) ;
	}

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	return true ;
}

bool CNetUser::onSetAudioSwitch( char* data , int len )
{
	STRUCT_SET_AUDIO_SWITCH* pReq = (STRUCT_SET_AUDIO_SWITCH*)data ;
	
	m_bSendAudio = pReq->sendAudio ;

//	printf("onSetAudioSwitch() sendAudio = %d\r\n" , m_bSendAudio ) ;

	STRUCT_SET_AUDIO_SWITCH_RESP resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) );
	resp.echo = 0 ;

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	return true ;
}
bool CNetUser::onGetAudioParm( char* data , int len )
{
	#ifdef SUPPORT_AUDIO
	STRUCT_GET_AUDIOPARM_RESPONSE resp ;
	memset(&resp,0x0,sizeof(STRUCT_GET_AUDIOPARM_RESPONSE));
	memcpy( &(resp.header) , data , sizeof( Cmd_Header ) );
	
	
	AudioParm adParam ;
	memset( &adParam , 0 , sizeof( adParam ) ) ;	
	GetAudioParam( &adParam ) ;
	resp.param.sampleRate = adParam.samplerate ;
	resp.param.audioType = adParam.audiotype ;
	resp.param.enBitwidth = adParam.enBitwidth ; 	
	resp.param.recordVolume = adParam.inputvolume ;
	resp.param.speakVolume = adParam.outputvolume ;
	resp.param.framelen = adParam.framelen ;
	resp.result = 0 ;
	resp.header.length =sizeof( STRUCT_GET_AUDIOPARM_RESPONSE )-sizeof( Cmd_Header );
	
	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;
	#endif
	
	return true ;
}

bool CNetUser::onSetWifiLED( char* data , int len )
{
	STRUCT_SET_WIFILED *pReq = (STRUCT_SET_WIFILED*)data ;	
	NDB("onSetWifiLED() status = %d\r\n" , pReq->status ) ;
	
	STRUCT_SET_WIFILED_RESPONSE resp ;
	memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) );	


	NETWORK_PARA		netset;
	memset(&netset,0x0,sizeof(NETWORK_PARA));
	//获取网络配置
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
	if(pReq->status)
	{
		netset.m_WifiConfig.LoginWifiDev.m_Reserved =(netset.m_WifiConfig.LoginWifiDev.m_Reserved )|0x01;
	}
	else
	{
		netset.m_WifiConfig.LoginWifiDev.m_Reserved =(netset.m_WifiConfig.LoginWifiDev.m_Reserved )&0xfe;
	}
	resp.result = 0 ;
	resp.header.length =sizeof(int);
	g_cParaManage->SetSystemParameter(SYSNET_SET,&netset);

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	return true ;
}
bool CNetUser::onGetWifiLED( char* data , int len )
{
	STRUCT_GET_WIFILED_RESPONSE resp ;
	memcpy( &resp.header , data , sizeof( Cmd_Header ) );
	

	NETWORK_PARA		netset;
	memset(&netset,0x0,sizeof(NETWORK_PARA));
	//获取网络配置
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);

	resp.status = (netset.m_WifiConfig.LoginWifiDev.m_Reserved )&0x01;
	resp.result = 0 ;
	resp.header.length =2*sizeof(int);

	TcpBlockSend( m_sessionHandle , (char*)&resp , sizeof( resp ) ) ;

	return true ;
}

bool CNetUser::onSetIframe(char* data , int len)
{
	if(data == NULL || len <=0 || len >sizeof(STRUCT_FORCE_I_FRAME_REQUEST))
		return false;
	int ret = 0;
	int result = 0;
	
	STRUCT_FORCE_I_FRAME_REQUEST* pReq = (STRUCT_FORCE_I_FRAME_REQUEST*)data;
	STRUCT_FORCE_I_FRAME_RESPONSE Resp;

	memset(&Resp, 0, sizeof(STRUCT_FORCE_I_FRAME_RESPONSE));
	
	if(0 < pReq->stream_type || pReq->stream_type < 4)
	{
		//printf("onSetIframe pReq->stream_type:%d\r\n", pReq->stream_type);
		if(pReq->stream_type==0){
			result= RequestIFrame(0, 0);
		}
		else if(pReq->stream_type==1||pReq->stream_type==2){
			 result=RequestIFrame(1, 0);
		}		
		//result = RequestIFrame(pReq->stream_type, 0);
		if(result != 0)
			result = 1;
	}
	else
		result = 1;
	
	Resp.header.channel = 0 ;
	Resp.header.commd = CMD_CODE_I_FRAME ;
	Resp.header.head = 0xaaaa5555;
	Resp.header.type = 0;
	Resp.header.length = sizeof(STRUCT_FORCE_I_FRAME_RESPONSE) - sizeof(Cmd_Header);
	Resp.result = result;
	
	ret = TcpBlockSend(m_sessionHandle , (char*)&Resp , sizeof(STRUCT_FORCE_I_FRAME_RESPONSE));
	if(ret != sizeof(STRUCT_FORCE_I_FRAME_RESPONSE))
	{
		ERR("######### faild to send respones! ##########\r\n ");
	}
	
	return true;
}

int CNetUser::netSetTimezone( char* data , int len )
{
	PARAMETEREXTEND ntp;
	datetime_setting systime;
	TimeZoneInfo* zone = (TimeZoneInfo*)(data+sizeof( Cmd_Header ));
	int ret = 0;
	char sendbuf[256] = {0};
	Cmd_Header resp;

	memset(&systime, 0, sizeof(datetime_setting));
	memset(&resp, 0 , sizeof(Cmd_Header));
	memset(&ntp, 0,sizeof(PARAMETEREXTEND));

	if(zone->m_tzindex < 0 || zone->m_tzindex > 91)
	{
		ERR("The parameter is error!r\n");
		return -1;
	}

	g_cParaManage->GetSysParameter(EXTEND_SET, &ntp);
	
	if(zone->m_tzindex != ntp.m_ntp.m_idx_tzname)
	{
		/*保存时间参数*/
		ntp.m_ntp.m_idx_tzname = zone->m_tzindex;

		/*手动设置，获取时区与UTC秒数差距*/
		if(zone->m_tzindex)
		{
			GetDiffByTzIdx(zone->m_tzindex, &ntp.m_ntp.m_diff_timezone);
		}
	}
	
	if(zone->m_ntp)
	{
		//ret = ntpclient_start(ntp.m_ntp.m_diff_timezone, 24*3600, NULL, ntp.m_ntp.m_idx_tzname == 0 ?1:0);	
		RestartNtpClient(ntp.m_ntp.m_diff_timezone);
		ret = 0;
	}

	if(ret == 0)
	{
		ntp.m_ntp.m_ntp_switch = zone->m_ntp;
		g_cParaManage->SetSystemParameter(EXTEND_SET, &ntp);
	}
	memcpy(&resp, data, sizeof(Cmd_Header));
	resp.length = sizeof(datetime_setting) + sizeof(int);
	memcpy(sendbuf, &resp, sizeof(Cmd_Header));
	memcpy(sendbuf + sizeof(Cmd_Header), &ret, sizeof(int));
	
	GlobalGetSystemTime(&systime);
	memcpy(sendbuf+sizeof(Cmd_Header)+ sizeof(int), &systime, sizeof(datetime_setting));
	ret = TcpBlockSend(m_sessionHandle, sendbuf, sizeof(datetime_setting)+sizeof(Cmd_Header)+sizeof(int));	
	
	return 0;
}

int CNetUser::netGetTimezone( char* data , int len )
{
	PARAMETEREXTEND ntp;
	TimeZoneInfo	zone;
	Cmd_Header resp;
	char sendbuf[256] = {0};
	int result = 0;
	int ret = 0;
	
	memset(&resp, 0 , sizeof( Cmd_Header ));
	memset(&zone, 0, sizeof(TimeZoneInfo));
	memset(&ntp, 0,sizeof(PARAMETEREXTEND));
	
	g_cParaManage->GetSysParameter(EXTEND_SET, &ntp);

	memcpy(&resp,data,sizeof( Cmd_Header ));
	resp.length = sizeof(int) + sizeof(TimeZoneInfo);
	memcpy(sendbuf, &resp, sizeof( Cmd_Header ));
	memcpy(sendbuf + sizeof( Cmd_Header ), &result, sizeof(int));
	
	zone.m_ntp = ntp.m_ntp.m_ntp_switch;
	zone.m_timezone = ntp.m_ntp.m_diff_timezone;
	zone.m_tzindex = ntp.m_ntp.m_idx_tzname;
	memcpy(sendbuf + sizeof( Cmd_Header ) + sizeof(int), &zone, sizeof(TimeZoneInfo));
	ret = TcpBlockSend(m_sessionHandle, sendbuf, sizeof( Cmd_Header ) + sizeof(int) + sizeof(TimeZoneInfo));

	if(ret)
		return 0;
	else
		return -1;
}

bool CNetUser::onSetDevConfig( char* data , int len )
{
	STRUCT_SET_DEVICE_CONFIG_REQUEST* pReq = (STRUCT_SET_DEVICE_CONFIG_REQUEST*)data;
	int ret = -1;
	STRUCT_SET_DEVICE_CONFIG_RESPONSE resp ;
	
	ret= UpdateDeviceConfigInfo((void*)&pReq->ConfigInfo);

	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header)) ;
    resp.ret = ret ;
    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;
	
	return true;
}

bool CNetUser::onCheckAppUpdateInfo( char* data, int len )
{
	int descrip_len = 0; /* 新版本描述信息的长度*/
	
	STRUCT_CHECK_SW_UPDATE_INFO_REQ* pReq = (STRUCT_CHECK_SW_UPDATE_INFO_REQ* )data;
	STRUCT_CHECK_SW_UPDATE_INFO_RESPONSE* resp ;

	printf("onCheckAppUpdateInfo()\n");
	
	resp = (STRUCT_CHECK_SW_UPDATE_INFO_RESPONSE* )calloc( 1, sizeof(STRUCT_CHECK_SW_UPDATE_INFO_RESPONSE) + 16*1024 );
	
	memcpy( (Cmd_Header* )&resp->header, &pReq->header, sizeof(Cmd_Header) ) ;

	resp->CheckFlag = ru_check_update_info( resp->NewVersion, 
						&(resp->UpdateFlag), resp->Description, &descrip_len, 
						get_uboot_version(), get_kernel_version(), get_fs_version(), get_app_version() );
	
    TcpBlockSend( m_sessionHandle, (char *)resp, sizeof(STRUCT_CHECK_SW_UPDATE_INFO_RESPONSE) + descrip_len );

	free( resp );

	return true;
}

bool CNetUser::onStartRemoteUpdate( char* data, int len )
{	
	STRUCT_START_REMOTE_UPDATE_REQ* pReq = (STRUCT_START_REMOTE_UPDATE_REQ* )data;
	STRUCT_START_REMOTE_UPDATE_RESPONSE resp ;

	printf("onStartRemoteUpdate()\n");
	
	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header));

	/* 如果正在进行升级，则不能升级*/
	if( GetNetServerObj()->IsUpdating() )
	{
		printf("onStartRemoteUpdate: has been updateing!\n");
		resp.Result = -1;
	}
	else
	{
		resp.Result = 0;
		
		GetNetServerObj()->SetUpdating(true);	
		
		ru_start_update(UPDATEFILE);
	}

    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;
	
	return true;
}

bool CNetUser::onGetRemoteUpdateStat( char* data, int len )
{
	int sRet = 0;
	STRUCT_GET_REMOTE_UPDATE_STAT_REQ* pReq = (STRUCT_GET_REMOTE_UPDATE_STAT_REQ* )data;
	STRUCT_GET_REMOTE_UPDATE_STAT_RESPONSE resp;

	printf("onGetRemoteUpdateStat()\n");
	
	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header));

	resp.Flag = 0;

	while( 1 )
	{
		ru_get_update_stat( &resp.UpdateFlag, &resp.process );

		sRet = TcpBlockSend( m_sessionHandle, (char *)&resp , sizeof(resp) ) ;
		if( !sRet )
		{
			/** 向IE发送进度失败，可能IE关闭了*/
			printf("onGetRemoteUpdateStat: send process failed!\n");
			return true;
		}
		
		printf("onGetRemoteUpdateStat: stat = %d, process = %d\n", resp.UpdateFlag,resp.process );

		/** 如果下载状态为暂停，错误，下载完成，就可以不用反馈进度了*/
		if( (UPDATE_STAT_PAUSED == resp.UpdateFlag) || 
			(UPDATE_STAT_ERROR == resp.UpdateFlag) ||
			(UPDATE_STAT_IN_UPDATE == resp.UpdateFlag) )
		{
			break;
		}
		
		sleep(1);  /** 每隔1s向IE上报一次进度*/
	}
	
	return true;
}

bool CNetUser::onCancelRemoteUpdate( char* data, int len )
{
	STRUCT_CANCEL_REMOTE_UPDATE_REQ* pReq = (STRUCT_CANCEL_REMOTE_UPDATE_REQ* )data;
	STRUCT_CANCEL_REMOTE_UPDATE_RESPONSE resp;

	printf("onCancelRemoteUpdate()\n");
	
	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header));
		
	if( GetNetServerObj()->IsUpdating() )
	{
		resp.Result = ru_cancel_update();
		if( 0 == resp.Result )
			GetNetServerObj()->SetUpdating(false);	
	}
	else
	{
		printf("onCancelRemoteUpdate: Device is not updateing!\n");
		resp.Result = -1;
	}
	
    TcpBlockSend( m_sessionHandle, (char *)&resp , sizeof( resp ) ) ;
	
	return true;
}

bool CNetUser::onPauseRemoteUpdate( char* data, int len )
{
	STRUCT_PAUSE_REMOTE_UPDATE_REQ* pReq = (STRUCT_PAUSE_REMOTE_UPDATE_REQ* )data;
	STRUCT_PAUSE_REMOTE_UPDATE_RESPONSE resp;

	printf("onPauseRemoteUpdate()\n");
	
	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header));
		
	if( GetNetServerObj()->IsUpdating() )
	{
		resp.Result = ru_pause_update();
	}
	else
	{
		printf("onPauseRemoteUpdate: Device is not updateing!\n");
		resp.Result = -1;
	}
	
    TcpBlockSend( m_sessionHandle, (char *)&resp , sizeof( resp ) ) ;
	
	return true;
}

bool CNetUser::onResumeRemoteUpdate( char* data, int len )
{
	STRUCT_RESUME_REMOTE_UPDATE_REQ* pReq = (STRUCT_RESUME_REMOTE_UPDATE_REQ* )data;
	STRUCT_RESUME_REMOTE_UPDATE_RESPONSE resp;

	printf("onResumeRemoteUpdate()\n");
	
	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header));
		
	resp.Result = ru_resume_update();
	
    TcpBlockSend( m_sessionHandle, (char *)&resp , sizeof( resp ) ) ;
	
	return true;
}

bool CNetUser::onGetDateFormat(char* data, int len)
{
	NDB("\r\n");
	STRUCT_COMMON_REQ* pReq = (STRUCT_COMMON_REQ* )data;
	STRUCT_DATE_FORMAT_REQ resp;
	COMMON_PARA localParam;
	
	memset(&localParam, 0x0, sizeof(COMMON_PARA));
	memset(&resp, 0x0, sizeof(STRUCT_DATE_FORMAT_REQ));

	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header));
	
	g_cParaManage->GetSysParameter(SYSCOMMON_SET, &localParam);
	resp.DateMode_s.DateMode = localParam.m_uDateMode;
	resp.DateMode_s.TimeMode = localParam.m_uTimeMode;
	resp.DateMode_s.DateSeparator = localParam.m_date_mode;
	
    TcpBlockSend( m_sessionHandle, (char *)&resp , sizeof( resp ) ) ;
	
	return true;
}

bool CNetUser::onSetDateFormat(char* data, int len)
{
	NDB("\r\n");
	STRUCT_DATE_FORMAT_REQ* pReq = (STRUCT_DATE_FORMAT_REQ* )data;
	STRUCT_COMMON_RESPONSE resp;
	COMMON_PARA localParam;
	memset(&localParam, 0x0, sizeof(COMMON_PARA));
	memset(&resp, 0x0, sizeof(STRUCT_COMMON_RESPONSE));

	g_cParaManage->GetSysParameter(SYSCOMMON_SET, &localParam);
	
	localParam.m_uDateMode = pReq->DateMode_s.DateMode;
	localParam.m_uTimeMode = pReq->DateMode_s.TimeMode;
	localParam.m_date_mode = pReq->DateMode_s.DateSeparator;

	g_cParaManage->SetSystemParameter(SYSCOMMON_SET, &localParam);
	
	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header));
	resp.Result = 0;
    TcpBlockSend( m_sessionHandle, (char *)&resp , sizeof( resp ) ) ;
	
	return true;
}

bool CNetUser::onGetDiskInfo( char* data, int len)
{
	NDB("\r\n");
	STRUCT_READ_DISK_REQUEST * pReq = (STRUCT_READ_DISK_REQUEST*) data;
	STRUCT_READ_DISK_ECHO resp;
	int sendLen = 0;
	memset(&resp, 0x0, sizeof(STRUCT_READ_DISK_ECHO));
	BlockDevInfo_S Info;
	
	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header));
	
	/*IPC只支持一个SD录像设备*/
	
	GetBlockDeviceInfo(0, &Info);
	if(Info.m_u8Exist == 2)
	{
		resp.header.length = sizeof(STRUCT_READ_DISK_ECHO) - sizeof(Cmd_Header);
		resp.diskNum = 1;
		memcpy(&resp.blockDevInfo, &Info, sizeof(BlockDevInfo_S));
		sendLen = sizeof(STRUCT_READ_DISK_ECHO);
	}
	else
	{	
		/*不存在SD卡，不发送BlockDevInfo_S*/
		resp.header.length = sizeof(STRUCT_READ_DISK_ECHO) - sizeof(Cmd_Header) -  sizeof(BlockDevInfo_S);
		sendLen = sizeof(STRUCT_READ_DISK_ECHO)- sizeof(BlockDevInfo_S);
	}
	
	TcpBlockSend(m_sessionHandle, (char*)&resp , sendLen);
	return true;
}

bool CNetUser::onGetVersion(char* data, int len)
{
	NDB("\r\n");
	STRUCT_COMMON_REQ *pReq = (STRUCT_COMMON_REQ*) data;
	STRUCT_VERSION_RESPONSE resp;
	memset(&resp, 0x0, sizeof(STRUCT_VERSION_RESPONSE));
	
	memcpy(&resp.header , &pReq->header , sizeof(Cmd_Header));
	resp.header.length = sizeof(STRUCT_VERSION_RESPONSE) - sizeof(Cmd_Header);

	sprintf(resp.AppVersion, "%s", get_app_version());
	sprintf(resp.UbootVersion, "%s", get_uboot_version());
	sprintf(resp.KernelVersion, "%s", get_kernel_version());
	sprintf(resp.RootfsVersion, "%s", get_fs_version());
	
	TcpBlockSend(m_sessionHandle, (char*)&resp , sizeof(STRUCT_VERSION_RESPONSE));
	
	return true;
}

bool CNetUser::onWifiNVRMode(char *data, int len)
{
	STRUCT_S_NOTIFY_IPC_WIFINVR_REQ *pReq = (STRUCT_S_NOTIFY_IPC_WIFINVR_REQ *)data;
	STRUCT_S_NOTIFY_IPC_WIFINVR_RSP resp;

	memset(&resp, 0x0, sizeof(STRUCT_S_NOTIFY_IPC_WIFINVR_RSP));
	memcpy(&resp.header, &pReq->header, sizeof(Cmd_Header));
	resp.header.length = sizeof(STRUCT_S_NOTIFY_IPC_WIFINVR_RSP) - sizeof(Cmd_Header);
	StartLowBiteStream();
	TcpBlockSend(m_sessionHandle, (char *)&resp, sizeof(STRUCT_S_NOTIFY_IPC_WIFINVR_RSP));

	return true;
}
bool CNetUser::onChangeStreamRate(char *data, int len)
{
	STRUCT_S_CHANGE_IPC_STREAM_RATE_REQ *pReq = (STRUCT_S_CHANGE_IPC_STREAM_RATE_REQ *)data;
	STRUCT_S_CHANGE_IPC_STREAM_RATE_RSP resp;

	Set_Net_Level(pReq->level);

	memset(&resp, 0x0, sizeof(STRUCT_S_CHANGE_IPC_STREAM_RATE_RSP));
	memcpy(&resp.header, &pReq->header, sizeof(Cmd_Header));
	resp.header.length = sizeof(STRUCT_S_CHANGE_IPC_STREAM_RATE_RSP) - sizeof(Cmd_Header);
	TcpBlockSend(m_sessionHandle, (char *)&resp, sizeof(STRUCT_S_CHANGE_IPC_STREAM_RATE_RSP));

	return true;
}
bool CNetUser::onGetViRate(char *data, int len)
{
	printf("#######%s %d########\n",__FUNCTION__,__LINE__);
	STRUCT_G_IPC_VENC_ATTR_REQ *pReq = (STRUCT_G_IPC_VENC_ATTR_REQ *)data;
	STRUCT_G_IPC_VENC_ATTR_RSP resp;
	VENC_ATTR_H264_VBR_S attr; 
	memset(&attr,0,sizeof(VENC_ATTR_H264_VBR_S));
	Get_Vi_Rate(&attr);

	memset(&resp, 0x0, sizeof(STRUCT_G_IPC_VENC_ATTR_RSP));
	memcpy(&resp.header, &pReq->header, sizeof(Cmd_Header));	
	memcpy(&resp.attr, &attr, sizeof(VENC_ATTR_H264_VBR_S));
	resp.header.length = sizeof(STRUCT_G_IPC_VENC_ATTR_RSP) - sizeof(Cmd_Header);
	TcpBlockSend(m_sessionHandle, (char *)&resp, sizeof(STRUCT_G_IPC_VENC_ATTR_RSP));

	return true;
}
bool CNetUser::onSetViRate(char *data, int len)
{
	
	printf("#######%s %d########\n",__FUNCTION__,__LINE__);
	STRUCT_S_IPC_VENC_ATTR_REQ *pReq = (STRUCT_S_IPC_VENC_ATTR_REQ *)data;
	STRUCT_S_IPC_VENC_ATTR_RSP resp;
	VENC_ATTR_H264_VBR_S attr; 
	memset(&attr,0,sizeof(VENC_ATTR_H264_VBR_S));
	memcpy(&attr,&pReq->attr,sizeof(VENC_ATTR_H264_VBR_S));
	Set_Vi_Rate(&attr);

	memset(&resp, 0x0, sizeof(STRUCT_S_IPC_VENC_ATTR_RSP));
	memcpy(&resp.header, &pReq->header, sizeof(Cmd_Header));	
	resp.header.length = sizeof(STRUCT_S_IPC_VENC_ATTR_RSP) - sizeof(Cmd_Header);
	TcpBlockSend(m_sessionHandle, (char *)&resp, sizeof(STRUCT_S_IPC_VENC_ATTR_RSP));

	return true;
}
bool CNetUser::onGetVideoKey(char *data, int len)
{
	if(data == NULL || len <= 0)
		printf("onGetVideoKey param is null\n");

	STRUCT_G_VIDEO_KEY_REQ *pReq = (STRUCT_G_VIDEO_KEY_REQ *)data;
	STRUCT_G_VIDEO_KEY_RESP resp;

	memset(&resp, 0, sizeof(resp));
	memcpy(&resp.header, &pReq->header, sizeof(Cmd_Header));
	resp.header.length = sizeof(resp.key);
	strcpy(resp.key, GetAesKey());
	TcpBlockSend(m_sessionHandle, (char *)&resp, sizeof(resp));

	return true;
}

bool CNetUser::onSetMotorTest(char *data, int len)
{
	STRUCT_S_MOTOR_TEST_REQ *pReq = (STRUCT_S_MOTOR_TEST_REQ*)data;
//	STRUCT_S_MOTOR_TEST_RESP resp;

	MotorTurnAroundTest(pReq->mode);
//	memset(&resp, 0, sizeof(resp));
//	memcpy(&resp.header, pReq->header, sizeof(Cmd_Header));
//	resp.echo =MotorTurnAroundTest(pReq->mode);
//	resp.header.length = sizeof(resp) - sizeof(Cmd_Header);

//	TcpBlockSend(m_sessionHandle, (char*)&resp, sizeof(resp));

	return true;
}

bool CNetUser::onResetTest(char *data, int len)
{
	STRUCT_SET_RESTORE_REQUEST *pReq = (STRUCT_SET_RESTORE_REQUEST*)data;
	//STRUCT_SET_RESTORE_ECHO resp;
	RestoreDefault((SYSTEM_PARAMETER *)PubGetParameterPtr());

	return true;
}

bool CNetUser::onNightVisionTest(char *data, int len)
{
	STRUCT_S_NIGHT_TEST_REQ *pReq = (STRUCT_S_NIGHT_TEST_REQ*)data;
	STRUCT_S_NIGHT_TEST_RESP resp;
	unsigned char status;
	memset(&resp, 0, sizeof(resp));
	memcpy(&resp.header,&pReq->header, sizeof(Cmd_Header));

	if(pReq->status != 0 && pReq->status != 1)
		resp.echo = -1;
	else 
	{
	       switch((unsigned char)pReq->status)
	       {
			case 0:
				status = (unsigned char)pReq->status + 3; 
				break;
			case 1:
				status = (unsigned char)pReq->status + 1; 
				break;
		 }		
		
		Set_NightSwtich(status);// 2 开3 关
		resp.echo = 0;
	}
	resp.header.length = sizeof(resp) - sizeof(Cmd_Header);

	TcpBlockSend(m_sessionHandle, (char*)&resp, sizeof(resp));

	return true;
}

bool CNetUser::onSetMdRegion(char *data, int len)
{
	STRUCT_S_MD_REGIN_REQ *pReq = (STRUCT_S_MD_REGIN_REQ*)data;
	STRUCT_S_MD_REGIN_RESP resp;

	memset(&resp, 0, sizeof(resp));
	memcpy(&resp.header, &pReq->header, sizeof(Cmd_Header));
	resp.echo = SetMdRegion(&pReq->region);
	resp.header.length = sizeof(resp) - sizeof(Cmd_Header);
	printf("set md region(%f,%f,%f,%f)\n", pReq->region.x, pReq->region.y, 
			pReq->region.width, pReq->region.height);
	TcpBlockSend(m_sessionHandle, (char*)&resp, sizeof(resp));
}

bool CNetUser::onGetMdRegion(char *data, int len)
{
	STRUCT_G_MD_REGIN_REQ *pReq = (STRUCT_G_MD_REGIN_REQ*)data;
	STRUCT_G_MD_REGIN_RESP resp;

	memset(&resp, 0, sizeof(resp));
	memcpy(&resp.header, &pReq->header, sizeof(Cmd_Header));
	GetMdRegion(&resp.region);
	printf("get md region(%f,%f,%f,%f)\n", resp.region.x, resp.region.y, 
			resp.region.width, resp.region.height);
	resp.header.length = sizeof(resp) - sizeof(Cmd_Header);
	TcpBlockSend(m_sessionHandle, (char*)&resp, sizeof(resp));
}
