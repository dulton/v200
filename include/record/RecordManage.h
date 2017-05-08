
#ifndef __RECORD_MANAGE_H_

#define __RECORD_MANAGE_H_

#include <time.h>
#include <pthread.h>

#include "common.h"
#include "systemparameterdefine.h"
#include "BufferManage.h"

#define DISPLAY_MAX_NUM (10)   //每次搜索文件的个数

typedef struct TagFindFileType		//查询条件使用的结构体
{
	struct tm time;					//查询时间
	int		m_Res1;					 //未用，设备类型0-HDD 1-MHDD 2-SD，
	int 	channel;                                    //通道1~16
	int	RecordType;                             //录像类型: 0-全部 1-报警 2-移动侦测 3-定时 5-手动
	int	m_Res2;                             //未用，查询方式 0 ，录像类型查询，1 司机名查询 2   车牌号查询
	char	 m_Res3[17];				//未用
	char	 m_Res4[17];				//未用
}FindFileType;

typedef struct tag_rec_diren	   //一条录像记录使用的结构体
{
	unsigned short	level; 		    //录像级别，0-无告警，1，轻度告警，2-严重告警
	unsigned short	m_res2;		    //未用
	unsigned int 		start_time;       //文件开始时间
	unsigned int 		end_time;	    // 文件结束时间
	unsigned int         	filesize;            //文件大小以K为单位，注意:由于历史原因，字节除以1000，而不是除以1024
	unsigned int	     	channel; 	    // 通道号
	int				m_filetype;      //文件类型，参考RECORD_KIND
	int				m_res3;  //告警类型，未用
	char				d_name[96];     //文件名(包含绝对路径)
}rec_dirent;

typedef struct								//查询结果使用的结构体 
{
	int	fileNb;								//文件个数
	rec_dirent    namelist[DISPLAY_MAX_NUM];	//查询结果
}RecordFileName;

typedef enum
{
	REC_SEARCH_SET,  		//从起点开始搜索 
	REC_SEARCH_NEXT 	      //从当前点往后搜索
}REC_SEARCH;

typedef struct 
{
	unsigned int		m_magic; // 魔数
	unsigned int		m_size;  // 结构体大小
}VIDEO_STHDR;
#define RECFILEHEADERMAGIC			0x12345678
#define IDX_MAGIC					0x08070620
#define FILELIST_HEADER_SIGNATURE	0x12345678
#define FILELIST_ITEM_SIGNATURE	0x20080610

typedef struct 
{
	unsigned char 		m_bitswidth;  // 音频采样的位宽
	unsigned char 		m_reserved1[3];
	unsigned int			m_bitRate;// 码率
	unsigned int			m_sampleRate;// 采样率
}ENCAUDIO_PARA;

typedef struct
{
	unsigned char				chlIndex;   // 通道号
	unsigned char				frameRate; // 帧率
	unsigned char				resolution; // 清晰度
	unsigned char				reserved1;
	char						chlName[16]; // 通道名称
}ENC_CH_PARA;

typedef struct 						//264文件头，一共512字节，其中400字节保留
{
	VIDEO_STHDR			m_Hdr;
	char						m_VerDev[28];  //设备名称
	char						m_VerFile;	// 文件版本
	unsigned char				m_BegYear; 
	unsigned char				m_BegMonth;
	unsigned char				m_BegDay;
	unsigned char				m_BegHour;
	unsigned char				m_BegMinute;
	unsigned char				m_BegSecond;  // 开始的年月日时分秒
	unsigned char				m_EndYear;
	unsigned char				m_EndMonth;
	unsigned char				m_EndDay;
	unsigned char				m_EndHour;
	unsigned char				m_EndMinute;
	unsigned char				m_EndSecond;// 结束的年月日时分秒
	unsigned char				m_ChlCount;  // t通道数目
	unsigned char				m_RecType;	// 录像类型
	unsigned char				m_FileProtected;//录像是否保护
	unsigned char				m_RecLevel;//录像等级
	unsigned char				m_VideoType; // 视频制式,0表示NTSC;1表示PAL
	unsigned char				m_bNonStop;// 早期保留未用，当前版本，0:正常结束，1:非正常结束，如断电;
	unsigned char				m_bEncrypt; // 视频数据是否加密，0:没有加密，1:有加密
	ENCAUDIO_PARA			m_Audio;  // 音频编码参数
	unsigned int				m_IndexTblOffset;// 索引表的相对文件偏移，注意，为了简化逻辑，保留idx文件，不追加索引到末尾，此值一直为0
	unsigned int				m_MovieOffset; //真实的录像数据位置如果为0XFFFF 为无效偏移(其数值没有加0x200, 真实Iframe的位置是m_MovieOffset+0x200)
	unsigned short			m_nDevType; //设备类型数字编号
	char						m_Reserved3;
	char						m_DateShowFmt;// 日期显示的格式暂时保留
	char						m_cDevType[12];// 设备类型字符串编码
	ENC_CH_PARA			m_ChInfo; //通道的视频编码参数

	char			recv[400];
}VideoFileInfo;

typedef struct	 //Idx文件头结构体
{
	unsigned int	m_magic;	// 魔数0x08070620
	unsigned 	int	m_header; // 固定为idx1
	unsigned int	m_block_size; 	// I帧索引区，或事件索引区的大小，不包含IDX_HEADER的大小。
}IDX_HEADER;
#define		IDX_MAGIC							0x08070620
#define		IDX_FLAG							0x31584449

typedef struct 	//idx文件的一条记录
{
	unsigned long m_iframetime; //*时间戳以秒为单位3600*hour+60*minute+second 
	unsigned long m_offset; 
	unsigned int m_sampleRate;// 采样率 
	unsigned int m_bitswidth; // 音频采样的位宽 
	unsigned char m_RecType; // 录像类型,参考RECORD_KIND
	unsigned char m_VideoType; // 视频制式 ,参考MACHINE_PARA，1 NTSC, 0  PAL
	unsigned char m_FrameRate; // 帧率 
	unsigned char m_Resolution; // 清晰度 ,参考Resolution_E
	unsigned char m_RecLevel; // 录像等级 ,参考RECORD_LEVEL
	unsigned char m_res[3]; //保留字段 	
}IFRAMELISTCONTENT;

typedef struct tag_LISTFILE_FILEHEADER //列表文件文件头部定义
{
	unsigned long 			m_signature; 		//此结构体的标识
	char					m_verDev[28]; 		// 未用
	char					m_verFile;    		//录像文件的版本，不同型号的设备可以有自己的版本信息，目前为8
	char					m_begYear;
	char					m_begMonth;
	char					m_begDay;
	char					m_begHour;
	char					m_begMinute;
	char					m_begSecond;
	char					m_endYear;
	char					m_endMonth;
	char					m_endDay;
	char					m_endHour;
	char					m_endMinute;
	char					m_endSecond;
	char 				m_pad1;			//保留
	unsigned short 		m_total_files; 		//文件列表中的总记录个数,包含被删除的记录
	unsigned short 		m_cur_file;   		//未用
	unsigned short		m_usdevType;		//未用
	char					m_szdevType[12];	//未用
	unsigned short		m_alarmfilecnt; 		//报警录像个数
	unsigned short		m_mdfilecnt; 		//移动侦测录像个数
	unsigned short		m_timerfilecnt; 		//定时录像个数
	unsigned short		m_maunalfilecnt; 	//手动录像个数
	unsigned char			m_downloaded;  		//未用
	char 				m_reserved[27];
}LISTFILE_FILEHEADER;

#if 0
typedef struct tag_LISTFILE_NAME	//列表中，每条文件记录结构体定义
{
	unsigned long 		m_signature; 		//每条记录的标志头部
	char 			m_name[96];		//文件的名称(包涵路径)
	unsigned short 	m_state; 			//文件删除标志 =0x55aa:文件被删除,!=0x55aa:文件存在,初始值为0
	unsigned short 	M_filestat;     		// 文件0: creat , 1: close 
	int 			  	m_start_hms;		//该文件生成的时间.=h*3600+m*60+s
	unsigned char 	m_record_type; 		//0:手动录像	1:定时录像2:报警录像3:移动帧测录像
	unsigned char		m_RecLevel;			//录像等级
	char 			m_channel;			//通道，1~16
	char 			m_pad2[1];			//未用
	int				m_end_time;  		//该文件结束的时间.=h*3600+m*60+s
	char 			m_drivername[17];	//未用
	char				m_vehiclenum[17];	//未用
	unsigned char		m_download; 		//是否锁定文件0x55:锁定其它:不锁定，未用
	char				m_pad3[20];			//未用
}LISTFILE_NAME;
#else
typedef struct				//定义事件
{
	unsigned int m_RecCh; // 录像通道,注意:从1开始编号
	unsigned int m_RecType; // 录像类型 ,参考RECORD_KIND
	unsigned int m_VideoType; // 视频制式，0表示pal，1表示ntsc
	unsigned int	m_FrameRate; // 帧率
	unsigned int	m_Resolution; // 清晰度，参考Resolution_E
	unsigned int  m_sampleRate;// 采样率,8000
	unsigned int m_bitswidth; // 音频采样的位宽,16
	unsigned char m_RecLevel;//参考RECORD_LEVEL
	unsigned char m_res[7]; //保留字段
	unsigned int 	start_time; /*事件开始时间*/
	unsigned int 	end_time;	/*事件结束时间*/
	int m_StartOffset;//录像片段，在文件中的开始偏移
	int m_EndOffset; //录像片段，在文件中的结束偏移
} EVEN_ITEM;

typedef struct tag_LISTFILE_NAME
{
	unsigned long 	m_signature; 	/*每条记录的标志头部*/
	char 			m_name[96];		/*文件的名称(包涵路径)*/
	unsigned short 	m_state; 		/*文件删除标志 =0x55aa:文件被删除,!=0x55aa:文件存在,初始值为0*/
	char 			m_pad2[2];	
	EVEN_ITEM 	stEvenItem;        
}LISTFILE_NAME;
#endif

typedef enum
{
	ITEM_STATE_OK = 0x0000,
	ITEM_STATE_DELETED = 0x55aa
}ITEM_STATE;
typedef enum
{
	ITEM_FILE_STAT_CREAT = 0x0,
	ITEM_FILE_STAT_CLOSE = 0x1
}ITEM_FILE_STAT;

typedef enum				//内部使用的枚举量
{
	RECORD_STOP,
	RECORD_PALSE,
	RECORD_CONTINUE,
	RECORD_START
}RECORDCTLCMD;
typedef enum				//内部使用的枚举量
{
	RECORD_STOPED,
	RECORD_PALSEED,		
	RECORD_ING	
}RECORDCTLSTATUS;
typedef enum				//录像类型枚举量
{
	TIMER_RECORD = 1,
	ALARM_RECORD = 2,
	MD_RECORD = 3,
	MW_RECORD = 4,//MICROWALE
	SD433_RECORD = 5,
	AUDIO_RECORD = 6
}RECORD_KIND;
typedef enum				//录像级别枚举量
{
	RECORD_LEVEL1 = 0,//非实时
	RECORD_LEVEL2 = 1,//录像未用到这个等级，如果设置这个等级，为非实时录像
	RECORD_LEVEL3 = 2//实时
}RECORD_LEVEL;

/*内部使用的宏定义*/
#define MIN_SYS_ALARM_DELAY			10/*此宏是为了保证各种报警录像最小延时一致*/
#define RECMD_DELAY				       MIN_SYS_ALARM_DELAY
#define RECALARM_DELAY			              MIN_SYS_ALARM_DELAY
#define MAX_RECFILENAMELEN			128
#define START_DEL_FILE					(300)//磁盘可用空间小于START_DEL_SPACE，开始删除，单位MB
#define STOP_DEL_FILE					(600)//磁盘可用空间大于STOP_DEL_FILE，停止删除，单位MB
#define RECORD_LIST_FILENAME			"recfilelist"//列表文件文件名
#define MAX_EVEN_NUM 					(3600/MIN_SYS_ALARM_DELAY)
//注意1:
//录像模块，不检测磁盘是否满
//外部检测，如果磁盘满而且不覆盖，则调用接口，停止录像
//如果可覆盖，磁盘已满，则调用接口，删除录像

//注意2:
//由宏定义控制外部告警录像时长，移动录像时长

//注意3:
//如果移动侦测开关，从开启转为关闭，外部应该调用SetMdAlarmLink，关闭录像
//如果移动侦测开关，从关闭转为开启，那么，在发送移动时，调用SetMdAlarmLink，告知录像模块，可以进行移动侦测录像
class RecordManage{
private:
	/*第一部分:控制逻辑*/
	static RecordManage * m_pInstance;
	pthread_t ctrlThreadId;	
	pthread_t dataThreadId;		
	int s32CtrlThreadQuit;
	int s32DataThreadQuit;
	SystemDateTime  m_datetime;//记录系统当前时间的变量
	SystemDateTime  m_preTime; //上一次检测时间跳变时的时间
	GROUPRECORDTASK m_RecSchedule;//定时录像、告警录像的时间段

	/*两个线程交互使用的变量*/
	RECORDCTLCMD			m_uRecordCmd[MAX_VIDEO_CHANNEL]; 	//写录像命令
	RECORDCTLSTATUS      	m_uRecordStatus[MAX_VIDEO_CHANNEL]; //写录像状态

	/*通道相关的变量*/	
	int 						m_uTimerValid[MAX_VIDEO_CHANNEL];//定时录像时间段是否有效
	int 						m_uEventValid[MAX_VIDEO_CHANNEL];	//外部告警录像时间段是否有效
	int 						m_uMDvalid[MAX_VIDEO_CHANNEL];	//移动录像时间段是否有效
	int 						m_uMwValid[MAX_VIDEO_CHANNEL];//微波告警录像时间段是否有效
	int 						m_uSD433valid[MAX_VIDEO_CHANNEL];	//门磁录像时间段是否有效
	int 						m_uAudioValid[MAX_VIDEO_CHANNEL];//声音告警录像时间段是否有效
	
	RECORD_KIND  			m_uRecordType[MAX_VIDEO_CHANNEL];//当前正在进行的录像类型
	RECORD_LEVEL  			m_uRecordLevel;//录像的等级
	RECORD_LEVEL  			m_uRecordLevelAlarm;//外部报警的录像等级
	RECORD_LEVEL  			m_uRecordLevelMd;//移动报警的录像等级
	RECORD_LEVEL  			m_uRecordLevelMw;//微波报警的录像等级
	RECORD_LEVEL  			m_uRecordLevelSD433;//门磁报警的录像等级
	RECORD_LEVEL  			m_uRecordLevelAudio;//声音报警的录像等级	
	RECORD_LEVEL  			m_uRecordLevelTimer;//定时报警的录像等级	
	
	int						m_RecStartTime[MAX_VIDEO_CHANNEL];//录像开始时间
	int						m_RecEndTime[MAX_VIDEO_CHANNEL];//录像结束时间	
	SystemDateTime  			m_stEndTime[MAX_VIDEO_CHANNEL];//录像文件的结束时间	
	
	int 						m_uAlarmRecDelay[MAX_VIDEO_CHANNEL];//外部告警录像延时
	int 						m_uMDRecDelay[MAX_VIDEO_CHANNEL];//移动录像延时
	int 						m_uMwAlarmRecDelay[MAX_VIDEO_CHANNEL];//外部告警录像延时
	int 						m_uSD433AlarmRecDelay[MAX_VIDEO_CHANNEL];//移动录像延时
	int 						m_uAudioAlarmRecDelay[MAX_VIDEO_CHANNEL];//外部告警录像延时
	
	unsigned int				m_u32ExAlarmLink;//外部告警录像，1个通道，1个比特	
	unsigned int				m_u32MdAlarmLink;//移动侦测录像，1个通道，1个比特	
	unsigned int				m_u32MwAlarmLink;//微波侦测录像，1个通道，1个比特	
	unsigned int				m_u32SD433AlarmLink;//门磁录像，1个通道，1个比特	
	unsigned int				m_u32AudioAlarmLink;//外部告警录像，1个通道，1个比特
	
	int						m_AudioStartTime[MAX_VIDEO_CHANNEL];//声音开始时间
	int						m_MDStartTime[MAX_VIDEO_CHANNEL];//移动开始时间	
	
	char 					m_264FileName[MAX_VIDEO_CHANNEL][MAX_RECFILENAMELEN];//包含路径
	char 					m_IdxFileName[MAX_VIDEO_CHANNEL][MAX_RECFILENAMELEN];//包含路径	
	VideoFileInfo				m_FileInfo[MAX_VIDEO_CHANNEL];
	FILE						*m_pRecFp[MAX_VIDEO_CHANNEL];
	int 						m_index_fds[MAX_VIDEO_CHANNEL];
	unsigned long 				m_offset[MAX_VIDEO_CHANNEL];								//I帧地址偏移

	
	/*磁盘相关的变量*/
	int 						m_uHddEio[MAX_VIDEO_CHANNEL];	//写录像过程中，是否发生了EIO错误
	int 						m_nCurrentHddNum;			//当前正在录像的磁盘
	int                                       m_s32HddFull;				//磁盘满
	char 					m_strRecDir[MAX_RECFILENAMELEN];				//当前录像目录
	unsigned char				m_uHddOverWrite;			//磁盘满覆盖标记
		
	//文件搜索相关的变量
	int 						m_s32IdxOffset;


	void InitRecordManage();
	//控制线程的主要任务
	void CheckRecordTask();	

	//子任务
	void CheckHdd();
	void CheckTimeValid();
	void  CheckRecordStart();
	void CheckTimeJump();
	void  CheckRecordEnd();	
	void CheckAlarmDelay();

	//控制函数
	int StopOneChannelRecord(int ch);	
	int StartOneChannelRecord(int ch,RECORD_KIND enRecordKind);	
	int PauseOneChannelRecord(int ch);	
	int ContinueOneChannelRecord(int ch,RECORD_KIND enRecordKind);	
	void RestartOneChannelRecord(int ch,RECORD_KIND enRecordKind);	
	void ChangeOneChannelRecord(int ch,RECORD_KIND enRecordKind);	

	//辅助函数
	int IsLowRecordType(int ch,RECORD_KIND enRecordKind);	
	int CheckAlarmRecordValid(int ch);
	int CheckMdRecordValid(int ch);	
	int CheckMwRecordValid(int ch);
	int CheckSd433RecordValid(int ch);
	int CheckAudioRecordValid(int ch);	
	int GetCurrentTimeValue();
	void RecordTime2SysTime(int s32RecordTime,SystemDateTime *pstSystemDateTime);	
	int GetRecordHour(SystemDateTime  *pstSystemDateTime);		
	int CheckTimerTable(unsigned char ch, RECORD_KIND enRecordKind);	
	int IsTimeJump();	
	int getYearMonthDay(int s32Year,int s32Month,int s32Day);
	int getOldRecordDir(char *pstrOldRecDir);
	int DelDir(char *pstrPathName);
	int DelOldRecord();	
	void DeleteHistoryVersionRecord();
	void fixNotCloseFile();
	int GetEndTimeFrom264(char *pstr264Name);
	int GetRecordOffsetFrom264(const char* pstr264Name, unsigned int start_time, unsigned int *pOffset);
	void UpdateListFile(char *pstrIdxName,int ch);
	void SetRecInfo(int ch,RECORD_KIND enRecordKind);
	
	/*第二部分:写数据*/
	//写数据子任务
	int OpenRecordFile(int ch);
	int WriteRecordFile(int ch);
	int CloseRecordFile(int ch);
	
	//辅助函数
	int TestAndCreateDir(char *dirname);
	int CreateRecordDirectory();
	int Create264File(int ch);
	int CreateIdxFile(int ch);	
	int CreateListFile(char *pstrListFile);
	int AddRecordFile2List(int ch);
	void PrepareRecordInfor(int ch);
	void ClearRecordInfor(int ch);
	int ModifyItemInList(int ch,SystemDateTime *pstEnd,char *pNewName);
	int Rewrite264Header(int ch);
	int AppendIdx(int ch);
	int Rename264AndFixItem(int ch,SystemDateTime *pstEnd);
	int FillEvenItem(int ch,EVEN_ITEM *pstEvenItem,IFRAMELISTCONTENT *pstIframeListContent,int start_time,int end_time,int s32StartOffset,int s32EndOffset,RECORD_LEVEL enRecordLevel);
	int FixEvenItenStartTime(EVEN_ITEM *pstEvenItem);
	int FixEvenItenEndTime(EVEN_ITEM *pstEvenItem);
	int ParserEvenFromIframeIndex(char *pstrIdxFileName, EVEN_ITEM *pstEvenItem, int *ps32EvenNum,int ch);
	int  AddOneEven2List(char *pstrDirName,EVEN_ITEM *pstEvenItem);
	int RederectFilePosition(int ch,int curtime);
	void Fill264Header(int ch,SystemDateTime  *pStart,SystemDateTime  *pEnd,
									unsigned int s32IndexTblOffs,unsigned int s32MovieOffset,int frameRate,int resolution,int bNonStop);	
	void FillListFileHeader(LISTFILE_FILEHEADER *pstListFileHeader,SystemDateTime  *pStart,SystemDateTime  *pEnd,
									unsigned short u16TotalFiles,
									unsigned short u16AlarmFileCnt,unsigned short u16MdFileCnt,
									unsigned short u16TimeFileCnt,unsigned short u16MaunalFileCnt);	
	void FillListFileName(LISTFILE_NAME *pstListFileName,char *pstrFileName,ITEM_STATE enState,
									ITEM_FILE_STAT enFileStat,int s32StartHms,unsigned char u8RecordType,int channel,int s32EndHms);
	void FillRecDirent(rec_dirent *pstRecDirent,RECORD_LEVEL enRecordLevel,int s32StartTime,int s32EndTime,int s32Size,
	                          int channel,RECORD_KIND enRecordKind,char *pstrFileName);	
	
public:
	static RecordManage* GetInstance();
	int StartRecordSystem();	
	int StopRecordSystem();	
	void SetRecSchedule(GROUPRECORDTASK *pRecSchedule);
	void SetMdAlarmLink(unsigned int u32MdAlarmLink,RECORD_LEVEL enRecordLevel);
	void SetExAlarmLink(unsigned int u32ExAlarmLink,RECORD_LEVEL enRecordLevel);
	void SetMwAlarmLink(unsigned int u32MwAlarmLink,RECORD_LEVEL enRecordLevel);
	void SetSD433AlarmLink(unsigned int u32SD433AlarmLink,RECORD_LEVEL enRecordLevel);
	void SetAudioAlarmLink(unsigned int u32AudioAlarmLink,RECORD_LEVEL enRecordLevel);
	
	void SetHddOverWrite(unsigned char u8HddOverWrite);
	int  FindRecordFile(FindFileType *findType,RecordFileName *RecordFile, REC_SEARCH enMode,int s32MaxItenNum);
	int  HaveRecordFile(struct tm time);
	int GetRecordOffset(const char* path, unsigned int start_time, unsigned int *pOffset);
	int  CheckOneEvenFromList(char *pstrDirName,EVEN_ITEM *pstEvenItem);
	
	void RecordDataProcess();	
	void RecordCtrlLoop();	
	RecordManage();
	~RecordManage();
};

#endif 
