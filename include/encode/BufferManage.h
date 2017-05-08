
#ifndef _BUFFER_MANAGE_H_
#define _BUFFER_MANAGE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include "CommonDefine.h"
#include "common.h"

//#define TEST_ENCODE_DATA  
//#define TEST_AENCODE_DATA  
#define MAX_FRAME_USER			MAX_BUFUSER_ID //	1
#define MAX_FRM_NUM			(30*150)
#define MAX_V_FRM_NUM			(30*30)
#define MAX_I_F_NUM			30

//最大预录时长
#define MAX_PRECORD_SEC		20//16
#define MAX_ARECORD_SEC		90

//720P buffer 大小
#define BUFFER_SIZE_720P	0x180000




#define	DEFAULT_IFRAMEOFFSET				0xffff

//#define ADJUST_STREAM
#ifdef ADJUST_STREAM
/*
IframeGop		I帧间隔			---------单位s
FrameDiffPos		读写指针间隔 ---------单位帧

*/
#define		IframeGop 			3
#define		FrameDiffPos		6
#endif
//帧类型
typedef enum{
	I_FRAME = 1,
	P_FRAME,
	A_FRAME,
	B_FRAME
}FrameType_E;

//系统日期时间结构体定义
typedef struct{
	char		 year;
	char		 month;
	char		 mday;
	char		 hour;
	char		 minute;
	char		 second;
	char		 week;		
	char		 reserve;
}SystemDateTime, *pSystemDateTime;

//一个视频帧或音频帧在缓冲池的信息结构体
typedef struct {
	unsigned long			FrmStartPos;/*此帧在buffer中的偏移*/
	unsigned long			FrmLength;  /*此帧的有效数据长度*/
	long long				Pts;			/*如果是视频帧，则为此帧的时间戳*/
	unsigned char			Flag;		/* 1 I 帧, 2 P 帧, 3 音频帧*/
	unsigned char			hour;		/*产生此帧的时间*/
	unsigned char			min;
	unsigned char			sec;
	venc_stream_t			venc_stream; /* 可能包含多个NAL单元 */
    bool                    talk;
}FrameInfo, *pFrameInfo;

typedef struct 
{
	unsigned int		m_nVHeaderFlag; // 帧标识，00dc, 01dc, 01wb
	unsigned int 		m_nVFrameLen;  // 帧的长度
	unsigned char		m_u8Hour;
	unsigned char		m_u8Minute;
	unsigned char		m_u8Sec;
	unsigned char		m_u8Pad0;// 代表附加消息的类型，根据这个类型决定其信息结构0 代表没有1.2.3 各代表其信息
	unsigned int		m_nILastOffset;// 此帧相对上一个I FRAME 的偏移只对Iframe 有用
	long long			m_lVPts;		// 时间戳
	unsigned int		m_bMdAlarm:1;/*bit0 移动侦测报警1:报警，0:没有报警*/
	unsigned int		m_FrameType:4;/*帧类型*/
	unsigned int 		m_Lost:1;
	unsigned int 		m_FrameRate:5;
	unsigned int		m_Res:21;	/*bit11-bit31 暂时保留*/
	unsigned int		m_nReserved;
}VideoFrameHeader;


typedef struct 
{
	unsigned int		m_nAHeaderFlag; // 帧标识，00dc, 01dc, 01wb
	unsigned int 		m_nAFrameLen;  // 帧的长度
	long long			m_lAPts;		// 时间戳
}AudioFrameHeader;

//帧缓冲池的结构定义
typedef struct {
	unsigned char  			*bufferstart;				/*媒体数据buf 起始地址*/
	unsigned long               		buffersize;				/*buf 空间大小*/
	unsigned long	      			writepos;				/*写指针偏移*/
	unsigned long				readpos;				/*读指针的偏移*/

	FrameInfo				FrmList[MAX_FRM_NUM];	/*buf 中存储的帧列表信息*/
	unsigned short		 	CurFrmIndex;			/*帧列表中的下标*/			
	unsigned short		 	TotalFrm;				/*buffer 中的总帧数*/
	
	unsigned short 			IFrmList[MAX_I_F_NUM];	/*最近n 个i 帧在FrmList中的数组下标信息*/
	unsigned short			IFrmIndex;				/*当前I 帧列表索引*/
	unsigned short			TotalIFrm;				/*总的I 帧数目*/
	unsigned short			ICurIndex;				//当前I帧序列
	
	unsigned long				circlenum;				/*buf覆盖的圈数*/

	unsigned long				m_u32MaxWpos;			/*最大写指针位置*/

//	IFRAMELISTCONTENT		BlockContent[BUFF_BLOCK_NUM];			/*块内数据内容*/
	
}FrameBufferPool, *pFrameBufferPool;

//一个FrameBufferPool 用户结构体定义
typedef struct{
	unsigned short		ReadFrmIndex;			/*此用户对帧缓冲池访问所用的帧索引*/
	unsigned short		reserve;
	unsigned long		ReadCircleNum;			/*此用户对帧缓冲池的访问圈数，初始时等于
												帧缓冲池中的circlenum*/
	unsigned int		diffpos;				/*读指针和写指针位置差值，单位为帧*/
	unsigned int 		throwframcount;			/*从开始计数丢帧的个数*/
}FrameBufferUser, *pFrameBufferUser;

//日期时间回调函数定义
typedef int (*DateTimeCallBack)(SystemDateTime *pSysTime);

class BufferManage{
public:
	//媒体缓冲池
	FrameBufferPool	m_FrameBufferPool;

	//对媒体缓冲池的访问用户信息
	FrameBufferUser	m_FrameBufferUser[MAX_FRAME_USER];

	//日期时间回调函数
	DateTimeCallBack	GetSysTime;

	//编码分辨率
	short 				m_resolution;

	//是否有音频
	char					m_have_audio;

	//通道号
	char					m_channel;

	unsigned int 			m_u32PCount;

	unsigned int 			m_u32IFrameOffset;
	
	//buffer 互斥锁
	pthread_mutex_t BufManageMutex;
	
	inline void InitBufferLock()
	{
		pthread_mutex_init(&BufManageMutex, NULL);
	}
	
	inline void AddBufferLock()
	{
		pthread_mutex_lock(&BufManageMutex);
	}
	
	inline void ReleaseBufferLock()
	{
		pthread_mutex_unlock(&BufManageMutex);
	}
	
public:
	BufferManage();
	virtual ~BufferManage(){};

//	int					m_nTestFd[8];
 
	
	BufferManage(DateTimeCallBack GetDatetime)
	{
		InitBufferLock();
		GetSysTime = GetDatetime;
	}
	

	//注册回调函数
	inline void RegisterCallBackFunc(DateTimeCallBack GetDatetime)
	{
		GetSysTime = GetDatetime;
	}

	/*****************************************************************************
	函数功能:构造函数
	输入参数:
	输出参数:无
	返  回   值:无
	使用说明:
	******************************************************************************/
	BufferManage(DateTimeCallBack GetDatetime, int ch);

	/*****************************************************************************
	函数功能:注册一个缓存池的使用用户
	输入参数:@userid: 用户id(目前只支持0)
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	virtual int RegisterUser(int userid);

	/*****************************************************************************
	函数功能:初始化缓存池使用用户信息
	输入参数:@userid: 用户id(目前只支持0)
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	virtual int InitUserInfo(int userid);

	/*****************************************************************************
	函数功能:复位指定消费者用户的读指针信息
	输入参数:@userid: 用户id(目前只支持0)
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	virtual int ResetUserInfo(int userid);

	/*****************************************************************************
	函数功能:初始化缓存池中的成员
	输入参数:无
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	int InitBuffer();

	/*****************************************************************************
	函数功能:创建缓存池
	输入参数:@resolution: 分辨率
				   @audioflg: 是否有音频
				   @bufsize: 分配buffer 大小，为0时根据分辨率指定
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:返回失败，一定不要使用此缓存池，上层一定要处理
	******************************************************************************/
	int CreateBufferPool(int resolution, int audioflg, unsigned long bufsize);

	/*****************************************************************************
	函数功能:销毁缓存池
	输入参数:无
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	int DestroyBufferPool();


	/*****************************************************************************
	函数功能:获取一个音频帧的信息
	输入参数:@Astream: 音频数据结构指针
	输出参数:@framelen:帧长度
				   @pts:时间戳
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	virtual int GetAudioFrameInfo(void *Astream, int *framelen, unsigned long long *pts);

	/*****************************************************************************
	函数功能:获取一个视频帧的信息
	输入参数:@Vstream: 视频数据结构指针
	输出参数:@frametype:帧类型(I or P)
				   @framelen:帧长度
				   @pts:时间戳
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	virtual int GetVideoFrameInfo(void *Vstream, int *frametype, int *framelen, unsigned long long *pts);

	/*****************************************************************************
	函数功能:存放一个视频帧到缓存池中
	输入参数:@Vstream: 视频数据结构指针
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	virtual int PutOneVFrameToBuffer(void *Vstream);

	/*****************************************************************************
	函数功能:存放一个音频帧到缓存池中
	输入参数:@Asteam: 音频数据结构指针
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	virtual int PutOneAFrameToBuffer(void *Astream,bool talk);

	/*****************************************************************************
	函数功能:存放一帧二进制流数据到缓存池中
	输入参数:@Asteam: 音频数据结构指针
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	virtual int PutOneBinaryDataToBuffer(char *BinaryStream, int len, unsigned long long pts);

	/*****************************************************************************
	函数功能:构造函数
	输入参数:@userid:用户id(0~2, 目前只支持0)
	输出参数:@buffer: 返回帧的起始地址
				   @pFrameInfo:返回此帧的信息
	返  回   值:成功返回0，否则返回-1
	使用说明:返回失败，请勿使用buffer 指针进行数据的获取
	******************************************************************************/
	virtual int GetOneFrameFromBuffer(int userid, unsigned char **buffer, FrameInfo *pFrameInfo);

	/*****************************************************************************
	函数功能:根据提前的时间修正开始获取数据的索引位置
	输入参数:@userid:用户id(0~2, 目前只支持0)
				   @secnum: 提前的秒数
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:内部调用
	******************************************************************************/
	virtual int CalculateReadIndexByTime(int userid, int secnum);

	/*****************************************************************************
	函数功能:开始获取音视频码流，并指定开始获取数据的索引位置
	输入参数:@userid:用户id (0~2, 目前只支持0)
				  @flag: 指定开始获取的方式0--当前i 帧位置,1--上次结束位置
				  之前最近的一个i 帧位置，2--当前时间之前多少秒的位置
				  @para:flag ==2时为提前的时间(单位为秒)
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:在刚启动录像时需要调用一次
	******************************************************************************/
	virtual int StartGetFrame(int userid, int flag, void * para);

	int GetAvDataBlock(int userid, char **buffer, int *len);

	
};
#endif
