#include "zmdnetlib.h"
#include "netserver.h"
#include "mobileserver.h"
#include "httpserver.h"


#ifdef SUPPORTONVIF
#include "nvtApp.h"
#include "nvtLib.h"
#include "rtspLib.h"
#endif

#ifdef SUPPORT_WIFI
#include "wificonfig.h"
#endif



//=================================================
//这是以前net工程定义的一些全局标志

int change_flag1 = 0x00;
int change_flag2 = 0x01;
extern DeviceConfigInfo 	ConfigInfo;

//==============================
//升级状态，转用CNetModule::IsUpdating()接口
int startupdate = 1 ;


CNetModule* CNetModule::m_instance = 0 ;


CNetModule::CNetModule()
{

}

CNetModule::~CNetModule()
{
	
}

//================================================
//启动netserver ;
bool CNetModule::StartNetServer( )
{
	NETWORK_PARA netset;
	
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
	if(netset.m_CenterNet.m_uVideoListenPt <= 0)
		netset.m_CenterNet.m_uVideoListenPt = TCPLISTENPORT;
	
	if( !GetNetServerObj()->StartServer( (char*)"0.0.0.0" , netset.m_CenterNet.m_uVideoListenPt ) )
		return false ;
	#if 0
	if(GetNetServerObj()->InitNtp() < 0)
	{
		ERR("fail to start ntp  \r\n");			
        return false ;
	}
	#endif
	return true ;
}

//================================================
//启动mobileserver
bool CNetModule::StartMobileServer()
{
	NETWORK_PARA netset;
	
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
	if(netset.m_CenterNet.m_uPhoneListenPt <= 0)
		netset.m_CenterNet.m_uPhoneListenPt = MOBILETCPLISTENPORT;
	
	//printf( "Mobile Server Start at %d \r\n" , netset.m_CenterNet.m_uPhoneListenPt ) ;
	
	if( !GetMobileServerObj()->StartServer( (char*)"0.0.0.0" , netset.m_CenterNet.m_uPhoneListenPt ) )
		return false ;
	
	
	NDB( "Start Mobile Server Successfully!\r\n" ) ;
	return true ;
}

//================================================
//启动网络服务器.
bool CNetModule::StartNetModule( )
{
	StartNetServer() ;
	
	//StartMobileServer() ;
	
	return true ;
}

//============================================
//启动http服务器
bool CNetModule::StartHttpServer( char *ip , unsigned short port )
{
	NETWORK_PARA netset;
	
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
	if(netset.m_CenterNet.m_uHttpListenPt <= 0)
		netset.m_CenterNet.m_uHttpListenPt = SERVER_PORT;
	
	
	NDB( "http Server Start at %d \r\n" , netset.m_CenterNet.m_uHttpListenPt ) ;
	
	if( !GetHttpServer()->StartServer( (char*)"0.0.0.0" , netset.m_CenterNet.m_uHttpListenPt )  )
		return false ;

	return true ;
}


//=======================================================
//启动网络设备
bool CNetModule::StartNetDevice( )
{
	GetNetServerObj()->PrintZmdnetlibVer();
	NETWORK_PARA netset;
	memset(&netset, 0x0, sizeof(NETWORK_PARA));
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);


	NET_WORK_CARD CardID;
	CardID = get_network_support();

	if(CardID == NET_WORK_CARD_LOCAL || CardID == NET_WORK_CARD_LOCAL_AND_WIFI)
	{
		SetLoaclNetMac();
		sleep(1);
		if(netset.m_Eth0Config.m_dhcp == 0)
		{
			GetNetServerObj()->SetNetAttrib(&netset,0);
		}
	}
	if(CardID == NET_WORK_CARD_WIFI || CardID == NET_WORK_CARD_LOCAL_AND_WIFI)
	{
		if(ConfigInfo.SupportInfo&(CONFIG_WIFI))
		{
			if(get_network_support()!=NET_WORK_CARD_LOCAL)
				InitWifi();	
			
		}
	}
	
	if(CardID == NET_WORK_CARD_LOCAL)
	{
	 	if(netset.m_Eth0Config.m_dhcp == 1)
		{
			if(GetNetServerObj()->RunDHCP((char*)LOCAL_NET_NAME) < 0)
				NDB("######### faild to set dhcp  ########\r\n");
			else
				NDB("######### set dhcp success  ########\r\n");					
		}
	}

	Do_ddns();
	Do_upnp();
	
	return true ;
}

///================================================
///启动onvif服务器
bool CNetModule::StartOnvifServer( )
{
#ifdef SUPPORTONVIF
    zmd_rtsp_initLib(rtsp_start_strm, rtsp_stop_strm, rtsp_get_strm);
    zmd_rtsp_startService();
    zmd_nvt_init_nvt_lib(nvt_ctrl_dev, nvt_set_param, nvt_get_param);
#endif
	return true ;
}

int CNetModule::JudgeNetworkStatus( )
{
	return GetNetServerObj()->JudgeNetworkStatus( ) ;
}

void CNetModule::Do_ddns( )
{
	GetNetServerObj()->RegistDDNS( ) ;
}

void CNetModule::Do_upnp( )
{
	GetNetServerObj()->RegistUPNP() ;
}

int CNetModule::SetNetAttrib( NETWORK_PARA *netset,int net_card_id )
{
	return GetNetServerObj()->SetNetAttrib( netset , net_card_id ) ;
}


bool CNetModule::IsUpdating( )
{
	return GetNetServerObj()->IsUpdating( )  ;
}

int CNetModule::SetLoaclNetMac( )
{
	char c_ip[CHAE_IP_LEN];
	memset(c_ip, 0x0, CHAE_IP_LEN);

	NETWORK_PARA netset;
	memset(&netset, 0x0, sizeof(NETWORK_PARA));
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);
	
	sprintf(c_ip, "%02x:%02x:%02x:%02x:%02x:%02x",netset.m_Eth0Config.m_uMac[0],
													netset.m_Eth0Config.m_uMac[1],
													netset.m_Eth0Config.m_uMac[2],
													netset.m_Eth0Config.m_uMac[3],
													netset.m_Eth0Config.m_uMac[4],
													netset.m_Eth0Config.m_uMac[5]);
		
	return GetNetServerObj()->SetMacAddr(c_ip, (char*)LOCAL_NET_NAME) ;
}

int CNetModule::SetDHCP(char* pNetName )
{
    return GetNetServerObj()->RunDHCP(pNetName) ;
}

int CNetModule::DownDHCP(char* pNetName )
{
    return GetNetServerObj()->DownDHCP(pNetName) ;
}


int CNetModule::GetDevIpaddr(char* pNetName, char *net_ip)
{
    return GetNetServerObj()->GetIPaddr(net_ip, pNetName) ;
}

int CNetModule::SetDevIpaddr(char* pNetName, char *net_ip)
{
	return GetNetServerObj()->SetIPaddr(net_ip, pNetName) ;
}

void CNetModule::BroadcastDeviceInfo()
{
	GetNetServerObj()->BroadcastDeviceInfo();
}




