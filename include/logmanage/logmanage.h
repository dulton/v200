
#ifndef __LOG_MANAGE_H_
#define __LOG_MANAGE_H_

#include <pthread.h>

#include "common.h"

#ifdef __cplusplus
	extern "C" {
#endif

#define STREAMING_LOG    		"SLOG"

#define TS_USELOG  	 			"USER"	// 用户日志

#define TF_USELOG				0x01   // 用户日志文件

#if 0
// 事件类型
#define  USER_EVENT_ALARM		0x01  // 报警事件开始
#define  USER_EVENT_SYSTEM		0x02  // 系统日志
#define  USER_EVENT_FAULT		0x03  // 系统故障
#define  USER_EVENT_ALARMEND	0x04  // 报警事件结束
#define USER_EVENT_OPERATE	0x05
#endif 


#define CUR_LOG_VERSION			0x0000001

#define DISKMOUNTPIONT			"/hdd00/p01/"

#define FOCUSLOGDIR           	"/log"

#define LOGDISPLAYMAX			12

// 录象文件查找的模式
/*  从起点开始搜索  */
#define   SEARCH_SET			0  

/*从当前点向前搜索*/
#define  SEARCH_FORWARD			1
/*从当前点向前搜索,但不循环*/
#define  SEARCH_NOCYCLE			0xff

/*从当前点向后搜索*/
#define  SEARCH_BACKWARD		2

/*从最后一页开始查看*/
#define   SEARCH_END			3

typedef enum
{
	USER_EVENT_ALARM = 0x01,	//报警日志
	USER_EVENT_SYSTEM = 0x02,  // 系统日志
	USER_EVENT_FAULT	= 0x03,  // 系统故障
	USER_EVENT_ALARMEND = 0x04,  // 报警事件结束
	USER_EVENT_OPERATE	 = 0x05,  // 操作日志
	USER_EVENT_MOTION = 0x06,//移动侦测报警
}USERLOGTYPE;


typedef enum
{
	ALARM_SUBNULL = 0,
	ALARM_SENSORIN,  // 传感器输入
	ALARM_MD_START, //移动侦测开始
	ALARM_MD_END,  // 移动侦测结束
	ALARM_ZONE_IN, // 防区报警

	ALARM_MAX_COUNT,
	
}ALARMSUBTYPE;


typedef enum
{
	FAULT_SUBNULL = 0,
	FAULT_HDD_FULL, // 硬盘满
	FAULT_IPADDR_CONFLICT, // IP冲突
	FAULT_HDD_ERROR,//硬盘错误
	FAULT_ILLGEGAL_ACCESS, //非法访问
	FAULT_VIDEO_LOSS,		// 视频丢失

	FAULT_MAX_COUNT,
	
}FAULTSUBTYPE;

typedef enum
{
	OPERATE_SUBNULL = 0,
		
	/*
   		 下面列举操作类型 如设置网络，添加用户等
	*/


	OPER_MAX_COUNT,
	
}OPERATESUBTYPE;


typedef enum
{

	NULL_TYPE = 0,
	MD_ALARM,      	// 移动侦测
	FANGZONE_ALARM, //防区报警
	FANGZONE_RP,   	// 防区报告

}LOGALARMTYPE_E;


typedef struct 
{
	char 		 		m_DevType[12];
	unsigned long  		m_u32DevType;
	unsigned long		m_u32Version;
	char 				m_LogType[4];
	char				m_cDateTime[14];
	char				m_reserved[26];	
}LogFileHeader;


typedef struct 
{

#if 0
	unsigned char 		m_u8TvSystem;  //  制式
	unsigned char 		m_u8Resolution; // 清晰度
	unsigned char 		m_u8framerate; // 帧率
	unsigned char	 	m_u8SensorNum;// 传感器编号
	unsigned long		m_u32IOffset;  // 文件偏移
	unsigned char		m_cSensorName[9]; // 传感器名称
	unsigned char		m_u8ChannelNum; // 通道号
	unsigned char       m_u8AFlag;//开始事件：m_aflag＝00；//结束事件：m_aflag=0xff
	unsigned char 		m_u8Reserved[21];
#else 

	unsigned char		m_u8AlarmType; // 1 : 移动侦测 ，2   防区报警   3   防区报告 
	unsigned char       m_u8AFlag;	//开始事件：m_aflag＝00；//结束事件：m_aflag=0xff
	unsigned char  		m_u8AlarmNum;  //报警代号
	unsigned char	 	m_u8FangNum;   // 防区号
	unsigned char  		m_u8AStatus[8];  //  警情
	unsigned char  		m_u8AZoneNum;  //分区号 
	unsigned char		m_u8Reserved[27];
	
#endif

}UserAlarmEvent;

typedef struct 
{
	int			  		m_u32OperCode;	// 操作码
	unsigned char		m_u8Reseved[36];

}UserOperEvent;


typedef struct 
{
    int			  		m_u32FaultCode; // 错误码
    unsigned char		m_u8Reseved[36];

}UserFaultEvent;

typedef struct 
{
    int			  		m_u32SystemCode; // 系统码
    unsigned char		m_u8Reseved[36];

}UserSystemEvent;

typedef struct 
{
    int			  		m_u32CH; // 移动报警通道
    unsigned char		m_u8Reseved[36];

}UserMotionEvent;

typedef struct 
{
	unsigned char		m_u8EventType; // 事件类型'A'  ALARM ,'O' OPERATION  'F': FAULT . 'S' : system  'M' Motion
	unsigned char		m_u8EDay; // 天
	unsigned char		m_u8EHour;// 小时
	unsigned char		m_u8EMinute;// 分钟
	unsigned char		m_u8ESec;// 秒
	unsigned char		m_u8Reserved[3];
	union 
	{
		UserAlarmEvent 		m_AlarmEvent;
		UserFaultEvent		m_FaultEvent;
		UserOperEvent		m_OperEvent;
		UserSystemEvent		m_SystemEvent;
		UserMotionEvent		m_MotionEvent;
	}t_user_event;
	
}EventLogItem;



typedef struct 
{

	unsigned char  		m_u8Year; // 日志年
	unsigned char  		m_u8Month;//日志月
	unsigned char		m_u8Day; // 日志天
	unsigned char		m_u8LogType; //日志类型,报警，操作，故障，所有
	unsigned char		m_u8SearchType; //针对翻页操作
	unsigned char		m_u8Reserved;
	
}FindUserLogItem;//finduserlogterm;


typedef struct 
{
	unsigned char 		m_u8ListNum;//文件个数
	unsigned char		m_u8StartNum; // 第一成员在数组中的编号
	EventLogItem 		m_Item[LOGDISPLAYMAX];
}FindLogItems;


class C_LogManage
{
	private:

		datetime_setting     	m_DateTime;			/*系统时间*/

		unsigned int			m_u32UserLogOffset;

		int					 	m_u32LastAlarmCount;

		int					 	m_u32LastOperCount;

		int						m_u32LastFaultCount;

		int 					m_HddStatus;

		pthread_mutex_t 		m_LogWriteMutex;
		
		static C_LogManage		*m_pInstance;

		inline void InitLogWriteLock();

		inline void AddLogWriteLock();

		inline void ReleaseLogWriteLock();

		void  GetSystemTime();
		
		void CheckLogFileDateValid();

		int CheckLogDirExist(unsigned char type);

		int  GetLogFileDir(char *dirname, unsigned char type);

		void  GetUserLogFileHeader(LogFileHeader *header);
		
		void RemoveLogFileOrDir(char *filename);


	public :
		C_LogManage();
		
		~C_LogManage();

		static C_LogManage *Instance();
		
		int	  WriteUserLog(unsigned char type, void *EventCode);
		
		int   GetUserLogList(FindUserLogItem *findtype, unsigned char maxcout, FindLogItems *items);

		int	WriteErrLog(int errNum, int errPid);
		
};


#ifdef __cplusplus
}	
#endif 

#endif 
