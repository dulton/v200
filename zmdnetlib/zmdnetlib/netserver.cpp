#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>	//add by hayson 20131227
#include <linux/sockios.h>
#include <linux/rtc.h>
#include <linux/mii.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <net/route.h> //add by hayson 20131025
#include <net/if_arp.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <asm/types.h>

#include "zmdntpclient.h"
#include "netserver.h"
#include "interfacedef.h"
#include "tcplib.h"
#include "netuser.h"
#include "ddnsfunction.h"
#include "upnp.h"
#include "zmdmail.h"
#include "ModuleFuncInterface.h"
#include "cspara.h"
#include "common.h"
#include "Video_MD.h"
#include "zmdnetlib.h"
#include "md5.h"
#include "gateway.h"
#include "AdapterMessage.h"

//=================================================
//一些扩展定义,由外部实现，而网络层需要调用的功能
extern int change_flag1 ;
extern int change_flag2 ;
extern PARAMETER_MANAGE*		g_cParaManage ;

#define USER_LIST_COUNT		16


#ifndef offsetof
#define offsetof(type, f) ((size_t) \
		((char *)&((type *)0)->f - (char *)(type *)0))
#endif

#define ifreq_offsetof(x)  offsetof(struct ifreq, x)


//#define IFF_DYNAMIC     0x8000  /* dialup device with changing addresses */

#define N_CLR            0x01
#define M_CLR            0x02
#define N_SET            0x04
#define M_SET            0x08
#define N_ARG            0x10
#define M_ARG            0x20

#define M_MASK           (M_CLR | M_SET | M_ARG)
#define N_MASK           (N_CLR | N_SET | N_ARG)
#define SET_MASK         (N_SET | M_SET)
#define CLR_MASK         (N_CLR | M_CLR)
#define SET_CLR_MASK     (SET_MASK | CLR_MASK)
#define ARG_MASK         (M_ARG | N_ARG)

/*
 * Here are the bit masks for the "arg_flags" member of struct options below.
 */

/*
 * cast type:
 *   00 int
 *   01 char *
 *   02 HOST_COPY in_ether
 *   03 HOST_COPY INET_resolve
 */
#define A_CAST_TYPE      0x03
/*
 * map type:
 *   00 not a map type (mem_start, io_addr, irq)
 *   04 memstart (unsigned long)
 *   08 io_addr  (unsigned short)
 *   0C irq      (unsigned char)
 */
#define A_MAP_TYPE       0x0C
#define A_ARG_REQ        0x10	/* Set if an arg is required. */
#define A_NETMASK        0x20	/* Set if netmask (check for multiple sets). */
#define A_SET_AFTER      0x40	/* Set a flag at the end. */
#define A_COLON_CHK      0x80	/* Is this needed?  See below. */

/*
 * These defines are for dealing with the A_CAST_TYPE field.
 */
#define A_CAST_CHAR_PTR  0x01
#define A_CAST_RESOLVE   0x01
#define A_CAST_HOST_COPY 0x02
#define A_CAST_HOST_COPY_IN_ETHER    A_CAST_HOST_COPY
#define A_CAST_HOST_COPY_RESOLVE     (A_CAST_HOST_COPY | A_CAST_RESOLVE)

/*
 * These defines are for dealing with the A_MAP_TYPE field.
 */
#define A_MAP_ULONG      0x04	/* memstart */
#define A_MAP_USHORT     0x08	/* io_addr */
#define A_MAP_UCHAR      0x0C	/* irq */

/*
 * Define the bit masks signifying which operations to perform for each arg.
 */

#define ARG_METRIC       (A_ARG_REQ /*| A_CAST_INT*/)
#define ARG_MTU          (A_ARG_REQ /*| A_CAST_INT*/)
#define ARG_TXQUEUELEN   (A_ARG_REQ /*| A_CAST_INT*/)
#define ARG_MEM_START    (A_ARG_REQ | A_MAP_ULONG)
#define ARG_IO_ADDR      (A_ARG_REQ | A_MAP_ULONG)
#define ARG_IRQ          (A_ARG_REQ | A_MAP_UCHAR)
#define ARG_DSTADDR      (A_ARG_REQ | A_CAST_HOST_COPY_RESOLVE)
#define ARG_NETMASK      (A_ARG_REQ | A_CAST_HOST_COPY_RESOLVE | A_NETMASK)
#define ARG_BROADCAST    (A_ARG_REQ | A_CAST_HOST_COPY_RESOLVE | A_SET_AFTER)
#define ARG_HW           (A_ARG_REQ | A_CAST_HOST_COPY_IN_ETHER)
#define ARG_POINTOPOINT  (A_CAST_HOST_COPY_RESOLVE | A_SET_AFTER)
#define ARG_KEEPALIVE    (A_ARG_REQ | A_CAST_CHAR_PTR)
#define ARG_OUTFILL      (A_ARG_REQ | A_CAST_CHAR_PTR)
#define ARG_HOSTNAME     (A_CAST_HOST_COPY_RESOLVE | A_SET_AFTER | A_COLON_CHK)

struct arg1opt 
{
	const char *name;
	unsigned short selector;
	unsigned short ifr_offset;
};

struct options 
{
	const char *name;
	const unsigned char  flags;
	const unsigned char  arg_flags;
	const unsigned short selector;
};

static const struct arg1opt Arg1Opt[] = {
	{"SIOCSIFMETRIC",  SIOCSIFMETRIC,  ifreq_offsetof(ifr_metric)},
	{"SIOCSIFMTU",     SIOCSIFMTU,     ifreq_offsetof(ifr_mtu)},
	{"SIOCSIFTXQLEN",  SIOCSIFTXQLEN,  ifreq_offsetof(ifr_qlen)},
	{"SIOCSIFDSTADDR", SIOCSIFDSTADDR, ifreq_offsetof(ifr_dstaddr)},
	{"SIOCSIFNETMASK", SIOCSIFNETMASK, ifreq_offsetof(ifr_netmask)},
	{"SIOCSIFBRDADDR", SIOCSIFBRDADDR, ifreq_offsetof(ifr_broadaddr)},
	{"SIOCSIFHWADDR",  SIOCSIFHWADDR,  ifreq_offsetof(ifr_hwaddr)},
	{"SIOCSIFDSTADDR", SIOCSIFDSTADDR, ifreq_offsetof(ifr_dstaddr)},
	{"SIOCSIFMAP",     SIOCSIFMAP,     ifreq_offsetof(ifr_map.mem_start)},
	{"SIOCSIFMAP",     SIOCSIFMAP,     ifreq_offsetof(ifr_map.base_addr)},
	{"SIOCSIFMAP",     SIOCSIFMAP,     ifreq_offsetof(ifr_map.irq)},
	/* Last entry if for unmatched (possibly hostname) arg. */
	{"SIOCSIFADDR",    SIOCSIFADDR,    ifreq_offsetof(ifr_addr)},
};


static const struct options OptArray[] = 
{
	{"metric",       N_ARG,         ARG_METRIC,      0},
    {"mtu",          N_ARG,         ARG_MTU,         0},
	{"txqueuelen",   N_ARG,         ARG_TXQUEUELEN,  0},
	{"dstaddr",      N_ARG,         ARG_DSTADDR,     0},
	{"netmask",      N_ARG,         ARG_NETMASK,     0},
	{"broadcast",    N_ARG | M_CLR, ARG_BROADCAST,   IFF_BROADCAST},
	{"hw",           N_ARG,         ARG_HW,          0},
	{"pointopoint",  N_ARG | M_CLR, ARG_POINTOPOINT, IFF_POINTOPOINT},
	{"mem_start",    N_ARG,         ARG_MEM_START,   0},
	{"io_addr",      N_ARG,         ARG_IO_ADDR,     0},
	{"irq",          N_ARG,         ARG_IRQ,         0},
	{"arp",          N_CLR | M_SET, 0,               IFF_NOARP},
	{"trailers",     N_CLR | M_SET, 0,               IFF_NOTRAILERS},
	{"promisc",      N_SET | M_CLR, 0,               IFF_PROMISC},
	{"multicast",    N_SET | M_CLR, 0,               IFF_MULTICAST},
	{"allmulti",     N_SET | M_CLR, 0,               IFF_ALLMULTI},
	{"dynamic",      N_SET | M_CLR, 0,               IFF_DYNAMIC},
	{"up",           N_SET        , 0,               (IFF_UP | IFF_RUNNING)},
	{"down",         N_CLR        , 0,               IFF_UP},
	{ NULL,          0,             ARG_HOSTNAME,    (IFF_UP | IFF_RUNNING)}
};

struct mii_data {
    unsigned short   phy_id;
    unsigned short   reg_num;
    unsigned short   val_in; 
    unsigned short   val_out;
};

enum  // interface status
{
	IFSTATUS_ERR = -1,
	IFSTATUS_DOWN = 0,
	IFSTATUS_UP = 1,
};


static void set_ifreq_to_ifname(struct ifreq *ifreq)
{
	memset(ifreq, 0, sizeof(struct ifreq));
	//strncpy_IFNAMSIZ(ifreq->ifr_name, G.iface);
	strncpy(ifreq->ifr_name, LOCAL_NET_NAME, IFNAMSIZ);
}

static int network_ioctl(int request, void* data, const char *errmsg)
{
	int skfd;
	
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("socket SOCK_DGRAM error");
	}
	int r = ioctl(skfd, request, data);
	if (r < 0 && errmsg)
		ERR("%s failed\n", errmsg);
	
	close(skfd);
	return r;
	
}

static int in_ether(char *bufp, struct sockaddr *sap)
{
	unsigned char *ptr;
	int i, j;
	unsigned char val;
	unsigned char c;

	sap->sa_family = ARPHRD_ETHER;
	ptr = (unsigned char *)sap->sa_data;

	i = 0;
	do {
		j = val = 0;

		/* We might get a semicolon here - not required. */
		if (i && (*bufp == ':')) {
			bufp++;
		}

		do {
			c = *bufp;
			if (((unsigned char)(c - '0')) <= 9) {
				c -= '0';
			} else if (((unsigned char)((c|0x20) - 'a')) <= 5) {
				c = (c|0x20) - ('a'-10);
			} else if (j && (c == ':' || c == 0)) {
				break;
			} else {
				return -1;
			}
			++bufp;
			val <<= 4;
			val += c;
		} while (++j < 2);
		*ptr++ = val;
	} while (++i < ETH_ALEN);

	return (int) (*bufp);	/* Error if we don't end at end of string. */
}



int  CNetServer::m_staticSipRegState = -1 ;
bool CNetServer::m_staticGetCallback = false ;
bool CNetServer::m_staticRegistNow = 0 ;

IMPLEMENT_SINGLEOBJ( CNetServer )

CNetServer::CNetServer()
{
	m_serverHandle = NULL ;
	m_nLoginTimes = 0 ;
	m_nConnectTimes = 0 ;
	m_pingSock = -1 ;
	m_nUserCount = 0 ;
	m_doDdns = 1 ;
	m_doUPNP = 0 ;
	m_nUpnpSwitch = 0 ;
	m_isUpdating = false ;
	m_talkOn = false ;
	m_netcnt =	0;
	m_netCard = -1;
	m_upnpSuccess = false;
	m_QuenceMsgID = -1;
}

CNetServer::~CNetServer()
{
	
}

int motion_detect_check_time( ALARMINDATESET m_TimeTblSet[8] )
{
	int i = 0;
	time_t ct;
	struct tm *timeinfo;
	unsigned int week = 0;
	unsigned int start = 0, end = 0, current = 0;

	time( &ct );
	timeinfo = gmtime( &ct );
	if( NULL == timeinfo )
		return -1;
	
	week = timeinfo->tm_wday;
	current = timeinfo->tm_hour*60 + timeinfo->tm_min;
	for( i = 0; i < 4; i++ )
	{
		if( m_TimeTblSet[week].m_TBLSection[i].m_u8Valid )
		{
			start = m_TimeTblSet[week].m_TBLSection[i].m_u16StartTime;		
			end = m_TimeTblSet[week].m_TBLSection[i].m_u16EndTime;

			if( (current >= start) && ( current <= end))
				return 0;
		}
	}
	
	return -1;
}

//============================================
//服务器辅助线程，主要处理一些不太紧急而又需要
//持续处理的事件，例如,回应广播ping,upnp,等
//后期一些定时的处理也可以加入此线程
void *CNetServer::checkNetCardThreadProc( void *pObj )				
{														
    NDB("threadid %d\n", (unsigned)pthread_self());															
    while( true )
    {
        GetNetServerObj()->CheckNetCardStatus( ) ;
        RELEASE_CPU( 2000 ) ;										
    }

    return 0 ;
}

void *CNetServer::ddnsThreadProc( void *pObj )
{
   NDB("threadid %d\n", (unsigned)pthread_self());	

    while( true ) 													
    {
        GetNetServerObj()->handleDDNS() ;
        RELEASE_CPU( 1000 ) ;										
    }

    return 0 ;
}

void *CNetServer::upnpThreadProc( void *pObj )
{
    NDB("threadid %d\n", (unsigned)pthread_self());	

    while( true ) 														
    {
        GetNetServerObj()->handleUPNP( ) ;
        RELEASE_CPU( 50000 ) ;										
    }

    return 0 ;
}

//===============================================
//报警处理线程
void *CNetServer::alarmThreadProc( void *pObj )
{													
   NDB("threadid %d\n", (unsigned)pthread_self());	
															
    GetNetServerObj()->handleAlarm( ) ;
    return 0 ;
}

void *CNetServer::pingThreadProc( void *pObj )
{														
    NDB("threadid %d\n", (unsigned)pthread_self());	
																
    while( true )
    {
        GetNetServerObj()->handlePing( ) ;
        RELEASE_CPU( 100 ) ;
    }

    return 0 ;
}

void *CNetServer::emailThreadProc( void *pObj )
{												
    NDB("threadid %d\n", (unsigned)pthread_self());	
												
    GetNetServerObj()->handleEmail( ) ;

    return 0 ;
}

//==============================================
//设备注册线程
void *CNetServer::registThreadProc( void *pObj )
{												
    NDB("threadid %d \r\n", (unsigned)pthread_self());
														
    GetNetServerObj()->handleRegist( ) ;
    return 0 ;
}

///----------------------------------------------
//启动服务器
bool CNetServer::StartServer( char *addr , unsigned short port )
{
    //初始化工作会话
    int i = 0 ;
    for( i = 0 ; i < MAX_NET_TCP_USER ; i ++ )
    {
        m_loginUserList[i].userobj = new CNetUser() ;
        m_loginUserList[i].userid = i ;
        m_loginUserList[i].used = 0 ;
        m_loginUserList[i].mediaid = INVALID_USERID ;
        m_loginUserList[i].alarmid = INVALID_USERID ;
    }

    //初始化medialist
    for( i = 0 ; i < MAX_MEDIA_PLAYER ; i ++ )
    {
        m_alarmUserList[i].used = 0 ;
        m_alarmUserList[i].ip = 0 ;
        m_alarmUserList[i].port = 0 ;
        m_alarmUserList[i].userid = 0 ;
        m_alarmUserList[i].devType = 0 ;
    }

    //记录启动时间
    m_serverStartTime = time( NULL ) ;								
    //创建一个tcp服务器
    m_serverHandle = CreateTcpServer( onServerTcpAccept , onServerTcpClose , onServerRecvData , onServerSessionIdle) ;
    if( !m_serverHandle )
    {
        ERR( "create netserver failed !!!\r\n" ) ;
        return false ;
    }

    //启动该tcp服务器
    if( !StartTcpServer( m_serverHandle , addr , port , true ))
    {
        ERR( "start net server failed !!!\r\n" ) ;
    }

	NET_WORK_CARD CardID;
	CardID = get_network_support();
	
	if(CardID == NET_WORK_CARD_LOCAL_AND_WIFI)
	{
	    //网卡检查线程
	    m_checkNetCardThread = CreateZmdThread( 0 , checkNetCardThreadProc ) ;					
	    if( !m_checkNetCardThread )
	    {
	        ERR( "create checkNetCardThreadProc thread failed !!!\r\n" ) ;
	    }
	}
    //ddns线程

    m_ddnsThread = CreateZmdThread( 0 , ddnsThreadProc ) ;								
    if( !m_ddnsThread )
    {
        ERR( "create ddnsThreadProc thread failed !!!\r\n" ) ;
    }

    //upnp线程
    m_upnpThread = CreateZmdThread( 0 , upnpThreadProc ) ;
    if( !m_upnpThread )
    {
        ERR( "create upnpThreadProc thread failed !!!\r\n" ) ;
    }															

    //报警线程
    #if 0
    m_alarmThread = CreateZmdThread( 0 , alarmThreadProc ) ;
    if( !m_alarmThread )
    {
        ERR( "create alarm thread failed !!!\r\n" ) ;
    }
	#endif
	
	//广播请求处理线程
    m_pingThread = CreateZmdThread( 0 , pingThreadProc ) ;
    if( !m_pingThread )
    {
        ERR( "create ping thread failed !!!\r\n" ) ;
    }

	//Email 线程
    m_emailThread = CreateZmdThread( 0 , emailThreadProc ) ;
    if( !m_emailThread )
    {
        ERR( "create email thread failed !!!\r\n" ) ;
    }

	//二维码注册线程
    m_registThread = CreateZmdThread( 0 , registThreadProc ) ;
    if( !m_registThread )
    {
        ERR( "create regist thread fialed !!\r\n" ) ;
    }	
	if(m_QuenceMsgID<0)
	{
		
		int ret =OSPQueueCreate((char *)MESSAGENAME,&m_QuenceMsgID);
		if(ret<0||m_QuenceMsgID<0)
		{
			printf("\n\n\n\n\n\n\n %s========================= OSPQueueCreate faild m_QuenceMsgID:%d,ret:%d\n",__FUNCTION__,m_QuenceMsgID,ret);
			//return false;
		}	
		printf("\n\n\n\n\n\n\n %s========================= OSPQueueCreate Success m_QuenceMsgID:%d\n",__FUNCTION__,m_QuenceMsgID);

	}

	return true ;
}

//=================================================
//获取一个空闲的用户结构.
int	CNetServer::getUnuseWorkSession( )
{
    //进入工作会话锁
    CAutoMutex atlck( &m_loginUserListMutex ) ;

    int i = 0 ;
    for( i = 0 ; i < MAX_NET_TCP_USER ; i ++ )
    {
        if( !m_loginUserList[i].used )
        {
            m_loginUserList[i].used = 1 ;
            return i ;
        }
    }

    return INVALID_USERID ;
}

//=============================================
//释放用户结构
void CNetServer::freeWorkSession( int userid )
{
    if( userid < 0 || userid >= MAX_NET_TCP_USER )
    {
        ERR( "freeWorkSession() userid error = %d\r\n" , userid ) ;
        return ;
    }

    CAutoMutex atlck( &m_loginUserListMutex ) ;
    m_loginUserList[userid].used = 0 ;
}

//============================================
//获取一个空闲的报警处理节点
int	CNetServer::getUnuseAlarmSession( int userid , unsigned int ip , unsigned int port , int devType )
{
    CAutoMutex atlck( &m_alarmUserListMutex ) ;

    int i = 0 ;
    //先检查是否已存在相同的节点								 

    for( i = 0 ; i < MAX_MEDIA_PLAYER ; i ++ )
    {
        if( m_alarmUserList[i].used )
        {
            if( m_alarmUserList[i].ip  == ip && m_alarmUserList[i].port == port )
                return INVALID_SESSION ;
        }
    }													
    for( i = 0 ; i < MAX_MEDIA_PLAYER ; i ++ )
    {
        if( !m_alarmUserList[i].used )
        {
            m_alarmUserList[i].used = 1 ;
            m_alarmUserList[i].ip = ip ;
            m_alarmUserList[i].port = port ;
            m_alarmUserList[i].userid = userid ;
            m_alarmUserList[i].devType = devType ;
            return i ;
        }
    }
    return INVALID_SESSION ;
}

//=============================================
//释放报警节点
void CNetServer::freeAlarmSession( int alarmid )
{
    if( alarmid < 0 || alarmid >= MAX_MEDIA_PLAYER )
    {
        ERR( "freeAlarmSession() alarmid error = %d\r\n" , alarmid ) ;
        return  ;
    }

    CAutoMutex atlck( &m_alarmUserListMutex ) ;
    m_alarmUserList[alarmid].used = 0 ;
}

//=================================================
//网络层通知连接到达
bool CNetServer::onServerTcpAccept( int &userData , void *session , sockaddr_in *client_addr )
{
	return getInstance()->onAccept( userData , session , client_addr ) ;
}

//===============================================
//网络层通知，连接被关闭
void CNetServer::onServerTcpClose( int userData )
{
    getInstance()->onClose( userData ) ;
}

//==============================================
//onSend
bool CNetServer::onServerSessionIdle( int userData )
{
    return getInstance()->onIdle( userData ) ;
}

//--------------------------------------------
//对接收到的数据进行分包
bool CNetServer::onServerRecvData( int userData , char *data , int len , int &used )
{
    return getInstance()->onReceive( userData , data , len , used ) ;
}

void CNetServer::onClose( int userData )
{
    //通知有链接关闭
    int userid = userData ;

    if( userid < 0 || userid >= MAX_NET_TCP_USER )
        return ;

    m_loginUserList[userid].userobj->onClose() ;

    m_nUserCount -- ;
	//printf( "onClose() user %d logout m_nUserCount = %d \r\n" , userid , m_nUserCount ) ;
}

bool CNetServer::onReceive( int userData , char *data , int len , int &used )
{
    int userid = userData ;

    if( userid < 0 || userid >= MAX_NET_TCP_USER )
        return false ;

    return m_loginUserList[userid].userobj->onReceive( data , len , used ) ;
}

bool CNetServer::onIdle( int userData )
{
    //通知空闲
    int userid = userData ;

    if( userid < 0 || userid >= MAX_NET_TCP_USER )
        return true ;

    return m_loginUserList[userid].userobj->onIdle() ;
}

bool CNetServer::onAccept( int &userData , void *session , sockaddr_in *client_addr )
{
    m_nConnectTimes ++ ;

    //通知有链接到达
    //获取工作会话(工作会话有最大限制.)
    int userid = getUnuseWorkSession() ;
    if( INVALID_USERID == userid )
    {
        return false ;
    }
    //获取到可用工作会话后，对会话进行初始化
    //由于不会被外部线程关闭，所以这里可以访问
    //m_loginUserList[userid].userobj.而不需要加锁

    userData = userid ;
    m_loginUserList[userid].userobj->ResetUser() ;
    m_loginUserList[userid].userobj->m_sessionHandle = session ;
    m_loginUserList[userid].userobj->m_nUserID = userid ;
    m_loginUserList[userid].userobj->m_remoteIP = client_addr->sin_addr.s_addr ;

    m_nLoginTimes ++ ;
    m_nUserCount ++ ;

	//printf( "onAccept() user %d login m_nUserCount = %d \r\n" , userid , m_nUserCount ) ;

    return true ;
}

//=============================================
//处理ping消息
void CNetServer::handlePing( )
{
    fd_set readfds;
    struct sockaddr_in bindaddr, remoteAddr ;
    //int sockfd = -1 ;
    char recvbuf[8 * 1024] ;
    socklen_t addrsize = sizeof( remoteAddr ) ;
    int nRecved = 0 ;

    //如果句柄非法，创建socket
    if( m_pingSock < 0 )
    {
        m_pingSock = socket(AF_INET, SOCK_DGRAM, 0);
        if(m_pingSock < 0)
        {
            ERR( "handlePing() socket() failed !!!!\r\n" ) ;
            RELEASE_CPU( 5000 ) ;
            return ;
        }

        bindaddr.sin_family = AF_INET;
        bindaddr.sin_port = htons(8080);
        bindaddr.sin_addr.s_addr = inet_addr("0.0.0.0");

        if( bind( m_pingSock , (const sockaddr *)&bindaddr , sizeof( bindaddr ) ) )
        {
            ERR( "handlePing() bind() failed !!!!\r\n" ) ;
            mysocketclose( m_pingSock ) ;
            m_pingSock = -1 ;
            RELEASE_CPU( 5000 ) ;
            return ;
        }

        int bOptVal = 1;
        int n = setsockopt(m_pingSock, SOL_SOCKET, SO_BROADCAST, (const char *)&bOptVal, sizeof(int));
        if( n < 0 ) 									
        {
            ERR( "setsockopt() SO_BROADCAST failed !!!\r\n" ) ;
            mysocketclose( m_pingSock ) ;
            m_pingSock = -1 ;						
            return ;
        }										
    }

    FD_ZERO(&readfds);
    FD_SET( m_pingSock , &readfds);
    int maxfd = m_pingSock ;

    timeval to;
    to.tv_sec = 2 ;
    to.tv_usec = 0 ;

    int n = select(maxfd + 1, &readfds, NULL , NULL, &to);
    if( n > 0 )
    {
        nRecved = recvfrom( m_pingSock , recvbuf , 8 * 1024 , 0 , (sockaddr *)&remoteAddr , &addrsize ) ;
        if( nRecved > 0 )
        {
            Cmd_Header *pHeader = (Cmd_Header *)recvbuf ;
			DelcurrentGW();
			
            switch( pHeader->commd )
            {
            case CMD_PING :
                onPingCmd( m_pingSock , recvbuf , nRecved , remoteAddr ) ;
                break ;
            case CMD_ID_PING :
                onIDPingCmd( m_pingSock , recvbuf , nRecved , remoteAddr ) ;
                break ;
            }
        }
    }
}

//==============================================
// alarm处理线程
void CNetServer::handleAlarm( )
{
	int i = 0;
	int MotionRet;

    MD_HANDLE *handle = MD_HANDLE::Instance();

    CAMERA_MD	motion;

    alarmType stAlarm;
    char buf[64];
    int iLen = 0;
    while( true )
    {
        g_cParaManage->GetSysParameter(SYSMOTION_SET, &motion);

        //是否要上传报警信息
        if( !(motion.m_Channel[0].m_uAalarmOutMode & 0x40))
        {
            RELEASE_CPU(100);
            continue;
        }

        MotionRet = handle->GetVideoMdStatus(0);

        if (MotionRet)
        {
			//打开自动锁，锁住报警用户列表
			//CAutoMutex atlck( &m_alarmUserListMutex ) ;
			if( motion_detect_check_time(motion.m_Channel[i].m_TimeTblSet) )
			{
				RELEASE_CPU(100);
				continue;
			}
			
			for( i = 0 ; i < MAX_MEDIA_PLAYER ; i ++ ){
				if( 1 == m_alarmUserList[i].used ){

					sockaddr_in addr ;

					addr.sin_family = AF_INET ;
					addr.sin_addr.s_addr = m_alarmUserList[i].ip ;
					addr.sin_port = m_alarmUserList[i].port ;

					int sock = socket( AF_INET , SOCK_STREAM , 0 ) ;
					if( sock < 0 )
					{
						continue ;
					}

					if( connect( sock , (sockaddr*)&addr , sizeof( addr ) ) < 0 )
					{
						printf( "alarm connect failed!! clear the alarm point!!\r\n" ) ;
						//m_alarmUserList[i].used = 0 ;
						mysocketclose( sock ) ;
						continue ;
					}

					STRUCT_ALARM_UPDATE_REQUEST request ;
					request.header.head   = 0xaaaa5555;
					request.header.commd  = CMD_ALARM_UPDATE;
					request.header.length = sizeof(request) - sizeof( Cmd_Header );
					request.header.channel = 0;
					request.header.type    = 0;

					request.alarmChl = 0 ;
					request.alarmType = 0 ;
					
					//只做一次send.失败了不管了
					int nSend = send( sock , (char*)&request , sizeof( request ) , 0 ) ;
					if( nSend != sizeof( request ) )
					{
						//发送失败,关闭这个通道？？？
						//m_alarmUserList[i].used = 0 ;
					}
					
					//释放30毫秒cpu
					RELEASE_CPU( 30 ) ;
					mysocketclose( sock ) ;
				}
			}

			//发生报警后，休眠1秒，等待报警位重置
			RELEASE_CPU( 1000 ) ;
		}
		else
		{
			RELEASE_CPU( 100 ) ;
		}
	}	
}

void CNetServer::handleUPNP( )
{
	NETWORK_PARA netset;
	char c_ip[CHAE_IP_LEN];
	char mac[CHAE_IP_LEN];
	char mask[CHAE_IP_LEN];
		
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset); 

    //未打开upnp开关，则不执行
    if(!netset.m_Eth0Config.m_upnp)
    {
        m_nUpnpSwitch = 0 ;
        RELEASE_CPU( 2000 ) ;
        return ;
    }

	int nNetCardID = JudgeNetworkStatus( );
	
	if(nNetCardID)
		GetLocalNetInfo(LOCAL_NET_NAME, c_ip, mask,mac);
	else
	{
		
		if(get_network_support()!=NET_WORK_CARD_LOCAL)
			GetLocalNetInfo(WIFI_NET_NAME, c_ip, mask,mac);
	
	}
		
	//正在升级
	if( m_isUpdating )
	{
		return ;
	}
	
	int now = time( NULL ) ;
	
	if(m_nNetCardID != nNetCardID ||
		m_localHttpPort != netset.m_CenterNet.m_uHttpListenPt ||
		m_localVideoPort != netset.m_CenterNet.m_uVideoListenPt ||
		m_localMobilePort != netset.m_CenterNet.m_uPhoneListenPt ||
		m_upnpHttpPort != netset.m_Eth0Config.m_http_port ||
		m_upnpVideoPort != netset.m_Eth0Config.m_v_port ||
		m_upnpMobilePort != netset.m_Eth0Config.m_phone_port || 
		(strcmp(m_ip , c_ip) != 0) ||
		IsChangeExtIP() ||
		now - m_upnpTime > 15*60 )
		{
			m_doUPNP =1 ;
		}
	
	if( m_doUPNP )
	{	
		m_isUPNPComplete = 0 ;
		
		if(netset.m_Eth0Config.m_upnp)
		{
			//printf( "regist UPNP!!\r\n" ) ; 
			int ret = upnp(netset.m_Eth0Config.m_v_port,
				netset.m_Eth0Config.m_http_port,
				netset.m_Eth0Config.m_phone_port,
				netset.m_CenterNet.m_uHttpListenPt,
				netset.m_CenterNet.m_uVideoListenPt,
				netset.m_CenterNet.m_uPhoneListenPt,
				netset.m_DomainConfig.m_u8Selected, 
				m_upnpHttpPort,
				m_upnpVideoPort,
				m_upnpMobilePort);

			//printf("################# 注册UPNP  ret:%d#################### \r\n",  ret);
			//如果注册成功，则记录状态
			if( !ret )
			{
				m_upnpSuccess = true;
				m_doDdns = 0 ;
				m_localHttpPort = netset.m_CenterNet.m_uHttpListenPt ;
				m_localVideoPort = netset.m_CenterNet.m_uVideoListenPt ;
				m_localMobilePort = netset.m_CenterNet.m_uPhoneListenPt ;
				m_upnpHttpPort = netset.m_Eth0Config.m_http_port ;
				m_upnpVideoPort = netset.m_Eth0Config.m_v_port ;
				m_upnpMobilePort = netset.m_Eth0Config.m_phone_port ;
				strcpy(m_ip , c_ip)  ;
				m_nNetCardID = nNetCardID ;
				m_nUpnpSwitch = netset.m_Eth0Config.m_upnp ;
				m_upnpTime = time( NULL ) ;
				//重新注册二维码
				m_staticRegistNow= 1 ;
				//重新注册ddns
				if( netset.m_DomainConfig.m_u8Selected )
					m_doDdns = 1;
				m_doUPNP = 0 ;
			}
			else
				m_upnpSuccess = false;
		}
	}
}

void CNetServer::handleDDNS( )
{

    NETWORK_PARA netset;
    int now = time( NULL ) ;
    if( now - m_ddnsTime > 60 )
    {
        //需要重新注册ddsn的
        m_doDdns = 1 ;
    }
    //是否需要立刻刷新Ddns
    if( m_doDdns )
    {
        g_cParaManage->GetSysParameter(SYSNET_SET, &netset);
        if(netset.m_DomainConfig.m_u8Selected)
        {
            ddns(netset.m_DomainConfig.m_server, netset.m_DomainConfig.m_s8Name, netset.m_DomainConfig.m_s8UserName, netset.m_DomainConfig.m_s32Passwd);
        }
        m_doDdns = 0 ;
        m_ddnsTime = now ;
    }

}

static int check_email_send_condition( int channel )
{
	CAMERA_MD motion;
	static time_t base = time(NULL);
       struct tm *base_time = NULL;
	struct tm *now_time = NULL;
	
	
	time_t now = time(NULL);
	
       base_time = gmtime(&base);
	int base_min = base_time->tm_hour*60 + base_time->tm_min;
//	printf("zzzzzzzzzzzzzz     base_GMT is: %s\r\n", asctime(base_time));

       now_time = gmtime(&now);
	int now_min = now_time->tm_hour*60 + now_time->tm_min;
//	printf("zzzzzzzzzzzzzzz    now_GMT is: %s\r\n", asctime(now_time));

	g_cParaManage->GetSysParameter( SYSMOTION_SET, &motion );
	
	if( GetRealTimeMotion() )
	{
		if( motion.m_Channel[channel].m_uAlarmInterval == 0 )
		{
			return 1;
		}
//		printf("zzzzzzzzzzzz     now = %d\n",now);
//		
//		printf("zzzzzzzzzzzz     base = %d\n",base);

//		printf("zzzzzzzzzzzz     (now -base)  = %d\n",now - base);
//		printf("errno = %d\n",errno);


		if((now -base) < (motion.m_Channel[channel].m_uAlarmInterval * 60))
		{
//			printf(" (base_min + motion.m_Channel[channel].m_uAlarmInterval - 1 ) >= now_min");
			return 0;

		}
#if 0		
		if( (base_min + motion.m_Channel[channel].m_uAlarmInterval - 1 ) >= now_min )
		{
			printf(" (base_min + motion.m_Channel[channel].m_uAlarmInterval - 1 ) >= now_min");
			return 0;
		}
#endif		
		else
		{
			printf("check_email_send_condition ok !!!!\n");
			base = now;
			return 1;
		}
	}

	return 0;
	
}

void CNetServer::handleEmail( )
{}
//==============================================
//二维码注册回调函数

#ifdef _BARCODE_SUP_
void CNetServer::regist_result_cb (reg_res_t *status)
{

	m_staticSipRegState= status->retcode  ;
	if( REGISTER_SUCCESS == m_staticSipRegState ){
		NDB("################# 注册二维码成功  regist barcode server success!####################\r\n" ) ;
	}
	else{
		//printf("################# 注册二维码失败 regist barcode server failed  ####################\r\n" ) ;
	}

	m_staticGetCallback = true ;
}
#endif
//二维码注册线程
void CNetServer::handleRegist( )
{
	#if 0
#ifdef _BARCODE_SUP_
	int ret ;	
	char srv[32] = "register.zmododns.com";
	char localip[16] ;

 	NETWORK_PARA netset;	
	regc_t regc;	
	time_t	preTime = 0 ;

	ret = zsip_init();

	memset( &regc , 0 , sizeof( regc ) ) ;
	

	if (ret < 0)
	{
		printf("zsip init fail .\n");
		return ;
	}
	
	while( !m_isUpdating )
	{
		g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
		
		strcpy(regc.srvname, srv);
		regc.srvport = 8888;

		//无设备编号，不进行注册
		if( strlen( (char*)netset.m_CenterNet.deviceid ) ==  0 || (strcmp((char*)netset.m_CenterNet.deviceid, "0000000000") == 0))
		{
			sleep(5) ;
			continue ;
		}


		sprintf( localip , "%d.%d.%d.%d" , netset.m_Eth0Config.m_uLocalIp[0] , netset.m_Eth0Config.m_uLocalIp[1] ,
			netset.m_Eth0Config.m_uLocalIp[2] , netset.m_Eth0Config.m_uLocalIp[3] ) ;
		//验证ip是否变更
		if( strcmp( localip , regc.local_ip ) != 0 ){
			strcpy(regc.local_ip, localip );
			printf( "localip change!!!!!!\r\n" ) ;
		}
		
		
		strcpy(regc.upnp_ip, "");
		strcpy(regc.usrname, "Loif");
		strcpy(regc.password, "Loif");
		regc.devtype = DEV_TYPE_INFO ;
	
		//设置变动则立即注册
		if(( m_staticSipRegState != REGISTER_SUCCESS ) || m_staticRegistNow  || m_staticRegistNow)
		{
			m_staticSipRegState = REGISTER_FAILED ;
			m_staticGetCallback = false ;
			m_staticRegistNow = false ;

			zsip_regc(&regc , regist_result_cb );	
			printf( "############### 注册二维码 ###################\r\n" ) ;	

			//等待注册成功
			while( !m_staticGetCallback )
			{
				sleep( 1 ) ;
			}
			preTime = time( NULL ) ;	
			
		}
		else
		{
			time_t now = time(NULL) ;
			if( now - preTime > 60*60 )
			{
				m_staticSipRegState = REGISTER_FAILED ;
				m_staticGetCallback = false ;
				m_staticRegistNow = false ;

				zsip_regc(&regc, regist_result_cb);	

				printf( "###############222 注册二维码 ###################\r\n" ) ;	
				//等待注册成功
				while( !m_staticGetCallback )
				{
					sleep( 1 ) ;
				}
				
				preTime = time( NULL ) ;	
			}
		}

		sleep(1 );
	}
	zsip_uninit();	
#endif

#else



#ifdef _BARCODE_SUP_
	int ret ;	
	char srv[32] = "register.zmododns.com";
	char localip[16] ;

 	NETWORK_PARA netset;	
	regc_t regc;	
	bool bChange=true ;
	time_t	preTime = 0 ;

	ret = zsip_init();

	memset( &regc , 0 , sizeof( regc ) ) ;
	

    if (ret < 0)
    {
        ERR("zsip init fail .\n");
        return ;
    }

    while( true )
    {
        g_cParaManage->GetSysParameter(SYSNET_SET, &netset);

        strcpy(regc.srvname, srv);
        regc.srvport = 8888;

		//无设备编号，不进行注册
		if( strlen( (char*)netset.m_CenterNet.deviceid ) ==  0 || (strcmp((char*)netset.m_CenterNet.deviceid, "0000000000") == 0))
		{
			sleep(5) ;
			continue ;
		}

		sprintf( localip , "%d.%d.%d.%d" , netset.m_Eth0Config.m_uLocalIp[0] , netset.m_Eth0Config.m_uLocalIp[1] ,
			netset.m_Eth0Config.m_uLocalIp[2] , netset.m_Eth0Config.m_uLocalIp[3] ) ;

		//验证ip是否变更
		if(memcmp(regc.local_ip, m_ip, CHAE_IP_LEN) != 0) 
				strcpy(regc.local_ip, m_ip);

		#if 0 
		if( strcmp( localip , regc.local_ip ) != 0 )
		{
			strcpy(regc.local_ip, localip );
			printf( "localip change!!!!!!\r\n" ) ;
			bChange = true ;
		}
		#endif
		
		strcpy(regc.upnp_ip, "");
		strcpy(regc.usrname, "Loif");
		strcpy(regc.password, "Loif");
		regc.devtype = DEV_TYPE_INFO;

		if( strcmp( (char*)netset.m_CenterNet.deviceid , regc.devid ) != 0 ){
			strncpy(regc.devid, (char*)netset.m_CenterNet.deviceid , 15 );
			regc.devid[15] = 0 ;	
			bChange = true ;
			NDB( "deviceid change!!!!!!\r\n" ) ;	
		}

		if( regc.local_video_port != netset.m_CenterNet.m_uVideoListenPt ||
			regc.local_http_port != netset.m_CenterNet.m_uHttpListenPt ||
			regc.local_phone_port != netset.m_CenterNet.m_uPhoneListenPt ||
			regc.upnp_http_port != netset.m_Eth0Config.m_http_port ||
			regc.upnp_video_port != netset.m_Eth0Config.m_v_port ||
			regc.upnp_phone_port != netset.m_Eth0Config.m_phone_port )
		{
			regc.local_video_port = netset.m_CenterNet.m_uVideoListenPt ;
			regc.local_http_port = netset.m_CenterNet.m_uHttpListenPt ;
			regc.local_phone_port = netset.m_CenterNet.m_uPhoneListenPt ;

			regc.upnp_http_port = netset.m_Eth0Config.m_http_port ;
			regc.upnp_video_port = netset.m_Eth0Config.m_v_port ;
			regc.upnp_phone_port = netset.m_Eth0Config.m_phone_port ;
			
			NDB( "******************port change!!!!!!\r\n" ) ;		
			bChange = true ; 
		}	

		//设置变动则立即注册
		if( bChange || ( m_staticSipRegState != REGISTER_SUCCESS ) || m_staticRegistNow ){
			m_staticSipRegState = REGISTER_FAILED ;
			m_staticGetCallback = false ;
			m_staticRegistNow = false ;

			zsip_regc(&regc , regist_result_cb );	
			bChange = false ;
			//printf( "****************change regist!!!!!\r\n" ) ;	

			//等待注册成功
			while( !m_staticGetCallback ){
				sleep( 1 ) ;
			}

			preTime = time( NULL ) ;	
			
		}
		else{
			time_t now = time(NULL) ;
			if( now - preTime > 60*60 ){
				m_staticSipRegState = REGISTER_FAILED ;
				m_staticGetCallback = false ;
				m_staticRegistNow = false ;

				zsip_regc(&regc, regist_result_cb);	

				//printf( "timeout regist barcode !!!!!\r\n" ) ;	

				//等待注册成功
				while( !m_staticGetCallback ){
					sleep( 1 ) ;
				}
				
				preTime = time( NULL ) ;	
			}
		}
		
		sleep(1 );
	}
	zsip_uninit();	
#endif

#endif
}

void CNetServer::onIDPingCmd( int sock , char *data , int len , sockaddr_in remoteAddr )
{
    STRUCT_ID_PING_ECHO resp ;
    NETWORK_PARA netset;

	if(len != sizeof( Cmd_Header ))
		return ;

	memset(&resp, 0x0, sizeof(STRUCT_ID_PING_ECHO));
	memset(&netset, 0x0, sizeof(NETWORK_PARA));
	
    g_cParaManage->GetSysParameter(SYSNET_SET, &netset);

	FillBroadcastInfo(&resp.device, &netset);
	resp.device.header.length = sizeof(STRUCT_ID_PING_ECHO) - sizeof(Cmd_Header);
    strncpy(resp.deviceID, (char *)netset.m_CenterNet.deviceid, 15) ;
    resp.deviceID[15] = 0 ;

    remoteAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

	sendto(sock, &resp, sizeof(STRUCT_ID_PING_ECHO), 0,(sockaddr*)&remoteAddr ,sizeof(struct sockaddr_in));
}

/**
 * @brief Filling device for broadcast information
 *
 * @param devTypeForReturn 		
 * @param ipaddr 	
 * @param netset 	
 * 
 * @return 0:success，-1:failed
 *
 */
int CNetServer::FillBroadcastInfo(STRUCT_PING_ECHO* Broadcast, NETWORK_PARA* netset)
{
	int m = 0;
	char c_ip[64] = {0x0};
    char mac[64] = {0x0};
    char mask[64] = {0x0};
	char gateway[64] = {0x0};
	
	if(Broadcast == NULL || netset == NULL)
		return -1;

    GetSoftWareVersion(&Broadcast->devInfo);

    m = JudgeNetworkStatus();
	if(m==1)
	{
		if (netset->m_Eth0Config.m_dhcp == 1 )
		{
			GetGw(gateway, (char *)LOCAL_NET_NAME ) ;
		}
		else
		{
			sprintf(gateway, "%d.%d.%d.%d", netset->m_Eth0Config.m_uGateWay[0], 
											netset->m_Eth0Config.m_uGateWay[1], 
											netset->m_Eth0Config.m_uGateWay[2],
											netset->m_Eth0Config.m_uGateWay[3]);
		}
		
		GetLocalNetInfo(LOCAL_NET_NAME,c_ip,mask,mac);
		
		/*add by hayson begin 2013.12.26*/
		/*bit0表示DHCP 开关；1开启，0关闭*/
		if(netset->m_Eth0Config.m_dhcp == 1)
			Broadcast->portInfo.recver |= (1 << 0);
		/*bit1网卡标志位 bit1=0表示有线，bit1=1表示无线*/
		Broadcast->portInfo.recver |= (0 << 1);
		/*add by hayson end 2013.12.26*/
	}
	else
	{
		if(get_network_support()!=NET_WORK_CARD_LOCAL)
		{
			if (netset->m_WifiConfig.WifiAddrMode.m_dhcp == 1)
			{
				GetGw( gateway , (char*)WIFI_NET_NAME ) ;
			}
			else
			{
				sprintf(gateway,"%d.%d.%d.%d",	netset->m_WifiConfig.WifiAddrMode.m_uGateWay[0],
												netset->m_WifiConfig.WifiAddrMode.m_uGateWay[1],
												netset->m_WifiConfig.WifiAddrMode.m_uGateWay[2],
												netset->m_WifiConfig.WifiAddrMode.m_uGateWay[3]);
			}
			GetLocalNetInfo(WIFI_NET_NAME, c_ip, mask,mac);
			/*add by hayson begin 2013.12.26*/
			/*bit0表示DHCP 开关；1开启，0关闭*/
			if(netset->m_WifiConfig.WifiAddrMode.m_dhcp == 1)
				Broadcast->portInfo.recver |= (1 << 0);
			/*bit1网卡标志位 bit1=0表示有线，bit1=1表示无线*/
			Broadcast->portInfo.recver |= (1 << 1);
			/*add by hayson end 2013.12.26*/

		}

		
	}
	
	strcpy(Broadcast->ipAddr.ipaddr, c_ip);
	strcpy(Broadcast->ipAddr.mac, mac);
	strcpy(Broadcast->ipAddr.geteway, gateway);
	strcpy(Broadcast->ipAddr.submask, mask);
	
	Broadcast->portInfo.webPort = netset->m_CenterNet.m_uHttpListenPt;
	Broadcast->portInfo.videoPort = netset->m_CenterNet.m_uVideoListenPt;
	Broadcast->portInfo.phonePort = netset->m_CenterNet.m_uPhoneListenPt;

	/*fill zsp seach head */
	Broadcast->header.head   = 0xaaaa5555;
	Broadcast->header.commd  = CMD_PING;
	Broadcast->header.length = sizeof(STRUCT_PING_ECHO) - sizeof( Cmd_Header );

	return 0;
}
void CNetServer::onPingCmd( int sock , char *data , int len , sockaddr_in remoteAddr )
{
    if( len < sizeof( Cmd_Header ) )
        return ;

    Cmd_Header *pHeader = (Cmd_Header *)data ;
    int type = pHeader->type ;
	
	if(( sizeof( Cmd_Header) != len )&&(type ==0))
		return ;

	char c_ip[64] = {0x0};
	char mac[64] = {0x0};
    char mask[64] = {0x0};
	
	char delimiters[] = " .,;:!-";
    char *token = NULL;
    char *maskaddr = NULL;
    char *gatewayaddr = NULL;
    char *ip = NULL ;
    int ret = -1;
    
	NETWORK_PARA 	netset;

	memset(&netset, 0x0, sizeof(NETWORK_PARA));
	g_cParaManage->GetSysParameter(SYSNET_SET, &netset);

	GetLocalNetInfo(LOCAL_NET_NAME,c_ip,mask,mac);

	//=========================
	//查询命令
	if(type ==0)
	{
		int i = 0;
		int j = 0;

		//add by albert 2013.6.19
		if( sizeof( Cmd_Header) != len )
			return ;
		
		STRUCT_PING_ECHO resp;
		memset(&resp, 0x0, sizeof(STRUCT_PING_ECHO));
		
		if(FillBroadcastInfo(&resp, &netset) < 0)
			ERR("Get DevInfo error\r\n");
		
        remoteAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

        ret = sendto(sock , &resp , sizeof( resp ) , 0 , (sockaddr *)&remoteAddr , sizeof(struct sockaddr_in));
    }
    else if(type == 1)	//设置命令
    {
        if( sizeof( STRUCT_PING_SETADDR_REQUEST ) != len )
            return ;

        STRUCT_PING_SETADDR_REQUEST *pReq = (STRUCT_PING_SETADDR_REQUEST *)data ;

        ip = (char *)pReq->ipAddr.ipaddr;
        maskaddr = (char *)pReq->ipAddr.submask;
        gatewayaddr = (char *)pReq->ipAddr.geteway;

        if(strcmp(mac, pReq->ipAddr.mac) == 0)
        {
            token = strsep (&ip, delimiters);
            netset.m_Eth0Config.m_uLocalIp[0] = atoi(token);
            token = strsep (&ip, delimiters);
            netset.m_Eth0Config.m_uLocalIp[1] = atoi(token);
            token = strsep (&ip, delimiters);
            netset.m_Eth0Config.m_uLocalIp[2] = atoi(token);
            token = strsep (&ip, delimiters);
            netset.m_Eth0Config.m_uLocalIp[3] = atoi(token);

            token = strsep (&maskaddr, delimiters);
            netset.m_Eth0Config.m_uMask[0] = atoi(token);
            token = strsep (&maskaddr, delimiters);
            netset.m_Eth0Config.m_uMask[1] = atoi(token);
            token = strsep (&maskaddr, delimiters);
            netset.m_Eth0Config.m_uMask[2] = atoi(token);
            token = strsep (&maskaddr, delimiters);
            netset.m_Eth0Config.m_uMask[3] = atoi(token);

            token = strsep (&gatewayaddr, delimiters);
            netset.m_Eth0Config.m_uGateWay[0] = atoi(token);
            token = strsep (&gatewayaddr, delimiters);
            netset.m_Eth0Config.m_uGateWay[1] = atoi(token);
            token = strsep (&gatewayaddr, delimiters);
            netset.m_Eth0Config.m_uGateWay[2] = atoi(token);
            token = strsep (&gatewayaddr, delimiters);
            netset.m_Eth0Config.m_uGateWay[3] = atoi(token);
            int echo = 0; 
           
            if(0 == SetNetAttrib(&netset, 0))
            {
                echo = 0;
            }
            else
            {
                echo = -1;
            }
            STRUCT_PING_SETADDR_ECHO resp ;
            memcpy( &resp.header , &pReq->header , sizeof( Cmd_Header ) ) ;
            resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;

            resp.echo = echo ;

            sendto(sock, &resp, sizeof( resp ) , 0, (struct sockaddr *)&remoteAddr , sizeof(struct sockaddr_in));
			netset.m_Eth0Config.m_dhcp = 0;
			 g_cParaManage->SetSystemParameter(SYSNET_SET, &netset);
            //printf( "********send ping resp************\r\n" ) ;
        }
    }
	else if(type == 2)
	{
		/*通过广播设置IP是为了解决设备出厂时ip在真实环境中可能是不可用的情况(如ip冲突)，所以使用广播设置*/

		STRUCT_PING_SETDHCP_REQUEST* pReq = (STRUCT_PING_SETDHCP_REQUEST *)data; 	
		
		if(strcmp(mac, pReq->mac) == 0)
        {
        	int m = 0;
			m = JudgeNetworkStatus();
			
			if(m==1)
			{
				NDB("broadcast set  local DHCP !\r\n");
				STRUCT_PING_SETADDR_ECHO resp ;
				int ret = -1;

				ret = RunDHCP((char*)LOCAL_NET_NAME);

	            memcpy( &resp.header , pHeader , sizeof( Cmd_Header ) ) ;
	            resp.header.length = sizeof( resp ) - sizeof( Cmd_Header ) ;
	            resp.echo = ret;
	            sendto(sock, &resp, sizeof( resp ) , 0, (struct sockaddr *)&remoteAddr , sizeof(struct sockaddr_in));
				
				netset.m_Eth0Config.m_dhcp = 1;
				g_cParaManage->SetSystemParameter(SYSNET_SET, &netset);
			}
		 }
	}
    return ;
}

int CNetServer::JudgeNetworkStatus()
{

	NET_WORK_CARD CardID;
	CardID = get_network_support();
	NDB("CardID=%d\n",CardID);
	if(CardID == NET_WORK_CARD_LOCAL || CardID == NET_WORK_CARD_LOCAL_AND_WIFI)
	{
		#if defined(DM368) 
			return 1;
		#endif

		int	value = -1;
		char buf[sizeof(struct ifreq)];
		struct ifreq* ifreq = (struct ifreq*)buf;
		struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifreq->ifr_data;

		set_ifreq_to_ifname(ifreq);

		if (network_ioctl(SIOCGMIIPHY, ifreq, "SIOCGMIIPHY") < 0) {
			return IFSTATUS_ERR;
		}

		mii->reg_num = 1;

		if (network_ioctl(SIOCGMIIREG, ifreq, "SIOCGMIIREG") < 0) {
			return IFSTATUS_ERR;
		}
		NDB("valout=%d\n",mii->val_out);

		value = (mii->val_out & 0x0004) ? 1 : 0 ;

		return value;
	}
	else if(CardID == NET_WORK_CARD_WIFI)
	{
		return 0;
	}
}


// 获取IP地址，子网掩码，MAC地址
int CNetServer::GetLocalNetInfo(  const char *lpszEth, char *szIpAddr, char *szNetmask, char *szMacAddr )
{
    int ret = 0;
    struct ifreq req;
    struct sockaddr_in *host = NULL;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( -1 == sockfd )
    {
        return -1;
    }

    bzero(&req, sizeof(struct ifreq));
    strcpy(req.ifr_name, lpszEth);
    if ( ioctl(sockfd, SIOCGIFADDR, &req) >= 0 )
    {
        host = (struct sockaddr_in *)&req.ifr_addr;
        strcpy(szIpAddr, inet_ntoa(host->sin_addr));
    }
    else
    {
        ret = -1;
    }

    bzero(&req, sizeof(struct ifreq));
    strcpy(req.ifr_name, lpszEth);
    if ( ioctl(sockfd, SIOCGIFNETMASK, &req) >= 0 )
    {
        host = (struct sockaddr_in *)&req.ifr_addr;
        strcpy(szNetmask, inet_ntoa(host->sin_addr));
    }
    else
    {
        ret = -1;
    }

    bzero(&req, sizeof(struct ifreq));
    strcpy(req.ifr_name, lpszEth);
    if ( ioctl(sockfd, SIOCGIFHWADDR, &req) >= 0 )
    {
        sprintf(
            szMacAddr, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)req.ifr_hwaddr.sa_data[0],
            (unsigned char)req.ifr_hwaddr.sa_data[1],
            (unsigned char)req.ifr_hwaddr.sa_data[2],
            (unsigned char)req.ifr_hwaddr.sa_data[3],
            (unsigned char)req.ifr_hwaddr.sa_data[4],
            (unsigned char)req.ifr_hwaddr.sa_data[5]
        );
    }
    else
    {
        ret = -1;
    }

    if ( sockfd != -1 )
    {
        close(sockfd);
        sockfd = -1;
    }

    return ret; 
} 



int CNetServer::GetIPaddr(char *net_ip, char *name)
{
	struct ifreq ifr;
	int ret = 0;
	int fd;
	
	strcpy(ifr.ifr_name, name);
	ifr.ifr_addr.sa_family = AF_INET;
	
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;
		
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) 
	{
		ret = -1;
	}
	else
		strcpy(net_ip,inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));

	close(fd);

	return	ret;
}



int CNetServer::SetIPaddr(char *net_ip, char *name)
{	
	struct sockaddr	addr;
	struct ifreq ifr;
	char gateway_addr[32] = {0};
	int ret = 0;
	int fd;	
	
	((struct sockaddr_in *)&(addr))->sin_family = PF_INET;
	((struct sockaddr_in *)&(addr))->sin_addr.s_addr = inet_addr(net_ip);

	ifr.ifr_addr = addr;
	strcpy(ifr.ifr_name,name);

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;

	GetGw(gateway_addr, name);		/* 在设置IP时会清除网络,所以先保存好网络设置 */

	if (ioctl(fd, SIOCSIFADDR, &ifr) != 0) 
	{
		ret = -1;
	}

	close(fd);

	//SetGwAddr(gateway_addr, name);		/*恢复网络的网关设置 */

	return	ret;
}

int CNetServer::GetMaskAddr(char *net_mask, char *name)
{
	struct ifreq ifr;
	int ret = 0;
	int fd;	
	
	strcpy(ifr.ifr_name, name);
	ifr.ifr_addr.sa_family = AF_INET;
	
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;
		
	if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) 
	{
		ret = -1;
	}
	
	strcpy(net_mask,inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));

	close(fd);

	return	ret;
}

int CNetServer::SetMaskAddr(char *mask_ip, char *name) /* name : eth0 eth1 lo and so on */
{	
	struct sockaddr	addr;
	struct ifreq ifr;
	char gateway_addr[32];
	int ret = 0;
	int fd;	
	
	((struct sockaddr_in *)&(addr))->sin_family = PF_INET;
	((struct sockaddr_in *)&(addr))->sin_addr.s_addr = inet_addr(mask_ip);
	ifr.ifr_netmask = addr;
	strcpy(ifr.ifr_name,name);

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;
		
	GetGw(gateway_addr, name); /* 在设置IP时会清除网络,所以先保存好网络设置 */

	if (ioctl(fd, SIOCSIFNETMASK, &ifr) != 0) 
	{
		ret = -1;
	}

	close(fd);

	//SetGwAddr(gateway_addr, name); /*恢复网络的网关设置 */

	return	ret;
}



int CNetServer::GetGw(char *gateway_addr, char *name)
{
	char buff[256];
	char ifname[32] = {0};
	int  nl = 0 ;
	struct in_addr gw;
	int flgs, ref, use, metric;
	unsigned  long d,g,m;

	FILE	*fp;
	
	if((fp = fopen("/proc/net/route", "r")) == NULL)
		return -1;	
		
	nl = 0 ;
	while( fgets(buff, sizeof(buff), fp) != NULL ) 
	{
		if(nl) 
		{
			int ifl = 0;
			if(sscanf(buff, "%s%lx%lx%X%d%d%d%lx",
				   ifname,&d, &g, &flgs, &ref, &use, &metric, &m)!=8) 
			{
				//continue;
				fclose(fp);
				return	-2;
			}

			ifl = 0;        /* parse flags */
			if(flgs&RTF_UP && (strcmp(name,ifname)== 0)) 
			{			
				gw.s_addr   = g;
					
				if(d==0)
				{
					strcpy(gateway_addr,inet_ntoa(gw));
					fclose(fp);
					return 0;
				}				
			}

		}
		nl++;
	}	
	
	if(fp)
	{
		fclose(fp);
		fp = NULL;
	}
	
	return	-1;
}


int CNetServer::SetGwAddr(char *gateway_addr, char *name)
{
	char old_gateway_addr[32];
	struct rtentry rt;
	unsigned long gw;
	int fd;
	int ret = 0;
	
	GetGw(old_gateway_addr, name);

	DelGwAddr(old_gateway_addr, name);
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ret = -1;
	}	

	gw = inet_addr(gateway_addr);
	memset((char *) &rt, 0, sizeof(struct rtentry));

	((struct sockaddr_in *)&(rt.rt_dst))->sin_addr.s_addr = 0;

	rt.rt_flags = RTF_UP | RTF_GATEWAY ;
	//rt.rt_flags = 0x03;

	((struct sockaddr_in *)&(rt.rt_dst))->sin_family = PF_INET;
	
	((struct sockaddr_in *)&(rt.rt_gateway))->sin_addr.s_addr = gw;	
	((struct sockaddr_in *)&(rt.rt_gateway))->sin_family = PF_INET;
	

	((struct sockaddr_in *)&(rt.rt_genmask))->sin_addr.s_addr = 0;
	((struct sockaddr_in *)&(rt.rt_genmask))->sin_family = PF_INET;
	rt.rt_dev = name;

	if (ioctl(fd, SIOCADDRT, &rt) < 0)
	{
		ret = -1;
	}
	close(fd);
	
	return	ret;
}


int CNetServer::DelGwAddr(char *gateway_addr, char *name)
{
	struct rtentry rt;
	unsigned long gw;
	int ret = 0;
	int fd;
	
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;
	
	gw = inet_addr(gateway_addr);
	memset((char *) &rt, 0, sizeof(struct rtentry));

	rt.rt_flags = RTF_UP | RTF_GATEWAY ;
	
	((struct sockaddr_in *)&(rt.rt_dst))->sin_family = PF_INET;
	
	((struct sockaddr_in *)&(rt.rt_gateway))->sin_addr.s_addr = gw;	
	((struct sockaddr_in *)&(rt.rt_gateway))->sin_family = PF_INET;

	
	((struct sockaddr_in *)&(rt.rt_genmask))->sin_addr.s_addr = 0;
	((struct sockaddr_in *)&(rt.rt_genmask))->sin_family = PF_INET;
	
	rt.rt_dev = name;
		
	if (ioctl(fd, SIOCDELRT, &rt) < 0) 	
	{
		ret = -1;
	}

	close(fd);

	return	ret;
}

char* CNetServer::safe_strncpy(char *dst, const char *src, size_t size)
{
    dst[size-1] = '\0';
    return strncpy(dst, src, size-1);
}

int CNetServer::SetMacAddr(char *addr, char *name) 
{												
	const struct arg1opt *a1op;
	const struct options *op;
	int 	sockfd;
	struct 	ifreq ifr;
	struct 	sockaddr sa;
	unsigned char mask;
	char 	*p = (char*)"hw";
	char 	host[128];
	
	/* Create a channel to the NET kernel. */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return -1;
	}
	
	/* get interface name */
	safe_strncpy(ifr.ifr_name, name, IFNAMSIZ);
	mask = N_MASK;
	
	for (op = OptArray ; op->name ; op++)
	{		
		if (strcmp("hw",op->name) == 0) 
		{
			if ((mask &= op->flags)) 
			{ 
				goto SET_NET_PHYADDR_FOUND_ARG;
			}
		}
	}
	close(sockfd);
	return -4;
	
SET_NET_PHYADDR_FOUND_ARG:
	
	a1op = Arg1Opt + 6;
	
	safe_strncpy(host, addr, (sizeof host));
	
	if (in_ether(host, &sa)) 
	{
		close(sockfd);
		return -2;
	}
	p = (char *) &sa;
	
	memcpy((((char *) (&ifr)) + a1op->ifr_offset), p, sizeof(struct sockaddr));
	
	if (ioctl(sockfd, a1op->selector, &ifr) < 0)
	{
		close(sockfd);
		return -3;
	}
	
	close(sockfd);

	return  0;
}
 


int CNetServer::SetNetAttrib(NETWORK_PARA *netset, int net_card_id )
{
	char c_ip[CHAE_IP_LEN];
	int m;
	if(net_card_id == 0)
	{
		KillUdhcpc((char*)LOCAL_NET_NAME);		
		system("ifconfig eth0 up");		
		sleep(4);
		 
		sprintf(c_ip,"%d.%d.%d.%d",netset->m_Eth0Config.m_uLocalIp[0],
									netset->m_Eth0Config.m_uLocalIp[1],
									netset->m_Eth0Config.m_uLocalIp[2],
									netset->m_Eth0Config.m_uLocalIp[3]);
		NDB("Set loacl IP addr %s\n",c_ip);
		SetIPaddr(c_ip, (char*)LOCAL_NET_NAME);

		memset(c_ip, 0x0, CHAE_IP_LEN);
		sprintf(c_ip,"%d.%d.%d.%d",netset->m_Eth0Config.m_uMask[0],
														netset->m_Eth0Config.m_uMask[1],
														netset->m_Eth0Config.m_uMask[2],
														netset->m_Eth0Config.m_uMask[3]);
		NDB("Set net mask %s\n",c_ip);
		SetMaskAddr(c_ip, (char*)LOCAL_NET_NAME);

		m = JudgeNetworkStatus();
		NDB("kb want to know =%d \n",m);
		if (m == 1)
		{
		memset(c_ip, 0x0, CHAE_IP_LEN);
		sprintf(c_ip,"%d.%d.%d.%d",netset->m_Eth0Config.m_uGateWay[0],
									netset->m_Eth0Config.m_uGateWay[1],
									netset->m_Eth0Config.m_uGateWay[2],
									netset->m_Eth0Config.m_uGateWay[3]);
		NDB("Set gateway addr %s %s\n",c_ip, LOCAL_NET_NAME);	
		SetGwAddr(c_ip, (char*)LOCAL_NET_NAME);		
		}
		hi_netcfg_update_dns(DNS_CFG_FILE, netset->m_DNS.m_umDNSIp, netset->m_DNS.m_usDNSIp);		
	}
	else if(net_card_id == 2)
	{
		#ifdef SUPPORT_WIFI
		KillUdhcpc((char*)WIFI_NET_NAME);
		sleep(2);
		sprintf(c_ip,"%d.%d.%d.%d",netset->m_WifiConfig.WifiAddrMode.m_uLocalIp[0],
									netset->m_WifiConfig.WifiAddrMode.m_uLocalIp[1],
									netset->m_WifiConfig.WifiAddrMode.m_uLocalIp[2],
									netset->m_WifiConfig.WifiAddrMode.m_uLocalIp[3]);
		NDB("Set wifi loacl IP addr %s\n",c_ip);
		SetIPaddr(c_ip, (char*)WIFI_NET_NAME);
		
		sprintf(c_ip,"%d.%d.%d.%d",netset->m_WifiConfig.WifiAddrMode.m_uMask[0],
										netset->m_WifiConfig.WifiAddrMode.m_uMask[1],
										netset->m_WifiConfig.WifiAddrMode.m_uMask[2],
										netset->m_WifiConfig.WifiAddrMode.m_uMask[3]);
		NDB("Set wifi net mask %s\n",c_ip);
		SetMaskAddr(c_ip, (char*)WIFI_NET_NAME);

		m = JudgeNetworkStatus();
		if (m == 0)
		{
		sprintf(c_ip,"%d.%d.%d.%d", netset->m_WifiConfig.WifiAddrMode.m_uGateWay[0],
									netset->m_WifiConfig.WifiAddrMode.m_uGateWay[1],
									netset->m_WifiConfig.WifiAddrMode.m_uGateWay[2],
									netset->m_WifiConfig.WifiAddrMode.m_uGateWay[3]);
		
		NDB("Set gateway addr %s %s\n",c_ip, WIFI_NET_NAME);	
		SetGwAddr(c_ip, (char*)WIFI_NET_NAME);		
		}
		hi_netcfg_update_dns(DNS_CFG_FILE, netset->m_WifiConfig.WifiAddrMode.m_umDNSIp, netset->m_WifiConfig.WifiAddrMode.m_usDNSIp);		
		#endif
	}


		
	return 0 ;
}

//==========================================
//验证用户
int	CNetServer::VerifyUser( char *user , char *pwd , int &permit )
{
	int echoCode = 0;
	int useridx = 0;

	//输入参数检查
	if( !user || !pwd )
		return 1 ;

	if( strlen( user ) > USER_NAME_LEN || strlen( pwd ) > USER_PASS_LEN )
		return 1 ;

	permit = 1 ;//默认普通权限

	if((strcmp("ZMDNVRLOGIN", user) == 0)&&(strcmp("Zmodo963852", pwd) == 0))
	{
		//printf("NVR Login\n");
		echoCode = 0;//登入成功
		permit = 0 ;//超级用户权限
		return echoCode ;
	}

	USERGROUPSET netUser;

	//获取用户列表
	g_cParaManage->GetSysParameter(SYSUSERPREMIT_SET,&netUser);
	
	for( useridx = 0 ; useridx < USER_LIST_COUNT ; useridx ++ )
	{
		//用户名不能为空
		if( strlen( netUser.m_UserSet[useridx].m_cUserName ) <= 0 )
			continue ;

		if(strcmp(netUser.m_UserSet[useridx].m_cUserName, user) == 0)
		{	
			
			if(strcmp(netUser.m_UserSet[useridx].m_s32Passwd, pwd) != 0)
			{
				echoCode = 1;//密码错误
				break;
			}
			echoCode = 0;//登入成功
			permit = netUser.m_UserSet[useridx].m_s32UserPermit;
			break;
		}
    }

	if(useridx >= USER_LIST_COUNT )
	{
		echoCode = 2;//用户不存在
	}

	return echoCode ;
	
}

int	CNetServer::VerifyUserByMd5( char *user , char *pwd , int &permit , char *userReal )
{
    int echoCode = 0;
    int index = 0;
    char *md5Str = NULL ;

    //输入参数检查
    if( !user || !pwd )
    {
        return 1 ;
    }

	permit = 1 ;//默认普通权限

	USERGROUPSET netUser;

    //获取用户列表
    g_cParaManage->GetSysParameter(SYSUSERPREMIT_SET, &netUser);

    //先比较明码
    for( index = 0 ; index < 16 ; index++ )
    {
        if(strcmp(netUser.m_UserSet[index].m_cUserName, user) == 0)
        {
            //密码错误
            if(strcmp(netUser.m_UserSet[index].m_s32Passwd, pwd ) == 0)
            {
                strcpy( userReal , netUser.m_UserSet[index].m_cUserName ) ;
                return 0 ;
            }
            else
            {
                return 1 ;
            }
        }
    }

	//再比较加密码
	for( index = 0 ; index < 16 ; index ++)
	{
		//用户名不能为空
		if( strlen( netUser.m_UserSet[index].m_cUserName ) <= 0 )
			continue ;

		md5Str = MDString( netUser.m_UserSet[index].m_cUserName ) ;
		if( strcmp ( md5Str , user )== 0 )
		{
			md5Str = MDString( netUser.m_UserSet[index].m_s32Passwd ) ;
			if( strcmp( md5Str , pwd ) == 0 )
			{
				strcpy( userReal , netUser.m_UserSet[index].m_cUserName ) ;
				return 0 ;
			}
			else
			{
				return 1 ;
			}

		}
	}

	return 2 ;
}

int	CNetServer::VerifyVideoPort(int port)
{
	if(port < 1025 || port > 65535)
		return -1;
	
	NETWORK_PARA netset;
	memset(&netset, 0x0,sizeof(NETWORK_PARA));
	
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset); 

	if(netset.m_CenterNet.m_uVideoListenPt == port || netset.m_Eth0Config.m_v_port == port)
		return 0;

	return -1;
}

void CNetServer::CheckNetCardStatus( )
{
	#if 0
	int netCard;
	char						BufferTmp[CHAE_IP_LEN] ;
	NETWORK_PARA netset;

	static bool IsSetDhcp = false ;
	
	memset(&netset,0x0,sizeof(NETWORK_PARA));
	netCard = JudgeNetworkStatus();

	if(m_netCard != netCard)
	{
		m_netCard = netCard;
		
		if(netCard == 1)
		{	
				g_cParaManage->GetSysParameter(SYSNET_SET,&netset);

				if (1 == netset.m_Eth0Config.m_dhcp)
				{
					//设置dhcp 
					if( !IsSetDhcp )
					{
						if(GetNetServerObj()->RunDHCP((char*)LOCAL_NET_NAME) < 0)
							printf("######### faild to set dhcp (func:%s line:%d)  ########\r\n",__func__, __LINE__);
						else
							printf("######### set dhcp success (func:%s line:%d)  ########\r\n",__func__, __LINE__);					
						IsSetDhcp = true ;	
					}
				}
				else
				{
					GetNetServerObj()->SetNetAttrib(&netset,0);
				}
			}
			else
			{
				#ifdef SUPPORT_WIFI	
				IsSetDhcp = false ;
				g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
				if (1 == netset.m_WifiConfig.WifiAddrMode.m_dhcp)
				{
					memset(BufferTmp, 0, CHAE_IP_LEN);
					GetNetServerObj()->GetGw(BufferTmp,(char*)WIFI_NET_NAME);
					delnet_gateway((char*)WIFI_NET_NAME, BufferTmp,0);
				}
				else
				{
					memset(BufferTmp, 0, CHAE_IP_LEN);
					sprintf(BufferTmp,"%d.%d.%d.%d",netset.m_WifiConfig.WifiAddrMode.m_uGateWay[0],netset.m_WifiConfig.WifiAddrMode.m_uGateWay[1],netset.m_WifiConfig.WifiAddrMode.m_uGateWay[2],netset.m_WifiConfig.WifiAddrMode.m_uGateWay[3]);
					delnet_gateway((char*)WIFI_NET_NAME, BufferTmp,0);
					hi_netcfg_update_dns(DNS_CFG_FILE, netset.m_DNS.m_umDNSIp, netset.m_DNS.m_usDNSIp);
					usleep(10000);
				}
				#endif
			}

	}

	#endif


	int m;
	
	NETWORK_PARA netset;
	char						BufferTmp[CHAE_IP_LEN] ;
	static bool IsSetDhcp = false ;

	memset(&netset,0x0,sizeof(NETWORK_PARA));
	m = JudgeNetworkStatus();
	
	if( change_flag2 == 1)
	{
		if(m==1)
			change_flag1=0;
		else
			change_flag1=1;
		if(m_netcnt++>2)
		{
			m_netcnt=0;
			change_flag2 = 0;
		}
	}
	
	if(m==1)
	{	
		if ( change_flag1 == 0x00)
		{
			g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
			
			if (1 == netset.m_Eth0Config.m_dhcp)
			{
				memset(BufferTmp,0, CHAE_IP_LEN);
				GetNetServerObj()->GetGw(BufferTmp,(char*)LOCAL_NET_NAME);
				delnet_gateway(NET_CARD_LOCAL, (char*)LOCAL_NET_NAME, (char*)WIFI_NET_NAME, BufferTmp);
				
				//设置dhcp 
				if( !IsSetDhcp )
				{
					if(GetNetServerObj()->RunDHCP((char*)LOCAL_NET_NAME) < 0)
						NDB("######### faild to set dhcp  ########\r\n");
					else
						NDB("######### set dhcp success  ########\r\n");							
					change_flag2 =1;
					IsSetDhcp = true ;
					
				}
			}
			else
			{
				memset(BufferTmp,0,CHAE_IP_LEN);
				sprintf(BufferTmp,"%d.%d.%d.%d",netset.m_Eth0Config.m_uGateWay[0],netset.m_Eth0Config.m_uGateWay[1],netset.m_Eth0Config.m_uGateWay[2],netset.m_Eth0Config.m_uGateWay[3]);
				
				delnet_gateway(NET_CARD_LOCAL, (char*)LOCAL_NET_NAME, (char*)WIFI_NET_NAME, BufferTmp);
				hi_netcfg_update_dns(DNS_CFG_FILE, netset.m_DNS.m_umDNSIp, netset.m_DNS.m_usDNSIp);
				usleep(10000);

			}
			change_flag1 = 0x01;
		}
	}
	else
	{
		
		if(get_network_support()!=NET_WORK_CARD_LOCAL)		
		{
			IsSetDhcp = false ;
			if (change_flag1 == 0x01)
			{
				g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
				if(netset.m_WifiConfig.WifiAddrMode.m_u8Selected == 1)/*add by hayson 2014.1.20*/
				{
					if (1 == netset.m_WifiConfig.WifiAddrMode.m_dhcp)
					{
						memset(BufferTmp,0,CHAE_IP_LEN);
						GetNetServerObj()->GetGw(BufferTmp,(char*)WIFI_NET_NAME);
						delnet_gateway(NET_CARD_WIFI, (char*)LOCAL_NET_NAME, (char*)WIFI_NET_NAME, BufferTmp);
						
						if(GetNetServerObj()->RunDHCP((char*)WIFI_NET_NAME) < 0)
								NDB("######### faild to set dhcp   ########\r\n");
						else
								NDB("######### set dhcp success   ########\r\n");	
					}
					else
					{
						memset(BufferTmp,0,CHAE_IP_LEN);
						sprintf(BufferTmp,"%d.%d.%d.%d",netset.m_WifiConfig.WifiAddrMode.m_uGateWay[0],netset.m_WifiConfig.WifiAddrMode.m_uGateWay[1],netset.m_WifiConfig.WifiAddrMode.m_uGateWay[2],netset.m_WifiConfig.WifiAddrMode.m_uGateWay[3]);
						delnet_gateway(NET_CARD_WIFI, (char*)LOCAL_NET_NAME, (char*)WIFI_NET_NAME, BufferTmp);
						hi_netcfg_update_dns(DNS_CFG_FILE, netset.m_DNS.m_umDNSIp, netset.m_DNS.m_usDNSIp);
						usleep(10000);

					}
				}
				change_flag1 = 0x00;
			}
			}
		
	}
}

//-----------------------------------------------------------------------------
//将DNS参数写到文件
//-----------------------------------------------------------------------------
int CNetServer::hi_netcfg_update_dns(char *cfgFile, unsigned char* dnsaddr, unsigned char* dnsaddr2)
{
	int  nRet = 0;
	char str[128] = {0};
	FILE *pFile;

	
	if((pFile = fopen(cfgFile, "w+b")) ==NULL)
	{
		return -1;
	}
	
	memset(str, 0, 128);
	sprintf(str, "search localdomain\n");
	nRet =  fputs(str, pFile);

	if (dnsaddr != 0)
	{
		memset(str, 0, 128);
		sprintf(str, "nameserver %d.%d.%d.%d\n",  dnsaddr[0],	
											      dnsaddr[1], 
											      dnsaddr[2],
											      dnsaddr[3]);
		nRet =  fputs(str, pFile);
	}

	if (dnsaddr2 != 0)
	{
		memset(str, 0, 128);
		sprintf(str, "nameserver %d.%d.%d.%d\n",  dnsaddr2[0],	
											      dnsaddr2[1], 
											      dnsaddr2[2],
											      dnsaddr2[3]);	
		nRet =  fputs(str, pFile);
	}	

	fclose(pFile);
	return 0;
}


void CNetServer::RegistDDNS( )
{
	m_doDdns = 1 ;
}

void CNetServer::RegistUPNP( )
{
	//m_doUPNP = 1 ;
}

bool CNetServer::RequestTalkOn( unsigned userid )
{
	CAutoMutex atlck( &m_taklMutex ) ;
	if( m_talkOn )
	{
		ERR("request Talkon failed request userid = %d , talkonid = %d\r\n" , userid , m_talkUserID ) ;
		return false ;
	}
	if(!GetSpeeker())
	{
		ERR("GetSpeeker() failed request userid = %d , talkonid = %d\r\n" , userid , m_talkUserID ) ;
		return false ;
	}
	m_talkUserID = userid ;
	m_talkOn = true ;

	NDB("request Talkon success userid = %d\r\n" , userid ) ;
	return true ;
}
bool CNetServer::RequestTalkOff( unsigned userid )
{
	CAutoMutex atlck( &m_taklMutex ) ;
	if( !m_talkOn )
		return false ;

	if( userid != m_talkUserID )
		return false ;

	m_talkOn = false ;
	NDB( "release talk userid = %d \r\n" , userid ) ;
	userid = 0 ;
	ReleaseSpeeker();

	return true ;
}


/*NTP初始化线程*/
void *CNetServer::ntpThreadProc(void *arg)
{
	char c_ip[64] = {0x0};
	char mac[64] = {0x0};
	char mask[64] = {0x0};
	
	PARAMETEREXTEND ntp;
	
	memset(&ntp, 0x0, sizeof(PARAMETEREXTEND));


	g_cParaManage->GetSysParameter(EXTEND_SET, &ntp);
	   while(1)
	   {
	   	c_ip[0] = '0';  
	   	int nNetCardID =  GetNetServerObj()->JudgeNetworkStatus( );
		int ret = -1;
	
		if(nNetCardID)
			ret = GetNetServerObj()->GetLocalNetInfo(LOCAL_NET_NAME, c_ip, mask, mac);
		else
		{
			
			if(get_network_support()!=NET_WORK_CARD_LOCAL)
			{
				ret = GetNetServerObj()->GetLocalNetInfo(WIFI_NET_NAME, c_ip, mask, mac);
			}
			
		}  
		   if(ret == 0)
		   {
			   if(c_ip[0] != '0')
			   {
				   sleep(3);
					if(ntp.m_ntp.m_ntp_switch)
					{
						//ntpclient_start(ntp.m_ntp.m_diff_timezone, 24 * 3600, NULL, ntp.m_ntp.m_idx_tzname == 0 ?1:0);	

						RestartNtpClient(ntp.m_ntp.m_diff_timezone);
					}
				   break;
			   }   
		   }
		   else
		   {
			   sleep(2);
			   continue;
		   }
	   }	
	   
	
}


int CNetServer::InitNtp()
{ 
	m_ntpThread = CreateZmdThread(0 , ntpThreadProc) ;
    if( !m_ntpThread )
    {
        ERR( "create NTP thread failed !!!\r\n" ) ;
		return -1;
    }

	return 0;
}


int CNetServer::CookBuf(FILE * fp, char* pNetName)
{

	char tmp[8] = {0};
    int ch;
	int i = 0;

	if(strcmp(pNetName, WIFI_NET_NAME) == 0 &&get_network_support()!=NET_WORK_CARD_LOCAL)
	{
		if(strcmp("ra0", WIFI_NET_NAME) == 0)
		{	
			for ( ; (ch = fgetc(fp)) != EOF; )
			{
				if(ch == 'r' || ch == 'a' || ch == '0')
				{
					tmp[i] = ch;
					i++;
				}
			}
		}
		else if(strcmp("wlan0", WIFI_NET_NAME) == 0)
		{
			for ( ; (ch = fgetc(fp)) != EOF; )
			{
				if(ch == 'w' || ch == 'l' || ch == 'a' || ch == 'n' || ch == '0')
				{
					tmp[i] = ch;
					i++;
				}
			}
		}

	}
	else if(strcmp(pNetName, LOCAL_NET_NAME) == 0)
	{
		for ( ; (ch = fgetc(fp)) != EOF; )
		{
			if(ch == 'e' || ch == 't' || ch == 'h' || ch == '0')
			{
				tmp[i] = ch;
				i++;
			}
		}
	}
	
	if(strstr(tmp, pNetName) != NULL)
	{
		return HI_TRUE;
	}
	
	return HI_SUCCESS;
}

int CNetServer::CheckPid(char* pid, char* pNetName)
{
	char cmd[64] = {0};
	FILE *read_fp;
	
	sprintf(cmd, "/proc/%s/cmdline", pid);
	read_fp = fopen(cmd, "r"); 	
	
	if ( read_fp != NULL ) 
	{ 
		if(CookBuf(read_fp, pNetName))
		{
			fclose(read_fp);
			return HI_TRUE;
		}
	}
    fclose(read_fp);
	
	return HI_SUCCESS;
}

long* CNetServer::FindPid(const char* pidName, char* pNetName)
{
	 DIR *dir;
	 struct dirent *next;
	 long* pidList=NULL;
	 int i=0;

	 dir = opendir("/proc");
	 if (!dir)
	 {
			 fprintf(stderr, "Cannot open /proc\n");
			 return pidList;
	 }

	 while ((next = readdir(dir)) != NULL)
	 {
		FILE *status;
		char filename[1024];
		char buffer[1024];
		char name[1024];

		if (strcmp(next->d_name, "..") == 0)
			 continue;

		if (!isdigit(*next->d_name))
			 continue;
		sprintf(filename, "/proc/%s/status", next->d_name);

		if (! (status = fopen(filename, "r")) )
		{
			 continue;
		}

		if (fgets(buffer, 1024-1, status) == NULL)
		{
			 fclose(status);
			 continue;
		}

		fclose(status);

		sscanf(buffer, "%*s %s", name);

		if ( pidName != NULL && name[0] != '\0')
		{
			 if (strcmp(name, pidName) == 0)
			 {
				if(CheckPid(next->d_name, pNetName))
				{
					pidList = ( long*)realloc((void*)pidList, sizeof(long) * (i+2));
					pidList[i++]=strtol(next->d_name, NULL, 0);
				}
			 }
		}
		else
		{
			closedir(dir);
			return pidList;
		}
			
	 }

	 if (pidList)
	 {
			 pidList[i]=0;
	 }

	closedir(dir);
	return pidList;
}

long CNetServer::GetDhcpPid(char *name, char* pNetName) 
{ 
	long ret = 0;
	long *pid_t=NULL;
 
	pid_t = FindPid(name, pNetName);
	
	if( pid_t != 0 && *pid_t != 0)
	{
		ret = *pid_t;
		free(pid_t);
	}
	else
		return HI_SUCCESS;

	return ret;
}


int CNetServer::KillUdhcpc(char* pNetName)
{
	long pid = 0;
	char cmd[64] = {0};

	NET_WORK_CARD CardID;
	CardID = get_network_support();
	
	if(CardID == NET_WORK_CARD_LOCAL || CardID == NET_WORK_CARD_WIFI)
	{
		system("killall -9 udhcpc");
		#ifdef DM368
		Setdhcpandmac(0);
		return HI_SUCCESS;
		#endif
	}
	else if(CardID == NET_WORK_CARD_LOCAL_AND_WIFI)
	{
		if((pid = GetDhcpPid((char*)"udhcpc", (char*) pNetName)))
		{
			sprintf(cmd, "kill %d", pid);
			system(cmd);
		}
	}
	
	return HI_SUCCESS;
}

int CNetServer::RunDHCP(char* pNetName)
{ 
	NDB("RunDHCP\r\n");
	T_MSGBUF	Msg;
	T_CMDDHCP	Dhcp;
	Dhcp.Aciton = 1;
	Dhcp.MsgCmd = CMD_SETDHCP;
	strcpy(Dhcp.NetName,pNetName);	
	Msg.mtype = 1;
	memcpy(Msg.mtext,&Dhcp,sizeof(Dhcp));


	OSPQueueSendMessage(GetOSPQueueSendQuenceMsgID(),&Msg,IPC_NOWAIT);
	printf("------------------------- RunDHCP OSPQueueSendMessage\r\n");
	return 0;
	int i = 0;
	char szStr[256] = {0};
	char c_ip[64] = {0x0};
	char mac[64] = {0x0};
	char mask[64] = {0x0};

	KillUdhcpc(pNetName);
	#ifdef DM368
	Setdhcpandmac(1);
	return 0;
	#endif
	
    sprintf(szStr, "ifconfig %s 0.0.0.0", pNetName); //clean the ip first
    system(szStr);
 
	sprintf(szStr, "udhcpc -i %s -b -R -s /usr/share/udhcpc/default.script&",
			pNetName);
	system(szStr);	

	/*Modify by hayson 2014.1.15*/ 
	/*Remove the waiting for results*/
	#if 0
	memset(szStr, 0, sizeof(szStr));
	for (i = 0; i <= 15; ++i)
	{
		sleep(1);
		if (GetLocalNetInfo(pNetName, c_ip,mac,mask) >= 0)
		{
			return 0;
		}	
	}
	#endif
	
	return 0;
}

int CNetServer::DownDHCP(char* pNetName)
{
	T_MSGBUF	Msg;
	T_CMDDHCP	Dhcp;
	Dhcp.Aciton = 0;
	Dhcp.MsgCmd = CMD_SETDHCP;
	strcpy(Dhcp.NetName,pNetName);
	
	Msg.mtype = 1;
	memcpy(Msg.mtext,&Dhcp,sizeof(Dhcp));
	OSPQueueSendMessage(GetOSPQueueSendQuenceMsgID(),&Msg,IPC_NOWAIT);
	printf("------------------------- DownDHCP OSPQueueSendMessage\r\n");
	return 0;
	
	return KillUdhcpc((char*)pNetName);
}


 
void CNetServer::PrintZmdnetlibVer()
{
    printf("\n    \033[32m****************************************************************\033[0m\n");
#ifdef REVISION
	printf("    \033[1;32m*      IPC  zmdnetlib svn version: [%d] \033[0m\n", REVISION);
#endif
    printf("    \033[1;32m*      IPC  zmdnetlib build version: [%s]  \033[0m\n"
           "    \033[1;32m*      IPC  zmdnetlib build time: [%s] [%s]   \033[0m\n"
           "    \033[32m****************************************************************\033[0m\n\n\n",
           ZMDNETLIB_BUILD_VERSION, __TIME__, __DATE__);

    return;
}

void CNetServer::DelcurrentGW()
{
	char BufferTmp[CHAE_IP_LEN] = {0};
	int net_card = JudgeNetworkStatus();

	NET_WORK_CARD CardID;
	CardID = get_network_support();

	if(CardID == NET_WORK_CARD_LOCAL_AND_WIFI)
	{
		if(net_card == 1)
		{
			memset(BufferTmp,0, CHAE_IP_LEN);
			GetNetServerObj()->GetGw(BufferTmp,(char*)LOCAL_NET_NAME);
			delnet_gateway(NET_CARD_LOCAL, (char*)LOCAL_NET_NAME, (char*)WIFI_NET_NAME, BufferTmp);
		}
		else
		{
			memset(BufferTmp,0, CHAE_IP_LEN);
			GetNetServerObj()->GetGw(BufferTmp,(char*)WIFI_NET_NAME);
			delnet_gateway(NET_CARD_WIFI, (char*)LOCAL_NET_NAME, (char*)WIFI_NET_NAME, BufferTmp);
		}
	}
}

/**
 * @brief broadcast information
 */
 
void CNetServer::BroadcastDeviceInfo()
{
	struct sockaddr_in broadcastAddr;
	NETWORK_PARA netset;
	STRUCT_ID_PING_ECHO resp;
	
	memset(&netset, 0x0, sizeof(NETWORK_PARA));
	memset(&resp, 0x0, sizeof(STRUCT_ID_PING_ECHO));

	g_cParaManage->GetSysParameter(SYSNET_SET, &netset);
	
	if(FillBroadcastInfo(&resp.device, &netset) < 0)
		ERR("Get DevInfo error\r\n");
	
	resp.device.header.length = sizeof(STRUCT_ID_PING_ECHO) - sizeof(Cmd_Header);
	strncpy(resp.deviceID, (char *)netset.m_CenterNet.deviceid, 15);
	resp.deviceID[15] = 0 ;
		
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(18080);
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    sendto(m_pingSock , &resp , sizeof(STRUCT_ID_PING_ECHO) , 0 , (sockaddr *)&broadcastAddr , sizeof(struct sockaddr_in));
}


