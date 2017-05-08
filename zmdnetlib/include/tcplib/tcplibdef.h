
#ifndef _JIANGHM_TCPLIB_TCPLIB_DEFINE_HEADER_32439287324432
#define _JIANGHM_TCPLIB_TCPLIB_DEFINE_HEADER_32439287324432


#ifdef _WIN32

#pragma warning( disable : 4786 )

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define mysocketclose	closesocket
#define MYEWOULDBLOCK	WSAEWOULDBLOCK
#define MYEINTR			WSAEINTR
#define socklen_t		int
#define RELEASE_CPU( var )		Sleep( (var) )


#else

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>

#define mysocketclose		close
#define MYEWOULDBLOCK		EWOULDBLOCK
#define MYEINTR				EINTR
#define mysock_len			socklen_t
#define RELEASE_CPU(var)	usleep((var)*1000)



#ifdef HAY_DEBUG
	#define TCP_PRINTF(fmt, args...) fprintf(stderr, "\033[1;32m             NETLIB TCP DEBUG(%s:%d):             \033[0m" fmt, __func__, __LINE__, ## args)

#else
	#define TCP_PRINTF(fmt, args...) 
#endif

#define ERR_TCP(fmt, args...) fprintf(stderr, "\033[1;31m             NETLIB TCP RROR(%s:%d):             \033[0m" fmt, __func__, __LINE__, ## args)



#endif//_WIN32


#ifndef uint32
#define	uint8	unsigned char
#define uint16	unsigned short
#define uint32	unsigned int
#endif


//直接从内存中获取32位和16位数值
//主要为了解决无法变更的未4字节对齐的协议的结构中
//直接取数值会引起对齐错误的问题。
#define GET_UINT32( var , buf )	((uint8*)(&var))[0] = buf[0] ;\
								((uint8*)(&var))[1] = buf[1] ;\
								((uint8*)(&var))[2] = buf[2] ;\
								((uint8*)(&var))[3] = buf[3] ;


#define GET_UINT16( var , buf ) ((uint8*)(&var))[0] = buf[0] ; \
								((uint8*)(&var))[1] = buf[1] ;


#define SET_UINT32( var , buf ) buf[0] = ((uint8*)(&var))[0] ;\
								buf[1] = ((uint8*)(&var))[1] ;\
								buf[2] = ((uint8*)(&var))[2] ;\
								buf[3] = ((uint8*)(&var))[3] ;


#define SET_UINT16( var , buf ) buf[0] = ((uint8*)(&var))[0] ; \
								buf[1] = ((uint8*)(&var))[1] ;

#ifndef DECLARE_SINGLEOBJ
//单件定义宏
//-------------------------------------
//  在头文件中申明
//	DECLARE_SINGLEOBJ( CSampleClass ) ;
//	在CPP文件中定义静态变量
//	IMPLEMENT_SINGLEOBJ( CSampleClass ) ;
//	注意单件的getInstance为非线程安全，
//  最好是在主线程初始化的时候调用一次
//-------------------------------------
#define DECLARE_SINGLEOBJ(type)		\
public:	\
	static type* m_instance ;\
	static type* getInstance(){\
		if( NULL == m_instance ){\
			m_instance = new type() ;\
		}\
		return m_instance ;\
	};\
	static void release(){\
		if( m_instance){\
			delete m_instance ;\
			m_instance = NULL ;\
		}\
	};

#define IMPLEMENT_SINGLEOBJ(type) \
	type* type::m_instance = NULL ;

#endif


//------------------------------------------
// 内存分配调试宏
// 用来调试内存泄露
// debugMalloc和debugFree中内置一个
// 计数器，该计数器记载当前用户有多少个malloc未
// 进行free操作
// 建议上层操作也是用DebugMalloc,和DebugFree宏
// 来进行内存的分配与销毁
// 关闭DEBUG_MALLOC宏，则使用系统默认的malloc与free
//------------------------------------------
#define DEBUG_MALLOC
#ifdef  DEBUG_MALLOC
#define DebugMalloc debugMalloc
#define DebugFree	debugFree

//如果需要使用内存调试宏，则定义函数
void	*debugMalloc( int size ) ;
void	debugFree( void *pmem )  ;

#else

#define DebugMalloc	malloc
#define DebugFree	free

#endif	//DEBUG_MALLOC

//================================
//定义是否支持720P码流
#ifdef _WIN32
//#define SUPPORT_720P
#endif


//编译选项
//================================
//定义是为IPC还是NVR编译lib
//如果为NVR编译lib，打开此宏
//如果为IPC编译lib，关闭此宏

#define IPC_CLIENT_COUNT		1
#define MAX_STREAM_USER			18

//===============================
//定义视频通道种类
typedef enum
{
    VGA_CHN_TYPE = 0 ,
    QVGA_CHN_TYPE = 1 ,
    D720P_CHN_TYPE = 2 ,
    ALARM_CHN_TYPE = 3 ,
    RECORD_CHN_TYPE = 4 ,
    D1080P_CHN_TYPE = 5
} ZMD_CHN_TYPE ;

//定义缓冲码流类型
typedef enum
{
    MAIN_CHN_BUF = 1 ,
    SUB_CHN_BUF = 2,
    HIGH_CHN_BUF = 3
} ZMD_BUF_TYPE ;


#define		INVALID_SESSION			-1
#define		INVALID_USERID			-1

//支持的最大客户端最大数量
#define MAX_NET_TCP_USER			12
//支持的最大视频播放数量
#define MAX_MEDIA_PLAYER			6

//tcp会话用来接收数据的缓冲区大小
//由于720P或者更大尺寸视频的I帧比较大，
//所以该缓冲区最好定义相对足够些。
#if (defined APP3518) || (defined APP3511) 
#define TCP_RECV_BUF_SIZE			256*1024
#endif

#ifdef DM368
#define TCP_RECV_BUF_SIZE			1024*1024
#endif

#ifdef APP3531
#define TCP_RECV_BUF_SIZE			1024*1024
#endif

//用户名缓冲区长度
#define CLIENT_NAME_LEN				16
//密码缓冲区长度
#define CLIENT_PASS_LEN				16

//设置从ipc读取数据的最大空闲秒数
//当超过此宏定义的时间内未收到数据则认为
//连接异常，断开重连
#define CLIENT_IDLE_TIME			5

#endif

