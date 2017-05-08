
#ifndef __MCU_COMMUNICATION_H__
#define __MCU_COMMUNICATION_H__

#include <pthread.h>
#include "common.h"
#include "ComTransmit.h"
#include "ptz.h"
#include "EncodeManage.h"
#include "HiIRApi.h"

#define   RCVMCURTC					0x81  // 接收实时时钟
#define   RCVMCUIRVALUE				0x82  // 接受IR 值
#define   RCVMCUDSP					0x83  // 接收显示字符命令
#define   RCVMCUWRKSTAT				0x84  // 接收工作状态
#define   RCVMCUOPRESULT			0x85  // 接收操作状态
#define   RCVMCULEDSTAT				0x86  // 接收LED状态
#define   RCVMCUFZONEALARM			0x87  // 接收防区报警

#define   MAX_IR_COUNT				3

//#define MAX_ALARM_COUNT			8
#define   MAX_ALARM_COUNT			MAX_REC_CHANNEL

#define   MCU_CMD_PTZOBJ			'C'


#define   MCU_CMD_KEY				'K'
#define   MCU_CMD_SHIFT				'S'
#define   MCU_CMD_RT				'T'
#define   MCU_CMD_RP				'I'
#define	  MCU_CMD_ALARM				'A'
#define   MCU_CMD_POWEROFF			'P'
#define   MCU_CMD_RECORD			'R'
#define	  MCU_CMD_STOPREC			'X'
#define	  MCU_CMD_ASTART			'Q'
#define   MCU_CMD_REPLY				'F'
#define   MCU_CMD_DEL				'D'
#define	  MCU_CMD_ZONEERROR			'H'

#define   MCU_CMD_PANEL_SPHONE 		'U'    // 专门针对中心、语音电话号码、系统选项
#define   MCU_CMD_PANEL_GPHONE 		'V'
#define   MCU_CMD_PANEL_SET			'Y'    // 写参数(除中心、语音电话号码、系统选项)
#define   MCU_CMD_PANEL_GET			'Z'

#define	  DVR_RPL_ALARM				'A'
#define	  DVR_RPL_POWEROFF			'P'
#define	  DVR_RPL_FANGACT			'C'
#define	  DVR_SET_TIME				'T'

#define	  DVR_VGA_SWITCH			'G'
#define	  DVR_SET_RFRING			0x89

//----------------------------------------------
//wxm 2010 03.26 电器开关
#define   MCU_CMD_SWITCH_STATUS		'b'
#define   MCU_CMD_SWITCH_END		'u'

#define   DVR_SWITCH_SAVE			'e'
#define	  DVR_SWITCH_REFRESH		'k'
#define	  DVR_SWITCH_COMM			'b'
#define	  DVR_SWITCH_TURN			'a'
//-----------------------------------------------

// 主机与DVR沟通的参数命令，取代主机键盘
// 1.密码设置
#define		PANELAttr_adminPwd		0x11	// 6 byte (012345)
//(请求电话拨进密码，双报警状态，防干扰状态)
#define		PANELAttr_DPA			0x12
// 2.联网设置
#define		PANELAttr_CmsPhone1		0x21
#define		PANELAttr_CmsPhone2		0x22
#define		PANELAttr_UserCode1		0x23
#define		PANELAttr_UserCode2		0x24
// 3.语音电话号码设置
#define		PANELAttr_VoiPhone1		0x31
#define		PANELAttr_VoiPhone2		0x32
#define		PANELAttr_VoiPhone3		0x33
#define		PANELAttr_VoiPhone4		0x34
// 4.系统设置
#define		PANELAttr_Option		0x40
// 5.无线对码
#define		PANELAttr_RemoteId		0x51
#define		PANELAttr_DetectId		0x52
// 6.防区设置
#define		PANELAttr_ZoneAttr		0x61
// 7.定时布撤防设置
#define		PANELAttr_Timer1		0x71
#define		PANELAttr_Timer2		0x72
#define		PANELAttr_Timer3		0x73
#define		PANELAttr_Timer4		0x74
// 8.系统维护
#define		PANELAttr_ClrLog		0x83
#define		PANELAttr_Default		0x84
//请求防区状态命令
#define		PANELAttr_ZoneStatus	0x48

typedef struct
{
	int rawdata;
    int sendata;
}IRData;

typedef struct
{
	int rawdata;
	int sendata;
}REMOTEDATA;

typedef struct
{
	unsigned char	  *m_pbuff;
	unsigned int	  m_u32Len;
	unsigned int	  m_u32RdPos;
	unsigned int	  m_u32WrPos;
}CommonRcvBuff;


typedef struct
{
	unsigned char 	m_cTxtContent[96];
	unsigned char	m_u8LineNum;
	unsigned char 	m_u8CursorPos;
}MCUCharaterInput;


typedef struct
{
	unsigned int 		m_u32KeyValue[MAX_IR_COUNT];
	unsigned char		m_u8WrPos;
	unsigned char	   	m_u8RdPos;
}REMOTEKEYBUF;

typedef struct
{
	unsigned char     m_u8WorkStatus;
	unsigned char	  m_u8OperateRply;
	unsigned char	  m_u8LedStatus;

}MCUWORKSATUS;

typedef struct
{
	unsigned char  m_u8AlarmNum;  //报警代号
	unsigned char  m_u8FangNum;   // 防区号
	unsigned char  m_u8AStatus[7];  //  警情
	unsigned char  m_u8CmdType; //命令类型
	unsigned char  m_u8AZoneNum;  //分区号 
	unsigned char  m_u8AlarmCH;
}ZoneAlarmStatus;

typedef struct
{
	int 		m_u8WriteCnt;
	int 		m_u8ReadCnt;
	ZoneAlarmStatus	m_AStatus[MAX_ALARM_COUNT];

}ZoneAlarmPool;

typedef struct
{
	unsigned char  m_u8AStatus[6];
	int 		status;
	int			timedelay;

}MDSend;

typedef enum
{
	OUTSIDE_BUFANG_E = 0,   /* 外出布防   */
	INSIDE_BUFANG_E,  /* 在家布防*/
	CEFANG_ACTION_E,  /*  撤防 */
	ALARM_ACTION_E,   /*  报警*/
	FANGZONE_FAULT_E, /*  防区故障*/
	FANGZONE_BYPASS_E, /*  防区旁路*/
	SYSTEM_ERROR_E,   /* 系统故障 */
	EXIT_BUDELAY_E,   /* 退出延时*/
}HOSTWORKSTATUS_E;

typedef struct 
{
	int 			sendflag;
	int 			m_Result;
	char			m_Path[64];
	char			m_Name[64];
	char			m_FileName[128];
	datetime_setting 	 m_DateTime;
	int			m_SendingPic;

}SnapImage;

class MCUCOMM
{

	private:
		CComTransmit		*m_pSerialComm;

		int 				m_nSerialFd;
	
		int					m_u8ParseState;

		int 				m_u8CMDId;

		int					m_u8CMDHdr;

		int 				m_u8CMDLen;

		int					m_u8CMDObj;
		
		int					m_u8CMDAddr;

		int 				m_u8ThreadCnt;

		int					m_nWeekDay;

		int					m_nRemoteRecord;

		int					m_nRemoteStopRec;

		int 				m_u8ShotPicFlag;

		int					m_u8AlarmZone;

		unsigned char			m_ZoneAlarmExist[MAX_ALARM_ZONE];
		unsigned char			m_ZoneAlarmExistForShot[MAX_ALARM_ZONE];

		REMOTEKEYBUF		m_RemoteKey;

		MCUWORKSATUS		m_McuWorkStatus;

		ZoneAlarmPool		m_AlarmPool;

		ZoneAlarmPool		m_AReportPool;

		ZoneAlarmPool		m_SGuiAlarm;

		ZoneAlarmPool		m_SGuiReport;

		ZoneAlarmPool		m_ScSAlarm;

		ZoneAlarmPool		m_ScSReport;

		ZoneAlarmPool		m_S3gAlarm;

		ZoneAlarmPool		m_S3gReport;
		
		ZoneAlarmStatus		m_AlarmSetPoolForUpdate;

		ZoneAlarmStatus		m_AlarmSend;
		//buffer 互斥锁
		pthread_mutex_t 	m_uSndMutex;
	
		static MCUCOMM		*m_pInstance;
		

		inline void InitSndLock()
		{
			pthread_mutex_init(&m_uSndMutex, NULL);
		}
		
		inline void AddSndLock()
		{
			pthread_mutex_lock(&m_uSndMutex);
		}
		
		inline void ReleaseSndLock()
		{
			pthread_mutex_unlock(&m_uSndMutex);
		}

		void InitCommPara();
		
		unsigned char CheckSum(unsigned char *buff, int len);

		void ParseRTCCMD(unsigned char *buffer, int len);

		void ParseDispCharaterCmd(unsigned char *buffer, int len);

		void ParseFangZoneAlarm(unsigned char *buffer, int len);

		void ParseIRCmd(unsigned char *buffer,  int len, int Type);

		void ParseMcuLedStat(unsigned char *buffer, int len);
		
		void ParseMcuOperateResult(unsigned char *buffer, int len);

		void ParseMcuWorkStatus(unsigned char *buffer, int len);

		void ParseMcuSubCMD(unsigned char *buffer, unsigned char CMDTYPE, int len);
		
		int ParseMCUCommand( unsigned char *buffer, int len);


		void ParseFangZoneReport(unsigned char *buffer, int len);

		void ParseFangZoneError(unsigned char *buffer, int len);		

		void ParseAutoSwitchStatus(unsigned char *buffer, int len);

		void ParseAutoSwitchEnd(unsigned char *buffer, int len);

		void ParseCMDPowerOff(unsigned char *buffer, int len);

		void ParseCMDStartRecord(unsigned char *buffer, int len);

		void ParseCMDStopRecord(unsigned char *buffer, int len);

		void ParseCMDStartAlarm(unsigned char *buffer, int len);
		
		void ParseCMDPanelSet(unsigned char *buffer, int len);
		
		void ParseCMDPanelGet(unsigned char *buffer, int len);

		void ParseCMDPanelSPhone(unsigned char *buffer, int len);
		
		void ParseCMDPanelGPhone(unsigned char *buffer, int len);

		void ParseCMDWriteMac(unsigned char *buffer, int len);

		int  SendMCUCmdAck(unsigned char *buff,  int length);


	public:

		MCUCOMM();
		~MCUCOMM();

		static  MCUCOMM* Instance();

		unsigned char		*m_pComBuff;

		CommonRcvBuff	  	m_CmdBuff;

		datetime_setting    m_datetime;

		datetime_setting	m_DateTimeBak;

		MCUCharaterInput  	m_CharaterInput;

		unsigned char		m_u8HostStatus;

		unsigned char 		m_nAckTimer;

		SnapImage		picforemail[CHANNEL_MAX];

		int 		InitMcuComm();

		int 		StartCommProcess();

		int 		SendCmd2Comm(unsigned char *buffer, int len);

		int 		StartMCUThread();

		int 		SetMcuRtc(datetime_setting datetime);
		
		int 		SendMcuShutDownCMD(int OffType);

		int			SendMcuRebootCMD(void);

		int 		SendMCUInfoSuccessACK();

		int 		SendMCUSwitchCtrlCmd(int No, int OnOff, int data2);

		int 		SendMCUBuzzerCtrl(int OnOff);

		void 		GetMcuRtc(datetime_setting *DateTime);

		int 		g3GetFangZoneAlarm(ZoneAlarmStatus * Status);

		int 		g3GetFangZoneReport(ZoneAlarmStatus * Status);

		int 		GetFangZoneAlarm(ZoneAlarmStatus *Status);

		int 		GetZoneAlarmReport(ZoneAlarmStatus *Status);

		int 		GuiGetFangZoneAlarm(ZoneAlarmStatus *Status);

		int 		GuiGetZoneAlarmReport(ZoneAlarmStatus *Status);

		int 		GetFangZoneUpload(ZoneAlarmStatus *Status);

		int 		GetRemoteRecordCtrl();

		unsigned int	GetRemoteCtrlKey();

		int 		SendBuFangCmd(int type, int stortype = -1);

		int 		SendAutoSwitchCmd(int type, int length , unsigned char *BUFF);

		int 		SendEAVSStatus(unsigned char CHN_STATUS, unsigned char EAVS_STATUS , unsigned char HDD_STATUS , unsigned char NET_STATUS);

		int 		GetPanelSetting(int mcmd, int subcmd);

		int 		GetAlarmZoneState(int  Num);

		int 		SendBackMCUAlarmInfo(ZoneAlarmStatus Status);
		
		int 		NetSendBackMCUAlarmInfo(ZoneAlarmStatus Status);

		int  		NetSendMCUCmdAck(unsigned char *buff,  int length);

		int 		StartShotPicThread();
		
		int			StartShotProcess();

		int 		GetAlarmInfo(ZoneAlarmStatus *Status);

		int  		SendMCUMDAck(unsigned int status, unsigned int delaytime);

		int		StartSerialProcess();

		int		CheckTimerTable(unsigned char port);

		int		GetAlarmZoneStateForShot(int  Num);


};


#endif 

