

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <queue>

#include <sys/mman.h>
#include <linux/fb.h>
//#include <asm/page.h>
#include <assert.h>

#include "common.h"


#include "EncodeManage.h"

#include "GpioApi.h"
#include "VideoEncode.h"
#include "hi_type.h"
#include "ModuleFuncInterface.h"
#include "RecordManage.h"
#include "RealTime.h"
#include "DebugPrint.h"
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#include "Video_MD.h"
#include "pppoe.h"

#include "zmdntpclient.h"
#include "IR_Cut.h"
//#include "bitrate_adjust.h"
#include "aes.h"
#include "md5File.h"
#include "uuid.h"
#include "AdapterMessage.h"

#define   PROCESSING				0xf8f8f8f8
#define   PROCESS_START			0x12345678
#define 	PROCESS_END				0x56565656

extern Audio *paudio;

extern int		SystemEncodeIsReboot;
extern VideoEncode 		*pVideo;
 
PARAMETER_MANAGE*	g_cParaManage = NULL;
extern BufferManage 		*pBufferManage[MAX_REC_CHANNEL];

//RECORDFILEHEADER*	g_cRecFileHeader = NULL;

DeviceConfigInfo 	ConfigInfo;

int  					g_nMainThreadId = 0;

int			NetLevel=4;

int						g_nCheckFsStep = 0;
bool 				g_bFormatSDCard = false;
extern int startupdate;
extern int NeedRebootEncode;
int 		FrameRate =25;
bool		Night =		false; 
bool 		device_on = true;
int			SETNTPTIME = 0;
extern HumanDetHandle  phd;

static  web_schedule schedule_config;
extern void  OnDeviceOnline(int status);  
int isFlip = 0;
int GetSoftWareVersion(TYPE_DEVICE_INFO *version)
{
	NETWORK_PARA netset;
	char mac[64]={0x0};
	memset(version,0x0,sizeof(TYPE_DEVICE_INFO));
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
#ifdef WIFI_18E_IPC
	sprintf(mac,"%02x%02x%02x%02x%02x%02x",netset.m_WifiConfig.WifiAddrMode.m_uMac[0],
	        netset.m_WifiConfig.WifiAddrMode.m_uMac[1], netset.m_WifiConfig.WifiAddrMode.m_uMac[2],
	        netset.m_WifiConfig.WifiAddrMode.m_uMac[3], netset.m_WifiConfig.WifiAddrMode.m_uMac[4],
	        netset.m_WifiConfig.WifiAddrMode.m_uMac[5]);
#else
	sprintf(mac,"%02x%02x%02x%02x%02x%02x",netset.m_Eth0Config.m_uMac[0],netset.m_Eth0Config.m_uMac[1],
	        netset.m_Eth0Config.m_uMac[2],netset.m_Eth0Config.m_uMac[3],netset.m_Eth0Config.m_uMac[4],
	        netset.m_Eth0Config.m_uMac[5]);
#endif
    version->DeviceType = DEV_TYPE_INFO;
	memcpy(version->DeviceName,ConfigInfo.DeviceName,32);
	memcpy(version->SerialNumber,mac,32);
	memcpy(version->HardWareVersion,ConfigInfo.HardWareVersion,16);
	memcpy(version->SoftWareVersion,ConfigInfo.AppVersion,16);
	version->VideoNum = ConfigInfo.Channel;
	version->AudioNum = (ConfigInfo.SupportInfo&(CONFIG_AUDIO))>0?1:0;
	version->SupportAudioTalk = (ConfigInfo.SupportInfo&(CONFIG_INTERCOM))>0?1:0;
	version->SupportStore = (ConfigInfo.SupportInfo&(CONFIG_RECODE))>0?1:0;;
	version->AlarmInNum = ALARMINNUM;
	version->AlarmOutNum = ALARMOUTNUM;
	version->SupportWifi = (ConfigInfo.SupportInfo&(CONFIG_WIFI))>0?1:0;
	version->LocalDeviceCapacity |=(1<<0); //支持移动侦测区域设置接口
	/*
	resver:
	bit0:
	bit1:
	bit2-bit4:升级方式10100,90S
	*/
	/*Support onvif*/
	if( (ConfigInfo.SupportInfo&(CONFIG_ONVIF))>0)
		version->resver|=(1<<0);
	/*Support Encrypt*/
	//if( (ConfigInfo.SupportInfo&(CONFIG_ENC))>0)
		version->resver|=(1<<1);/*加密为通用功能，所有版本都支持加密*/
	
	version->resver|=(1<<2); /*新升级方式*/
	version->resver|=(0<<3); 
	version->resver|=(1<<4); /*升级时间90s*/
	version->resver|=(1<<5); /*支持广播开启设备DHCP*/
	#ifdef VIDEO_AES_ENC
	version->resver|=(1<<7);//视频加密
	
	#endif
	
	/*设备功能支持*/
	//if( (ConfigInfo.SupportInfo&(CONFIG_P2P))>0)
		version->DeviceType |= (1<<8); /*P2P支持现在默认都支持*/
	if( (ConfigInfo.SupportInfo&(CONFIG_PT))>0)
		version->DeviceType |= (1<<9); /*PZ支持*/
	if( (ConfigInfo.SupportInfo&(CONFIG_ZOOM))>0)
		version->DeviceType |= (1<<10); /*ZOOM支持*/
	if( (ConfigInfo.SupportInfo&(CONFIG_INTERCOM))>0)	
		version->DeviceType |= (1<<11); /*对讲支持*/	
	if( (ConfigInfo.SupportInfo&(CONFIG_WIFI))>0)
		version->DeviceType |= (1<<12); /*wifi支持*/
	//printf();
	return 0;
}


//重启系统
void RebootSystem()
{
	wifi_disable();
	sleep(3);
	printf("send kill to system\n");
	kill(1, SIGTERM);
	return ;
}
void SystemResourceExit()
{
	if( (ConfigInfo.SupportInfo&(CONFIG_RECODE))>0)
	{
		StopRecordSystem();
	}
	
	return ;
}



/*进程与线程信号处理函数*/
void SignalHandler(int signum)
{
	int pid = 0;

	
	pid = (unsigned)pthread_self();	

	for(int i = 0; i < 5; i ++)
	{
		if(signum != 17)
		printf("SignalHander(signal:%d)(pid:%d)\n", signum, pid);
	}
	if(signum == 11)
	sleep(3);

	//fprintf(stderr,"Thread id is [%d] Receive Signal %s\n", pid,strsignal(signum));


	/*
	if((pid <encpid)
		&&(pid != g_nMainThreadId)
		)
	{
		printf("thread  pid no find : %d mask signalhandler \n", pid);
		return;
	}
	*/
	
//	Sysdebug_msg("Thread id is [%d] Receive Signal %s", pid,strsignal(signum));

	
	switch( signum)
	{
		case SIGCHLD:
			
			break;
			
		case SIGSEGV:
			
			RebootSystem();
		//	SetRecordCtrlThread(THREAD_PAUSE);
		//	CloseAllChannelRecord();
			break;
			
		case SIGILL:	
		case SIGFPE:
			break;
				
		case SIGINT:
		case SIGTERM:
		case SIGKILL:	
			exit(0);
			break;
			
		case SIGABRT:
		case SIGSTOP:
			exit(0);
			break;

		default:

			break;	

	}
	
}



/*******************************************************************************
设置各种抓取信号的函数
*********************************************************************************/	   
void SignalRegister()
{
	//系统信号处理
	signal(SIGINT,SignalHandler);	

	signal(SIGQUIT,SignalHandler);

	signal(SIGABRT,SignalHandler );

	signal(SIGFPE,SignalHandler );
	
	signal(SIGILL,SignalHandler);

	signal(SIGKILL,SignalHandler);

	signal(SIGSEGV,SignalHandler);

	signal(SIGSEGV,SignalHandler);

	signal(SIGPIPE,SignalHandler);

	signal(SIGTERM,SignalHandler );

	signal(SIGCHLD,SignalHandler);
	

	signal(SIGSTOP,SignalHandler);

	signal(SIGUSR1,SignalHandler);	

	signal(SIGUSR2,SignalHandler);

	
/*
       SIGHUP        1       Term    Hangup detected on controlling terminal
                                     or death of controlling process
       SIGINT        2       Term    Interrupt from keyboard
       
       SIGQUIT       3       Core    Quit from keyboard
       SIGILL        4       Core    Illegal Instruction
       SIGABRT       6       Core    Abort signal from abort(3)
       SIGFPE        8       Core    Floating point exception//
       SIGKILL       9       Term    Kill signal
       SIGSEGV      11       Core    Invalid memory reference
       SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
       SIGALRM      14       Term    Timer signal from alarm(2)
       SIGTERM      15       Term    Termination signal
       SIGUSR1   30,10,16    Term    User-defined signal 1
       SIGUSR2   31,12,17    Term    User-defined signal 2
       SIGCHLD   20,17,18    Ign     Child stopped or terminated
       SIGCONT   19,18,25            Continue if stopped
       SIGSTOP   17,19,23    Stop    Stop process
       SIGTSTP   18,20,24    Stop    Stop typed at tty
       SIGTTIN   21,21,26    Stop    tty input for background process
       SIGTTOU   22,22,27    Stop    tty output for background process	
*/  

}




/*************************************************
  功能描述    
  			检测与修复文件系统子进程退出时
  			通知父进程的执行函数入口
   参数:          
   			参数signum 为信号值 
   			参数si为信号信息结构体
   			
  			
**************************************************/

void ChkFsSigchldHandler(int signum, siginfo_t *si, void *p)
{
	int status;

	if(signum != SIGCHLD)
	{
		printf("error signal send handler : %d \n", signum);
		return;
	}

	if (si != NULL) 
	{
		printf("signo: %d\n", si->si_signo);
		printf("errno: %d\n", si->si_errno);
		printf("code : %d\n", si->si_code);
		printf("pid  : %d \n", si->si_pid);
	}
	
	waitpid(si->si_pid, &status, 0);	
	printf("process exit status : %d \n", status);
	
	g_nCheckFsStep = PROCESS_END;
	
}

/*获得系统时间的函数*/
void  GlobalGetSystemTime(datetime_setting *datetime)
{	
	GetSystemTime((SystemDateTime *)datetime);
}






/*获得硬盘是否存在*/
/*
int GetHDDExistStatus()
{
	int retval = 0;
	DIR *dir = NULL;

	RECORDMODULE		*RecordModule = NULL;

	RecordModule = RECORDMODULE::Instance();

	if ( NULL == (dir = opendir(SD_DEVICE_EXSIT) ))
	{
		DPRINTF("hdd no exit\n");
		return retval;		
	}
	else
	{
		closedir(dir);
		dir = NULL;
	}
	retval = RecordModule->m_uHddStatus;
	
	return retval;
}

void  GetHddWorkSatus(int *FullSatus, int *ErrorStatus)
{

	RECORDMODULE		*RecordModule = NULL;

	RecordModule = RECORDMODULE::Instance();
	
	RecordModule->GetHDDWorkStatus(FullSatus, ErrorStatus);
}

*/
/*检测硬盘容量，看是否需要删除*/
/*
int Check_HDD_Status(int *CurHdd)
{
	int retval = -1;
	C_FILEMANAGE	*FileManage = NULL;

	
	FileManage = C_FILEMANAGE::Instance();
	if(FileManage != NULL)
	{
		retval = FileManage->Check_HDD_Status(CurHdd);
	}

	return retval;
}
*/

/*获得编码参数*/
void GetEncodePara(CAMERA_PARA *para)
{	
	if((para) && (g_cParaManage))
	{
	//	syspara = g_cParaManage->GetParameterPtr();
	//	dprintf("channel  valid : %x  res : %d \n", syspara->m_Camera.m_uChannelValid, syspara->m_camera.m_channel_para[0].m_resolution);
	//	memcpy(para, &(syspara->m_Camera), sizeof(CAMERA_PARA));
		g_cParaManage->GetSysParameter(SYSCAMERA_SET, (void *) para);
		
	}

}





/*检测移动硬盘存在状态*/
/*
void CheckRemovableBlkDeviceStatus()
{
	BlockDevManage *BlockManage = NULL;
	int retval = -1;

	BlockManage = BlockDevManage::Instance();

	if(BlockManage)
	{
		retval = BlockManage->ScanuDiskChange();
	}
	
}
*/

/*
获得移动硬盘文件系统的状态
返回0:代表存在，1:代表不存在
*/
/*
int GetRemovableDeviceStatus()
{
	int retval = S_FAILURE;
	BlockDevInfo_S Info ;

	GetBlockDeviceInfo(BLOCKDEVICE_UDISK1, &Info);

	if(Info.m_u8Exist == 2)
	{
		retval = S_SUCCESS;
	}
	return retval;

}
*/





/*保存系统参数*/
void PubSaveSystemParameter(SYSTEM_PARAMETER *para)
{
	if((para) && (g_cParaManage))
	{
		g_cParaManage->SaveSystemParameter(para);

	//	g_cRecordModule->init(1); //重新获得参数
	
	}
	
}


/*从FLASH 中加载系统参数*/
void LoadSystemDefaultParameter(SYSTEM_PARAMETER *para)
{

#if 1
	if((para) && (g_cParaManage))
	{
		g_cParaManage->LoadDefaultParaToFile(para);
		sleep(1);
		system("/sbin/reboot");

	}
#endif

}
void RestoreDefault(void *para)
{

#ifdef CONFIG_VAR
	system("rm /var/config.dvr /var/p2p_config.ini");    
#else
	system("rm /config/wpa_supplicant.conf /config/config.dvr /config/p2p_config.ini");	
#endif
	sleep(3);
	//LoadSystemDefaultParameter((SYSTEM_PARAMETER *)para);
	PTZ_Operate(0x00,CMD_SET_RESET,0,0);

	RebootSystem();
}

SYSTEM_PARAMETER *PubGetParameterPtr()
{
	SYSTEM_PARAMETER *para = NULL;
	
	if(g_cParaManage)
	{
		para = g_cParaManage->GetParameterPtr();
	}

	return para;
}

void PubReadSystemParameter(SYSTEM_PARAMETER *para)
{

	if((para) && (g_cParaManage))
	{
		g_cParaManage->ReadSystemParameter(para);
	}

}





/*重起编码，其功能包括停止编码，启动编码*/
void  RestartEncodeSystem()
{
		if(SystemEncodeIsReboot == 1)
		{
			return ;
		}
		SystemEncodeIsReboot = 1;
		StopEncodeSystem(1, 1);
		sleep(2);
		StartEncodeSystem();
		SystemEncodeIsReboot = 0;
		NeedRebootEncode = 0;
		printf("The Encode  Restart Complete\n");

}



/*获得选中通道录像文件名*/

int  GetCurrentRecordFile(int ch, char *filename)
{
	int retval = S_FAILURE;

#if 0
	if((filename) && (g_cRecordModule))
	{
		retval = g_cRecordModule->GetCurrentRecordFile(ch, filename);
	}

#endif 

	return retval;
}


/*判断指定文件是否存在*/
int JudgeFileExist(char *filename)
{
	int fd = -1;

	fd = open(filename, O_RDONLY);
	if(fd == -1 )
	{

		return S_FAILURE;
	}
	
	close(fd);
	fd = -1;
	
	return S_SUCCESS;
}



/*判断指定目录是否存在*/
int JudgeDirExist(char *dirname)
{
	DIR *dir = NULL;

	//	return S_FAILURE;
//	dprintf("dir name : %s \n", dirname);
	dir = opendir(dirname);
	if(dir != NULL)
	{
		closedir(dir);
		dir =NULL;
		return S_SUCCESS;
	}	

	return S_FAILURE;
}




void LoadTestParameter()
{
#if 0
	if(g_cParaManage)
	{
		g_cParaManage->LoadTestParameter();
	}
#endif 
}

/*
unsigned int GetCurrentMonthRecordStatus( int ide_num, FindFileType *findType)
{
	int ret = 0;
	C_FILEMANAGE	*FileManage = NULL;

	FileManage = C_FILEMANAGE::Instance();
	
	if(FileManage)
	{
		ret = FileManage->ScanAllHDDDateStatus(findType);
	}

	return ret;
}

int AddRecordFileList(char *filename)
{
	int ret = -1;
	C_FILEMANAGE	*FileManage = NULL;

	FileManage = C_FILEMANAGE::Instance();
	
	if(FileManage)
	{
		ret = FileManage->AddOneList2Filelist(filename);
	}

	return ret;
	
}


int RenameRecordFile(char* Filename, int Mode, int RecType, int Flag)
{
	int ret = S_FAILURE;


	C_FILEMANAGE *FileManage = NULL;
	FileManage = C_FILEMANAGE::Instance();
	
	if(FileManage)
	{
		ret = FileManage->ModifyRecFileNameFromIndexFile(Filename, Mode, RecType, Flag);
	}

	return ret;
}

int SetAlarmFileAlarmCode(int ch)
{
	int ret = S_FAILURE;


	C_FILEMANAGE *FileManage = NULL;
	FileManage = C_FILEMANAGE::Instance();
	
	if(FileManage)
	{
		ret = FileManage->AlarmFileAlarmCode(ch);
	}

	return ret;
}

int PubCreatRecordFilelist(char * path)
{
	int ret = -1;
	C_FILEMANAGE	*FileManage = NULL;

	FileManage = C_FILEMANAGE::Instance();
	
	if(FileManage)
	{
		ret = FileManage->CreatRecordFilelist(path);
	}

	return ret;
	
}

*/
int  FindRecordFile(FindFileType *findType,RecordFileName *RecordFile, int mode, int searchmax)
{
    REC_SEARCH enMode = REC_SEARCH_SET;
    
    if(mode != 0)
        enMode = REC_SEARCH_NEXT;
    
     return FindRecordFile(findType,RecordFile,enMode);
}
/*
int  SetRecordFile(char *filename, int mode)
{
	int ret = -1;
	C_FILEMANAGE	*FileManage = NULL;

	FileManage = C_FILEMANAGE::Instance();
	
	if(FileManage)
	{
		ret = FileManage->ModifyRecFileStatusFromIndexFile(filename,mode);
	}

	return ret;
}

#if 1
void SendRemoteKey(unsigned int key)
{

	m_nGlobalKey = key;
	
	m_nKeyLeft = 1;

}

unsigned int ReadRemoteKey()
{
	unsigned int ret = RAW_INVALID_KEY;

	MCUCOMM *pMcuCom = NULL;

	pMcuCom = MCUCOMM::Instance();

	
		
	return ret;	

}
#endif 

*/

int  ExportInOrOutSysParameter(int mode)
{
	int retval = S_FAILURE;
	
	if(g_cParaManage)
	{
		if(mode)
		{ // 参数导入
			retval = g_cParaManage->ExportInAllPara(g_cParaManage->m_syspara);
		}	
		else 
		{ // 参数导出
			retval = g_cParaManage->ExportOutAllPara();
		}
	}

	return retval;
}



#if 1
char * LeftTrim(char *str)
{
	int  len = strlen(str);

	if(len <= 0)
	{
		return NULL;
	}
	while ((len > 0) && ((*str == ' ') ||(*str == 0x09) ||(*(str) == '\x0a') ||(*(str) == '\x0d')))
	{
		str++;
		len--;
	}
	
	if(len > 1)
	{
		return str;
	}
	else 
	{
		return NULL;
	}
	
}

void RightTrim(char *str)
{	
	int  len = strlen(str);

	while ((len > 0) && 
		((*(str + len - 1) == ' ') ||(*(str + len-1) == '\x0a') ||(*(str + len - 1) == '\x0d'))) /*LF/CR*/
	{
		len--;
	}
	
	*(str + len) = '\0';
}


unsigned char CaculateWeek(unsigned char day, unsigned char month , unsigned char year)
{
	 unsigned short tmp;
	// unsigned char year,month,day;
	 unsigned short month_max[]={
		 0,
		 0,
		 31,
		 31+28,
		 31+28+31,
		 31+28+31+30,
		 31+28+31+30+31,
		 31+28+31+30+31+30,
		 31+28+31+30+31+30+31,
		 31+28+31+30+31+30+31+31,
		 31+28+31+30+31+30+31+31+30,
		 31+28+31+30+31+30+31+31+30+31,
		 31+28+31+30+31+30+31+31+30+31+30,
		 31+28+31+30+31+30+31+31+30+31+30+31
		 };
 

	 tmp = 6+year+(3+year)/4+month_max[month]+day-1;

	 if ((year%4 == 0) && (month>2))
	 	tmp = tmp+1;
	 
	 return tmp%7;
	 
 }



 unsigned char PubGetMaxDate(int month , int year)
 {
	 unsigned char maxdaytable[]={31,28,31,30,31,30,31,31,30,31,30,31};
	 unsigned char tmp;  

	 if ((month > 0) && (month < 13))
	 {
		 tmp = maxdaytable[month-1];
		 if ((month == 2) && (year%4 == 0)) 
		 	tmp = 29;
	 }
	 else 
	 {
		tmp = 0xff;
	 }
	 
	 return tmp;
 }    


#endif 


#if 0
void CheckUdiskWorkLoop()
{
	int count = 0;
	int firstmount = 0;
	int pid = 0;
	
	pid = getpid();

	printf("Get Udisk work thread id : %d  \n", pid);
	
	while(1)
	{

/*
		if(PubJudgeOsdStatus(OSD_FORMAT_HDD_MODE))
		{
			sleep(1);
			continue;
		}
*/		
		count++;
		if(count >= 50)
		{

			count = 0;
			CheckRemovableBlkDeviceStatus();
		}

		
		
		if(GetRemovableDeviceStatus() == S_SUCCESS)
		{
			if(firstmount == 0)
			{
			
				firstmount = 1;
				DupSysLog2uDisk();

			}
			
		}
		else 
		{
			firstmount = 0;
		}
		
		usleep(200000);

	}

	pthread_exit(0);
		
}

void *CheckUdiskWork(void *arg)
{
	CheckUdiskWorkLoop();

	return NULL;
}

void StartUdiskEvent()
{
	pthread_t  pid = 0;

	if(pthread_create(&pid, NULL, CheckUdiskWork, (void*)&g_nUdiskState) < 0)
	{
		printf("check udisk status thread failure \n");
	}

}

#endif 



int CopyFile2File(const char *source, const char *dest)
{
	int srcfd = -1, desfd = -1;
	struct stat  src_stat;
	int retval = S_FAILURE;
	char  *buffer = NULL;
	int len = 0;

	srcfd = open(source, O_RDONLY);
	if(srcfd == -1)
	{
		return S_FAILURE;
	}
	
	stat(source, &src_stat);
	
	DPRINTF(" desfile name : %s \n", dest);

	desfd = open(dest , O_RDWR|O_CREAT, 0777);
	if(desfd == -1)
	{
		goto exit_copy;
	}

	buffer = (char *)malloc(sizeof(char)*32768);

	if(buffer == NULL)
	{
		goto exit_copy;
	}

	while(1)
	{
		len = read(srcfd, buffer, 32768);
		if(len <= 0)
		{
			retval = len;
			DPRINTF("public read src file exit: %d \n", retval);
			break;
		}
		
		retval = write(desfd, buffer, len);
		if(retval <= 0)
		{
			printf("write des file exit :%d  \n", retval);
			break;
		}
		
	}
	

exit_copy:

	fsync(desfd);

	if(srcfd != -1)
	{
		close(srcfd);
		srcfd = -1;
	}
	
	if(desfd != -1)
	{
		close(desfd);
		desfd = -1;	
	}
	
	if(buffer != NULL)
	{
		free(buffer);
		buffer = NULL;
		
	}	
	return retval;
	
}



/*  同步系统时钟*/
void PubSyncSytemTime( datetime_setting datetime)
{
	struct tm t;
	int ret=0;
	time_t tp;
	t.tm_year = datetime.year + 100;
	t.tm_mon = datetime.month - 1;
	t.tm_mday = datetime.day;
	t.tm_hour = datetime.hour;
	t.tm_min = datetime.minute;
	t.tm_sec = datetime.second;
	tp = mktime(&t);
	ret = stime(&tp);
	if(ret != 0)
	{
		printf("set sys date failed !\n");
	}
	else
	{
		printf("set sys date success\n");
	}

}

void WaitForSystemTime()
{

	datetime_setting  systime;
	memset(&systime, 0, sizeof(datetime_setting));

	rtc_time_t loadTm;
	memset(&systime, 0, sizeof(datetime_setting));
	if(DS_RTC_Open() < 0)
	{
		printf("Open misc rtc file failure!\n");
	//	return ;
	}
	#if 0
	loadTm.date = 19;
	loadTm.hour = 9;
	loadTm.minute = 59;
	loadTm.month = 4;
	loadTm.second = 00;
	loadTm.weekday = 0 ;
	loadTm.year = 2012 - 2000;
	DS_RTC_SetTime(&loadTm);
	#endif
	
	DS_RTC_GetTime(&loadTm) ;
	systime.second = loadTm.second;
	systime.minute = loadTm.minute;
	systime.hour = loadTm.hour;
	systime.week = loadTm.weekday;
	systime.day = loadTm.date;
	systime.month = loadTm.month;
	systime.year = loadTm.year;
	VerifySystemTime(&systime);
printf("loadTm ---------- year = %d,mon = %d,date = %d,hour = %d,min = %d,sec = %d week %d \n",
			loadTm.year,loadTm.month,loadTm.date,loadTm.hour,loadTm.minute,loadTm.second, loadTm.weekday);
printf("realtime---------- = %d,mon = %d,date = %d,hour = %d,min = %d,sec =  %d \n",
			systime.year,systime.month,systime.day,systime.hour,systime.minute,systime.second);
	PubSyncSytemTime(systime);
}
void VerifySystemTime(datetime_setting  *psystime)
{
	if(psystime == NULL)
		return ;
	datetime_setting lasttime;
	memset(&lasttime,0x0,sizeof(datetime_setting));
	char buffer[32] = {0x0};
	sprintf(buffer,"%s%s",GetConfigDir(),"lasttime");
	int fd = open(buffer, O_RDWR);
	if(fd <0)
	{
		return ;
	}
	if(sizeof(datetime_setting)!= read(fd,&lasttime,sizeof(datetime_setting)))
	{
		return ;
	}
	if(lasttime.year > psystime->year)
	{
		goto RET;
	}
	if((lasttime.month> psystime->month)||(psystime->month>12))
	{
		goto RET;
	}
	if((lasttime.day> psystime->day)||(psystime->day>31))
	{
		goto RET;
	}
	if((lasttime.hour> psystime->hour)||(psystime->hour>12))
	{
		goto RET;
	}
	if((lasttime.minute> psystime->minute)||(psystime->minute>59))
	{
		goto RET;
	}
	if(fd > 0)
	close(fd);
	return ;
RET:
/*	printf("---------------------------Check time year:%d,mouth:%d,day:%d,hour:%d,minute:%d\n",\
	lasttime.year,lasttime.month,lasttime.day,lasttime.hour,lasttime.minute);
	*/	
	memcpy(psystime,&lasttime,sizeof(datetime_setting));
	if(fd > 0)
		close(fd);
	return ;
	
}
void BackupRealTime()
{
	static  int ticket =0;
	if(ticket++<60)/*每隔1分钟备份一次时间*/
	{	
		return ;
	}
	ticket=0;
	datetime_setting realtime;
	memset(&realtime,0x0,sizeof(datetime_setting));
	GlobalGetSystemTime(&realtime);

	
	char buffer[32] = {0x0};		
	sprintf(buffer,"%s%s",GetConfigDir(),"lasttime");
	int fd = open(buffer, O_CREAT|O_RDWR|O_TRUNC,0777);	
	if(fd <0)
	{
		perror(" open lasttime");
		return ;
	}
	printf("---------------------------BackupRealTime 20%02d-%02d-%02d %02d:%02d\n",\
		realtime.year,realtime.month,realtime.day,realtime.hour,realtime.minute);
	write(fd , &realtime , sizeof(datetime_setting));
	if(fd)
		close(fd);
	
}
unsigned int strtime2sec(char *str)
{
	//时间转换22:50:41---> h*3600+m*60+s*/
	char strtime[16]={0x0};
	int hour,min,second;
	memcpy(strtime,str,16);
	strtime[2]=0;
	strtime[5]=0;
	strtime[8]=0;
	hour = atoi(&strtime[0]);
	min = atoi(&strtime[3]);
	second = atoi(&strtime[6]);

	//printf("%s-----------------------strtime2sec:(h:%d,m:%d,s:%d)\n",str,hour,min,second);
	return hour*3600+min*60+second;
}
void SetChkFsSigchldHandler()
{
	struct sigaction act;
	
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = &ChkFsSigchldHandler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGCHLD, &act,NULL);	

}

void ClearChkFsSigchldHandler()
{
	struct sigaction act;
	
	memset(&act, 0 , sizeof(act));
	
	act.sa_handler = SIG_DFL;
	
	sigaction(SIGCHLD, &act, NULL);
	
}


/*获得最后一天录像的日期*/
int  GetLastRecordDate(datetime_setting *datetime, int *HddId)
{
	int fd = S_FAILURE;
	int retval = S_FAILURE;
	SysLastRecordStatus	LastRecordStatus;

	if(datetime == NULL)
	{
		return S_FAILURE;
	}

	fd = open(LASTDAY_RECORDFILE, O_RDONLY);
	if(fd < 0)
	{
		printf("open last day record file failure  \n");
		return S_FAILURE;
	}

	retval = read(fd, &LastRecordStatus, sizeof(SysLastRecordStatus));

	if((retval < 0) || (LastRecordStatus.m_uMagic != LASTDAY_MAGIC ))
	{
		printf("get last day magic failure %x : %x\n", retval, LastRecordStatus.m_uMagic);
		goto GETLASTDAY_FAILURE;
	}

	memcpy(datetime, (&LastRecordStatus.m_sDateTime), sizeof(datetime_setting));

	if((datetime->year < 100)
		&&((datetime->month > 0) && (datetime->month < 13))
		&&((datetime->day > 0) && (datetime->day < 32)))
	{
		printf("get last record date success  \n");
		retval = S_SUCCESS;
	}

	*HddId = LastRecordStatus.m_nHddId;
	
GETLASTDAY_FAILURE:
	close(fd);
	
	return retval;
	
}

/*检测文件系统，并修复其中的错误*/
int CheckFat32FileSystem(char *dev_name) 
{
	int pid;
	const char *str = "/dosfsck";
	const char *str1 = "-a";
	const char *str2 = "-V";
	const char *str3 = "-w";
    const  char *str5= "-d 2008-12-12";
	char str6[100] ={0};
	char *agv[9];
	int status = 0;
	int i = 0;
	char datedir[32] = {0};
	datetime_setting	t_date;
	int HddId = 0;

	if(GetLastRecordDate(&t_date, &HddId) != S_SUCCESS)
	{
		GlobalGetSystemTime(&t_date);
	}
	
	strcpy(datedir, str5);
	
	datedir[5] = t_date.year%100/10 + '0';
	datedir[6] = t_date.year%10  +  '0';
	datedir[8] = t_date.month %100/10 + '0';
	datedir[9] = t_date.month%10 + '0';

	datedir[11] = t_date.day %100 /10+ '0';
	datedir[12] = t_date.day %10 + '0';
	
	printf("check file system device :%s  date : %s \n",dev_name, datedir);
	
	sprintf(str6, "%s", dev_name);
	agv[0] = (char*)(str+1);
	agv[1] = (char*)str1;
	agv[2] = (char*)str2;
	agv[3] = (char*)str3;
	agv[4] = datedir;
	agv[5] = str6;
	agv[6]=NULL;
	
	g_nCheckFsStep = PROCESSING;
	
	SetChkFsSigchldHandler();		
	
	pid=vfork();
	
	if (pid) 
	{

	}
	else if ( 0 == pid) 
	{
		execve(str, agv, NULL);
		printf("exec failure \n");		
		exit(1);
	}

	while(g_nCheckFsStep != PROCESS_END)
	{
		i++;

		printf("*");

		if(i%20 == 0)
		{
			printf("\n");
		}
		
		status ^= 0x01;
//		FEED_DOG();
		//PubFeedDog();
		usleep(300000);
		
	}	

	ClearChkFsSigchldHandler();
	
	return 0;
	
}


//冒泡排序函数
void BubbleSort(int * in,int count)
{
	int x = 0;
	int y = 0;
	int  temp = 0;
	
	for(y=0;y<count-1;y++)
	{
		for(x=1;x<count-y;x++)
		{
			if((*(in+x))>(*(in+x-1)))
			{
				temp=(*(in+x-1));
				(*(in+x-1))=(*(in+x));
				(*(in+x))=temp;
			}
		}
	}
	
} 

/*
int PubGetAllRecordDir(int hddId, RecordDateDirSearchType * SearchDateDir)
{

	int retval = S_FAILURE;
	C_FILEMANAGE	*FileManage = NULL;

	FileManage = C_FILEMANAGE::Instance();
	
	if(FileManage)
	{
		retval = FileManage->GetRecordDateDirOfHDD(hddId,  SearchDateDir);
	}

	return retval;
}
*/

int PubGetTvSysMode()
{
	int TvSys = 0;
	MACHINE_PARA  MachinePara;

	PubGetSysParameter(SYSMACHINE_SET, (void *)&MachinePara);

	TvSys = MachinePara.m_uTvSystem;
	
	return TvSys;
	
}



int  InitSysParameter()
{
	int ret = S_FAILURE;

	g_cParaManage = PARAMETER_MANAGE::Instance();

	if(g_cParaManage == NULL)
	{
		printf("System Manage Failure \n");
	}
	else 
	{
		ret = S_SUCCESS;
	}
	HiGpioInit();
	return ret;
}



/*关闭所有通道录像*/
void CloseAllChannelRecord()
{


}

void SetRecordCtrlThread(int status)
{
	if(status == THREAD_STOP || status == THREAD_PAUSE)
	{
		StopRecordSystem();
		return;
	}

	if(status == THREAD_START || status == THREAD_CONTINUE)
	{
		StopRecordSystem();
		return;
	}

	return;
}

void RecordThereadExit()
{

}


void *RemoveAllRecordFileThread(void *arg)
{
	plog("%s %d %s tid:[%d] pid:[%d] ppid:[%d]\n", __FILE__,__LINE__,__FUNCTION__,(int)pthread_self(),(int)getpid(),(int)getppid());
	pthread_detach(pthread_self());

	StopRecordSystem();
	sleep(3);
	printf("RemoveAllRecordFile   ~~~~~~~~~befor\r\n");
	int nRet = system("rm /hdd00/p01/record -rf");
	printf("RemoveAllRecordFile   ~~~~~~~~~end			   nRet [%d]\r\n",nRet);

//	sleep(20);
	StartRecordSystem();
	g_bFormatSDCard = false;

	return NULL;
}


int ZSipRemoveAllRecordFile()
{

	g_bFormatSDCard = true;

	int fd;
	fd = access("/hdd00/p01/record", R_OK|W_OK);
	if(fd == -1)
	{
		printf("RemoveAllRecordFile    no record dir\r\n");
		
		g_bFormatSDCard = false;
		return 2;
	}

	pthread_t threadId;
	if(pthread_create(&threadId , NULL, RemoveAllRecordFileThread, NULL) < 0)
	{
		plog("create RemoveAllRecordFileThread failure \n");
		return S_FAILURE;
	}
	return 0;
}


int RemoveAllRecordFile()
{

	StopRecordSystem();

	int fd;
	fd = access("/hdd00/p01/record", R_OK|W_OK);
	if(fd == -1)
	{
		printf("RemoveAllRecordFile    no record dir\r\n");
		return 2;
	}
	
	printf("RemoveAllRecordFile   ~~~~~~~~~befor\r\n");
	int nRet = system("rm /hdd00/p01/record -rf");
	printf("RemoveAllRecordFile   ~~~~~~~~~end             nRet [%d]\r\n",nRet);
	
	return 0;
}

int FormatSD(int *process)
{


	 ZSipRemoveAllRecordFile();		
	return 0;

#if 0
    printf("==11111111111======================*process:%d\n",*process);
	int ch = 0;
	if((process==NULL)||(DoingFormat))
	{
		printf("process is NULL or doing format\n");
		*process = 0;
		return -1;
	}
	*process = 0;
	RECORDMODULE		*RecordModule = NULL;
	RecordModule = RECORDMODULE::Instance();	
	if(RecordModule)
	{
		DoingFormat =true;
		for(ch = 0; ch < CHANNEL_MAX; ch++)
		{
			RecordModule->CloseRecordFile(ch);
			RecordModule->m_uRecordAction[ch] = RECORD_PAUSE;
			printf("%d,1111RECORD_STOP is runing!\n",__LINE__);
			RecordModule->m_uHddErrorCnt[ch] = 0;
		}
		RecordModule->SetRecordCtrlThread(THREAD_PAUSE);
		printf("%d,2222THREAD_STOP is runing!\n",__LINE__);
		usleep(50000);
		RecordModule->StopAllChannelRecord();
		printf("%d,StopAllChannelRecord is runing!\n",__LINE__);
	}
	else
	{
		  printf("==2222222222222======================*process:%d\n",*process);
		return -1;
		// *process = -1;
	}

	if(RecordModule)
	{

		RecordModule->ResetRecodParm();
	}
	//printf("%d,############### rm file!\n",__LINE__);
	BlockDevManage  *BlkDevManage = BlockDevManage::Instance();

	s_Format.DoFormat = &DoingFormat;
	s_Format.Process  = process;
	s_Format.StartRecordCallback =StartRecordSystem;
	#if 0
	int fd;
	fd = open(SD_DEVICE_NAME,O_RDONLY);
	if(fd < 0)
		printf("@@@@open fd failed!\n");
	#endif
	if(BlkDevManage)
		 BlkDevManage->StartNewFormatDisk((void*)(&s_Format));
  	printf("==3333333333333======================*process:%d\n",*process);
#endif

	return 0;

}



int SetVideoFlipMirror(int mode)
{
	printf("\n\n\n\n\n\n SetVideoFlipMirror ######################## :%d\n",mode);

	#ifdef APP3511
	if(mode == 1)
	{
#ifdef IPCVGA
			#ifndef SUPPORTOVERTURN			
			{
				Setov7725Reg(DC_SET_FLIP,0x2);
				Setov7725Reg(DC_SET_MIRROR,0x2);
				Setov7725Reg(DC_SET_FLIP,0x1);
				
			}
			#else
			{
				Setov7725Reg(DC_SET_FLIP,0x1);
				Setov7725Reg(DC_SET_MIRROR,0x1);
				Setov7725Reg(DC_SET_FLIP,0x2);
			}	
			#endif
#else
		
			#ifndef SUPPORTOVERTURN	
			
			{
				SetMt9d131Reg(MT_DC_SET_MIRROR,0x04);
				SetMt9d131Reg(MT_DC_SET_MIRROR,0x03);
				SetMt9d131Reg(MT_DC_SET_MIRROR,0x01);
			}
			#else
			{
			
				SetMt9d131Reg(MT_DC_SET_MIRROR,0x04);
				SetMt9d131Reg(MT_DC_SET_MIRROR,0x01);
			
			}	
			#endif
		
#endif

	}
	else if(mode == 2)
	{
#ifdef IPCVGA
			#ifndef SUPPORTOVERTURN		
			{
				Setov7725Reg(DC_SET_FLIP,0x2);
				Setov7725Reg(DC_SET_MIRROR,0x2);
				Setov7725Reg(DC_SET_MIRROR,0x1);
			}
			#else
			{
				Setov7725Reg(DC_SET_FLIP,0x1);
				Setov7725Reg(DC_SET_MIRROR,0x1);
			
				Setov7725Reg(DC_SET_MIRROR,0x2);
			}
			#endif
#else
			#ifndef SUPPORTOVERTURN	

			{
				SetMt9d131Reg(MT_DC_SET_MIRROR,0x04);
				SetMt9d131Reg(MT_DC_SET_MIRROR,0x03);
				SetMt9d131Reg(MT_DC_SET_MIRROR,0x02);
			}
			#else
			{
				SetMt9d131Reg(MT_DC_SET_MIRROR,0x04);
				SetMt9d131Reg(MT_DC_SET_MIRROR,0x02);

			}
			#endif

#endif
	}
	else if(mode == 3)
	{
#ifdef IPCVGA

		#ifndef SUPPORTOVERTURN		
		{
			Setov7725Reg(DC_SET_FLIP,0x1);
			Setov7725Reg(DC_SET_MIRROR,0x1);
		}
		#else
		{
			Setov7725Reg(DC_SET_FLIP,0x2);
			Setov7725Reg(DC_SET_MIRROR,0x2);
		}

		#endif
#else
		#ifndef SUPPORTOVERTURN	

		{
			SetMt9d131Reg(MT_DC_SET_MIRROR,0x04);
			SetMt9d131Reg(MT_DC_SET_MIRROR,0x03);
			SetMt9d131Reg(MT_DC_SET_MIRROR,0x03);
		}
		#else
		{
			
			SetMt9d131Reg(MT_DC_SET_MIRROR,0x04);
			SetMt9d131Reg(MT_DC_SET_MIRROR,0x03);		
		}
		#endif

#endif

	}
	else
	{
#ifdef IPCVGA
		#ifndef SUPPORTOVERTURN		
		{
			Setov7725Reg(DC_SET_FLIP,0x2);
			Setov7725Reg(DC_SET_MIRROR,0x2);
		
		}			
		#else
		{
			Setov7725Reg(DC_SET_FLIP,0x1);
			Setov7725Reg(DC_SET_MIRROR,0x1);
		}	
		#endif
#else
		#ifndef SUPPORTOVERTURN	
		{
			SetMt9d131Reg(MT_DC_SET_MIRROR,0x04);
			SetMt9d131Reg(MT_DC_SET_MIRROR,0x03);					
		}
		#else
		{
			SetMt9d131Reg(MT_DC_SET_MIRROR,0x04);
		}
		#endif
#endif
	}

#endif

#ifdef APP3518

VideoMirrorFlipSet(mode);

#endif
	return 0;
}


int PubGetSysParameter(int type, void * para)
{
	int retval = S_FAILURE;

	if(g_cParaManage)
	{
		retval = g_cParaManage->GetSysParameter(type, para);
	}

	return retval;
}

int PubSetSysParameter(int type, void * para)
{
	int retval = S_FAILURE;

	if(g_cParaManage)
	{
		retval = g_cParaManage->SetSystemParameter(type, para);
	}

	return retval;
}
int PubGetSinglePara(int type, void * value)
{
	int retval = S_FAILURE;

	if(g_cParaManage)
	{
		retval=g_cParaManage->GetSingleParameter(type,  value);
	}
	
	return retval;

}


/*播放完成需要完成的动作*/
int PubCompeletPlay()
{

	return 0;
}

void  PubDrawSpiltScreen(int OnOff)
{


}



/*****************************************************************************
函数功能:
输入参数:
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int InitDiskDevice()
{

	return -1;
}


/*****************************************************************************
函数功能:
输入参数:
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/

void InitDecodeSystem()
{
	return ;
}

/*****************************************************************************
函数功能: 根据已知的文件名得到指定通道的文件路径
输入参数:@pFileName: 文件名
			  @pFilePath:文件路径
			  @ch: 通道号1~4
输出参数:@DestFilePath:得到的有效路径
返  回   值: 成功返回1，否则返回0, 
使用说明:
******************************************************************************/
int GetValidFilePath(char *pFileName,  char *pFilePath, char *DestFilePath, int ch)
{
#if 1
	int len = 0;
//	char alarmflag = 0;

	sprintf(DestFilePath, "%s/%s", pFilePath, pFileName);
//	DEBUGMSG(MOD_INTF, ("DestFilePath = %s\n", DestFilePath));

	len = strlen(DestFilePath);


	if(S_SUCCESS==JudgeFileExist(DestFilePath))
	{
		printf ("file %s exist\n", DestFilePath);
		return 1;
	}
	
	//printf("----DestFilePath = %s \n",DestFilePath);
#endif 	
	return 0;
}

/*****************************************************************************
函数功能: 显示或去除分割线
输入参数:@onoff: 1--显示，0--不显示
输出参数:无
返  回   值:无
使用说明:
******************************************************************************/
void DrawSplitLine(int onoff)
{

	//pdesk->DrawSplitScreenEx(4, onoff);
	PubDrawSpiltScreen(onoff);
	
}


int StartJPEGDecode(char *filename)
{
	#if 0
	int jpgfd = -1;
	int Ret;
	
	jpgfd = open(filename, O_RDONLY);
	if(jpgfd == -1)
	{
		return -1;
	}
	
	StartDecodeOutput(1,0);

	Ret = MPIDecodeJPG(filename);
	#endif
	return 0;
	
}

int StartRealTimeProcess()
{
	CRealTime *Rtctime = CRealTime::Instance();

	//开启获取RTC时钟
	if(Rtctime->Start() == 0)
	{
		KEY_INFO("start real time thread successful!\n");
		return 0;
	}
	
	KEY_INFO("start real time thread failed!\n");	
	return -1;	
}


int GetSystemTime(SystemDateTime *pSysTime)
{
	struct tm *t ;  
	time_t timer = 0;
	
	timer = time(NULL);
	t = localtime(&timer);

	/*需要注意的是年份加上1900，月份加上1才是实际的时间*/
	/*把时间的年换算成以2000年为基准的时间*/
	if( t->tm_year > 99) // 1900 为基准
		pSysTime->year = t->tm_year -100;
	else 
		pSysTime->year = 9;


	pSysTime->month = t->tm_mon + 1;
	pSysTime->mday = t->tm_mday;
	pSysTime->hour = t->tm_hour;
	pSysTime->minute = t->tm_min;
	pSysTime->second = t->tm_sec;
	pSysTime->week= CaculateWeek(pSysTime->mday, pSysTime->month, pSysTime->year);

	return 0;
}





void InitSystemMD()
{
	MD_HANDLE  *VideoMd = NULL;
	CAMERA_MD  MdSet;
	int ch = 0;

	VideoMd = MD_HANDLE::Instance();
	VideoMd->InitMDConfig();
	PubGetSysParameter(SYSMOTION_SET, (void *)&MdSet);
	
	for(ch = 0; ch < CHANNEL_MAX; ch++)
	{
		VideoMd->ResetMotionArea(ch);
		VideoMd->MotionDetectionUserCfg(ch, MdSet.m_Channel[ch].m_MDMask, MdSet.m_Channel[ch].m_uMDSensitive);
	}
	
}
//meshare set  0低 1中 2高
//parameter 	0-高，1-较高，2-中，3:低
void MotionDetectionSet(int sensitive )
{
	CAMERA_MD  MdSet;
	PubGetSysParameter(SYSMOTION_SET, (void *)&MdSet);
	if((sensitive==0)&&(MdSet.m_Channel[0].m_uMDSensitive != 2))
	{
		MdSet.m_Channel[0].m_uMDSensitive = 2;
	}
	else if((sensitive==1)&&(MdSet.m_Channel[0].m_uMDSensitive != 1))
	{
		MdSet.m_Channel[0].m_uMDSensitive = 1;
	}	
	else if((sensitive == 2)&&(MdSet.m_Channel[0].m_uMDSensitive != 0))
	{
		MdSet.m_Channel[0].m_uMDSensitive = 0;
	}
	else
	{
		return ;
	}
	PubSetSysParameter(SYSMOTION_SET, (void *)&MdSet);
	MD_HANDLE  *VideoMd = NULL;
	VideoMd = MD_HANDLE::Instance();

	VideoMd->MotionDetectionUserCfg(0, MdSet.m_Channel[0].m_MDMask, MdSet.m_Channel[0].m_uMDSensitive);
}

void StopSystemMD()
{
	MD_HANDLE	*VideoMd = NULL;

	VideoMd = MD_HANDLE::Instance();

	VideoMd->StopMdProcess();
	
}





void SetMotionDetetionPara(int ch, MD_SETUP MD, int Sensetive)
{
	MD_HANDLE *VideoMd = NULL;
	
	VideoMd = MD_HANDLE::Instance();
	VideoMd->ResetMotionArea(ch);
	VideoMd->MotionDetectionUserCfg(ch, MD.m_MDMask, MD.m_uMDSensitive);
}



/*
void CheckuDiskStatusProc()
{
	BlockDevManage  *BlkDevManage = BlockDevManage::Instance();

	BlkDevManage->StartCheckuDisk();
}
*/
/*

格式化硬盘或者U盘  
type : 为格式化设备类型，0，1 为硬盘， 2  U盘或 移动设备
num  为 第几号设备 0  ~~1
*/
/*
int  FormatBlockDevice(int type, int Num)
{

	BlockDevManage  *BlkDevManage = BlockDevManage::Instance();

	return BlkDevManage->FormatAllTypeBlockDevice(type, Num);

}
*/



int StartAlarmHandleProcess()
{
	//AlarmHandle   *pAlarmHandle = NULL;


//	pAlarmHandle = AlarmHandle::Instance();

	//return pAlarmHandle->CreateAlarmHandleThread();
	return 0;
}


/*

 TYPE : 具体见类型 USERLOGTYPE  ，

 eventcode : 见具体结构体 ALARM  见结构体  UserAlarmEvent   ，其他为INT 类型

*/

int WriteSystemLog(unsigned char type, void * EventCode)
{
	return -1;
	
}
#ifdef PIVOT_3516C_1_5

/**
motorManage API
*/
static MotorManage*pMotorManage;
int InitMotorManage()
{
	pMotorManage = MotorManage::GetInstance();
	if(pMotorManage == NULL)
	{
		printf("err,pMotorManage = NULL\n");
		exit(-1);
	}

	return S_SUCCESS;
}

int StartMotorSystem()
{
	return pMotorManage->StartMotorSystem();
}

int PushMsgManual(MotorMsgManual *pMsgManual)
{
	return pMotorManage->PushMsgManual(pMsgManual);
}

int PushMsg915(PresetPointEvent *pMsg915)
{
	return pMotorManage->PushMsg915(pMsg915);
}

int PushMsgMw(MotorMsg *pMsgMw)
{
	return pMotorManage->PushMsgMw(pMsgMw);
}
int GetCurrentContrlMod()
{
	return pMotorManage->GetCurrentContrlMod();
}
int IsMotorRunning()
{ 
	//printf("Moto is running\n");
	//printf("Moto to test=%d\n",pMotorManage->IsMotorRunning());
	return pMotorManage->IsMotorRunning();
}
int GetMicroFlag()
{
//	printf("test micro 3\n");
	return pMotorManage->GetMicroFlag();
}
void SetMicroFlag(int flag)
{
	pMotorManage->SetMicroFlag(flag);
}
#endif



/*

 ch  通道号 ，

 type 录像类型 具体见枚举类 RECORD_KIND 

Filename 为返回的录像名称

返回  成功返回0  ，失败返回-1
*/
/*
int GetLastRecordFileName(int ch, int type, char *FileName,ZoneAlarmStatus *alarm)
{
	C_FILEMANAGE *FileManage = NULL;

	FileManage = C_FILEMANAGE::Instance();
	
	return FileManage->GetLastRecordFileName(ch, type, FileName,alarm);
	
}
*/
//获取移动侦测属性
int GetMotionInfo(int ch)
{
	int md_retval = 0;
	int md_switch_retval = 0;
		
	MD_HANDLE	*pMDClass = NULL;
	pMDClass = MD_HANDLE::Instance();

	CAMERA_MD md_para;
	g_cParaManage->GetSysParameter(SYSMOTION_SET,&md_para);	
	memcpy(&md_switch_retval,&md_para.m_Channel[ch].m_uMDSwitch,sizeof(md_switch_retval));
	
	md_retval = ((pMDClass->GetVideoMdStatus(ch))&& md_switch_retval);
		
	return md_retval;

}
int GetRealTimeMotion()
{
	#if 0
	MD_HANDLE	*pMDClass = NULL;
	pMDClass = MD_HANDLE::Instance();
	return pMDClass->GetRealMdStatus();
	#endif
	return 0;
}


int Get_Md_Flag()
{
	#if 0
	MD_HANDLE	*pMDClass = NULL;


	pMDClass = MD_HANDLE::Instance();

	return pMDClass->Get_Md_Flag();
#endif
	return 0;


}

int p2p_get_motion(int chl)
{
	int ret = 0;	

	/*判断开关*/
	if(0 == GetWebScheduleSwitch(1))
		return 0;

	/*判断移动侦测*/
	if((1 == Get_Md_Flag()))
	{	
		//notice_to_audio = 1;
		ret = 1;
	}	
	else
	{	
		ret = 0;
	}

	if(ret)
		printf("###MD upload !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
	
	return ret;
}



int Setdhcpandmac(int flag)
{
	NETWORK_PARA netset;
	char mac[128];
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
	if(flag == 1)
	{
		printf("start set mac addr\n");
		sprintf(mac,"ifconfig eth0 hw ether %02x:%02x:%02x:%02x:%02x:%02x",netset.m_Eth0Config.m_uMac[0],netset.m_Eth0Config.m_uMac[1],netset.m_Eth0Config.m_uMac[2],netset.m_Eth0Config.m_uMac[3],netset.m_Eth0Config.m_uMac[4],netset.m_Eth0Config.m_uMac[5]);
		printf("Set mac addr %s\n",mac);	
		system(mac);
		sleep(1);
	}
	else 
	{
		return 0;
	}
	
	return 0;
}
/*
根据文档<<NTP参数存储的问题以及解决方案_20130814_1328.docx>>，提供时区索引，返回相差秒数
索引不符合范围，返回S_FAILURE
*/
int GetDiffByTzIdx(unsigned char m_idx_tzname,int *pm_diff_timezone)
{
    int tz_diff_table[] = {
							0,
                            -43200,
                            -39600,
                            -36000,
                            -32400,
                            -28800,
                            -28800,
                            -25200,
                            -25200,
                            -25200,
                            -21600,
                            -21600,
                            -21600,
                            -21600,
                            -18000,
                            -18000,
                            -18000,
                            -16200,
                            -14400,
                            -14400,
                            -14400,
                            -14400,
                            -14400,
                            -12600,
                            -10800,
                            -10800,
                            -10800,
                            -10800,
                            -10800,
                            -7200,
                            -3600,
                            -3600,
                            0,
                            0,
                            0,
                            0,
                            3600,
                            3600,
                            3600,
                            3600,
                            3600,
                            7200,
                            7200,
                            7200,
                            7200,
                            7200,
                            7200,
                            7200,
                            7200,
                            7200,
                            10800,
                            10800,
                            10800,
                            10800,
                            10800,
                            12600,
                            14400,
                            14400,
                            14400,
                            14400,
                            16200,
                            18000,
                            18000,
                            18000,
                            19800,
                            19800,
                            20700,
                            21600,
                            21600,
                            23400,
                            25200,
                            25200,
                            28800,
                            28800,
                            28800,
                            28800,
                            28800,
                            32400,
                            32400,
                            32400,
                            34200,
                            34200,
                            36000,
                            36000,
                            36000,
                            36000,
                            36000,
                            39600,
                            43200,
                            43200,
                            43200,
                            46800,        
                            };  

    if(m_idx_tzname >= sizeof(tz_diff_table)/sizeof(tz_diff_table[0]))
        return S_FAILURE;
    
    *pm_diff_timezone = tz_diff_table[m_idx_tzname];
    
    return S_SUCCESS;
}
int InitDeviceConfigInfo()
{

	int retval = -1;
	int fd = -1;

	fd = open(DEVICE_CONFIG_BIN, O_RDWR);
	printf("InitDeviceConfigInfo :%d\n",fd);
	/*不存在就创建设备配置文件*/
	if(fd < 0)
	{
		
		fd = open(DEVICE_CONFIG_BIN,  O_CREAT|O_RDWR,0777);
		memset(&ConfigInfo,0x0,sizeof(DeviceConfigInfo));
		memcpy(ConfigInfo.UbootVersion,UBOOTVERSION,16);
		memcpy(ConfigInfo.KernelVersion , KERNELVERSION,16);
		memcpy(ConfigInfo.RootfsVersion ,ROOTFSVERSION,16);
		memcpy(ConfigInfo.AppVersion ,SOFTWAREVERSION,16);
		memcpy(ConfigInfo.HardWareVersion , HARDWAREVERSION,16);
		memcpy(ConfigInfo.DeviceName , DEVICENAME,32);
		memcpy(ConfigInfo.CreatVersionDate , "2013-11-22",10);
#ifdef WIFI_18E_IPC

		ConfigInfo.SupportInfo = CONFIG_ENC|CONFIG_AUDIO|CONFIG_P2P|CONFIG_INTERCOM|CONFIG_WIFI;
#else
		ConfigInfo.SupportInfo = CONFIG_ENC|CONFIG_FLIP;//|AUDIO|INTERCOM|WIFI|PT|PTZ|RECODE|P2P;
#endif
		ConfigInfo.Channel = 1;
		ConfigInfo.Resolution =5;
		if( fd >= 0 )
		{			
			retval = write(fd , &ConfigInfo , sizeof(DeviceConfigInfo));
			if( retval > 0 )
			{
				fsync(fd);
			}
			close(fd);
			fd = -1;
		}
		return S_SUCCESS;
	}
	/*存在,就读取配置文件*/
	else
	{
		memset(&ConfigInfo,0x0,sizeof(DeviceConfigInfo));
		read(fd , &ConfigInfo , sizeof(DeviceConfigInfo));
		/*比较app版本是否需要更新*/
		if(strcmp((const char*)ConfigInfo.AppVersion,SOFTWAREVERSION)!=0)
		{
			memcpy(ConfigInfo.AppVersion ,SOFTWAREVERSION,16);
            if(-1 == lseek(fd, 0, SEEK_SET)) 
            { 
                close(fd); 
                printf("----- %s,lseek error \n",__FUNCTION__); 
                return -1; 
            }
			
			write(fd , &ConfigInfo , sizeof(DeviceConfigInfo));
		}
		printf("---------read configinfo--------\n");
		printf("UbootVersion:%s\n",ConfigInfo.UbootVersion);
		printf("KernelVersion:%s\n",ConfigInfo.KernelVersion);
		printf("RootfsVersion:%s\n",ConfigInfo.RootfsVersion);
		printf("AppVersion:%s\n",ConfigInfo.AppVersion);
		printf("DeviceName:%s\n",ConfigInfo.DeviceName);
		printf("HardWareVersion:%s\n",ConfigInfo.HardWareVersion);
		printf("SupportInfo:%08x\n",ConfigInfo.SupportInfo);
		printf("Channel:%d\n",ConfigInfo.Channel);
		printf("AD_Direction:%d\n",ConfigInfo.AD_Direction);
		printf("IrCut_AdMax:%d\n",ConfigInfo.IrCut_AdMax);
		printf("IrCut_AdMin:%d\n",ConfigInfo.IrCut_AdMin);
		printf("-------------------------------------\n");
		Config_IRCutAD(ConfigInfo.IrCut_AdMax,ConfigInfo.IrCut_AdMin,ConfigInfo.AD_Direction);
		close(fd);
		
	}	
	return retval;
	
}

int UpdateDeviceConfigInfo(void *data)
{
/*
0：设置成功
1：校验错误
2：平台版本不匹配（不同平台配置文件不能互相设置）
其他：未知错误

版本Va.b.c.d
0：Hi3507
1：Hi3518A
2：HI3518C
3：Hi3516C

*/
	
	int retval = -1;
	DeviceConfigInfo *pDeviceinfo =(DeviceConfigInfo *)data;
	printf("---------download configinfo--------\n");
	printf("UbootVersion:%s\n",pDeviceinfo->UbootVersion);
	printf("KernelVersion:%s\n",pDeviceinfo->KernelVersion);
	printf("RootfsVersion:%s\n",pDeviceinfo->RootfsVersion);
	printf("AppVersion:%s\n",pDeviceinfo->AppVersion);
	printf("DeviceName:%s\n",pDeviceinfo->DeviceName);
	printf("HardWareVersion:%s\n",pDeviceinfo->HardWareVersion);
	printf("SupportInfo:%08x\n",pDeviceinfo->SupportInfo);
	printf("Channel:%d\n",pDeviceinfo->Channel);
	printf("-------------------------------------\n");
	

	if(memcmp(pDeviceinfo->AppVersion,ConfigInfo.AppVersion,2)!=0)
	{
		/*平台版本不匹配*/
		printf("version is error\n");
		return 2;
	}	
	
	int fd = -1;
	fd = open(DEVICE_CONFIG_BIN, O_RDWR);
	/*不存在就创建设备配置文件*/
	if(fd < 0)
	{
		fd = open(DEVICE_CONFIG_BIN,  O_CREAT|O_RDWR,0777);
		memset(&ConfigInfo,0x0,sizeof(DeviceConfigInfo));
		memcpy(ConfigInfo.UbootVersion,UBOOTVERSION,16);
		memcpy(ConfigInfo.KernelVersion , KERNELVERSION,16);
		memcpy(ConfigInfo.RootfsVersion ,ROOTFSVERSION,16);
		memcpy(ConfigInfo.AppVersion ,SOFTWAREVERSION,16);
		memcpy(ConfigInfo.HardWareVersion , HARDWAREVERSION,16);
		memcpy(ConfigInfo.DeviceName , DEVICENAME,32);
		memcpy(ConfigInfo.CreatVersionDate , "2013-11-22",10);
#ifdef WIFI_18E_IPC
		ConfigInfo.SupportInfo = CONFIG_ENC|CONFIG_ONVIF|CONFIG_AUDIO|CONFIG_INTERCOM|CONFIG_WIFI|CONFIG_RECODE|CONFIG_P2P; //|CONFIG_PTZ |CONFIG_PT
	#ifdef PT_IPC
		ConfigInfo.SupportInfo |=CONFIG_PT;
	#endif
#else
		ConfigInfo.SupportInfo = CONFIG_ENC|CONFIG_ONVIF|CONFIG_AUDIO|CONFIG_INTERCOM|CONFIG_WIFI|CONFIG_PT|CONFIG_ZOOM|CONFIG_RECODE|CONFIG_P2P;
#endif
		ConfigInfo.Channel = 1;
		ConfigInfo.Resolution =5;
		//ConfigInfo.AD_Direction =1;
		if( fd >= 0 )
		{	
			if(-1 == lseek(fd, 0, SEEK_SET))
			{
				close(fd);
				printf("----- %s,lseek error \n",__FUNCTION__);
				return -1;
			}
			retval = write(fd , &ConfigInfo , sizeof(DeviceConfigInfo));
			if( retval > 0 )
			{
				fsync(fd);
			}
			close(fd);
			fd = -1;
		}
		return S_SUCCESS;
	}
	/*存在,就读取配置文件*/
	else
	{
		
		memset(&ConfigInfo,0x0,sizeof(DeviceConfigInfo));		
		memcpy(&ConfigInfo,pDeviceinfo,sizeof(DeviceConfigInfo));
		if(-1 == lseek(fd, 0, SEEK_SET))
		{
			close(fd);
			printf("----- %s,lseek error \n",__FUNCTION__);
			return -1;
		}
		Config_IRCutAD(ConfigInfo.IrCut_AdMax,ConfigInfo.IrCut_AdMin,ConfigInfo.AD_Direction);

		pVideo->VideoMirrorFlipSet(4);		

		write(fd,&ConfigInfo , sizeof(DeviceConfigInfo));
		fsync(fd);
		close(fd);
		printf("---------write configinfo--------\n");
		printf("UbootVersion:%s\n",ConfigInfo.UbootVersion);
		printf("KernelVersion:%s\n",ConfigInfo.KernelVersion);
		printf("RootfsVersion:%s\n",ConfigInfo.RootfsVersion);
		printf("AppVersion:%s\n",ConfigInfo.AppVersion);
		printf("DeviceName:%s\n",ConfigInfo.DeviceName);
		printf("HardWareVersion:%s\n",ConfigInfo.HardWareVersion);
		printf("SupportInfo:%08x\n",ConfigInfo.SupportInfo);
		printf("Channel:%d\n",ConfigInfo.Channel);
		printf("Resolution:%d\n",ConfigInfo.Resolution);
		printf("AD_Direction:%d\n",ConfigInfo.AD_Direction);
		printf("IrCut_AdMax:%d\n",ConfigInfo.IrCut_AdMax);
		printf("IrCut_AdMin:%d\n",ConfigInfo.IrCut_AdMin);
		printf("-------------------------------------\n");

	}	
	
	return 0;
	
}
bool GetSpeeker()
{
	return AudioGetSpeeker();

}
bool ReleaseSpeeker()
{
	return AudioReleaseSpeeker();
}
void AudioInputManage(bool cmd)
{
	if(paudio)
		return paudio->AudioInput(cmd);
}
/*
cmd:false 关闭，true:开启
return :false:failed  true:success
*/
bool AudioInputCtrl(bool cmd)
{
	CAMERA_PARA  encode_para;	
	memset(&encode_para,0x0,sizeof(CAMERA_PARA));
	GetEncodePara(&encode_para);
	if(cmd)
	{
		encode_para.m_ChannelPara[0].m_uAudioSwitch = 1;
	}		
	else
	{
		encode_para.m_ChannelPara[0].m_uAudioSwitch = 0;
	}
	g_cParaManage->SetSystemParameter(SYSCAMERA_SET, (void *)(&encode_para));
	AudioInputManage(cmd);
	return true;
		
}
/*
return :false:off  true:on
*/
bool GetAudioInputStatus()
{
	CAMERA_PARA  encode_para;	
	memset(&encode_para,0x0,sizeof(CAMERA_PARA));
	GetEncodePara(&encode_para);
	if(encode_para.m_ChannelPara[0].m_uAudioSwitch == 1)
	{
		return true;
	}		
	else
	{
		return false;
	}
	return true;
}
/*
录像控制
chn :			通道
type:			录像类型0 普通录像1 报警录像
recodeswitch:		录像开关true:开false:关
controltype:		0: 设置 1:获取
返回值:		-1 失败  0 成功
*/

int RecodeControl(unsigned int chn, char type, bool *recodeswitch , char controltype)
{
	if(( chn>=CHANNEL)||(type>1)||(controltype>1)||recodeswitch == NULL)
	{
		return -1;
	}

	GROUPRECORDTASK			RecSchedule;
	memset(&RecSchedule,0x0,sizeof(GROUPRECORDTASK));
	g_cParaManage->GetSysParameter(SYSRECSCHEDULE_SET, (void *) (&RecSchedule));	
	CAMERA_MD	motion;
	g_cParaManage->GetSysParameter(SYSMOTION_SET,&motion);
	
	int week =0;

	if(controltype == 0)/*设置*/
	{
		if(type == 0)/*普通录像*/
		{
			for(week = 0; week < 8; week++)
			{
				if(*recodeswitch == true)
				{

						
					RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uTimeTbl[0].m_u16StartTime = 0;
					RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uTimeTbl[0].m_u16EndTime = 23*60+59;
					RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uTimeTbl[0].m_u8Valid =0x01;
					RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uTimeTbl[0].m_validChannel =0x01;
					RecSchedule.m_ChTask[chn].m_uTimerSwitch =0x01;
					RecSchedule.m_ChTask[chn].m_uPowerRecEnable =0x01;
							
				}
				else
				{

					memset(RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uTimeTbl,0x0,4*sizeof(TIMETBLSECTION));

				}
			}
			g_cParaManage->SetSystemParameter(SYSRECSCHEDULE_SET, &RecSchedule);
		}
		else/*报警录像*/
		{
			for(week = 0; week < 8; week++)
			{
				if(*recodeswitch == true)
				{

						
					RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_u16StartTime = 0;
					RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_u16EndTime = 23*60+59;
					RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_u8Valid =0x01;
					RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_validChannel =0x01;
					RecSchedule.m_ChTask[chn].m_uTimerSwitch =0x01;
					RecSchedule.m_ChTask[chn].m_uPowerRecEnable =0x01;
					motion.m_Channel[chn].m_uAalarmOutMode|=0x20;
							
				}
				else
				{

					memset(RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl,0x0,4*sizeof(TIMETBLSECTION));

				}
			}
			g_cParaManage->SetSystemParameter(SYSMOTION_SET, &motion);

		}
		
	
		
		return 0;
		
	}
	else/*获取录像状态*/
	{
		if(type == 0)/*普通录像*/
		{
			for(week = 0; week < 8; week++)
			{
				
				unsigned short starttime = RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_u16StartTime ;
				unsigned short endtime = RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_u16EndTime;
				unsigned char valid = RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_u8Valid;
				unsigned short validChannel = RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_validChannel ;
				if((endtime>starttime)&&(valid>0)&&(validChannel>0))
				{
					*recodeswitch = true;
					return 0;
				}
				
			}
		}
		else
		{
			for(week = 0; week < 8; week++)
			{
				
				unsigned short starttime = RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_u16StartTime ;
				unsigned short endtime = RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_u16EndTime;
				unsigned char valid = RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_u8Valid;
				unsigned short validChannel = RecSchedule.m_ChTask[chn].m_TimerTask[week].m_uMdTbl[0].m_validChannel ;
				if((endtime>starttime)&&(valid>0)&&(validChannel>0)&&(motion.m_Channel[chn].m_uAalarmOutMode&0x20))
				{
					*recodeswitch = true;
					return 0;
				}
				
			}

		}


	}
	*recodeswitch = false;
	return 0;
	
}

/*add by hayson begin 2013.12.26*/
const char* get_wifi_name()
{


#ifdef WIFI_18E_IPC

   return "wlan0"; 
#else
   return "ra0"; 
#endif





}

const char* get_local_name()
{
	return "eth0";
}

const char* get_mac_id_file()
{
	return MAC_ADDR_FILE;
}

const char* get_ie_version()
{
	return CAB_VERSION;
}
const char*  get_tools_path()
{
	return TOOLS_ROOT_PATH;
}


NET_WORK_CARD get_network_support()
{
#ifdef WIFI_18E_IPC
	return NET_WORK_CARD_WIFI;
 #elif defined(MININVR_IPC)
 	return NET_WORK_CARD_LOCAL_AND_WIFI;
 #else
    return NET_WORK_CARD_LOCAL;
 #endif
}


/*add by hayson end 2013.12.26*/

/* add by mike 2014-01-09*/
const char* get_app_version()
{
	return SOFTWAREVERSION;
}

const char* get_uboot_version()
{
	return UBOOTVERSION;
}

const char* get_kernel_version()
{
	return KERNELVERSION;
}

const char* get_fs_version()
{
	return ROOTFSVERSION;
}

NET_HD_REOLUTION get_hd_resolution()
{
#ifdef HI1080P_IPC
    return HD_1080P;
#else
	return HD_720P;
#endif
}


int Crtl_ptzPara(int cmd,STRUCT_SET_PTZ_REQUEST *pReq)
{
	switch(pReq->cmd)
	{
		//停止
		case CMD_STOP : 		
			PTZ_Operate(0x00,CMD_STOP,0,0);
			break ;
		//左转
		case CMD_LEFT : 
		//	printf("-----CMD_LEFT---------isFlip-==[%d]-------------\n",isFlip);
			if(isFlip ==3)
			PTZ_Operate(0x00,CMD_RIGHT, pReq->para1,0);
			else
			PTZ_Operate(0x00,CMD_LEFT, pReq->para1,0);/*para0 表速度*/
			break ;
		//右转
		case CMD_RIGHT :
		//	printf("------CMD_RIGHT--------isFlip-==[%d]-------------\n",isFlip);
			if(isFlip ==3)
			PTZ_Operate(0x00,CMD_LEFT, pReq->para1,0);
			else
			PTZ_Operate(0x00,CMD_RIGHT, pReq->para1,0);
			break ;
		//上转
		case CMD_UP :
		//	printf("------CMD_UP--------isFlip-==[%d]-------------\n",isFlip);
			if(isFlip ==3)
			PTZ_Operate(0x00,CMD_DOWN, pReq->para1,0);
			else
			PTZ_Operate(0x00,CMD_UP, pReq->para1,0);
			break ;
		//下转
		case CMD_DOWN :
		//	printf("-------CMD_DOWN-------isFlip-==[%d]-------------\n",isFlip);
			if(isFlip ==3)
			PTZ_Operate(0x00,CMD_UP, pReq->para1,0);
			else
			PTZ_Operate(0x00,CMD_DOWN, pReq->para1,0);
			break ;
		//调用巡航线路
		case CMD_CALL_CRIUSE :
			PTZ_Operate(0x00,CMD_CLRCRIUES_LINE,pReq->para0+40,pReq->para1);/*para0 表巡航线路 para0+40 表示第一条线路*/
			break ;
		//自动左右转动
		case CMD_AUTOSCAN :
			PTZ_Operate(0x00,CMD_STOP,0,0);
			usleep(50000);
			PTZ_Operate(0x00,CMD_AUTOSCAN,pReq->para1,0);
			break ;
		//调用预置点
		case CMD_CALLPRESET :
			PTZ_Operate(0x00,CMD_CAL_PRESET,pReq->para0,0);
			break ; 
		//F+
		case CMD_FOCUSFAR:
			break ;
		//F-
		case CMD_FOCUSNAER :
			break ; 
		//稀
		case CMD_IRISOPEN :
			break ; 
		//密
		case CMD_IRISCLOSE :
			break ; 
		//放大	
		case CMD_ZOOMTELE :
			break ;
		//缩小
		case CMD_ZOOMWIDE :
			break ; 
		//设置预置点
		case CMD_SETPRESET :
			PTZ_Operate(0x00,CMD_SET_PRESET,pReq->para0,0);
			break ;
		
		//设置巡航停留时间
		case CMD_SET_DWELLTIME :
			{
				switch(pReq->para0)
				{
				case 0x02 :
					PTZ_Operate(0x00,CMD_SET_DWELLTIME,0x33,pReq->para0);
					break ;
				case 0x04 :
					PTZ_Operate(0x00,CMD_SET_DWELLTIME,0x34,pReq->para0);
					break ;
				case 0x06 :
					PTZ_Operate(0x00,CMD_SET_DWELLTIME,0x35,pReq->para0);
					break ;
				case 0x08 :
					PTZ_Operate(0x00,CMD_SET_DWELLTIME,0x36,pReq->para0);
					break ;
				case 0x0a :
					PTZ_Operate(0x00,CMD_SET_DWELLTIME,0x37,pReq->para0);
					break ;
				}
			}
			break;
		case CMD_V_SCAN:
			{
				PTZ_Operate(0x00,CMD_STOP,0,0);
				usleep(50000);
				PTZ_Operate(0x00,CMD_V_SCAN,pReq->para1,0);
			}
			break ;
		case CMD_DEL_PRESET :
			{
				PTZ_Operate(0x00,CMD_DEL_PRESET,pReq->para0,0);
			}
			break ;
		default:
			break;

		}
	return 0;

}
void NightMode(bool night)
{
	static int mode=-1;
	if(mode==(int)night)
	{
		return ;
	}
	mode = (int)night;
	#if 0
	HI_BOOL bColor2Grey=HI_TRUE;
	if(!night)bColor2Grey=HI_FALSE;
	HI_MPI_VENC_SetColor2Grey(0,(const VENC_COLOR2GREY_S*)&bColor2Grey);
	HI_MPI_VENC_SetColor2Grey(1,(const VENC_COLOR2GREY_S*)&bColor2Grey);
	HI_MPI_VENC_SetColor2Grey(2,(const VENC_COLOR2GREY_S*)&bColor2Grey);
	
	RequestIFrame(0,0);
	RequestIFrame(1,0);
	#else
	VI_CSC_ATTR_S stCSCAttr;
	HI_MPI_VI_GetCSCAttr(0,&stCSCAttr);
	CAMERA_ANALOG  lchaPara;	
	g_cParaManage->GetSysParameter(SYSANALOG_SET,&lchaPara);
	printf("before========================u32SatuVal[%d]\n",stCSCAttr.u32SatuVal);
	if(night)
		stCSCAttr.u32SatuVal = 0;
	else
	{
		stCSCAttr.u32SatuVal = (lchaPara.m_Channels[0].m_nSaturation)*100/255;
		printf("tell kb stCSCAttr.u32SatuVal=%d\n",stCSCAttr.u32SatuVal);
	}
	HI_MPI_VI_SetCSCAttr(0, &stCSCAttr); 
	printf("after========================u32SatuVal[%d]\n",stCSCAttr.u32SatuVal);

	#endif
	ISP_PUB_ATTR_S stPubAttr;
	HI_MPI_ISP_GetPubAttr(0,&stPubAttr);
	printf("before========================[%f]\n",stPubAttr.f32FrameRate);
	if(night)
	{
		stPubAttr.f32FrameRate=15;//stPubAttr.f32FrameRate/2;
	}
	else
	{
		stPubAttr.f32FrameRate=30;//stPubAttr.f32FrameRate*2;
	}
	HI_MPI_ISP_SetPubAttr(0,&stPubAttr);
	printf("========================NightMode:%d,[%f]\n",night,stPubAttr.f32FrameRate);
}

const char* GetConfigDir()
{
	#ifdef CONFIG_VAR
	#define CONFIG_DIR	"/var/"
	#else	
	#define CONFIG_DIR	"/config/"
	#endif
	static char config_dir[] = CONFIG_DIR;
	printf("GetConfigDir:%s\n",config_dir);
	return config_dir;
}
int GetDeviceExpendCapacity()
{
	int  Capacity =0;

	/***************bit0-bit7****************/
	Capacity |= (1<<0); /* 支持设备开关与告警schedule的设置功能 */
	Capacity |= (1<<1); /* 支持sync接口 */
	// 算法未完善暂时屏蔽
	#if 0
	if( (ConfigInfo.SupportInfo&(CONFIG_AUDIO))>0)
	Capacity |= (1<<2); /* 支持异常声音检测告警 */
	#endif
	//Capacity |= (1<<3); /* 表示spoe一拖二套包NVR，需要引导添加ipc */
	Capacity |= (1<<4); /* 支持告警云录像 */
	Capacity |= (1<<5); /* 支持普通云录像 */
	Capacity |= (1<<6); /*时区设置和MeShare的NTP做设备同步\夏令时（每天更新offset）*/
	/****************************************/



	/***************bit8-bit15****************/
	/*设备功能支持*/
	if( (ConfigInfo.SupportInfo&(CONFIG_P2P))>0)
		Capacity|= (1<<8); /*P2P支持*/
	if( (ConfigInfo.SupportInfo&(CONFIG_PT))>0)
		Capacity|= (1<<9); /*PZ支持*/
	if( (ConfigInfo.SupportInfo&(CONFIG_ZOOM))>0)
		Capacity |= (1<<10); /*ZOOM支持*/
	if( (ConfigInfo.SupportInfo&(CONFIG_INTERCOM))>0)
		Capacity|= (1<<11); /*对讲支持*/
	if( (ConfigInfo.SupportInfo&(CONFIG_WIFI))>0)
		Capacity |= (1<<12); /*wifi支持*/
	/*********************************************/


	
	/***************bit16-bit23****************/

	if( (ConfigInfo.SupportInfo&(CONFIG_AUDIO))>0)
		Capacity|= (1<<16); /*音频*/
	if( (ConfigInfo.SupportInfo&(CONFIG_RECODE))>0)
	{
		Capacity |= (1<<17); /*录像及SD卡操作*/
		Capacity |= (1<<21); /*SD Card*/		
		Capacity |= (1<<29); /*是否支持SD卡格式化*/

	}
	
	Capacity |= (1<<18); /*Disney定制机型*/

	/*****************************************/

	Capacity |= (1<<19); /*夜视设置*/
	Capacity |= (1<<20); /*图像翻转*/

	//Capacity |= (1<<27); /*全双工对讲*/

	/***************bit24-bit31****************/
	//#ifdef ZINK
	Capacity |= (1<<30); 
	//#endif
	/******************************************/

	printf("--------------Capacity:%08x\n",Capacity);
	

	return Capacity;
}
unsigned long long GetDeviceSupplyCapacity()
{
	unsigned long long Capacity =0;
	Capacity |= (1<<1); // 支持区域设置
	Capacity |= (1<<2); // 光敏
	#ifdef OPENSOUND
	if( (ConfigInfo.SupportInfo&(CONFIG_AUDIO))>0)		
		Capacity |= (1<<7);
	#endif
	printf("--------------GetDeviceSupplyCapacity:%llx\n",Capacity);

	return Capacity;
}

int ZmdGetHostByName(const char* host, char* ip)
{
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    int ret;
    struct sockaddr_in *addr;
    char ipbuf[16];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Allow IPv4 */
    hints.ai_flags = 0; 
    hints.ai_protocol = 0; /* Any protocol */
    hints.ai_socktype = SOCK_STREAM;

    ret = getaddrinfo(host, NULL,&hints,&res);

    if (ret != 0)
    {
        perror("getaddrinfo");
        return -1;
    }

    for (cur = res; cur != NULL; cur = cur->ai_next) {
        addr = (struct sockaddr_in *)cur->ai_addr;
        sprintf(ip, "%s", inet_ntop(AF_INET, 
                    &addr->sin_addr, ipbuf, 16));
        break;
    }
    freeaddrinfo(res);
    return ret;
}
#define NTP_SERVER "ntpusa.meshare.com"

void RestartNtpClient(int timezone_offset)
{
	char ip[32];
	
	strcpy(ip, NTP_SERVER);
	printf("\n\n\n\n-------------------------------------RestartNtpClient:%s\n",ip);
	ntpclient_stop();	
	
	ZmdGetHostByName(NTP_SERVER, ip);
	ntpclient_start(timezone_offset, 24*3600, ip, 0);	
}
int OnSetTimezoneByMeshare(int timezone_index, int timezone_offset)
{
	printf("OnSetTimezoneByMeshare:%d\n", timezone_index);

	PARAMETEREXTEND ntp;
	datetime_setting systime;

	memset(&ntp, 0,sizeof(PARAMETEREXTEND));

	if(timezone_index < 0 || timezone_index > 91)
	{
		printf("The parameter is error!r\n");
		return -1;
	}

	PubGetSysParameter(EXTEND_SET, &ntp);
	
	/*保存时间参数*/
	ntp.m_ntp.m_idx_tzname = timezone_index;
	if(timezone_offset != -1)
		ntp.m_ntp.m_diff_timezone = timezone_offset;
	else
		GetDiffByTzIdx(timezone_index,&ntp.m_ntp.m_diff_timezone);
	PubSetSysParameter(EXTEND_SET, &ntp);
	
	ntp.m_ntp.m_ntp_switch = 1;
	
	PubSetSysParameter(EXTEND_SET, &ntp);
	#ifdef SUPPORT_P2P
	int utc_timestamp = p2p_get_server_timestamp();
	if(utc_timestamp > 0)
		MeshareNtpCallback(utc_timestamp);
	#endif
	GlobalGetSystemTime(&systime);
	return 0;
}

//-1:normal 0:abnormal voice 1:baby cry
int GetAbnormalNoiseDetect()
{	
	int ret=-1;
	if(paudio!=NULL)
		ret =paudio->GetAudioAlarm();
	
	//printf("------------------GetAbnormalNoiseDetect:%d\n",ret);
	return ret;
}
//异常声音告警抓图后清除m_Audioalarm
void ClearAbnormalNoise()
{
	int value=-1;
	if(paudio!=NULL)
		paudio->SetAudioAlarmValue(value);
}
#ifdef SUPPORT_P2P

static pthread_mutex_t md_mutex = PTHREAD_MUTEX_INITIALIZER;  
static pthread_cond_t  md_cond = PTHREAD_COND_INITIALIZER;  
static std::queue<p2p_broadcast_alarm_t> g_p2p_broadcast_alarm_queue;
#endif

void BroadcastAlarmsEx(P2pAlarmType alarm_type, int chl, void* alarm_info)
{
	#ifdef SUPPORT_P2P
	if(!p2p_is_online())
		return;
	printf("######################## BroadcastAlarmsEx\n");
	pthread_mutex_lock(&md_mutex);
	p2p_broadcast_alarm_t alarm = {alarm_type, chl, alarm_info};
	g_p2p_broadcast_alarm_queue.push(alarm);
	pthread_mutex_unlock(&md_mutex); 
	pthread_cond_broadcast(&md_cond);
	#endif
	
}

void BlockWaitForAlarms()
{
	#ifdef SUPPORT_P2P
	pthread_mutex_lock(&md_mutex);  
	if(g_p2p_broadcast_alarm_queue.size() == 0)
		pthread_cond_wait(&md_cond,&md_mutex);
    pthread_mutex_unlock(&md_mutex);  
	#endif
}

int P2pGetOneAlarm(p2p_broadcast_alarm_t * alarm)
{	
	#ifdef SUPPORT_P2P
	int ret = -1;
	pthread_mutex_lock(&md_mutex); 

	if(g_p2p_broadcast_alarm_queue.size())
	{
		*alarm = g_p2p_broadcast_alarm_queue.front();
		g_p2p_broadcast_alarm_queue.pop();
		ret = 0;
	}
	pthread_mutex_unlock(&md_mutex);
	
	return ret;
	#else
	return 0;
	#endif
}

int SysGetDefaultConfig(DevDefaultConfig* config)
{
	if(config== NULL)
		return -1;
	
	int fd = -1;
	int ret = -1;
	
    fd = open(DEV_DEFAULT_FILE,   O_CREAT | O_RDWR, 0777);
	if(fd < 0)
	{
		printf("open DEV_DEFAULT_FILE failure !! \r\n");
		return -1;
	}
	
	ret = read(fd, config, sizeof(DevDefaultConfig)) ;
	if(ret <= 0)
	{
		printf(" read DEV_DEFAULT_FILE file failure \r\n"); 
		close(fd);
		return -1;
	}
	close(fd);
	
	return  0;
}

int SysSetDefaultConfig(DevDefaultConfig* config)
{
	if(config== NULL)
		return -1;
	int fd = -1;
	int ret = -1;
	
    fd = open(DEV_DEFAULT_FILE,  O_CREAT | O_RDWR, 0777);
	if(fd < 0)
	{
		printf(" open DEV_DEFAULT_FILE failure \r\n");
		return -1;
	}
	
	ret = write(fd, config, sizeof(DevDefaultConfig)) ;
	if(ret <= 0)
	{
		printf(" write DEV_DEFAULT_FILE file failure \r\n"); 
		close(fd);
		return -1;
	}
	fsync(fd);
	close(fd);
	
	return  0;
}
#define DBG(fmt, args...) fprintf(stderr, "\033[1;32m DEBUG(%s:%d):			   \033[0m" fmt, __func__, __LINE__, ## args)
/*
* @处理多个时间段中间的重复时间
* @hayson
*/
/* 标准时间段处理 */
int webserver_insert_period(schedule_time *schedule, config_schedule_time * local_schedule)
{
	if(schedule == NULL)
		return -1;

	int count = 0;
	int tmp_count = 0;

	/* 去重存放 */
	for(int i = 0; i < (WEBSEVER_SCHEDULE_MAX_NUM+1); i++)
	{
		if(local_schedule->schedule[i].valid)
		{
			/* 第一种情况 schedule与local_schedule 相交*/
			/* 前相交，开始时间去重 */
			if((schedule->off_at < local_schedule->schedule[i].off_at) && 
				(local_schedule->schedule[i].off_at <= schedule->on_at ))
			{
				local_schedule->schedule[i].off_at = schedule->off_at;
				count++;
			}

			/* 后相交， 结束时间去重 */
			if((schedule->off_at <= local_schedule->schedule[i].on_at) &&
				(local_schedule->schedule[i].on_at < schedule->on_at))
			{
				local_schedule->schedule[i].on_at = schedule->on_at;
				count++;
			}

			if(count)
				break;

			/* 第二种情况，local_schedule包含schedule，不处理*/
			/* 开始时间点结束时间点都在local_schedule 内*/
			if((local_schedule->schedule[i].off_at <= schedule->off_at) && 
			(schedule->on_at <= local_schedule->schedule[i].on_at))
			{
				tmp_count++;
			}

			if(tmp_count)
				break;			

		}
		else /* 时间段保存 */
		{
			local_schedule->schedule[i].off_at = schedule->off_at;
			local_schedule->schedule[i].on_at = schedule->on_at; 
			local_schedule->schedule[i].valid = 1;
			break;
		}
	}
		
	return 0;
}
/*
* @将P2P设置的时间段，分解成本地时间段，处理跨天等特殊逻辑
* @hayson
*/
int webserver_schedule_conver_to_config(schedule_time * schedule)
{
	if(schedule == NULL)
		return -1;
	
	config_schedule_time * day_period = NULL;
	config_schedule_time * next_day_period = NULL;

	/*一周为周期*/
	for(int i = 0; i < 7; i++)
	{
		/* 设备开关 */
		if((schedule->repeat_day & (1 << i)))
		{
			/* 设备开关时间段 */
			if(schedule->flag == 0)
			{
				day_period = &schedule_config.device_time[i];
				if(i == 6)
					next_day_period = &schedule_config.device_time[0];
				else
					next_day_period = &schedule_config.device_time[(i+1)];
			}/* 告警时间段 */
			else if(schedule->flag == 1)
			{
				day_period = &schedule_config.notify_time[i];
				if(i == 6)
					next_day_period = &schedule_config.notify_time[(i+1)];
				else
					next_day_period = &schedule_config.notify_time[(i+1)];
			}

			
			/* 全天情况 */
			if(schedule->off_at == 0 && schedule->on_at == 0)
			{
				day_period->day = 1;
			}
			else
			{
				/* 不跨天时间段 */
				if(schedule->off_at <= schedule->on_at)
				{
					if(webserver_insert_period(schedule, day_period) < 0)
						return -1;
				}
				else /* 跨天时间段 */
				{
					/* 跨天时间段以当地晚上12点为基准，拆分为两个时间段分别存放 */
					schedule_time tmp_schedule = {0};
					memcpy(&tmp_schedule, schedule, sizeof(schedule_time));

					/* 前一个时间段插入当天*/
					tmp_schedule.on_at = 1440;
					if(webserver_insert_period(&tmp_schedule, day_period) < 0)
						return -1;
					
					/* 后一个时间段插入第二天*/
					tmp_schedule.on_at = schedule->on_at;
					tmp_schedule.off_at = 0;
					
					if(webserver_insert_period(&tmp_schedule, next_day_period) < 0)
						return -1;

					
				}
			}
		}
	}

	return 0;
}

/*
* @时间段处理
* @hayson
*/
int webserver_schedule_pro(web_sync_param_t* sync_data)
{
	if(sync_data == NULL)
		return -1;

	memset(&schedule_config.device_time, 0x0, sizeof(config_schedule_time)*7);
	memset(&schedule_config.notify_time, 0x0, sizeof(config_schedule_time)*7);
	/* 暂定义最多十个时间段*/
	for(int i = 0; i < WEBSEVER_SCHEDULE_MAX_NUM; i++)
	{
		if(sync_data->time_list[i].schedule_id)
		{
			if( webserver_schedule_conver_to_config(&sync_data->time_list[i]) < 0 )
			{
				DBG("##############  schedule[%d]_conver_to_config failed #########\r\n", i);
			}
		}
	}

	
	for(int j = 0; j < 11; j++)
	{
		for(int k = 0; k < 7; k++)
		{
			if(schedule_config.notify_time[k].schedule[j].valid)
			{
				DBG("######################## week[%d][%d]###############################################\r\n", k, j);
				DBG("day=[%d]\r\n", schedule_config.notify_time[k].day);
				DBG("off_at=[%d]\r\n", schedule_config.notify_time[k].schedule[j].off_at);
				DBG("on_at=[%d]\r\n", schedule_config.notify_time[k].schedule[j].on_at);
				DBG("valid=[%d]\r\n", schedule_config.notify_time[k].schedule[j].valid);
				DBG("#######################################################################\r\n");
			}
		}

	}
	return 0;
}

int webserver_get_device(web_sync_param_t* sync_data)
{
	PubGetSysParameter(WEB_SET, (void *)sync_data);	
#if 0
	printf("######################################################\n");
	printf("sync_key:%s\n",sync_data->sync_key);
	printf("time_zone:%s\n",sync_data->time_zone);
	printf("mute:%d\n",sync_data->mute);
	printf("device_on:%d\n",sync_data->device_on);
	printf("device_schedule:%d\n",sync_data->device_schedule);
	printf("cvr_on:%d\n",sync_data->cvr_on);


	printf("time_list[0].flag:%d\n",sync_data->time_list[0].flag);
	printf("time_list[0].repeat_day:%d\n",sync_data->time_list[0].repeat_day);
	printf("time_list[0].off_at:%d\n",sync_data->time_list[0].off_at);
	printf("time_list[0].on_at:%d\n",sync_data->time_list[0].on_at);	
	printf("######################################################\n");
#endif

	return 0;
}
//web :sensitivity	int	否	1：低灵敏度，2：中灵敏度，3：高灵敏度
//local :// 移动侦测灵敏度/*4个等级:0-高，1-较高，2-中，3:低*/
int WebSensitivity_to_Local(int websensitive)
{
	int local = 1;
	if(websensitive==1)
	{
		local = 2;
	}
	else	if(websensitive==2)
	{
		local = 1;
	}
	else	if(websensitive==3)
	{
		local = 0;
	}
	return local;
}
int webserver_set_device(web_sync_param_t* sync_data)
{

	printf("---------------------webserver_set_device------------------------------\n");
	printf("sync_key:%s\n",sync_data->sync_key);
	printf("mute:%d\n",sync_data->mute);
	printf("device_on:%d\n",sync_data->device_on);
	printf("device_schedule:%d\n",sync_data->device_schedule);
	printf("cvr_on:%d\n",sync_data->cvr_on);
	printf("sensitivity:%d\n",sync_data->sensitivity);
	printf("device_volume:%d\n",sync_data->device_volume);
	printf("admax:%d\n",sync_data->ircut_admax);
	printf("admin:%d\n",sync_data->ircut_admin);
	PubSetSysParameter(WEB_SET, (void *)sync_data);
	webserver_schedule_pro(sync_data);
	AudioInputCtrl((bool)(sync_data->mute));
	WebSetIPCMirrorFilp((int)sync_data->imageflip_switch);
	Set_NightSwtich(sync_data->nightvision_switch);
	UpdateConfigFileAD(sync_data->ircut_admin,sync_data->ircut_admax);
	//
	#ifdef OPENSOUND
	if( (ConfigInfo.SupportInfo&(CONFIG_AUDIO))>0){		
		SetAudioVolumeLevel(sync_data->device_volume);		
	}	
	#endif
	return 0;
}
int GetDeviceVolume(){
	
}
/*
type:0设备开关1:移动报警开关2:音频报警开关
返回值0:在当前时间段关闭1:在当前时间段开启

*/
int GetWebScheduleSwitch(int type)
{
	int i=0;
	web_sync_param_t sync_data;
	PubGetSysParameter(WEB_SET, (void *)(&sync_data));

	datetime_setting datetime;
	GlobalGetSystemTime(&datetime);
	int CurTime =  datetime.hour*60+datetime.minute;
	
	/*　1: 设备开 0: 设备关 .默认值为1　*/
	if(sync_data.device_on == 0)
	{
		return 0;
	}
	/* 判断设备时间开关，bit 为1 表示开，0表示关， Bit0: by location ;Bit1: by time */
	if((sync_data.device_schedule&0x2))
	{
		/* 检测时间段 */
		for(i = 0; i < (WEBSEVER_SCHEDULE_MAX_NUM+1); i++)
		{
			if(schedule_config.device_time[datetime.week].day)
			{
				//DBG("########### device_off_alltime week[%d] off_at[%d]###########\r\n", datetime.week, 
																				// schedule_config.device_time[datetime.week].schedule[i].off_at);
				return 0;
			}
			else
			{
				/* 时间段生效，时间段内报警关闭*/
				if(schedule_config.device_time[datetime.week].schedule[i].valid)
				{
					if((CurTime >= schedule_config.device_time[datetime.week].schedule[i].off_at) && 
					(CurTime< schedule_config.device_time[datetime.week].schedule[i].on_at))	
					{
						//DBG("########## device_off_period ##########\r\n");
						return 0;
					}
				}
			}
		}	
	}
	/* 设备开关检测 */
	if(type == 0)
		return 1;
	
	/* 设备不用再判断所有类型的报警开关，直接上报即可 */
	return 1;	
}

void DeviceSwtich()
{
	
	//printf("aaaaaaaaaaaaaaa\n");
	if(device_on&&0==GetWebScheduleSwitch(0))
	{
		printf("-------------------------------->StopEncodeSystem!!\n");
		device_on = false;
		//StopEncodeSystem(0,0);
		StopRecordSystem();
		
	}
	if(device_on == false&&1==GetWebScheduleSwitch(0))
	{
		device_on = true;
		printf("-------------------------------->StartEncodeSystem!!\n");
		//StartEncodeSystem();
		StartRecordSystem();
	}
	
}

/* 设置mini nvr的网络连接状态,1:connect 0:disconnect*/ 
static int nvrwifiroute=0;
int SysSetNvrNetStatus(int route) 
{ 
	nvrwifiroute = route;
	return 0; 
}

int SysGetNvrNetStatus() 
{ 
	printf("--------------nvrwifiroute:%d\n",nvrwifiroute);
	return nvrwifiroute;
	
}
int GetDeviceType()
{
	return 0;
}
int  Get_Net_Status()
{
	return 0;
}
int  Get_Net_Level()
{
	return 0;
}

int  Set_Net_Level(int level)
{

	return 0;
}
int  Get_Vi_Rate(VENC_ATTR_H264_VBR_S   	*vBr)
{return -1; }
int  Set_Vi_Rate(VENC_ATTR_H264_VBR_S   	*vBr)
{
	return -1;
}



int  Set_TimerAlarm()
{
	
	return 0;
}


int StartLowBiteStream()

{
	//return initBitrateControl(Get_Md_Status,Get_Net_Status,Get_Net_Level,Set_TimerAlarm);
	return -1;
}
//@date: '2014-07-16'
//@channel: channel number
//@info: malloc memory inside
//@num: file number
//return 0 on success
//if success and num is not 0, user must free info
int SearchRecordFileByDate( const char* date,
							 int chl, 
							 p2p_find_playback_file_t** list,
							 int *num )
{
	REC_SEARCH enMode = REC_SEARCH_SET;
	FindFileType findType;
	RecordFileName result;
	std::vector<rec_dirent> buf;

	*num = 0;
	*list = NULL;
	memset(&findType, 0, sizeof(findType));
	strptime(date, "%Y-%m-%d", &findType.time);
	struct tm *t = &findType.time;
	t->tm_year -= 100; //base 2000
	t->tm_mon += 1; //base 1
	printf("%d-%d-%d %d:%d:%d\n", t->tm_year, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec );
	findType.channel = (1<<chl);
	//Sort_File_list(t->tm_mday, t->tm_mon, t->tm_year+2000, false);

	do {
		int ret = FindRecordFile(&findType, &result , enMode); 
		printf("fnum:%d, fileNb:%d\n", ret, result.fileNb);
		if(ret == 0 )
		{
			*num += result.fileNb;
			for(int i=0; i<result.fileNb; i++)
			{
				buf.push_back(result.namelist[i]);
			}
		}
		else
			break;
		enMode = REC_SEARCH_NEXT; 
	}while(result.fileNb >= 10);

	if(*num)
	{
		*list = (p2p_find_playback_file_t*)malloc(*num*sizeof(p2p_find_playback_file_t));
		if(*list == NULL)
		{
			printf("SearchRecordFileByDate malloc failed!\n");
			*num = 0;
			return 0;
		}
	}
	for(int i=0; i<(int)buf.size(); i++)
	{
		//@record_type: 0 plan, 1 alarm
		static int rec_type[6] = {0, 1, 1, 0, 0, 0};
		(*list)[i].alarm_level = buf[i].level;
		(*list)[i].channel = buf[i].channel;
		(*list)[i].file_size = buf[i].filesize*1024;
		(*list)[i].file_type = rec_type[buf[i].m_filetype];
		(*list)[i].is_finished = (buf[i].start_time != buf[i].end_time);
			
		sprintf((*list)[i].create_time,"%02d:%02d:%02d", buf[i].start_time/3600, 
			(buf[i].start_time%3600)/60, (buf[i].start_time%3600)%60);
		sprintf((*list)[i].finish_time,"%02d:%02d:%02d", buf[i].end_time/3600, 
			(buf[i].end_time%3600)/60, (buf[i].end_time%3600)%60);
		strcpy((*list)[i].create_date, date);
		strcpy((*list)[i].file_path, buf[i].d_name);
		printf("########### [%s %s-%s][level:%d]\n", date, (*list)[i].create_time, (*list)[i].finish_time,(*list)[i].alarm_level );
	}
	return 0;
}

/* 	
	此函数将本地的被动设备列表的全部内容同步到服务器上
*/
int SetSubDeviceListToWeb(int num, const sub_device_info *sd_list)
{
	#ifdef SUPPORT_P2P
	return p2p_upload_sub_device_list(num, sd_list);
	#else
	return 0;
	#endif
}
/* 	
	此函数将服务器上的被动设备列表的全部内容同步到本地 
*/

int SetSubDeviceListToLocal(int num, const sub_device_info *sd_list)
{
	for(int i=0; i<num; i++)
	{
		printf("%s:id[%s],name[%s],type[%d],chl[%d],pwd[%s]\n",
			__FUNCTION__, sd_list[i].physical_id, sd_list[i].device_name, 
			sd_list[i].device_type, sd_list[i].channel_id, sd_list[i].local_pwd);
	}
	return 0;
}
bool SetIPCMirrorFilp(int comid)
{

	CAMERASENSOR_PARA	  sensor;
	g_cParaManage->GetSysParameter(SENSOR_SET,&sensor);

	switch ( comid )
	{
		case CMD_S_PIC_NORMAL:
		{
				if (sensor.m_picMode != 4)
				{
					sensor.m_picMode = 4;
					SetVideoFlipMirror(sensor.m_picMode);
				}
			}
			break;
		case CMD_S_PIC_FLIP:
			{	
				if (sensor.m_picMode != 1)
				{
					sensor.m_picMode = 1;
					SetVideoFlipMirror(sensor.m_picMode);
				}
			}
			break;
		case CMD_S_PIC_MIRRORON:
			{
				if (2 != sensor.m_picMode)
				{
					sensor.m_picMode = 2;
					SetVideoFlipMirror(sensor.m_picMode);
				}
			}
			break;
		case CMD_S_PIC_FLIPMIRROR:
			{
				if (3 != sensor.m_picMode)
				{
					sensor.m_picMode = 3;
					SetVideoFlipMirror(sensor.m_picMode);

				}
			}
			break;
		case CMD_S_PIC_COLORON:
			{
				sensor.m_colorMode = 1;
			#ifdef APP3511
				SetMt9d131Reg(MT_DC_SET_BLACKOUTPUT,0x00);
			#endif
			#if (defined APP3518) 
				VideoColourSet(sensor.m_colorMode);
			#endif
			}
			break;
		case CMD_S_PIC_COLOROFF:
			{
				sensor.m_colorMode = 2;
			#ifdef APP3511
				SetMt9d131Reg(MT_DC_SET_BLACKOUTPUT,0x08);
			#endif	
			#if (defined APP3518) 
				sensor.m_colorMode = 2;
				VideoColourSet(sensor.m_colorMode);
			#endif
			}
			break;
		case CMD_S_SENSORFREQ_50:
			{
		#ifdef APP3511
				SetMt9d131Reg(DC_SET_POWERFREQ,0x01);
				sensor.m_PowerFreq = 1;
			#endif	
			#if (defined APP3518) 
				sensor.m_PowerFreq = 1;
				VideoPowerSet(sensor.m_PowerFreq);
			#endif
			}
			break;
		case CMD_S_SENSORFREQ_60:
			{
			#ifdef APP3511
				SetMt9d131Reg(DC_SET_POWERFREQ,0x02);
				sensor.m_PowerFreq = 2;
			#endif
				
			#if (defined APP3518) 
				sensor.m_PowerFreq = 2;
				VideoPowerSet(sensor.m_PowerFreq);
			#endif
			}
			break;
		/*室外模式*/
		case CMD_S_SENSORFREQ_OUT:
		{
		#if (defined APP3518) 
			sensor.m_PowerFreq = 0;
			VideoPowerSet(sensor.m_PowerFreq);
		#endif
			break;
		}	
		/*自动抗闪模式*/
		case CMD_S_SENSORFREQ_AUTO:
		{
		#if (defined APP3518) 
			sensor.m_PowerFreq = 3;
			VideoPowerSet(sensor.m_PowerFreq);
		#endif
			break;
		}		
	
		default:
			return false ;
	}
	g_cParaManage->SetSystemParameter(SENSOR_SET,&sensor); 

	return true ;

}
int WebSetIPCMirrorFilp(int cmdid)
{
	int id=0;
	switch(cmdid)
	{
		case 0:
			id = CMD_S_PIC_NORMAL;
			isFlip = 0;
			break;
		case 1:
			id = CMD_S_PIC_FLIP;					
			break;
		case 2:
			id = CMD_S_PIC_MIRRORON;	
			break;
		case 3:
			id = CMD_S_PIC_FLIPMIRROR;
			isFlip = 3;
//			printf("--------------isFlip-==[%d]-------------\n",isFlip);
			break;
		default:
			return S_FAILURE;
	}
	printf("#####WebSetIPCMirrorFilp   [%x,%d]\n",id,cmdid);
	SetIPCMirrorFilp(id);
	return S_SUCCESS;
}
int Web_Sync_MirrorFilp_Value(int mode)
{

	//printf("web  图像控制 0：正常，1：上下翻转，2：左右镜像，3:180度旋转（翻转加镜像）\n");
	//printf("zsp   // 1: 翻转  2 镜像 3 镜像翻转 4 正常\n");
	int WebValue=0;
	if(mode==CMD_S_PIC_FLIP)
	{
		WebValue = 1;
	}
	else if(mode==CMD_S_PIC_MIRRORON)
	{
		WebValue = 2;
	}
	else if(mode==CMD_S_PIC_FLIPMIRROR)
	{
		WebValue = 3;
	
	}
	else if(mode==CMD_S_PIC_NORMAL)
	{
		WebValue = 0;
	}
	else
	{
		return 0;
	}
	web_sync_param_t sync_data;
	PubGetSysParameter(WEB_SET, (void *)&sync_data);	
	sync_data.imageflip_switch=WebValue;
	PubSetSysParameter(WEB_SET, (void *)&sync_data);
	char strval[16]={0};
	sprintf(strval,"%d",WebValue);
	printf("------------->strval:%s\n",strval);
	p2p_sync_value((const char *)"imageflip",(const char *)strval);
	
	return 0;
	
}
int GetAlarmInterval()
{
	//kb
	//return 1;
	web_sync_param_t web_data;
	PubGetSysParameter(WEB_SET, (void *)(&web_data));
	//printf("-------Alarm interval  [%d]-------\n",web_data.alarm_interval);
	return web_data.alarm_interval;
}
int GetDeviceSwitchStatus() 
{ 
	web_sync_param_t web; 

	memset(&web, 0, sizeof(web)); 
	PubGetSysParameter(WEB_SET, &web); 

	return web.device_on; 
} 
int UpdateConfigFileAD(int min,int max)
{
	if(min<=0||max<=0)
	{
		return 0;
	}
	int fd = -1;
	fd = open(DEVICE_CONFIG_BIN, O_RDWR);
	/*存在,就读取配置文件*/
	if(fd>0)
	{

		lseek(fd, 0, SEEK_SET);
		read(fd , &ConfigInfo , sizeof(DeviceConfigInfo));
		if(-1 == lseek(fd, 0, SEEK_SET))
		{
			close(fd);
			printf("----- %s,lseek error \n",__FUNCTION__);
			return -1;
		}
		ConfigInfo.IrCut_AdMax = max;
		ConfigInfo.IrCut_AdMin = min;
		//ConfigInfo.AD_Direction =1;
		write(fd,&ConfigInfo , sizeof(DeviceConfigInfo));
		Config_IRCutAD(ConfigInfo.IrCut_AdMax,ConfigInfo.IrCut_AdMin,ConfigInfo.AD_Direction);
		fsync(fd);
		close(fd);
		printf("---------write configinfo--------\n");
		printf("UbootVersion:%s\n",ConfigInfo.UbootVersion);
		printf("KernelVersion:%s\n",ConfigInfo.KernelVersion);
		printf("RootfsVersion:%s\n",ConfigInfo.RootfsVersion);
		printf("AppVersion:%s\n",ConfigInfo.AppVersion);
		printf("DeviceName:%s\n",ConfigInfo.DeviceName);
		printf("HardWareVersion:%s\n",ConfigInfo.HardWareVersion);
		printf("SupportInfo:%08x\n",ConfigInfo.SupportInfo);
		printf("Channel:%d\n",ConfigInfo.Channel);
		printf("Resolution:%d\n",ConfigInfo.Resolution);
		printf("AD_Direction:%d\n",ConfigInfo.AD_Direction);
		printf("IrCut_AdMax:%d\n",ConfigInfo.IrCut_AdMax);
		printf("IrCut_AdMin:%d\n",ConfigInfo.IrCut_AdMin);
		printf("-------------------------------------\n");

	}	
	
	return 0;
	
}

void* p2p_start_thread(void*)
{
	#ifdef SUPPORT_P2P
	printf("%s [%lu]\n", __FUNCTION__, pthread_self());
	pthread_detach(pthread_self());
	
	p2p_init_info_t info;
	TYPE_DEVICE_INFO version;
	NETWORK_PARA netset;	
	char device_id[16]= {0};

	GetSoftWareVersion(&version);	
	PubGetSysParameter(SYSNET_SET,&netset);
	memset(&info, 0, sizeof(info));
	
	while(!(version.DeviceType & (1<<8))
	|| strlen(device_id) == 0 
	|| !strcmp(device_id, "0000000000")
	|| !strcmp(device_id, "000000000000000"))
	{		
		GetSoftWareVersion(&version);
		PubGetSysParameter(SYSNET_SET,&netset);
		memcpy(device_id, netset.m_CenterNet.deviceid, 15);	
		printf("---------------------->%s\n",device_id);
		sleep(5);
	}	
/**/
	info.device_type = GetDeviceType();
	info.device_capacity = version.DeviceType;
	info.device_extend_capacity = GetDeviceExpendCapacity();
	info.device_supply_capacity = GetDeviceSupplyCapacity();
	info.device_alarm_in_num = version.AlarmInNum;
	info.device_video_num = version.VideoNum;
	info.device_id = device_id;
	info.config_dir = GetConfigDir();
	info.uboot_version = (char*)ConfigInfo.UbootVersion;
	info.kernel_version = (char*)ConfigInfo.KernelVersion;
	info.rootfs_version = (char*)ConfigInfo.RootfsVersion;
	info.app_version = (char*)ConfigInfo.AppVersion;
	info.device_name = (char*)ConfigInfo.DeviceName;
	
	info.high_resolution = (const char*)("1280*720");
	info.secondary_resolution = (const char*)("320*240");
	info.low_resolution = (const char*)("320*240");
	
	#ifdef WIFI_18E_IPC
	info.use_wired_network = 0;
	info.network_interface = get_wifi_name();
	#else
	info.network_interface = get_local_name();
	info.use_wired_network = 1;
	#endif
	/*回调函数*/
	//info.set_sub_device_list_callback = SetSubDeviceListToLocal;
	//info.set_preset_list_callback = SetPrestListToLocal;
    info.find_playback_list_callback = SearchRecordFileByDate;
	info.timing_callback = MeshareNtpCallback;
	info.aes_key = GetAesKey();
	info.on_md_region_ctrl = MdRegionCtrlCallback;
	#ifdef ZINK
	info.on_device_online_callback = OnDeviceOnline;
	#endif
	printf("\n\n\n\n\n\n\n\n\n\n\n\n---------------------->%s\n",device_id);
	p2p_init(&info, sizeof(info));
	#endif
	return NULL;
}
void P2P_Start()
{
	#ifdef SUPPORT_P2P
	pthread_t thr;

	pthread_create(&thr, NULL, p2p_start_thread, NULL);
	#endif
}
int  CheckRecordTime()
{
	return SETNTPTIME;
}
int  NtpIfEnable()
{
	PARAMETEREXTEND ext;

	PubGetSysParameter(EXTEND_SET, &ext);

	return ext.m_ntp.m_ntp_switch;
}

void MeshareNtpCallback(time_t seconds)
{
	printf("ntp call back:%lu\n", seconds);
	PARAMETEREXTEND ntp;
	
	struct timespec tv_set;
	
	PubGetSysParameter(EXTEND_SET, &ntp);
	
	seconds += ntp.m_ntp.m_diff_timezone;
	
	tv_set.tv_sec = seconds;
	tv_set.tv_nsec = 0;
	
	clock_settime(CLOCK_REALTIME, &tv_set);
	SETNTPTIME = 1;
}

static char g_aes_key[36]={0};
void InitDeviceAesKey()
{

  uuid_t uu;
  memset(g_aes_key, 0, 36);  
  if(access(DEVICE_AES_KEY, F_OK))
  {
		uuid_generate( uu );
		for(int i=0;i<16;i++)
		{
		  sprintf(g_aes_key+i*2, "%02X",uu[i]);
		}  
		FILE *file = fopen(DEVICE_AES_KEY, "wb");
		if(file)
		{
		  fwrite(g_aes_key, 1, 32, file);
		  fclose(file);
		  printf("write aes key [%s]\n", g_aes_key);
		}
		else
		{
		  printf("create file [%s] error!\n", DEVICE_AES_KEY);
		}
  }
  else
  {
		FILE *file = fopen(DEVICE_AES_KEY, "rb");    
		if(file)
		{
			fread(g_aes_key, 1, 32, file);
			fclose(file);
			printf("get aes key [%s]\n", g_aes_key);
		}
		else
		{
			printf("open file [%s] error!\n", DEVICE_AES_KEY);
		}
  } 

}
char *GetAesKey()
{
	#ifdef VIDEO_AES_ENC
	
	return g_aes_key;	
	#else
	return (char *)"";
	#endif
	
}
int AesEncrypt(unsigned char *input, unsigned char *output)
{ 
	mbedtls_aes_context ctx; 
	int ret; 
	unsigned char key_str[100]={0}; 
	unsigned char iv_str[100]={0}; 
	strcpy((char*)key_str,(const char*)GetAesKey()); 

	mbedtls_aes_init( &ctx ); 

	mbedtls_aes_setkey_enc( &ctx, key_str, 256 ); 
	ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, 256, iv_str, input, output); 
	//printf("enc ret %d\n",ret); 
	mbedtls_aes_free( &ctx ); 
	return 0; 
}

int AesDecrypt(unsigned char *input, unsigned char *output)
{
	#if 0
	mbedtls_aes_context ctx;	
	int ret;
	
	unsigned char key_str[100]={0};
    unsigned char iv_str[100]={0};
	strcpy((char*)key_str,(const char*)"12345678123456781234567812345678");
	mbedtls_aes_init( &ctx );
	//key_len = unhexify( key_str, hex_key_string );
   //unhexify( iv_str, hex_iv_string );
	//printf("data len %d\n", data_len);

	mbedtls_aes_setkey_dec( &ctx, key_str, 256 );
    ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, 256, iv_str, input, output);
	//printf("enc ret %d\n",ret);
	mbedtls_aes_free( &ctx );
	#endif
	return 0;
}
/*
函数功能:时间转换函数
输入参数:SystemDateTime
输出参数:
返	回	 值:秒数
使用说明:把系统时间，转换为自1900年1月1日经过的秒数
*/
int SysTime2Time(SystemDateTime *pSysTime)
{
	struct tm myTm;
	myTm.tm_year = pSysTime->year +2000 -1900;
	myTm.tm_mon = pSysTime->month - 1;
	myTm.tm_mday =  pSysTime->mday;
	myTm.tm_hour = pSysTime->hour;
	myTm.tm_min = pSysTime->minute;
	myTm.tm_sec = pSysTime->second;
	
	return mktime(&myTm);
}

int ForceIdrInsertion(int channel,int streamtype)
{
	return ResetUserData2IFrame(channel,streamtype,0);
}

/***************************************************************************** 
函数功能: 
输入参数:@userid:用户id(1~MAX_BUFUSER_ID ) 
@streamtype 为码流类型0 :720 1:VGA 
输出参数:@buffer: 返回帧的起始地址 
@pFrameInfo:返回此帧的信息 
返 回 值:成功返回0，否则返回-1 
使用说明:返回失败，请勿使用buffer 指针进行数据的获取 
******************************************************************************/ 

int GetOneFrame4Record(int channel,int streamtype, int userid, unsigned char **buffer, FrameInfo *m_frameinfo)
{ 
	if(pBufferManage[streamtype]!=NULL)
	return pBufferManage[streamtype]->GetOneFrameFromBuffer(userid, buffer, m_frameinfo);

	return -1;
}



#if 1
int ZD_DEBUG_Printf(const char *fmt, ...)
{
	char buffer[512];
	va_list argptr;
	int cnt;
	
	va_start(argptr, fmt);
	cnt = vsnprintf(buffer,sizeof(buffer), fmt, argptr);
	va_end(argptr);

	printf("\033[32m%s\033[0m\n",buffer);	
	
	return(cnt);
}
#else
#define ZD_DEBUG_Printf(x...)
#endif
/**
BlockManage API
*/
static BlockDevManage *pBlockDevManage;
/*****************************************************************************
函数名称:
函数功能:初始化块设备模块
输入参数:无
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明: 
******************************************************************************/
int InitBlockDevice()
{
	pBlockDevManage = BlockDevManage::GetInstance();
	if(pBlockDevManage == NULL)
	{
		printf("err,pBlockDevManage = NULL\n");
		exit(-1);
	}

	return S_SUCCESS;
}
/*****************************************************************************
函数名称:
函数功能:获取块设备数量
输入参数:无
输出参数:无
返	回	 值: 块设备数量
使用说明: 
******************************************************************************/
int GetBlockDeviceNum()
{
	return pBlockDevManage->GetBlockDeviceNum();
}
/*****************************************************************************
函数名称:
函数功能:获取块设备信息
输入参数:enBlkDevId
输出参数:pstBlockDevInfo
返	回	 值: 0: 成功-1: 失败
使用说明: 
******************************************************************************/
int GetBlockDeviceInfo(BLK_DEV_ID enBlkDevId, BlockDevInfo_S *pstBlockDevInfo)
{
	if(pBlockDevManage==NULL ||((ConfigInfo.SupportInfo)&CONFIG_RECODE) <= 0)
	{
		return S_FAILURE;
	}
	BLK_DEV_INFO stBlkDevInfo;
	memset(&stBlkDevInfo,0,sizeof(BLK_DEV_INFO));
	memset(pstBlockDevInfo,0,sizeof(BlockDevInfo_S));

	pBlockDevManage->GetBlockDeviceInfo(enBlkDevId,  &stBlkDevInfo);
	if(g_bFormatSDCard)
	{
		printf("[GetBlockDeviceInfo]    formating~~!~~~!!~!~!~!\r\n");
		pstBlockDevInfo->m_u8Exist = 3;
	}
	else
	{
		if(stBlkDevInfo.enBlkDevStatus == BLK_DEV_STATUS_NOT_EXIST)
			pstBlockDevInfo->m_u8Exist = 0;
		else if(stBlkDevInfo.enBlkDevStatus == BLK_DEV_STATUS_NOT_MOUNTE)
			pstBlockDevInfo->m_u8Exist = 1;
		else if(stBlkDevInfo.enBlkDevStatus == BLK_DEV_STATUS_MOUNTED)
			pstBlockDevInfo->m_u8Exist = 2;
	}
	
	pstBlockDevInfo->m_u32Capacity = stBlkDevInfo.u32TotalCapacityMB;
	pstBlockDevInfo->m_u32FreeSpace = stBlkDevInfo.u32FreeSpaceMB;
	strcpy((char *)pstBlockDevInfo->m_cDevName,stBlkDevInfo.strDeviceNode);
	return S_SUCCESS;
}
int GetBlockDeviceInfo(int type, BlockDevInfo_S * Info)
{
	if(((ConfigInfo.SupportInfo)&CONFIG_RECODE) > 0)
	{
		return GetBlockDeviceInfo((BLK_DEV_ID)type,Info);
	}
	return 0;
}
int GetMountInfo()
{
	if(pBlockDevManage==NULL ||((ConfigInfo.SupportInfo)&CONFIG_RECODE) <= 0)
	{
		return S_FAILURE;
	}
	if(pBlockDevManage->stBlkDevInfo[0].enBlkDevStatus == BLK_DEV_STATUS_MOUNTED)
	{
		return 0;
	}
	return S_FAILURE;

}

/*****************************************************************************
函数名称:
函数功能:挂载块设备
输入参数:enBlkDevId
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明: 
******************************************************************************/
int MountBlockDevice(BLK_DEV_ID enBlkDevId)
{
	return pBlockDevManage->MountBlockDevice(enBlkDevId);
}
/*****************************************************************************
函数名称:
函数功能:卸载块设备
输入参数:enBlkDevId
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明: 
******************************************************************************/
int UmountBlockDevice(BLK_DEV_ID enBlkDevId)
{
	return pBlockDevManage->UmountBlockDevice(enBlkDevId);
}
/*****************************************************************************
函数名称:
函数功能:格式化块设备
输入参数:enBlkDevId
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明: vfat格式
******************************************************************************/
int  FormatBlockDevice(BLK_DEV_ID enBlkDevId)
{
	return pBlockDevManage->Fat32Format(enBlkDevId);
}

/*
Record API
*/
static RecordManage *pRecordManage;
/*****************************************************************************
函数名称:
函数功能:初始化录像模块
输入参数:无
输出参数:无
返	回	 值: 0: 成功-1: 失败
使用说明: 
******************************************************************************/
int InitRecordManage()
{
	pRecordManage = RecordManage::GetInstance();
	if(pRecordManage == NULL)
	{
		printf("err,pRecordManage = NULL\n");
		exit(-1);
	}

	return S_SUCCESS;
}
/*****************************************************************************
函数功能:启动录像模块线程
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int StartRecordSystem()
{
	if(pRecordManage==NULL)
		return 0;

	if(GetBlockDeviceNum() == 0)
		return S_FAILURE;
	if(GetMountInfo()==-1)
	{
		printf("============== mount error\n");
		return S_FAILURE;
	}
		
	plog("StartRecordSystem\r\n");
	return pRecordManage->StartRecordSystem();
}
/*****************************************************************************
函数功能: 停止录像模块线程
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1, 
使用说明:
******************************************************************************/
int StopRecordSystem()
{
	if(GetBlockDeviceNum() == 0)
		return S_FAILURE;
	if(pRecordManage==NULL)
		return 0;
	plog("StopRecordSystem~\r\n");
	return pRecordManage->StopRecordSystem();
}
void SetRecSchedule(GROUPRECORDTASK *pRecSchedule)
{
	if(((ConfigInfo.SupportInfo)&CONFIG_RECODE) > 0&&pRecordManage)
		pRecordManage->SetRecSchedule(pRecSchedule);
	else
		return;
}
int  FindRecordFile(FindFileType *findType,RecordFileName *RecordFile,REC_SEARCH enMode)
{
	if(((ConfigInfo.SupportInfo)&CONFIG_RECODE) > 0&&pRecordManage)
		return pRecordManage->FindRecordFile(findType,RecordFile,enMode,DISPLAY_MAX_NUM);
	else
		return -1;
}
int  HaveRecordFile(struct tm time)
{
	if(((ConfigInfo.SupportInfo)&CONFIG_RECODE) > 0&&pRecordManage)
		return pRecordManage->HaveRecordFile(time);
	else
		return -1;
}
void SetMdAlarmLink(unsigned int u32MdAlarmLink,RECORD_LEVEL enRecordLevel)
{

	if(((ConfigInfo.SupportInfo)&CONFIG_RECODE) > 0&&pRecordManage)
		return pRecordManage->SetMdAlarmLink(u32MdAlarmLink,enRecordLevel);
	else
		return;
}	
void SetExAlarmLink(unsigned int u32ExAlarmLink,RECORD_LEVEL enRecordLevel)
{
	if(((ConfigInfo.SupportInfo)&CONFIG_RECODE) > 0&&pRecordManage)
		return pRecordManage->SetExAlarmLink(u32ExAlarmLink,enRecordLevel);
	else
		return;
}	
void SetMwAlarmLink(unsigned int u32MwAlarmLink,RECORD_LEVEL enRecordLevel)
{
	if(((ConfigInfo.SupportInfo)&CONFIG_RECODE) > 0&&pRecordManage)
		return pRecordManage->SetMwAlarmLink(u32MwAlarmLink,enRecordLevel);
	else
		return;
}	
void SetSD433AlarmLink(unsigned int u32SD433AlarmLink,RECORD_LEVEL enRecordLevel)
{
	if(((ConfigInfo.SupportInfo)&CONFIG_RECODE) > 0&&pRecordManage)
		return pRecordManage->SetSD433AlarmLink(u32SD433AlarmLink,enRecordLevel);
	else
		return ;
}		
void SetAudioAlarmLink(unsigned int u32AudioAlarmLink,RECORD_LEVEL enRecordLevel)
{
	if(((ConfigInfo.SupportInfo)&CONFIG_RECODE) > 0&&pRecordManage)
		return pRecordManage->SetAudioAlarmLink(u32AudioAlarmLink,enRecordLevel);
	else
		return;
}	
/*
功能:给出264的绝对路径，以及点播时间，返回大于点播时间的第一个I帧，在264文件中的偏移
输入:path,264的绝对路径
	start_time,点播时间,hour*3600+min*60+sec
输出:pOffset
返回:0,成功
	-1，文件不能打开，或者点播时间不合理，或者pOffset为空
*/
int getRecordOffset(const char* path, unsigned int start_time, unsigned int *pOffset)
{
	if(pRecordManage)
	return pRecordManage->GetRecordOffset(path, start_time, pOffset);

	return -1;
}
/*
功能:把zsp协议的录像段回放偏移，转换为文件的物理位置偏移
输入:FileName，录像文件
            Startime，事件的开始时间
            Endtime，事件的结束时间
输出:FilePos，文件的物理位置偏移
	     FileLen，该事件的录像的大小(不包含512字节文件头)
返回:0，成功
	     -1，失败
*/
int ChangeClientOffset(char* FileName, unsigned int Startime, unsigned int Endtime, unsigned int* FilePos, unsigned int* FileLen)
{	
	if(FileName == NULL 
		|| FilePos == NULL
		|| FileLen == NULL
		|| strlen(FileName) == 0)
	{
		return S_FAILURE;
	}
	
	char dir[64] = {0};
	memset(dir,0,sizeof(dir));
	memcpy(dir,FileName,29);//拷贝后，为/hdd00/p01/record/2014-12-30/	

	int m_RecCh = FileName[64];
	if(m_RecCh >= 'A')
		m_RecCh = m_RecCh - 'A' + 10;
	else
		m_RecCh = m_RecCh - '0';
		
	EVEN_ITEM stEvenItem;
	stEvenItem.m_RecCh = m_RecCh;
	stEvenItem.start_time = Startime;
	stEvenItem.end_time = Endtime;	

	
	int s32Ret;
	s32Ret = pRecordManage->CheckOneEvenFromList(dir,&stEvenItem);
	if(s32Ret != S_SUCCESS)
	{
		return S_FAILURE;
	}

	*FilePos = stEvenItem.m_StartOffset;
	*FileLen = stEvenItem.m_EndOffset - stEvenItem.m_StartOffset;

	printf("%s(%d)->Startime=%d,Endtime=%d,*FilePos=%d,*FileLen=%d\n",__FILE__,__LINE__,Startime,Endtime,*FilePos,*FileLen);
	
	return S_SUCCESS;
}

/*****************************************************************/
int SetMdRegion(P2P_MD_REGION_CHANNEL* region)
{
	P2P_MD_REGION_CHANNEL mdset;
	memset(&mdset,0x0,sizeof(P2P_MD_REGION_CHANNEL));

	g_cParaManage->GetSysParameter(MD_SET,&mdset);
	mdset.x = region->x;
	mdset.y = region->y;
	mdset.width= region->width;
	mdset.height= region->height;
	g_cParaManage->SetSystemParameter(MD_SET, &mdset);
	
	if(phd)SetROI(phd,1280*mdset.x,720*mdset.y,1280*mdset.width,720*mdset.height,0);
	printf("ZSP---------------------SetMdRegion[%f,%f,%f,%f]\n",mdset.x,mdset.y,mdset.width,mdset.height);

	return 0;
}
int GetMdRegion(P2P_MD_REGION_CHANNEL* region)
{
	P2P_MD_REGION_CHANNEL mdset;
	memset(&mdset,0x0,sizeof(P2P_MD_REGION_CHANNEL));

	g_cParaManage->GetSysParameter(MD_SET,&mdset);
	region->x =mdset.x;
	region->y =mdset.y;
	region->width=mdset.width;
	region->height=mdset.height;
	printf("ZSP---------------------GetMdRegion[%f,%f,%f,%f]\n",mdset.x,mdset.y,mdset.width,mdset.height);

	return 0;
}
/*
  * 区域侦测的配置或者获取
  * @op_type 1 为设置， 2为获取
  */

void MdRegionCtrlCallback(int op_type, p2p_md_region_t* reg)
{
	P2P_MD_REGION_CHANNEL mdset;
	memset(&mdset,0x0,sizeof(P2P_MD_REGION_CHANNEL));
	g_cParaManage->GetSysParameter(MD_SET,&mdset);
	if(op_type ==1)
	{
		mdset.x = reg->x;
		mdset.y = reg->y;
		mdset.width= reg->width;
		mdset.height= reg->height;
		g_cParaManage->SetSystemParameter(MD_SET, &mdset);
		printf("P2P---------------------SetMdRegion[%f,%f,%f,%f][%f,%f,%f,%f]\n",reg->x,reg->y,reg->width,reg->height,mdset.x,mdset.y,mdset.width,mdset.height);
		if(phd)SetROI(phd,1280*mdset.x,720*mdset.y,1280*mdset.width,720*mdset.height,0);
	}
	else if(op_type==2)
	{
		reg->x =mdset.x;
		reg->y =mdset.y;
		reg->width=mdset.width;
		reg->height=mdset.height;		
		printf("P2P---------------------GetMdRegion[%f,%f,%f,%f]\n",mdset.x,mdset.y,mdset.width,mdset.height);
	}

}

int GetOSPQueueSendQuenceMsgID()
{
	static int QuenceMsgID = -1;
	if(QuenceMsgID<0)
	{
		
		int ret =OSPQueueCreate((char *)MESSAGENAME,&QuenceMsgID);
		if(ret<0||QuenceMsgID<0)
		{
			printf("\n\n\n\n\n\n\n %s========================= OSPQueueCreate faild m_QuenceMsgID:%d,ret:%d\n",__FUNCTION__,QuenceMsgID,ret);
			//return false;
		}	
		printf("\n\n\n\n\n\n\n %s========================= OSPQueueCreate Success m_QuenceMsgID:%d\n",__FUNCTION__,QuenceMsgID);

	}
	return QuenceMsgID;	
}
bool GetDeviceStatus()
{
	return device_on;
}


