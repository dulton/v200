

#ifndef _JIANGHM_ZMD_IPCAMERA_INTERFACE_HEADER_33432329324
#define _JIANGHM_ZMD_IPCAMERA_INTERFACE_HEADER_33432329324

//=================================================
// 注意: 
// 信令协议会运行在Arm上,有些arm系统要求严格的字节对齐
// 所以信令结构定义也要求严格字节对齐，字节无法对齐请
// 用内存拷贝，或者填充字节
// 有些结构中包含结构的，更需要严格检查结构是否字节对齐
//
#include "hi_type.h"
#include "hi_comm_rc.h"

#include "systemparameterdefine.h"
#include "ptz.h"
#include "RecordManage.h"


//==================================
//用户名长度定义，所有与用户名相关的操作均
//使用该宏来定义用户名buf长度。方便后期调整
#define USER_NAME_LEN		16
//==================================
//同上
#define USER_PASS_LEN		16

//预置点信息buf


#ifdef __cplusplus
extern "C" {
#endif
	
#pragma pack( 1 )
	
	//===============================================
	//消息头定义
    typedef struct
    {
		int						head;
		int						length;		//数据长度,去除head
		unsigned char			type;
		unsigned char			channel;
		unsigned short			commd;
	}Cmd_Header ;
	
	//==============================================
	//用来定义消息头的宏.
	//后期协议扩展后的消息体定义也要使用该宏,保障每个消息体
	//中消息头的定义都一样。
#define DEF_CMD_HEADER		Cmd_Header		header
	
	//===============================================
	// 以下是具体信令定义.
	// 新增定义请保持相同格式，变长数据也请用注释说明成员情况。
	// 消息字  消息请求结构 消息回应结构 
	// 消息请求结构命名 STRUCT_XXX_XXX_REQUEST
	// 消息回应结构命名 STRUCT_XXX_XXX_ECHO
	//==============================================
#define CMD_FORMATHDD			0x9c02	//网络批量格式化硬盘
	
	//==============================================
	//烧写设备MAC地址
#define CMD_S_MAC				0x9c00	
	
	typedef struct
	{
		DEF_CMD_HEADER ;
		char		macAddr[6] ;			//需要设置的mac地址
	}STRUCT_SET_MAC_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER ;
		int			echo ;					//设置结果 0 = 成功 , -1 = 失败
	}STRUCT_SET_MAC_ECHO ;
	
	//==============================================
	//获取设备MAC地址
#define CMD_G_MAC				0x9c01	
	
	typedef struct
	{
		DEF_CMD_HEADER ;
	}STRUCT_GET_MAC_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER;
		char		macAddr[6] ;		//mac地址
	}STRUCT_GET_MAC_ECHO ;
	
	//===================================================
	//PING包
#define CMD_PING				0x9001	
	
	//==========================================================
	//ping 设置addr。CmdHeader.Type = 0 
	
	typedef struct
	{
		char			ipaddr[20];
		char			geteway[20];
		char			submask[20];
		char			mac[20];
	}ipaddr_tmp;
	
	typedef struct
	{
		unsigned short	webPort;//用于给搜索工具返回web监听端口
		unsigned short	videoPort;//用于给搜索工具返回video监听端口
		unsigned short	phonePort;//用于给搜索工具返回phone监听端口
		unsigned short	recver;
	}devTypeInfo;
	

	typedef struct
	{
		int	alarmType;	//报警类型0:移动侦测1:IO报警
		int	alarmChl;	//报警通道号或者报警IO口,从0开始
		int	recv[2];
	}alarmType;
	
	typedef struct
	{
		DEF_CMD_HEADER;
	}STRUCT_PING_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER;	
		TYPE_DEVICE_INFO	devInfo ;
		devTypeInfo			portInfo ;
		ipaddr_tmp			ipAddr ;
	}STRUCT_PING_ECHO ;
	
	//==========================================================
	//ping 设置addr。CmdHeader.Type = 1 
	typedef struct
	{
		DEF_CMD_HEADER;
		ipaddr_tmp		ipAddr;
	}STRUCT_PING_SETADDR_REQUEST ;

	//ping 设置addr。CmdHeader.Type = 2 ,开启DHCP
	typedef struct
	{
		DEF_CMD_HEADER;
		char		mac[20];
	}STRUCT_PING_SETDHCP_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER;	
		int			echo ;
	}STRUCT_PING_SETADDR_ECHO ;
	
	//===========================================================
	
#define CMD_START_VIDEO			0x9002	//请求视频
#define CMD_START_SUBVIDEO		0x90a2	//请求子码流
#define CMD_STOP_VIDEO			0x9003	//关闭视频
#define CMD_AUDIO_ON			0x9004  //设备声音开启
#define CMD_AUDIO_OFF			0x9005  //设备声音关闭

#define CMD_TALK_ON				0x9006  //对讲开启
	
	typedef struct
	{
		unsigned char 			sampleRate; 		//采样率 0:8K ,1:12K,  2: 11.025K, 3:16K ,4:22.050K ,5:24K ,6:32K ,7:48K;	
		unsigned char 			audioType;			//编码类型0 :g711 ,1:2726
		unsigned char 			enBitwidth;			//位宽0 :8 ,1:16 2:32
		unsigned char			recordVolume;		//设备当前输入音量0 --31 
		unsigned char 			speakVolume;		//设备当前输出音量0 --31
		unsigned short			framelen ;		//音频帧大小(80/160/240/320/480/1024/2048)
		unsigned char			reserved ;		//保留		
	}Audio_Coder_Param ;

	typedef struct
	{
		DEF_CMD_HEADER;	
		//int			audio_code ;	//0:G711  1:G726
	}STRUCT_TALK_ON_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER;	
		int					talkFlag ;
		Audio_Coder_Param	audioParam ;
	}STRUCT_TALK_ON_ECHO ;

#define CMD_TALK_OFF			0x9007  //对讲关闭
	
	typedef struct
	{
		DEF_CMD_HEADER;	
	}STRUCT_TALK_OFF_REQUEST ;

#define CMD_TALK_DATA			0x9008	//对讲数据
	
	typedef struct
	{
		DEF_CMD_HEADER;	
		char			audioData[0] ;		//长度header中标志
	}STRUCT_TALK_DATA ;

	//===================================================
	//获取设备型号，登陆
#define CMD_DECIVE_TYPE			0x9009	//设备型号
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		char			name[USER_NAME_LEN] ;
		char			pwd[USER_PASS_LEN] ;
	}STRUCT_DEVICE_TYPE_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER ;
		TYPE_DEVICE_INFO	deviceInfo ;	//设备信息
		int					echoCode ;		//登陆成功为0 ,失败则为错误码
		int					permit ;		//权限？
	}STRUCT_DEVIDE_TYPE_ECHO ;
	
	//=========================================================
	//报警通知
#define CMD_ALARM_UPDATE		0x9010	
	
	typedef struct
	{
		DEF_CMD_HEADER ;
		int	alarmType;						//报警类型0:移动侦测1:IO报警
		int	alarmChl;						//报警通道号或者报警IO口,从0开始
		int	recv[2];
	}STRUCT_ALARM_UPDATE_REQUEST ;
	
	
	
#define CMD_SEND_SPEED			0x9020	//控制视频回放传输速度
	
	//=========================================================
	//请求实时视频直播
#define CMD_START_720P          0x5000	//请求720P
#define CMD_START_VGA			0x9002	//请求视频 /*panjy 说明 修改命令名称*/
#define CMD_START_QVGA			0x90a2	//请求子码流
	
	//可以填写上面3个信令
	typedef struct
	{
		DEF_CMD_HEADER ;
	}STRUCT_START_LIVING ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int					result ;
	}STRUCT_START_LIVING_ECHO ;
	
	//==========================================================
	// 设置系统参数
#define CMD_S_DEV_PARA			0xa100	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		SYSTEM_PARAMETER	param ;			//系统参数
	}STRUCT_SET_DEV_PARAM_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int				echo ;				//回应结果(0:OK-1:Failed)
	}STRUCT_SET_DEV_PARAM_ECHO ;
	
	
	//=========================================================
	// 云台命令
#define CMD_S_PTZ				0xa300	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		PTZ_CMD_E	cmd;
		unsigned short para0;
		unsigned short para1;
	}STRUCT_SET_PTZ_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int	echo ;							//回应结果(0:OK-1:Failed)
	}STRUCT_SET_PTZ_ECHO ;
	
	
	//=========================================================
	// 重启命令
#define CMD_S_REBOOT			0xa400	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		unsigned int	headFlag1	;		//0x55555555
		unsigned int	headFlag2	;		//0xaaaaaaaa
	}STRUCT_SET_REBOOT_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int echo ;							//回应结果(0:OK-1:Failed)
	}STRUCT_SET_REBOOT_ECHO ;
	
	
	
	//=========================================================
	// 重启完成命令
#define CMD_S_REBOOT_OK			0xa4ff	
	
	//=========================================================
	// 升级命令
#define CMD_S_UPDATE			0xa500	
	typedef struct
	{
		DEF_CMD_HEADER	;
		unsigned int		checkFlag ;			//must be 0x5555aaaa
		unsigned int		fileLength ;		//升级文件长度
		char				filename[96] ;		//升级文件名
		//char				file[0] ;			//文件内容，长度由fileLength描述
	}STRUCT_SET_UPDATE_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int			echo ;						//0:succeed,1:malloc failed,2:get shared memory failed，
		//3:file length incorrect,4:file checksum incorrect,5:file version same
	}STRUCT_SET_UPDATE_ECHO ;
	
	
	//==========================================================
#define CMD_S_UPDATE_OK			0xa5ff	// 升级完成命令
	
	
	//==========================================================
	// 与client对时
#define CMD_S_NTP				0xa600	
	typedef struct
	{
		DEF_CMD_HEADER	;
		datetime_setting		datetime ;	
	}STRUCT_SET_NTP_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int echo ;							//
	}STRUCT_SET_NTP_ECHO ;
	
	//===========================================================
	// 恢复出厂设置
#define	CMD_S_RESTORE			0xa700	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		unsigned int	headFlag1	;		//0x55555555
		unsigned int	headFlag2	;		//0xaaaaaaaa
	}STRUCT_SET_RESTORE_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int echo ;
		
	}STRUCT_SET_RESTORE_ECHO ;
	
	
#define CMD_S_PANEL_RESTORE		0xa755	// 恢复报警主机出厂设置
#define CMD_S_PANEL_STAT		0xa800	// 报警主机布撤防
#define CMD_S_AUTOSWCTL			0xa900  // 电器开关控制
	
	//==========================================================
	// 控制3G连接或断线
#define CMD_S_3GCTL				0xaa00	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int			status ;				//取值：0或者非0
	}STRUCT_SET_3GCTL_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int			echo ;					//
	}STRUCT_SET_3GCTL_ECHO ;
	
	//==========================================================
	
#define CMD_S_3G_REPOWER		0xaa05	// 控制3g断电复位
	
	
	//==========================================================
	// 恢复单通道默认颜色
#define CMD_S_CHN_ANA			0xa10f	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_RESET_CHNCOLOR_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int				echo ;
		ANALOG_CHANNEL	chnSet ;
	}STRUCT_RESET_CHNCOLOR_RESPONSE ;
	
	
	//===========================================================
	//设置单通道模拟量	
#define CMD_S_CHN_ANALOG		0xa110	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		ANALOG_CHANNEL	chnSet ;
	}STRUCT_SET_CHNANALOG_REQUEST ;
	
	//无回应
	
	//===========================================================
	//获取单通道模拟量
#define CMD_G_CHN_ANALOG		0xa111	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_GET_CHNANALOG_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int			echo ;
		ANALOG_CHANNEL	chnSet ;
	}STRUCT_GET_CHNANALOG_ECHO ;
	
	
	//======================================================
	//获取WIFI热点
#define CMD_G_WIFI_AP			0xa112	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_GET_WIFIAP_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		TYPE_WIFI_LOGIN		wifiAP[0] ;
	}STRUCT_GET_WIFIAP_ECHO ;
	
	//========================================================
	//连接WIFI热点
#define CMD_S_WIFI_CONNECT		0xa113	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		TYPE_WIFI_LOGIN		wifiAP ;
	}STRUCT_SET_WIFICONNECT_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int			echo ;
	}STRUCT_SET_WIFICONNECT_ECHO ;
	
	
	//====================================================
	//获取连接状态
#define CMD_G_WIFI_STATUS		0xa114	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		TYPE_WIFI_LOGIN		wifiAP ;
	}STRUCT_GET_WIFISTATUS_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		TYPE_WIFI_LOGIN		wifiAP ;
	}STRUCT_GET_WIFISTATUS_ECHO ;
	
	
	
	//===========================================================
	//// 读取设备系统参数
#define CMD_R_DEV_PARA			0x9100	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_READ_DEV_PARAM_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		SYSTEM_PARAMETER	param ;			//系统参数
	}STRUCT_READ_DEV_PARAM_ECHO ;
	
	//=========================================================
#define CMD_R_ALARMINFO			0x9600	// 查询报警信息
	
	//====================================================
	// 查询日志
#define CMD_R_LOG				0x9700	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		//FindUserLogItem		logFind ;		//
	}STRUCT_READ_LOG_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		//FindLogItems		logItem ;		//
	}STRUCT_READ_LOG_ECHO ;
	
	
	//=================================================
	//// 读状态信息
#define CMD_R_DEV_INFO			0x9800	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_READ_DEVINFO_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		TYPE_DEVICE_INFO	devInfo ;
		BlockDevInfo_S		storageDev[2]  ;	//2个存储设备信息
	}STRUCT_READ_DEVINFO_ECHO ;
	
#define CMD_R_PANEL_INFO		0x9810	// 读报警主机状态信息
	
	//===================================================
	// 检索回放列表
#define CMD_R_SEARCH_PLAYLIST	0x9900  
	
	typedef struct
	{
		DEF_CMD_HEADER	;	
		int				mode ;					//填0
		FindFileType	findType ;				
	}STRUCT_READ_PLAYLIST_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		RecordFileName		result ;			//查询结果
	}STRUCT_READ_PLAYLIST_ECHO ;
	
	//================================================
	
#define CMD_G_SEARCH_RECORD_DAY	0x9901 	
typedef struct
{
	DEF_CMD_HEADER	;	
	struct tm		day_start ;	
	struct tm		day_end ;	
	int				channel;	
	char   			res[140];
}STRUCT_GET_RECORD_DAY_REQUEST ;	
	//================================================
	// 回放开始命令
#define CMD_PLAYBACK_PLAY		0x9903	
	
	typedef struct
	{
		unsigned int	p_mode; 		// 0-回放 1- 下载
		unsigned int	p_offset;		// 回放 - 时间偏移量  下载 - 大小偏移量(K)
		unsigned int 	start_time;		/*文件开始时间*/
		unsigned int 	end_time;		/* 文件结束时间*/
		unsigned int    filesize;		/*文件大小以K为单位*/
		int				m_filetype;		//0-手动，1-定时，2-报警
		char			d_name[96]; 	/*文件名(有路径)*/
	}PLAYBACK_ITEM;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		PLAYBACK_ITEM	playItem ;
	}STRUCT_PLAYBACK_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int			echo ;
	}STRUCT_PLAYBACK_ECHO ;
	
	
#define CMD_PLAYBACK_STOP		0x9905	// 回放停止命令
	
	//=================================================
	// 登陆请求命令
#define CMD_REQ_LOGIN			0x9a00	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		char			name[USER_NAME_LEN] ;
		char			pwd[USER_PASS_LEN] ;
	}STRUCT_REQ_LOGIN_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int	permit  ;
		int echo ;
	}STRUCT_REQ_LOGIN_ECHO ;
	
	//================================================
	// 读3G连接状态和型号
#define CMD_R_3G_INFO			0x9b00	
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		unsigned int		l3gtype ;	//0xaaaa5555
	}STRUCT_READ_3GINFO_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int		echo ;
		int		lg3linkstate ;		//0:no 3g connect，1:3g is dialing，2:3g is connecting
		int		lg3type ;			//无线modem型号选择：0x00~0xff
		char	lip_buff[16] ;		//
	}STRUCT_READ_3GINFO_ECHO ;
	
	//====================================================
	//added by liql-2012-07-13
	//图像设置信令
#define CMD_S_PIC_NORMAL		0xa115	//正常
#define CMD_S_PIC_FLIP			0xa116	//翻转ON
#define CMD_S_PIC_MIRRORON	0xa117    //镜像
#define CMD_S_PIC_FLIPMIRROR	0xa118	//镜像翻转
#define CMD_S_PIC_COLORON		0xa119	//彩色
#define CMD_S_PIC_COLOROFF		0xa120	//黑白
	
	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_SET_PIC_REQUEST ;
	
	//=====================================================
	//设置sensor工作电压
#define CMD_S_SENSORFREQ_50		0xa121	//sensor工作电压50HZ
#define CMD_S_SENSORFREQ_60		0xa122	//sensor工作电压60HZ
#define CMD_S_SENSORFREQ_OUT 	0x9123	//室外模式
#define CMD_S_SENSORFREQ_AUTO	0x9124	//自动模式

	
	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_SET_SENSORF_REQUEST ;
	

	//====================================================
	//获取预置点信息
#define CMD_S_GET_PRESET_POINT	0xA123
	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_GET_PRESET_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER	;
		char			presetData[514] ;	//最大512个预置点信息+1字节停留时间+1字节滑竿速度
	}STRUCT_GET_PRESET_ECHO ;

	
#define CMD_CMS_REQUEST_VIDEO		0x7000
#define CMD_CMS_STOP_VIDEO		0x7001
#define CMD_CMS_START_PLAYBACK	0x7002
	
#define	CMD_REGSMSPLAT			0xb001
#define	CMD_SMSPLATHEATBEAT		0xb002
#define	CMD_SMSUPDATE			0xb003
	
	//=======================================================
#define  CMD_SET_VIDEO_CODE             0x7788//liql

	typedef struct
	{
		DEF_CMD_HEADER	;
		CAMERA_PARA		param ;
	}STRUCT_SET_VIDEO_CODE_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER	;
		int				echo ;
	}STRUCT_SET_VIDEO_CODE_ECHO ;
	
	//======================================================
	//获取设备视频编码信息
#define  CMD_R_VIDEO_CODE_PARA			0x7a71//liql
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		
	}STRUCT_READ_VIDEOPARAM_REQUEST ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		CAMERA_PARA		param ;
	}STRUCT_READ_VIDEOPARAM_ECHO ;
	
	//added by panjy 发送报警信息
#define CMD_MOTION_DETECT      0x5030
	
	//=======================================================
	// NVR 通知IPC报警服务监听端口
#define CMD_NVR_PORT 0x5050
	typedef struct
	{
		DEF_CMD_HEADER	;
		unsigned short		alarmListenPort ;
		unsigned short		devType ;
	}STRUCT_NVR_PORT_REQUEST ;
	
	
	//=======================================================
	//获取快照
#define CMD_G_SNAPSHOT			0x9040
	//======================================================
	// client -> server
	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_GET_SNAPSHOT_REQUEST ;
	
	//======================================================
	//server -> client 
	typedef struct
	{
		DEF_CMD_HEADER	;
		int			echo ;
		unsigned int	fileLength ;
		char			imageContent[0] ;
	}STRUCT_GET_SNAPSHOT_ECHO ;
	
	
	//==============================================
	//传感器阀值设定
#define CMD_S_SENSOR_ALARMINFO			0x9042
	
	//=================================
	// client -> server
	typedef struct
	{
		DEF_CMD_HEADER	;
		float		temperature_min ;			//温度最低值
		float		temperature_max ;			//温度最高值
		float		humidity_min ;			//湿度最低值
		float		humidity_max ;			//湿度最高值
		int		sampleRate ;				//采样周期
	}STRUCT_SET_SENSORALARMINFO_REQUEST ;
	
	//=====================================================
	//server -> client 
	typedef struct
	{
		DEF_CMD_HEADER	;
		int		echo ;
	}STRUCT_SET_SENSORALARMINFO_ECHO ;

	
	//=====================================================
	//传感器状态获取
#define CMD_G_SENSOR_ALARMINFO			0x9044
	
	//====================================================
	// client -> server
	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_GET_SENSORALARMINFO_REQUEST ;
	
	//=====================================================
	//server -> client 
	
	typedef struct
	{
		int 		sensorIndex ;
		float		curTem ;
		float		curHum ;
	}STRUCT_SENSOR_CURINFO ;
	
	typedef struct
	{
		DEF_CMD_HEADER	;
		int		echo ;
		float		minTem ;
		float		maxTem ;
		float		minHum ;
		float		maxHum ;
		int		sampleRate ;
		int		sensorCount ;
		STRUCT_SENSOR_CURINFO sensorInfo[0] ;
	}STRUCT_GET_SENSORALARMINFO_ECHO ;
	
	//====================================================
	//读取当前通道的设置信息
#define CMD_GET_CHN_INFO		0x9022
	
	//单个通道的信息.

	typedef struct
	{
		int				chnid ;			//通道号
		unsigned int 	ipcaddr ;		//该通道所绑定的ipc地址
		unsigned short	ipcport ;		//该通道所绑定的ipc端口
		unsigned short	chnstatus ;		//通道状态,目前定义 1-已连接 。 0-未连接
		unsigned short	ipcType ;		//IPC类型: 0-私有协议，1-onvif
		unsigned short	res ;			//保留字段
	}NVR_CHN_INFO ;

	typedef struct
	{
		DEF_CMD_HEADER	;
	}STRUCT_GET_CHNINFO_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER	;
		int				autoSearchFlag ;
		int				chnCount ;
		NVR_CHN_INFO	chn[0] ;
	}STRUCT_GET_CHNINFO_ECHO ;

	//=================================================
	//设置NVR通道的IPC信息
#define	CMD_SET_CHN_IPC			0x9024
	typedef struct
	{
		DEF_CMD_HEADER	;		
		char				ipcAddr[16] ;	//ipc地址
		unsigned short		ipcPort ;		//ipc端口
		unsigned short		ipcType ;		//ipc类型
		char				username[16] ;	//登陆用户名
		char				password[16] ;	//登陆密码
	}STRUCT_SET_CHNIPC_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER	;		
		int			echo ;					//0:成功，
	}STRUCT_SET_CHNIPC_ECHO ;

	//=================================================
	//删除NVR通道所绑定的IPC信息
#define CMD_DEL_CHN_IPC			0x9026
	typedef struct
	{
		DEF_CMD_HEADER	;		
		
	}STRUCT_DEL_CHNIPC_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER	;		
		int				echo ;				//0 :成功
	}STRUCT_DEL_CHNIPC_ECHO ;

	//=================================================
	//指定通道进行自动搜索
#define CMD_SET_AUTOADD			0x9028
	typedef struct
	{
		DEF_CMD_HEADER	;		

	}STRUCT_SET_AUTOADD_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER	;		
		int				echo ;
		NVR_CHN_INFO	chnInfo ;	
	}STRUCT_SET_AUTOADD_ECHO ;

	//=================================================
	//设置自动搜索开关
#define CMD_SET_AUTOSEARCH		0x9030
	typedef struct
	{
		DEF_CMD_HEADER	;		
		int			autoSearchFlag ;
	}STRUCT_SET_AUTOSEARCH_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER	;		
		int			echo ;
	}STRUCT_SET_AUTOSEARCH_ECHO ;

	//===================================================
	//PING包
#define CMD_ID_PING				0x9050	
	
	//==========================================================
	//ping 设置addr。CmdHeader.Type = 0 
	typedef struct
	{
		DEF_CMD_HEADER;
	}STRUCT_ID_PING_REQUEST ;
	
	typedef struct
	{
		STRUCT_PING_ECHO	device; 
		char				deviceID[16] ;
	}STRUCT_ID_PING_ECHO ;

	//==========================================================
	//设置设备ID信令
#define CMD_S_DEVICEID		0x9054
	typedef struct
	{
		DEF_CMD_HEADER;
		char			mac[6] ;
		char			res[2] ;
		char			deviceID[16] ;
	}STRUCT_SET_DEVICEID_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER;
		int				echo ;		//0成功,其他失败错误码
	}STRUCT_SET_DEVICEID_ECHO ;

	//=========================================================
	//读取设备ID 信令
#define CMD_G_DEVICEID		0x9056

	typedef struct
	{
		DEF_CMD_HEADER;
	}STRUCT_GET_DEVICEID_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER;
		char			mac[6] ;
		char			res[2] ;
		char			deviceID[16] ;
	}STRUCT_GET_DEVICEID_ECHO ;
	
	//add by table : 定义udp重启信令
#define CMD_SET_UDP_REBOOT	0x9036
	typedef struct
	{
		DEF_CMD_HEADER;
		unsigned int	headFlag1	;		//0x55555555
		unsigned int	headFlag2	;		//0xaaaaaaaa
		char			rebootIP[16] ;	
	}STRUCT_UDP_REBOOT_REQUEST ;	
	
//升级状态提示
#define CMD_UPDATE_STATUS		0x9038
	typedef struct
	{
		DEF_CMD_HEADER ;
		unsigned short	percent;		//百分比0-100
		unsigned short	status ;		//升级状态，0擦除状态,1升级状态
	}STRUCT_UPDATE_STATUS ;

//获取设备音频设置
#define CMD_G_TALK_SETTING		0x9060

	typedef struct
	{
		DEF_CMD_HEADER ;
	}STRUCT_GET_TALK_SETTING_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER ;

		Audio_Coder_Param		audioParam ;
		//unsigned short		micVol;		//设备麦克音量 0-31;
		//unsigned short		spkVol ;	//设备扬声器音量 0-31;
	}STRUCT_GET_TALK_SETTING_ECHO ;

//设备音频设置
#define CMD_S_TALK_SETTING		0x9062
	typedef struct
	{
		DEF_CMD_HEADER ;
		unsigned short		micVol ;	//设备麦克音量0-31 ;
		unsigned short		spkVol ;	//设备麦克音量 0-31;
	}STRUCT_SET_TALK_SETTING_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER ;
		int					echo ;		//0成功
	}STRUCT_SET_TALK_SETTING_ECHO ;

//========================================================
//是否发送音频,无回应
//该信令只是通知服务器，本链接发送的媒体流中
//是否该包含音频
#define CMD_SET_AUDIOSWITCH		0x9066
	typedef struct
	{
		DEF_CMD_HEADER ;
		int		sendAudio ;			//0不发送音频 , 1发送音频
	}STRUCT_SET_AUDIO_SWITCH ;

	typedef struct
	{
		DEF_CMD_HEADER ;
		int		echo ;			//0成功 , 其他失败，错误码
	}STRUCT_SET_AUDIO_SWITCH_RESP ;

	//开始录像
#define CMD_START_RECORD		0x9068
	typedef struct
	{
		DEF_CMD_HEADER ;
	}STRUCT_START_RECORD_REQUEST ;

	typedef struct
	{
		DEF_CMD_HEADER ;
		int		echo ;			//0：成功，1,设备故障,无存储设备
	}STRUCT_STOP_RECORD_ECHO ;
	//停止录像
#define CMD_STOP_RECORD			0x9070
	typedef struct
	{
		DEF_CMD_HEADER ;		
	}STRUCT_STOP_RECORD ;
//开始备份
#define CMD_RECORD_BACKUP		0x9072
	typedef struct
	{
		DEF_CMD_HEADER ;
		char	pathname[128];	//需要备份的目标录像文件名
	}STRUCT_RECORD_BACKUP_REQUEST ;
//备份文件状态
#define CMD_RECORD_BACKUP_STATE	0x9074
	typedef struct
	{
		DEF_CMD_HEADER ;
		unsigned short		state ;		//备份状态 0,正在拷贝,1 目标文件无效, 2 设备故障，3备份完成
		unsigned short		precent ;	//进度百分比0-100
	}STRUCT_RECORD_BACKUP_STATE ;
//获取手动录像状态
#define CMD_G_RECORD_STATUS 0x9076
typedef struct _STRUCT_GET_RECORD_STATUS {
    DEF_CMD_HEADER;
    int     status;  //手动录像状态，0 手动录像未开启，1 手动录像已开启
} STRUCT_GET_RECORD_STATUS_ECHO;

#define CMD_REGIST_G_RECORD_STATUS 0x9077   //client注册获取通道录像状态信息,nvr端会定时发送录像状态到client,client需要监听5310端口,短连接
typedef struct 
{	
	DEF_CMD_HEADER ;
	int  echo;   //0 表示成功
}STRUCT_REGIST_G_RECORD_ECHO;

#define CMD_RECORD_STATUS 0x9078
typedef struct 
{	
	DEF_CMD_HEADER ;
	int status[32];//当前32个通道的录像状态
	int  changeFlag;//标志数组status[0 -31]中对应的值是否改变, 1为改变
}STRUCT_CHANNEL_RECORD_STATUS;

#define CMD_G_CHANNEL_PARA  0x9080
typedef struct _STRUCT_G_CHANNEL_INFO_RESPONSE {
    DEF_CMD_HEADER;
    int    result;  //0 成功，1 失败
    CHANNEL_PARA    channel_para;
} STRUCT_G_CHANNEL_PARA_RESPONSE;

#define CMD_S_CHANNEL_PARA  0x9081
typedef struct _STRUCT_S_CHANNEL_INFO_RESPONSE {
    DEF_CMD_HEADER;
    int    result;  //0 成功，1 失败
} STRUCT_S_CHANNEL_PARA_RESPONSE;

#define CMD_G_WIFI_AP_CONFIG    0x9084
typedef struct _STRUCT_G_WIFI_AP_CONFIG_RESPONSE {
    DEF_CMD_HEADER;
    int         result;  //0 成功，1 失败
   // ApSettings  apInfo;
} STRUCT_G_WIFI_AP_CONFIG_RESPONSE;

#define CMD_S_WIFI_AP_CONFIG    0x9085  //设置wifi参数
typedef struct _STRUCT_S_WIFI_AP_CONFIG_RESPONSE {
    DEF_CMD_HEADER;
    int         result;  //0 成功，1 失败
} STRUCT_S_WIFI_AP_CONFIG_RESPONSE;

#define CMD_R_USB_INFO      0x9086      //读取USB信息
typedef struct _STRUCT_R_USB_INFO_RESPONSE {
    DEF_CMD_HEADER;
    BlockDevInfo_S  usb_info[2];
} STRUCT_R_USB_INFO_RESPONSE;

#define CMD_GET_WIFILED   0x9088  //读取WIFI指示灯
typedef struct _STRUCT_GET_WIFILED_RESPONSE {    
    DEF_CMD_HEADER;
    int result;         //0 成功, 非0 失败
    int status;         //0 关闭, 1 开启
} STRUCT_GET_WIFILED_RESPONSE;

#define CMD_SET_WIFILED   0x908a  //设置WIFI指示灯
typedef struct _STRUCT_SET_WIFILED {    
    DEF_CMD_HEADER;
    int status;         //0 关闭, 1开启
} STRUCT_SET_WIFILED;
typedef struct _STRUCT_SET_WIFILED_RESPONSE {   
    DEF_CMD_HEADER;
    int result;         //0 成功, 非0 失败
} STRUCT_SET_WIFILED_RESPONSE;

#define CMD_GET_AUDIOPARM 0x9090  //获取音频参数
typedef struct _STRUCT_GET_AUDIOPARM_RESPONSE {
    DEF_CMD_HEADER;
    int result;     //0 成功, 非0 失败
    Audio_Coder_Param param;
} STRUCT_GET_AUDIOPARM_RESPONSE;

typedef struct
{
	int  m_ntp;			 	//NTP对时开关
	int  m_timezone;     	//跟UTC 秒数的差距,有正负,范围 [-12*3600, 13*3600]
	int  m_tzindex;        	//当前时区在时区表(保存所有时区)的索引[1-91], 0代表自动获取时区
	int  m_daylight;		//此时区是否启用夏令时
	char reserved[12];
} TimeZoneInfo;

/*added by hayson  begin 2013.8.14*/
#define CMD_SET_TIMEZONE	0x9120		/* 设置时区*/
#define CMD_GET_TIMEZONE	0x9122		/*获取时区*/
#define CMD_CODE_I_FRAME	0x9092		/*强制I 帧请求*/

/*请求包*/
typedef struct      
{
	DEF_CMD_HEADER ;
	int 	stream_type;				/*0:高清通道, 1:普清通道,2:流畅通道*/
}STRUCT_FORCE_I_FRAME_REQUEST;

/*响应包*/
typedef struct      
{
	DEF_CMD_HEADER ;
	int		result;						/*0：成功，1：失败*/
}STRUCT_FORCE_I_FRAME_RESPONSE;

/*added by hayson  end 2013.8.24*/


/*added by hayson  begin 2013.11.26*/
#define CMD_S_DEVCONFIG  0x9057
typedef struct
{
	DEF_CMD_HEADER;
	DeviceConfigInfo 	ConfigInfo;
}STRUCT_SET_DEVICE_CONFIG_REQUEST ;

typedef struct
{
	DEF_CMD_HEADER;
	int ret;				/*(0：设置成功1：校验错误2：平台版本不匹配)*/
}STRUCT_SET_DEVICE_CONFIG_RESPONSE ;


/*added by hayson  end 2013.11.26*/

/**
 * add by mike,2013-12-19
 * 增加自动升级需要的对应ZSP接口协议
 */
#define CMD_CHECK_SW_UPDATE_INFO	0x9125		/* 检测软件版本更新信息 */
/*请求包*/
typedef struct      
{
	DEF_CMD_HEADER ;
}STRUCT_CHECK_SW_UPDATE_INFO_REQ;

/*响应包*/
typedef struct      
{
	DEF_CMD_HEADER ;
	int		CheckFlag;				/* 检测是否成功，0:成功，-1:失败 */
	char	NewVersion[32];			/* 最新APP版本 */
	int		UpdateFlag;				/* 是否有新版本更新 ，1：是，0: 否 */
	char	Description[0];			/* 新版本软件的描述信息 */
	
}STRUCT_CHECK_SW_UPDATE_INFO_RESPONSE;

#define CMD_START_REMOTE_UPDATE		0x9126		/* 开始自动升级 */
/*请求包*/
typedef struct		
{
	DEF_CMD_HEADER ;
}STRUCT_START_REMOTE_UPDATE_REQ;

/*响应包*/
typedef struct		
{
	DEF_CMD_HEADER ;
	int	Result; /* 0:表示成功响应指令，开始自动升级，-1:请求失败*/
	
}STRUCT_START_REMOTE_UPDATE_RESPONSE;

#define CMD_GET_REMOTE_UPDATE_STAT	0x9127	/* 获取自动升级状态 */
/* 请求包 */
typedef struct		
{
	DEF_CMD_HEADER ;
}STRUCT_GET_REMOTE_UPDATE_STAT_REQ;

/* 响应包 */
typedef struct		
{
	DEF_CMD_HEADER ;
	int Flag;			/* 是否正确响应标识,0:是，-1:否 */
	int UpdateFlag; 	/* 0:当前没有在进行远程升级，1:正在下载文件，2:升级被暂停
						3: 下载完成，正在升级，4:升级错误 */
	int process;		/* 如果当前正在进行远程升级，则为进度信息 */

}STRUCT_GET_REMOTE_UPDATE_STAT_RESPONSE;

#define CMD_CANCEL_REMOTE_UPDATE	0x9128		/* 取消自动下载 */
/*请求包*/
typedef struct		
{
	DEF_CMD_HEADER ;
}STRUCT_CANCEL_REMOTE_UPDATE_REQ;

/*响应包*/
typedef struct		
{
	DEF_CMD_HEADER ;
	int	Result; 		/* 是否取消成功,0:取消成功，-1:取消失败 */
} STRUCT_CANCEL_REMOTE_UPDATE_RESPONSE;

#define CMD_PAUSE_REMOTE_UPDATE 0x9129		/* 暂停自动下载 */
/*请求包*/
typedef struct		
{
	DEF_CMD_HEADER ;
}STRUCT_PAUSE_REMOTE_UPDATE_REQ;

/*响应包*/
typedef struct		
{
	DEF_CMD_HEADER ;
	int	Result; 		/* 暂停结果0:暂停成功，-1:暂停失败 */
} STRUCT_PAUSE_REMOTE_UPDATE_RESPONSE;

#define CMD_RESUME_REMOTE_UPDATE	0x9130		/* 恢复自动下载 */
/*请求包*/
typedef struct		
{
	DEF_CMD_HEADER ;
}STRUCT_RESUME_REMOTE_UPDATE_REQ;

/*响应包*/
typedef struct		
{
	DEF_CMD_HEADER ;	
	int Result; 		/* 恢复结果0:恢复成功，-1:恢复失败 */
} STRUCT_RESUME_REMOTE_UPDATE_RESPONSE;

/*add by hayson begin 2014.6.25*/
#define CMD_GET_DATE_FORMAT 	0x9131 	/* 获取时间格式 */
#define CMD_SET_DATE_FORMAT 	0x9132	/* 设置时间格式 */

typedef struct      
{
	unsigned char 		DateMode;		/* 日期显示的方式	0 表示dd-mm-yyyy ,1表示yyyy-mm-dd,2表示mm-dd-yyyy */
	unsigned char		TimeMode;		/* 时间的格式:0 表示24 hours, 1 表示am/pm */
	unsigned char		DateSeparator;	/* 日期分隔符: - | / 三种 */
	unsigned char		reser;			/* 保留 */
}STRUCT_DATE_FORMAT;

/*请求包*/
typedef struct		
{
	DEF_CMD_HEADER ;
}STRUCT_COMMON_REQ;

typedef struct		
{
	DEF_CMD_HEADER ;	
	int Result; 		/* 恢复结果0:恢复成功，-1:恢复失败 */
} STRUCT_COMMON_RESPONSE;

/*日期格式包*/
typedef struct      
{
	DEF_CMD_HEADER ;
	STRUCT_DATE_FORMAT DateMode_s;
}STRUCT_DATE_FORMAT_REQ;
/* add by hayson end*/

/* 新增获取录像设备信息接口*/
#define CMD_R_DEV_DISK  0x9801	
typedef struct
{
		DEF_CMD_HEADER	;
}STRUCT_READ_DISK_REQUEST ;

typedef struct
{
		DEF_CMD_HEADER	;
		char diskNum;
		BlockDevInfo_S blockDevInfo;
}STRUCT_READ_DISK_ECHO ;

/*获取ipc设备版本信息*/
#define CMD_GET_IPC_VERSION 0x9011
typedef struct
{
	DEF_CMD_HEADER;
	char	UbootVersion[16];
	char	KernelVersion[16];
	char	RootfsVersion[16];
	char	AppVersion[16];	
}STRUCT_VERSION_RESPONSE;

/* 
** ssid烧录新增信令
** @hayson 2014.12.9
*/
#define CMD_S_DEFAULT_CONFIG 0x9058 /* 设置缺省信息*/
#define SSID_LEN	32

/* 缺省信息 */
typedef struct
{
	char ap_ssid[SSID_LEN];			/* 热点名称*/
	char ap_password[SSID_LEN];		/* 热点密码 */
	char reserve[448];				/* 预留*/
}DevDefaultConfig;


/*请求包*/
typedef struct		
{
	DEF_CMD_HEADER;
	DevDefaultConfig config;
}STRUCT_S_DEFAULT_CONFIG_REQ;

/* 响应包 */
typedef struct		
{
	DEF_CMD_HEADER;
	int ret;
}STRUCT_S_DEFAULT_CONFIG_RESPONSE;

#define CMD_G_DEFAULT_CONFIG 0x9059 /* 获取缺省信息*/
/*请求包*/
typedef struct		
{
	DEF_CMD_HEADER;
}STRUCT_G_DEFAULT_CONFIG_REQ;

/* 响应包 */
typedef struct		
{
	DEF_CMD_HEADER;
	DevDefaultConfig config;
}STRUCT_G_DEFAULT_CONFIG_RESPONSE;


/* 
** miniNVR设置ipc备用wifi
** @hayson 2015.1.27
*/

#define CMD_SET_IPC_WIFI_BACK 0x9012

/*请求包*/
typedef struct		
{
	DEF_CMD_HEADER ;
	TYPE_WIFI_LOGIN	SendData;
}STRUCT_S_BACKUP_IPC_WIFI_REQ;
/* 
** miniNVR通知ipc网络状态
** @hayson 2015.2.10
*/
#define CMD_NOTIFY_IPC_NETSTATUS 0x9013

/*请求包*/
typedef struct		
{
	DEF_CMD_HEADER ;
	int	 route;
}STRUCT_S_NOTIFY_IPC_REQ;

/*
** 设置门灯控制参数
*/
#define CMD_S_DOOR_LAMP_PARAM 0x9600

typedef struct 
{
	unsigned char light_switch;//Light 开关1:开2:关3:自动默认值为2(关)
	unsigned char white_switch;//白炽灯开关1:开2:关默认:1(开)
	unsigned char breath_switch;//颜色切换(呼吸功能) 1:开2:关默认为2(关)
	unsigned char color_red;
	unsigned char color_green;
	unsigned char color_blue;
} DoorLampParam;

/* 请求包 */
typedef struct
{
	DEF_CMD_HEADER;
    DoorLampParam SendData;
} STRUCT_S_DOOR_LAMP_PARAM_REQ;

/* 响应包 */
typedef struct 
{
	DEF_CMD_HEADER;
	int result; /* 0, 成功. -1, 失败 */
} STRUCT_S_DOOR_LAMP_PARAM_RESP;


/* wifiNvr 通知ipc 使用特定码流*/
#define CMD_S_WIFINVR_NOTIFY 0x9601


/* 请求包 */
typedef struct 
{
	DEF_CMD_HEADER;
} STRUCT_S_NOTIFY_IPC_WIFINVR_REQ;

/* 响应包 */
typedef struct 
{
	DEF_CMD_HEADER;
	int echo; /* 0, 支持; -1, 不支持; 或者不响应 */
} STRUCT_S_NOTIFY_IPC_WIFINVR_RSP;

/* wifiNVR  通知IPC 改变码率*/
#define CMD_S_CHANGE_IPC_STREAM_RATE 0x9602

/* 请求包 */
typedef struct
{
	DEF_CMD_HEADER;
	int level; 
} STRUCT_S_CHANGE_IPC_STREAM_RATE_REQ;

/* 响应包 */
typedef struct 
{
	DEF_CMD_HEADER;
	int echo;
} STRUCT_S_CHANGE_IPC_STREAM_RATE_RSP;

#define CMD_G_IPC_VENC_ATTR 0x9603 


typedef struct 
{ 
DEF_CMD_HEADER; 
} STRUCT_G_IPC_VENC_ATTR_REQ; 

typedef struct 
{ 
DEF_CMD_HEADER; 
VENC_ATTR_H264_VBR_S attr; 
} STRUCT_G_IPC_VENC_ATTR_RSP; 

#define CMD_S_IPC_VENC_ATTR 0x9604 

typedef struct 
{ 
DEF_CMD_HEADER; 
VENC_ATTR_H264_VBR_S attr; 
} STRUCT_S_IPC_VENC_ATTR_REQ; 

typedef struct 
{ 
DEF_CMD_HEADER; 
int result; 
} STRUCT_S_IPC_VENC_ATTR_RSP; 



/* 获取视频加密的密钥 */
#define CMD_G_VIDEO_KEY 0x9636

/* 请求包 */
typedef struct
{
	DEF_CMD_HEADER;
} STRUCT_G_VIDEO_KEY_REQ;

/* 响应包 */
typedef struct
{
	DEF_CMD_HEADER;
	char key[1024];
} STRUCT_G_VIDEO_KEY_RESP;

// 获取电池电量
#define CMD_G_BATTERY_ADC_VALUE 0x9637

/* 请求包 */
typedef struct
{
	DEF_CMD_HEADER;
} STRUCT_G_BATTERY_ADC_VALUE_REQ;

/* 响应包 */
typedef struct
{
	DEF_CMD_HEADER;
	int value;
} STRUCT_G_BATTERY_ADC_VALUE_RESP;

#ifdef FACTORY_TEST
#define CMD_S_FACTORY_STATE 0x9640

/* 请求包 */
typedef struct
{
	DEF_CMD_HEADER;
	char ssid[256]; // 根据ssid解析设备状态
} STRUCT_S_FACTORY_STATE_REQ;

/* 响应包 */
typedef struct
{
	DEF_CMD_HEADER;
	int ret; // 0,成功;-1失败
} STRUCT_S_FACTORY_STATE_RESP;
#endif



//设置移动侦测区域
#define CMD_S_MD_REGIN 0x9045

/* 请求包 */
typedef struct
{
	DEF_CMD_HEADER;
	P2P_MD_REGION_CHANNEL region;
} STRUCT_S_MD_REGIN_REQ;

/* 响应包 */
typedef struct
{
	DEF_CMD_HEADER;
	int echo; // 0,成功; -1, 失败
} STRUCT_S_MD_REGIN_RESP;

//获取移动侦测区域
#define CMD_G_MD_REGIN 0x9046

/* 请求包 */
typedef struct
{
	DEF_CMD_HEADER;
} STRUCT_G_MD_REGIN_REQ;

/* 响应包 */
typedef struct
{
	DEF_CMD_HEADER;
	P2P_MD_REGION_CHANNEL region;
} STRUCT_G_MD_REGIN_RESP;

//测试电机转动

#define CMD_S_MOTOR_TEST 0x9033

/* 请求包 */
typedef struct
{  
	DEF_CMD_HEADER;  
	char mode; // 0, 停止;1,转动
} STRUCT_S_MOTOR_TEST_REQ; 

/* 响应包 */
/*
typedef struct
{  
	DEF_CMD_HEADER;  
	int echo; // 0, 成功;-1,失败
} STRUCT_S_MOTOR_TEST_RESP; 
*/

#define CMD_S_NIGHT_SWITCH_TEST 0x9034 

/* 请求包 */
typedef struct
{  
	DEF_CMD_HEADER;  
       char status; // 0, 关;1, 打开
} STRUCT_S_NIGHT_TEST_REQ; 

/* 响应包 */
typedef struct
{
	DEF_CMD_HEADER;
	int echo;//0 成功；-1 失败
} STRUCT_S_NIGHT_TEST_RESP;

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif

