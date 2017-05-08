#ifndef _JIANGHM_ZMD_NET_LIB_HEADER_324987432987432
#define _JIANGHM_ZMD_NET_LIB_HEADER_324987432987432

#include "parametermanage.h"
#include "systemparameterdefine.h"
#include "interfacedef.h"
#include "ModuleFuncInterface.h"

extern PARAMETER_MANAGE		*g_cParaManage ;				

#define GetNetModule	CNetModule::getInstance

#define SERVER_PORT				80		//http服务器端口
#define	TCPLISTENPORT			8000	// 普通(8000) 联通(3389 1433)
#define	MOBILETCPLISTENPORT		9000	// 普通(9000) 联通(3389 1433)
#define	UDPLISTENPORT			8080	//未使用

#define	PACKSIZE				88
#define	TALKBUFSIZE				100*PACKSIZE


/*get network card name*/
#define	LOCAL_NET_NAME  get_local_name()
#define	WIFI_NET_NAME get_wifi_name()

/*get config file*/
#define MAC_ID_FILE get_mac_id_file()


//申明为单件
class CNetModule
{
    //保护构造函数，防止外部实例化
protected:
    CNetModule() ;
    ~CNetModule() ;
protected:
    static CNetModule *m_instance ;	//单件句柄
public:
    //单件接口
    static CNetModule *getInstance()
    {
        if( 0 == m_instance )
        {
            m_instance = new CNetModule() ;
        }
        return m_instance ;
    };
    static void release()
    {
        if( m_instance)
        {
            delete m_instance ;
            m_instance = 0 ;
        }
    };

public:

    //开放的功能接口
    bool StartNetDevice( ) ;

    //=================================
    //判定网络状态
    int JudgeNetworkStatus( ) ;

    //==================================
    //刷新ddns状态
    void Do_ddns( ) ;

    //==================================
    //刷新upnp状态
    void Do_upnp( ) ;

    //==================================
    //设置网卡属性.
    int	 SetNetAttrib( NETWORK_PARA *netset, int net_card_id ) ;
    int  SetLoaclNetMac( ); /*设置有线MAC*/
	int  SetDHCP(char* pNetName );/*打开DHCP*/
	int  DownDHCP(char* pNetName );/*关闭DHCP*/
	
	//网络操作接口
	//return 0: success
	int GetDevIpaddr(char* pNetName, char *net_ip);
	int SetDevIpaddr(char* pNetName, char *net_ip);

    //==================================
    //启动网络模块
    bool StartNetModule( ) ;

    //=================================================
    //启动http服务器
    bool StartHttpServer( char *ip , unsigned short port ) ;

    //=======================================
    //查询是否正在升级状态
    bool IsUpdating( ) ;

    ///================================================
    ///启动onvif服务器
    bool StartOnvifServer( ) ;

/**
* @brief 广播设备信息
*/
    void BroadcastDeviceInfo();
    
protected:
    //启动服务器
    bool StartNetServer( ) ;
    bool StartMobileServer() ;
};


#endif
