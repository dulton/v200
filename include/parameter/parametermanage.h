
#ifndef __PARAMETER_MANAGE_H__
#define __PARAMETER_MANAGE_H__

#include <pthread.h>

#include "common.h"
#include "systemparameterdefine.h"

#ifdef CONFIG_VAR
#define	PARA_FILENAME       "/var/config.dvr"
#else
#define	PARA_FILENAME       "/config/config.dvr"
#endif

#define SUPER_PASSWOD			0x999999

#define PARACONFIG_BASENAME		"config.dvr"
#define	EXTERN_PARACONFIG_PATH	"/usb/p01/sysconf/"

#define PARA_MAGIC_NUM			0x12345678		

typedef struct 
{
	int 	m_magic;  // Ä§Êý
	int		m_version; // °æ±¾
	char	m_reserved[40];

}PARACONFFILEHEADER;

class PARAMETER_MANAGE
{

private:
	int		m_config_fd;
	static PARAMETER_MANAGE *m_pInstance;

	pthread_mutex_t 	m_uWrMutex;

	inline void InitWriteLock()
	{
		pthread_mutex_init(&m_uWrMutex, NULL);
	}
	
	inline void AddWriteLock()
	{
		pthread_mutex_lock(&m_uWrMutex);
	}
	
	inline void ReleaseWriteLock()
	{
		pthread_mutex_unlock(&m_uWrMutex);
	}

public:	

	SYSTEM_PARAMETER	*m_syspara;

	PARAMETER_MANAGE();
	
	~PARAMETER_MANAGE();

	static PARAMETER_MANAGE *Instance();
	
	int GetMacAddr(SYSTEM_PARAMETER *para);
	
	int SaveParameter2File(SYSTEM_PARAMETER *para);
	
	int LoadParameterFromFile(SYSTEM_PARAMETER *para);

	int LoadParameterDefault(SYSTEM_PARAMETER *para);

	int LoadDeviceID(NETWORK_PARA *network);
	int LoadNetWorkDefault(NETWORK_PARA *network);

	int LoadSecurityDefault(PASSWORD_PARA *security);

	int LoadRecTaskDefault(REC_TASK_PARA *task);

	int LoadMachineParaDefault(MACHINE_PARA *machine);

	int LoadCameraSetDefault(CAMERA_PARA *cameraset);
	
	int LoadAnalogDefault(CAMERA_ANALOG *analog);

	int LoadAlarmSetDefault(ALARM_PARA *alarm);

	int LoadSensorSetDefault(SENSOR_PARA *sensor_alarm);

//	int LoadVechileSetDefault(VECHIL_SETTING *vechile);

	int LoadPowerManageDefault(POWER_MANAGE *pw_manage);

	int LoadNormalParaDefault(COMMON_PARA *common_para);
	
	int LoadCameraBlindDefault(CAMERA_BLIND *bcd);

//	int LoadDefaultTrainParameter(GUARD_RUN_SETUP *guard_setup);

	int LoadDefaultMDParameter( CAMERA_MD *md_set);

	// add by HY
	int LoadDefaultVideoLossParameter( CAMERA_VideoLoss *vl_set);

	int LoadDefaultBDParameter( CAMERA_BLIND *bd_set);

	int LoadAlarmInParameter(ALARMZONEGROUPSET *ai_set);
	// add by HY end

	int LoadSysRunInfoDefault(SYSTEM_RUNINFO *systeminfo);

//	int LoadDefaultTrainRunParamemter(TRAIN_RUNPARA_SETUP  *train_runpara);

	int LoadDefaultUserParameter(USERGROUPSET  *UserSet);

	int LoadDefaultSysMainetanceParaMeter(SYSTEMMAINETANCE *Mainetance);

	int LoadDefaultDisplaySetParameter(VIDEODISPLAYSET *DisplaySet);
	
	int  LoadDefaultExceptHandleParameter(GROUPEXCEPTHANDLE *ExceptHandle);

	int LoadDefaultPcDirParameter(PCDIR_PARA *pcdir);
	
	int LoadDefaultOsdInsertParameter(VIDEOOSDINSERT *OsdInsert);

	int LoadDefaultAlarmInParameter(GROUPALARMINSET *AlarmIn);

	void LoadAlarmZoneParameter(ALARMZONEGROUPSET *AlarmZone);

	void LoadRecordTaskDefaultParameter(GROUPRECORDTASK *RecTask);
	
	void LoadPTZParameter(PTZ_PARA *ptzpara);

	void LoadDefaultPELCOParameter(PELCO_CmdCfg *pelcopara);
	
	void LoadDefaultPicTimerParameter(PICTURE_TIMER *picTmrpara);
	
	void LoadDefaultVoPicParameter(VODEV_ANALOG *voPicpara);

	void LoadMegaSetDefault(MegaEyes_PARA *MegaSet);

	void LoadPtzLinkSet(GroupZonePtzLinkSet *PtzLink);

	void LoadCamSensorParameter(CAMERASENSOR_PARA	*pSensor);
	int LoadExtendApSettings(PARAMETEREXTEND *pApSettings);
	int LoadWebSetDefault(web_sync_param_t *pweb);
	int LoadMdSetDefault(P2P_MD_REGION_CHANNEL *pmd);

	void CheckNetWorkParameter(NETWORK_PARA *network);

	void CheckSecurityParameter(PASSWORD_PARA *pswd);

	void CheckRecTaskParameter(REC_TASK_PARA *Task_Para);

	void CheckMachineParameter(MACHINE_PARA *machine);

	void CheckCameraSetParameter(CAMERA_PARA *camera);

	int CheckAnalogParameter(CAMERA_ANALOG *analog);

	void CheckAlarmOutParameter(ALARM_PARA *alarm_out);

	void  CheckSensorParameter(SENSOR_PARA *sensor);

//	void CheckVechileParameter(VECHIL_SETTING *vechile);

	void CheckPowerManageParameter(POWER_MANAGE *pw_manage);

	int CheckCommonParameter(COMMON_PARA *comm_para);

	int CheckCamerBlind(CAMERA_BLIND *bcd);

	int CheckCameraMotionDetection(CAMERA_MD *md_set);

//	int CheckTrainGuardParameter(GUARD_RUN_SETUP *guard_setup);

//	int CheckTrainRunPara(TRAIN_RUNPARA_SETUP *TrainRun_Para);

	int CheckSystemRunInfo(SYSTEM_RUNINFO *runinfo);

	int CheckSystemParameter(SYSTEM_PARAMETER *para);

	int  CheckPTZParameter(PTZ_PARA  *ptzpara);

	int CheckPELCOParameter(PELCO_CmdCfg *pelcopara);
	
	int CheckPicTimerParameter(PICTURE_TIMER *picTmrpara);
	
	int CheckVoPicParameter(VODEV_ANALOG *voPicpara);

	void CheckOsdInsertParameter(VIDEOOSDINSERT *OsdInsert);
	
	void CheckPcDirParameter(PCDIR_PARA *pcdir);

	void CheckSysExceptParameter(GROUPEXCEPTHANDLE *ExceptHandle);

	void CheckDisplaySetParameter(VIDEODISPLAYSET *DisplaySet);

	void CheckSysMainetanceParameter(SYSTEMMAINETANCE *mainetance);

	void CheckUserGroupParameter(USERGROUPSET  *UserSet);

	void CheckAlarmInParameter(GROUPALARMINSET *AlarmIn);

	void CheckRecordTaskParameter(GROUPRECORDTASK *RecordTask);
	
	int CheckMegaSet(MegaEyes_PARA *MegaSet);

	int  CheckAlarmZoneParameter(ALARMZONEGROUPSET *AlarmZone);

	//int CheckDefenceScheduleParameter(DefenceScheduleSet *DefSchedule);

	int CheckPtzLinkParameter(GroupZonePtzLinkSet  *PtzLink);

	int CheckExtendApSettings(PARAMETEREXTEND *pExtendApSettings);
	int CheckWebSettings(web_sync_param_t *pweb);
	
	int CheckMdSettings(P2P_MD_REGION_CHANNEL *pmd);
	int CheckCamSensorParameter(CAMERASENSOR_PARA 	*pSensor);
	int ExportOutAllPara();

	int ExportInAllPara(SYSTEM_PARAMETER *para);

	int  SaveSystemParameter(SYSTEM_PARAMETER *para);

	int ReadSystemParameter(SYSTEM_PARAMETER *para);

	SYSTEM_PARAMETER *GetParameterPtr();

	int LoadDefaultParaToFile(SYSTEM_PARAMETER *para);

	int GetSysParameter(int type,  void* para);

	int GetSingleParameter(int type, void * value);

	int SetSystemParameter(int type,  void* para);

};


#endif 

