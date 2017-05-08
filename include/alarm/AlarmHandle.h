
#ifndef __ALARM_HANDLE__

#define __ALARM_HANDLE__

#include "common.h"
#include "systemparameterdefine.h"

typedef enum
{
	ALARMVIOCE_OUT = 0,
	ONSCREENDISPLAY,
	SENDALARMCENTER,
	PTZ_LINK,
	OUTMODE_MAX,
}ALARMOUT_MODEE;

typedef struct
{
	unsigned char 		m_u8State; 
	unsigned char			m_u8ActionE;
	unsigned char 		m_u8AimId;
	unsigned char			m_u8Reserved;
		
}ChannelUionHandler;

typedef enum
{
	ALARM_INIT = 0,
	ALARM_START,
	ALARM_HANDLING,
	ALARM_END,
}ALARM_HANDLE_E;

typedef enum
{
	EX_HDDFULL = 0,
	EX_IPCONFLICT,
	EX_ILLIAGEACCESS,
	EX_HDDERROR,
	EX_VIDEOLOSS,
	
}EXCEPT_TYPE_E;

#define ALARM_OUTPUT_MAX					4

class  AlarmHandle
{

	private:
		static  AlarmHandle		*m_pInstance;
		int						m_s32ThreadStatus;	
		int 					m_nMainetanceInterval;
		int						m_nFirstDec;  
		int						m_nErrorAlarmOut;
		int						m_nErrorAlarmMode[8];  // 暂时支持8个异常处理 异常处理  0 组: 硬盘满  1 :  ip  冲突 2 非法访问 3  硬盘错误  4 视频丢失
		int						m_nVideoLoss;
		int						m_nExceptStatus;
		int						m_nAlarmInStatus;
		
		datetime_setting		m_ADateTime;
	
		unsigned char			m_uMdStatus[CHANNEL_MAX];

		unsigned char			m_u8ExAOutMode[CHANNEL_MAX][OUTMODE_MAX];

		unsigned char			m_u8MdAOutMode[CHANNEL_MAX][OUTMODE_MAX];

		unsigned short			m_u16AlarmOutDelay[ALARM_OUTPUT_MAX];
		
		unsigned int			m_u32AlarmRecValid;

		unsigned int			m_u32ExAlarmOut;

		unsigned int			m_u32MdAlarmOut;

		unsigned int			m_u32MdRecValid;

		unsigned int			m_u32VoiceAlarmDelay;

		unsigned int			m_u32MDVoiceAlarmDelay;//声音输出延时

		unsigned int			m_u32MDOutputDelay;//外接报警输出延时

		unsigned int			m_u32Voiceoutput;
		
		unsigned int			m_u32IOoutput;

		unsigned int			m_u32VideoLossVoiceAlarmDelay;//声音输出延时

		unsigned int			m_u32VideoLossOutputDelay;//外接报警输出延时

		unsigned int			m_u32VideoLossVoiceoutput;
		
		unsigned int			m_u32VideoLossIOoutput;

		unsigned int			m_u32VideoBlindVoiceAlarmDelay;//声音输出延时

		unsigned int			m_u32VideoBlindOutputDelay;//外接报警输出延时

		unsigned int			m_u32VideoBlindVoiceoutput;
		
		unsigned int			m_u32VideoBlindIOoutput;

		unsigned int			m_u32HdderrVoiceAlarmDelay;//声音输出延时

		unsigned int			m_u32HdderrVoiceoutput;

		unsigned int			m_u32AlarminVoiceoutput;
		
		unsigned int			m_u32AlarminIOoutput;

		unsigned int			m_u32OutputAlarmDelay;

		
		POWER_MANAGE			m_PowerManage;

		SYSTEMMAINETANCE		m_Mainetance;	

		GROUPEXCEPTHANDLE		m_ExceptHandle;

		GROUPALARMINSET			m_AlarmIn;
		
		CAMERA_MD				m_CameraMD;

		ALARMZONEGROUPSET		m_AlarmZoneSet;

		GroupZonePtzLinkSet		m_ZonePtzLinkSet;

		//DefenceScheduleSet		m_DefScheduleSet;

		ChannelUionHandler		m_UionHandle[MAX_ALARM_ZONE][CHANNEL_MAX];

		int						m_u32BuFangAction[8];

		int						m_u32CeFangAction[8];
		
		void HandleExternAlarmIn();

		void HandleMDAlarm();
		
		void HandleAlarmIn();

		void HandleSystemMainetance();

		void HandleExceptEvent();

		void HandleMachinePowerOnOff();

		void ShutDownMachineReady();

	
	public:	
		AlarmHandle();
		~AlarmHandle();

		static AlarmHandle*Instance();

		int AlarmHandleProcess();

		void ResetAlarmOut();

		void GetAlarmLinkRecStatus(unsigned int *AlarmRec, unsigned int *MdRec);

		

		void HandleDefenceAction();

		void HandlePictureTimerAction(int forinit);

		int CreateAlarmHandleThread();

		int GetExceptErrorAlarmOutStatus(int Type);

		int GetMDAlarmOutStatus(int ch , int type);

		void HandleVideoLoss();

		void HandleVideoBlind();

};



#endif 

