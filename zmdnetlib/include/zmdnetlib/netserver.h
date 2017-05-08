#ifndef _JIANGHM_NET_TCP_SERVER_HEADER_34324897243342
#define _JIANGHM_NET_TCP_SERVER_HEADER_34324897243342

#include "tcplibdef.h"
#include "coreobj.h"
#include "interfacedef.h"

#define GetNetServerObj		CNetServer::getInstance

/*********************************************/

#define CHAE_IP_LEN			64
#define ZMDNETLIB_BUILD_VERSION "V1.6.1"     

/*********************************************/

//#define HAY_DEBUG

#ifdef HAY_DEBUG
	#define NDB(fmt, args...) fprintf(stderr, "\033[1;32m             NETLIB DEBUG(%s:%d):             \033[0m" fmt, __func__, __LINE__, ## args)

#else
	#define NDB(fmt, args...) 
#endif

#define ERR(fmt, args...) fprintf(stderr, "\033[1;31m             ZMDNETLIB ERROR (%s:%d):             \033[0m" fmt, __func__, __LINE__, ## args)





				


#ifdef _BARCODE_SUP_
#include "zsip_api.h"
#endif

class CNetUser ;

typedef struct
{
    int		userid ;
    int		mediaid ;

    int		used ;
} STRUCT_MEDIA_USER ;

typedef struct
{
    CNetUser		*userobj ;		//播放用户,此处从堆中分配对象
    int				userid ;		//播放用户id .
    int				mediaid ;		//记录用户在media队列中的索引号
    int				alarmid ;		//记录用户在alarm队列中的索引号

    int				used ;			//是否使用
} STRUCT_LOGIN_USER ;

typedef struct
{
    int userid ;
    unsigned int ip ;
    unsigned int port ;
    int devType ;
    int	used ;
} STRUCT_ALARM_USER ;

class CNetServer
{
protected:
    CNetServer() ;
    ~CNetServer() ;

    DECLARE_SINGLEOBJ( CNetServer )

public:
    //启动net server
    bool						StartServer( char *addr , unsigned short port ) ;

    //获取空闲的工作会话
    int							getUnuseWorkSession( ) ;
    //释放工作会话
    void						freeWorkSession( int userid ) ;

    bool						StartPingServer( ) ;
    int							JudgeNetworkStatus() ;

    void						CheckNetCardStatus( ) ;
	
	//验证用户名密码
	int							VerifyUser( char* user , char* pwd , int& permit ) ;
	int							VerifyUserByMd5( char* user , char* pwd , int& permit , char* userReal ) ;
	/*add by hayson 2014.1.17*/
	/*验证视频端口*/
	int 						VerifyVideoPort(int port);
	
	int							getUnuseAlarmSession( int userid , unsigned int ip , unsigned int port , int devType ) ;
	void						freeAlarmSession( int alarmid );
	/*获取指定网卡网关*/
	/*@gateway_addr  网关*/
	/*@name 指定网卡*/
	/*@ 返回:获取成功返回0 失败返回 < 0*/
	int							GetGw(char *gateway_addr, char *name);
	int 						DelGwAddr(char *gateway_addr, char *name);
	int 						SetGwAddr(char *gateway_addr, char *name);
	//设置网卡属性
	int 						GetIPaddr(char *net_ip, char *name);
	int 						SetIPaddr(char *net_ip, char *name);
	int 						GetMaskAddr(char *net_mask, char *name);
	int 						SetMaskAddr(char *mask_ip, char *name);
	int 						SetMacAddr(char *addr, char *name);
	
	int							SetNetAttrib( NETWORK_PARA *netset, int net_card_id);
	int 						RunDHCP(char* pNetName);/*打开DHCP*/
	int							DownDHCP(char*pNetName);/*关闭DHCP*/
	void						RegistDDNS( ) ;
	void						RegistUPNP( ) ;
	int  						InitNtp(); /*init ntp*/
	void 						PrintZmdnetlibVer();/*printf zmdnetlib version*/
	void 						DelcurrentGW(); /*删除多余网关 add by hayson*/
	void 						BroadcastDeviceInfo();/*广播设备信息 add by hayson*/
    bool						IsUpdating( )
    {
		return 	(bool)m_isUpdating ;
	}
	
    void						SetUpdating( bool bSet )
    {
		m_isUpdating = bSet ;
	};

	bool						IsTalkOn( ) ;
	bool						RequestTalkOn( unsigned userid );
	bool						RequestTalkOff( unsigned userid ) ;

    int							GetTcpUserCount( )
    {
		return m_nUserCount ;
	};
	bool                        IsUpnpSuccess(){return m_upnpSuccess;}

protected:
	// tcp工作会话管理
	STRUCT_LOGIN_USER			m_loginUserList[MAX_NET_TCP_USER] ;
	CMutex						m_loginUserListMutex ;
	
	//tcp媒体会话管理
	//STRUCT_MEDIA_USER			m_mediaUserList[MAX_MEDIA_PLAYER] ;
	//CMutex						m_mediaUserListMutex ;
	
	//报警会话
	STRUCT_ALARM_USER			m_alarmUserList[MAX_MEDIA_PLAYER] ;
	CMutex						m_alarmUserListMutex ;
	
	//调试用，记载客户端成功登陆次数和连接次数
	int							m_nLoginTimes ;
	int							m_nConnectTimes ;
	
	//记载已连接的tcp数量
	int							m_nUserCount ;
	
	//用来处理ping事件的sock句柄
	int							m_pingSock ;
	
	//是否正在升级文件
	int							m_isUpdating ;
	//是否已完成Upnp的设置
	int							m_isUPNPComplete ;
	//是否需要重新注册DDNS
	int							m_doDdns ;
	//DDns上次心跳时间
	unsigned int				m_ddnsTime ;
	
	//是否需要重新进行UPNP动作
	int							m_doUPNP ;
	//UPNP上次心跳时间
	unsigned int				m_upnpTime ;

	//当前使用网卡
	int							m_netCard ;
	
	//对讲标志状态
	bool			m_talkOn ;
	int				m_talkUserID ;
	CMutex			m_taklMutex ;
	int				m_serverStartTime ;
	
	//======================================
	//当前upnp注册信息，用来检查配置是否变化
	unsigned int				m_localHttpPort ;
	unsigned int				m_localVideoPort ;
	unsigned int				m_localMobilePort ;

	unsigned int				m_upnpHttpPort ;
	unsigned int				m_upnpVideoPort ;
	unsigned int				m_upnpMobilePort ;
	
			char 				m_ip[CHAE_IP_LEN];
	unsigned int				m_nNetCardID ;
	unsigned int				m_nUpnpSwitch ;
	bool                        m_upnpSuccess;

	//======================================
	//当前ddns注册信息，用来检查配置是否变化
	

	unsigned int				m_netcnt;
	int							m_QuenceMsgID;
protected:
	//=============================================
	//服务器线程句柄
	void*						m_serverHandle ;
	void*						m_serverThread ;
	void*						m_alarmThread ;
	void*						m_pingThread ;
	void*						m_emailThread ;
	void*						m_registThread ;
	void*						m_checkNetCardThread ;
	void*						m_ddnsThread ;
	void*						m_upnpThread ;
	void	 					*m_ntpThread;
protected:
    //===============================================
    //给网络层的回调函数
    static bool onServerTcpAccept( int &userData , void *session , sockaddr_in *client_addr ) ;
    static void onServerTcpClose( int userData ) ;
    static bool onServerRecvData( int userData , char *data , int len , int &used ) ;
    static bool onServerSessionIdle( int userData ) ;

    //==============================================
    //服务器辅助线程，用来处理一些不紧急的业务
    //例如: ping消息的回应,upnp,ddns等
    static void *checkNetCardThreadProc( void *pObj ) ;							

    //==============================================					
    //ddns线程
    static void *ddnsThreadProc( void *pObj ) ;

    //==============================================
    //upnp线程
    static void *upnpThreadProc( void *pObj ) ;

    //==============================================							
    //ping线程
    static void *pingThreadProc( void *pObj ) ;

    //==============================================
    //报警消息处理线程
    static void *alarmThreadProc( void *pObj ) ;

    //==============================================
    //mail发送线程
    static void *emailThreadProc( void *pObj ) ;

    //==============================================
    //设备注册线程
    static void *registThreadProc( void *pObj ) ;
	
	//==============================================
    //NTP 初始化线程    
	static void *ntpThreadProc( void *arg);/*int ntp proc*/
protected:

    //==============================================
    //事件处理函数
    void onClose( int userData ) ;
    bool onReceive( int userData , char *data , int len , int &used ) ;
    bool onIdle( int userData ) ;
    bool onAccept( int &userData , void *session , sockaddr_in *client_addr ) ;


    //==============================================
    //ping 消息的处理
    void handlePing( ) ;
    void handleAlarm( ) ;
    void handleUPNP( ) ;
    void handleDDNS( ) ;
    void handleEmail( ) ;
    void handleRegist( ) ;
#ifdef _BARCODE_SUP_
	static void regist_result_cb (reg_res_t *status) ;
#endif
	static int  m_staticSipRegState ;
	static bool	m_staticGetCallback ;
	static bool m_staticRegistNow ;

	int	FillBroadcastInfo(STRUCT_PING_ECHO* Broadcast, NETWORK_PARA* netset); /*add by hayson 2014.5.5*/
    void onPingCmd( int sock , char *data , int len , sockaddr_in remoteAddr ) ;
    void onIDPingCmd( int sock , char *data , int len , sockaddr_in remoteAddr ) ;
    void onUdpReboot( int sock , char *data , int len , sockaddr_in remoteAddr ) ;					

	int	hi_netcfg_update_dns(char *cfgFile, unsigned char* dnsaddr, unsigned char* dnsaddr2);
	
	int GetLocalNetInfo(  const char* lpszEth, char* szIpAddr, char* szNetmask, char* szMacAddr ) ;

	/*add by hayson begin 20131028 */
	/*dhcp 处理*/
	int KillUdhcpc(char* pNetName);
	long GetDhcpPid(char *name, char* pNetName);
	long* FindPid(const char* pidName, char* pNetName);
	int CheckPid(char* pid, char* pNetName);
	int CookBuf(FILE * fp, char* pNetName);
	char* safe_strncpy(char *dst, const char *src, size_t size);
};


#endif



