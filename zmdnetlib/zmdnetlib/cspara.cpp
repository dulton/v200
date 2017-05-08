#include "ModuleFuncInterface.h"
#include "cspara.h"
#include "EncodeManage.h"
#include "pppoe.h"
#include "zmdnetlib.h"
#include "netserver.h"
#ifdef SUPPORT_WIFI
#include "wificonfig.h"
#endif

extern PARAMETER_MANAGE*  g_cParaManage;
extern unsigned int	g_ntpnow;
extern int  SendOneHeartBeat();
extern int change_flag2;
extern    int change_flag1;

#ifdef DM368
extern int DatePlayStyle ; //
extern int TimePlayStyle ; // 0: 12小时， 1：24小时
extern "C" 
{									
    int AVSERVER_setMotionPre(int MotioneEnable, int MotioneCEnable, int MotioneCValue, int MotioneLevel, unsigned char *blockmask);
    int VIDEO_streamOSDEnable(int streamId, int enable);		
    void VIDEO_streamOsdPrm(OSDPrm *osdPrm, int id);						
}

extern void OsdDateTimeFormatSet(COMMON_PARA  CommPara_t);


//设置通道摄像头参数
extern "C" int SnAewb_UiSetBrtness(int rsiBrtness);
extern "C" int SnAewb_UiSetContrast(int rsiContrast);
extern "C" int SnAewb_UiSetSatDgr(int rsiSatDgr);
//===================================================

#endif


int csSetNetWorkPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;
    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_NetWork.m_changeinfo))
    {
        g_cParaManage->GetSysParameter(SYSNET_SET, &plocalPara->m_NetWork);
        int i, needUpdate = 0;

        int lochCenterNet = 0; // 位0 中央服务器设置
        if(pcsPara->m_NetWork.m_changeinfo & 0x0001)
        {
            for(i = 0; i < 4; i++)
            {
                if(plocalPara->m_NetWork.m_CenterNet.m_uCenterIP[i] != pcsPara->m_NetWork.m_CenterNet.m_uCenterIP[i])
                {
                    plocalPara->m_NetWork.m_CenterNet.m_uCenterIP[i] = pcsPara->m_NetWork.m_CenterNet.m_uCenterIP[i];
                    needUpdate++;
                    lochCenterNet |= 0x01; // 平台IP -- 移到 m_uAccDomain
                }
            }
            if(plocalPara->m_NetWork.m_CenterNet.m_uPhoneListenPt != pcsPara->m_NetWork.m_CenterNet.m_uPhoneListenPt)
            {
                plocalPara->m_NetWork.m_CenterNet.m_uPhoneListenPt = pcsPara->m_NetWork.m_CenterNet.m_uPhoneListenPt;
                needUpdate++;
                lochCenterNet |= 0x02;	// 手机监听端口 默认 9000
                needreboot = 1;
            }
            if(plocalPara->m_NetWork.m_CenterNet.m_uVideoListenPt != pcsPara->m_NetWork.m_CenterNet.m_uVideoListenPt)
            {
                plocalPara->m_NetWork.m_CenterNet.m_uVideoListenPt = pcsPara->m_NetWork.m_CenterNet.m_uVideoListenPt;
                needUpdate++;
                lochCenterNet |= 0x04;	// 视频监听端口 目前默认 8000
                needreboot = 1;
            }
            if(plocalPara->m_NetWork.m_CenterNet.m_uHttpListenPt != pcsPara->m_NetWork.m_CenterNet.m_uHttpListenPt)
            {
                plocalPara->m_NetWork.m_CenterNet.m_uHttpListenPt = pcsPara->m_NetWork.m_CenterNet.m_uHttpListenPt;
                needUpdate++;
                lochCenterNet |= 0x08;  // http 监听端口 目前默认80
                needreboot = 1;
            }
            if(plocalPara->m_NetWork.m_CenterNet.m_uEnrolPort != pcsPara->m_NetWork.m_CenterNet.m_uEnrolPort)
            {
                plocalPara->m_NetWork.m_CenterNet.m_uEnrolPort = pcsPara->m_NetWork.m_CenterNet.m_uEnrolPort;
                needUpdate++;
                lochCenterNet |= 0x10;	// 平台注册端口 7979 固定
            }
            if(plocalPara->m_NetWork.m_CenterNet.m_Switch != pcsPara->m_NetWork.m_CenterNet.m_Switch)
            {
                plocalPara->m_NetWork.m_CenterNet.m_Switch = pcsPara->m_NetWork.m_CenterNet.m_Switch;
                needUpdate++;
                lochCenterNet |= 0x20; // 平台连接开关
            }
            for(i = 0; i < 15; i++)
            {
                if(plocalPara->m_NetWork.m_CenterNet.deviceid[i] != pcsPara->m_NetWork.m_CenterNet.deviceid[i])
                {
                    plocalPara->m_NetWork.m_CenterNet.deviceid[i] = pcsPara->m_NetWork.m_CenterNet.deviceid[i];
                    needUpdate++;
                    lochCenterNet |= 0x40; // 平台设备注册ID
                }
            }
            for(i = 0; i < 16; i++)
            {
                if( plocalPara->m_NetWork.m_CenterNet.passwd[i] != pcsPara->m_NetWork.m_CenterNet.passwd[i])
                {
                    plocalPara->m_NetWork.m_CenterNet.passwd[i] = pcsPara->m_NetWork.m_CenterNet.passwd[i];
                    needUpdate++;
                    lochCenterNet |= 0x80;
                }
            }
            if( plocalPara->m_NetWork.m_CenterNet.m_heartbeat != pcsPara->m_NetWork.m_CenterNet.m_heartbeat )
            {
                plocalPara->m_NetWork.m_CenterNet.m_heartbeat = pcsPara->m_NetWork.m_CenterNet.m_heartbeat;
                needUpdate++;
                lochCenterNet |= 0x100;
            }

        }

        int lochDns = 0; // 位1 DNS服务器设置
        if(pcsPara->m_NetWork.m_changeinfo & 0x0002)
        {
            for(i = 0; i < 4; i++)
            {
                if(plocalPara->m_NetWork.m_DNS.m_umDNSIp[i] != pcsPara->m_NetWork.m_DNS.m_umDNSIp[i])
                {
                    plocalPara->m_NetWork.m_DNS.m_umDNSIp[i] = pcsPara->m_NetWork.m_DNS.m_umDNSIp[i];
                    needUpdate++;
                    lochDns |= 0x01;	//  DNS:默认 202.96.134.133
                }
            }
            for(i = 0; i < 4; i++)
            {
                if(plocalPara->m_NetWork.m_DNS.m_usDNSIp[i] != pcsPara->m_NetWork.m_DNS.m_usDNSIp[i])
                {
                    plocalPara->m_NetWork.m_DNS.m_usDNSIp[i] = pcsPara->m_NetWork.m_DNS.m_usDNSIp[i];
                    needUpdate++;
                    lochDns |= 0x02;	//  DNS:默认 202.96.128.166
                }
            }
        }
		int seteth0=0;		
		char c_ip[128] = {0};
		char netstr[128] = {0};
		if(pcsPara->m_NetWork.m_changeinfo & 0x0004)
		{
			change_flag2 = 0x01;
			if(plocalPara->m_NetWork.m_Eth0Config.m_dhcp != pcsPara->m_NetWork.m_Eth0Config.m_dhcp)
			{
				if(pcsPara->m_NetWork.m_Eth0Config.m_dhcp==0)
				{
					seteth0 ++;
				}
				
			}
			for(i=0; i<4; i++)
			{
				if(plocalPara->m_NetWork.m_Eth0Config.m_uLocalIp[i] != pcsPara->m_NetWork.m_Eth0Config.m_uLocalIp[i])
				{
					plocalPara->m_NetWork.m_Eth0Config.m_uLocalIp[i] = pcsPara->m_NetWork.m_Eth0Config.m_uLocalIp[i];
					needUpdate++;
					seteth0++;
				}
			}
			for(i=0; i<4; i++)
			{
				if(plocalPara->m_NetWork.m_Eth0Config.m_uMask[i] != pcsPara->m_NetWork.m_Eth0Config.m_uMask[i])
				{
					plocalPara->m_NetWork.m_Eth0Config.m_uMask[i] = pcsPara->m_NetWork.m_Eth0Config.m_uMask[i];
					needUpdate++;
					seteth0++;
				}
			}
			for(i=0; i<4; i++)
			{
				if(plocalPara->m_NetWork.m_Eth0Config.m_uGateWay[i] != pcsPara->m_NetWork.m_Eth0Config.m_uGateWay[i])
				{
					plocalPara->m_NetWork.m_Eth0Config.m_uGateWay[i] = pcsPara->m_NetWork.m_Eth0Config.m_uGateWay[i];
					needUpdate++;
					seteth0++;
				}
			}
			for(i=0; i<6; i++)
			{
				if(plocalPara->m_NetWork.m_Eth0Config.m_uMac[i] != pcsPara->m_NetWork.m_Eth0Config.m_uMac[i])
				{
					plocalPara->m_NetWork.m_Eth0Config.m_uMac[i] = pcsPara->m_NetWork.m_Eth0Config.m_uMac[i];
					needUpdate++;
					seteth0++;
				}
			}

			if(plocalPara->m_NetWork.m_Eth0Config.m_dhcp != pcsPara->m_NetWork.m_Eth0Config.m_dhcp)
			{
				plocalPara->m_NetWork.m_Eth0Config.m_dhcp = pcsPara->m_NetWork.m_Eth0Config.m_dhcp;
				needUpdate++;
				if(pcsPara->m_NetWork.m_Eth0Config.m_dhcp==1)
				{
					if(GetNetModule()->SetDHCP((char*)LOCAL_NET_NAME) < 0)
						NDB("######### faild to set dhcp   ########\r\n");
					else
						NDB("######### set dhcp success   ########\r\n");
				}
				
			}
			if(plocalPara->m_NetWork.m_Eth0Config.m_upnp != pcsPara->m_NetWork.m_Eth0Config.m_upnp)
			{
				plocalPara->m_NetWork.m_Eth0Config.m_upnp = pcsPara->m_NetWork.m_Eth0Config.m_upnp;
				needUpdate++;
			}
			if(plocalPara->m_NetWork.m_Eth0Config.m_v_port != pcsPara->m_NetWork.m_Eth0Config.m_v_port)
			{
				plocalPara->m_NetWork.m_Eth0Config.m_v_port = pcsPara->m_NetWork.m_Eth0Config.m_v_port;
				needUpdate++;
				GetNetModule()->Do_upnp( ) ;//g_upnpnow = 1;
			}
			if(plocalPara->m_NetWork.m_Eth0Config.m_http_port != pcsPara->m_NetWork.m_Eth0Config.m_http_port)
			{
				plocalPara->m_NetWork.m_Eth0Config.m_http_port = pcsPara->m_NetWork.m_Eth0Config.m_http_port;
				needUpdate++;
				GetNetModule()->Do_upnp( ) ;//g_upnpnow = 1;
			}
			if(plocalPara->m_NetWork.m_Eth0Config.m_plat_port != pcsPara->m_NetWork.m_Eth0Config.m_plat_port)
			{
				plocalPara->m_NetWork.m_Eth0Config.m_plat_port = pcsPara->m_NetWork.m_Eth0Config.m_plat_port;
				needUpdate++;
				GetNetModule()->Do_upnp( ) ;//g_upnpnow = 1;
			}
			if( plocalPara->m_NetWork.m_Eth0Config.m_phone_port != pcsPara->m_NetWork.m_Eth0Config.m_phone_port)
			{
				plocalPara->m_NetWork.m_Eth0Config.m_phone_port = pcsPara->m_NetWork.m_Eth0Config.m_phone_port;
				needUpdate++;
				GetNetModule()->Do_upnp( ) ;//g_upnpnow = 1;
			}

		}
		if(seteth0>0)
		{
			GetNetModule()->SetNetAttrib(&pcsPara->m_NetWork,0);
		}
		
        int lochEth1Config = 0; // 位3 本机第二个网口的设置: 实际未使用
        if(pcsPara->m_NetWork.m_changeinfo & 0x0008)
        {
            for(i = 0; i < 4; i++)
            {
                if(plocalPara->m_NetWork.m_Eth1Config.m_uLocalIp[i] != pcsPara->m_NetWork.m_Eth1Config.m_uLocalIp[i])
                {
                    plocalPara->m_NetWork.m_Eth1Config.m_uLocalIp[i] = pcsPara->m_NetWork.m_Eth1Config.m_uLocalIp[i];
                    needUpdate++;
                    lochEth1Config |= 0x01;
                }
            }
            for(i = 0; i < 4; i++)
            {
                if(plocalPara->m_NetWork.m_Eth1Config.m_uMask[i] != pcsPara->m_NetWork.m_Eth1Config.m_uMask[i])
                {
                    plocalPara->m_NetWork.m_Eth1Config.m_uMask[i] = pcsPara->m_NetWork.m_Eth1Config.m_uMask[i];
                    needUpdate++;
                    lochEth1Config |= 0x02;
                }
            }
            for(i = 0; i < 4; i++)
            {
                if(plocalPara->m_NetWork.m_Eth1Config.m_uGateWay[i] != pcsPara->m_NetWork.m_Eth1Config.m_uGateWay[i])
                {
                    plocalPara->m_NetWork.m_Eth1Config.m_uGateWay[i] = pcsPara->m_NetWork.m_Eth1Config.m_uGateWay[i];
                    needUpdate++;
                    lochEth1Config |= 0x04;
                }
            }
            for(i = 0; i < 6; i++)
            {
                if(plocalPara->m_NetWork.m_Eth1Config.m_uMac[i] != pcsPara->m_NetWork.m_Eth1Config.m_uMac[i])
                {
                    plocalPara->m_NetWork.m_Eth1Config.m_uMac[i] = pcsPara->m_NetWork.m_Eth1Config.m_uMac[i];
                    needUpdate++;
                    lochEth1Config |= 0x08;
                }
            }
            if(plocalPara->m_NetWork.m_Eth1Config.m_dhcp != pcsPara->m_NetWork.m_Eth1Config.m_dhcp)
            {
                plocalPara->m_NetWork.m_Eth1Config.m_dhcp = pcsPara->m_NetWork.m_Eth1Config.m_dhcp;
                needUpdate++;
                lochEth1Config |= 0x10;
            }
            if(plocalPara->m_NetWork.m_Eth1Config.m_upnp != pcsPara->m_NetWork.m_Eth1Config.m_upnp)
            {
                plocalPara->m_NetWork.m_Eth1Config.m_upnp = pcsPara->m_NetWork.m_Eth1Config.m_upnp;
                needUpdate++;
                lochEth1Config |= 0x20;
            }
            if(plocalPara->m_NetWork.m_Eth1Config.m_v_port != pcsPara->m_NetWork.m_Eth1Config.m_v_port)
            {
                plocalPara->m_NetWork.m_Eth1Config.m_v_port = pcsPara->m_NetWork.m_Eth1Config.m_v_port;
                needUpdate++;
                lochEth1Config |= 0x40;
            }
            if(plocalPara->m_NetWork.m_Eth1Config.m_http_port != pcsPara->m_NetWork.m_Eth1Config.m_http_port)
            {
                plocalPara->m_NetWork.m_Eth1Config.m_http_port = pcsPara->m_NetWork.m_Eth1Config.m_http_port;
                needUpdate++;
                lochEth1Config |= 0x80;
            }
            if(plocalPara->m_NetWork.m_Eth1Config.m_plat_port != pcsPara->m_NetWork.m_Eth1Config.m_plat_port)
            {
                plocalPara->m_NetWork.m_Eth1Config.m_plat_port = pcsPara->m_NetWork.m_Eth1Config.m_plat_port;
                needUpdate++;
                lochEth1Config |= 0x0100;
            }

        }

        int lochPppoeSet = 0; // 位4 PPPoE设置--保存后处理
        if(pcsPara->m_NetWork.m_changeinfo & 0x0010)
        {
            for(i = 0; i < 32; i++)
            {
                if(plocalPara->m_NetWork.m_PppoeSet.m_s8UserName[i] != pcsPara->m_NetWork.m_PppoeSet.m_s8UserName[i])
                {
                    plocalPara->m_NetWork.m_PppoeSet.m_s8UserName[i] = pcsPara->m_NetWork.m_PppoeSet.m_s8UserName[i];
                    needUpdate++;
                    lochPppoeSet |= 0x01; // PPPoE用户名
                }
            }
            for(i = 0; i < 16; i++)
            {
                if(plocalPara->m_NetWork.m_PppoeSet.m_s32Passwd[i] != pcsPara->m_NetWork.m_PppoeSet.m_s32Passwd[i])
                {
                    plocalPara->m_NetWork.m_PppoeSet.m_s32Passwd[i] = pcsPara->m_NetWork.m_PppoeSet.m_s32Passwd[i];
                    needUpdate++;
                    lochPppoeSet |= 0x02;  // PPPoE密码
                }
            }
            if(plocalPara->m_NetWork.m_PppoeSet.m_u8PppoeSelected != pcsPara->m_NetWork.m_PppoeSet.m_u8PppoeSelected)
            {
                plocalPara->m_NetWork.m_PppoeSet.m_u8PppoeSelected = pcsPara->m_NetWork.m_PppoeSet.m_u8PppoeSelected;
                needUpdate++;
                lochPppoeSet |= 0x04;  // PPPoE连接开关
            }

        }

        int lochNatConfig = 0;	// 位5 网桥设置: --直接保存
        if(pcsPara->m_NetWork.m_changeinfo & 0x0020)
        {
            for(i = 0; i < 32; i++)
            {
                if(plocalPara->m_NetWork.m_NatConfig.m_u8NatIpAddr[i] != pcsPara->m_NetWork.m_NatConfig.m_u8NatIpAddr[i])
                {

                    plocalPara->m_NetWork.m_NatConfig.m_u8NatIpAddr[i] = pcsPara->m_NetWork.m_NatConfig.m_u8NatIpAddr[i];
                    needUpdate++;
                    lochNatConfig |= 0x01;
                }
            }
            if(plocalPara->m_NetWork.m_NatConfig.m_u8NatIpValid != pcsPara->m_NetWork.m_NatConfig.m_u8NatIpValid)
            {

                plocalPara->m_NetWork.m_NatConfig.m_u8NatIpValid = pcsPara->m_NetWork.m_NatConfig.m_u8NatIpValid;
                needUpdate++;
                lochNatConfig |= 0x02;
            }
        }

        int lochDomainConfig = 0;	// 位6 域名设置
        if(pcsPara->m_NetWork.m_changeinfo & 0x0040)
        {
            for(i = 0; i < 32; i++)
            {
                if(plocalPara->m_NetWork.m_DomainConfig.m_s8Name[i] != pcsPara->m_NetWork.m_DomainConfig.m_s8Name[i])
                {
                    plocalPara->m_NetWork.m_DomainConfig.m_s8Name[i] = pcsPara->m_NetWork.m_DomainConfig.m_s8Name[i];
                    needUpdate++;
                    lochDomainConfig |= 0x01;
                }
            }
            for(i = 0; i < 16; i++)
            {
                if(plocalPara->m_NetWork.m_DomainConfig.m_s8UserName[i] != pcsPara->m_NetWork.m_DomainConfig.m_s8UserName[i])
                {
                    plocalPara->m_NetWork.m_DomainConfig.m_s8UserName[i] = pcsPara->m_NetWork.m_DomainConfig.m_s8UserName[i];
                    needUpdate++;
                    lochDomainConfig |= 0x02;
                }
            }
            for(i = 0; i < 16; i++)
            {
                if(plocalPara->m_NetWork.m_DomainConfig.m_s32Passwd[i] != pcsPara->m_NetWork.m_DomainConfig.m_s32Passwd[i])
                {
                    plocalPara->m_NetWork.m_DomainConfig.m_s32Passwd[i] = pcsPara->m_NetWork.m_DomainConfig.m_s32Passwd[i];
                    needUpdate++;
                    lochDomainConfig |= 0x04;
                }
            }
            if(plocalPara->m_NetWork.m_DomainConfig.m_u8Selected != pcsPara->m_NetWork.m_DomainConfig.m_u8Selected)
            {
                plocalPara->m_NetWork.m_DomainConfig.m_u8Selected = pcsPara->m_NetWork.m_DomainConfig.m_u8Selected;
                needUpdate++;
                lochDomainConfig |= 0x08;
            }
            if(plocalPara->m_NetWork.m_DomainConfig.m_server != pcsPara->m_NetWork.m_DomainConfig.m_server)
            {
                // 1:表示3322.org, 2:表示dynDDNS.org
                plocalPara->m_NetWork.m_DomainConfig.m_server = pcsPara->m_NetWork.m_DomainConfig.m_server;
                needUpdate++;
                lochDomainConfig |= 0x10;
            }
            lochDomainConfig |= 0x20;
        }

        int lochftp = 0; // 位7 ftp设置--直接保存
        if(pcsPara->m_NetWork.m_changeinfo & 0x0080)
        {
            for(i = 0; i < 32; i++)
            {
                if(plocalPara->m_NetWork.m_ftp.m_server[i] != pcsPara->m_NetWork.m_ftp.m_server[i])
                {
                    plocalPara->m_NetWork.m_ftp.m_server[i] = pcsPara->m_NetWork.m_ftp.m_server[i];
                    needUpdate++;
                    lochftp |= 0x01;
                }
            }
            for(i = 0; i < 32; i++)
            {
                if(plocalPara->m_NetWork.m_ftp.m_account[i] != pcsPara->m_NetWork.m_ftp.m_account[i])
                {
                    plocalPara->m_NetWork.m_ftp.m_account[i] = pcsPara->m_NetWork.m_ftp.m_account[i];
                    needUpdate++;
                    lochftp |= 0x02;
                }
            }
            for(i = 0; i < 16; i++)
            {
                if(plocalPara->m_NetWork.m_ftp.m_password[i] != pcsPara->m_NetWork.m_ftp.m_password[i])
                {
                    plocalPara->m_NetWork.m_ftp.m_password[i] = pcsPara->m_NetWork.m_ftp.m_password[i];
                    needUpdate++;
                    lochftp |= 0x04;
                }
            }
            if(plocalPara->m_NetWork.m_ftp.m_port != pcsPara->m_NetWork.m_ftp.m_port)
            {
                plocalPara->m_NetWork.m_ftp.m_port = pcsPara->m_NetWork.m_ftp.m_port;
                needUpdate++;
                lochftp |= 0x08;
            }
        }

        int lochemail = 0;	// 位8 email设置--直接保存
        if(pcsPara->m_NetWork.m_changeinfo & 0x0100)
        {
            for(i = 0; i < 64; i++)
            {
                if( plocalPara->m_NetWork.m_email.m_title[i] != pcsPara->m_NetWork.m_email.m_title[i])
                {
                    plocalPara->m_NetWork.m_email.m_title[i] = pcsPara->m_NetWork.m_email.m_title[i];
                    needUpdate++;
                    lochemail |= 0x01;
                }
            }
            for(i = 0; i < 32; i++)
            {
                if( plocalPara->m_NetWork.m_email.m_server[i] != pcsPara->m_NetWork.m_email.m_server[i])
                {
                    plocalPara->m_NetWork.m_email.m_server[i] = pcsPara->m_NetWork.m_email.m_server[i];
                    needUpdate++;
                    lochemail |= 0x01;
                }
            }
            for(i = 0; i < 32; i++)
            {
                if( plocalPara->m_NetWork.m_email.m_addr[i] != pcsPara->m_NetWork.m_email.m_addr[i])
                {
                    plocalPara->m_NetWork.m_email.m_addr[i] = pcsPara->m_NetWork.m_email.m_addr[i];
                    needUpdate++;
                    lochemail |= 0x02;
                }
            }
            for(i = 0; i < 32; i++)
            {
                if( plocalPara->m_NetWork.m_email.m_account[i] != pcsPara->m_NetWork.m_email.m_account[i])
                {
                    plocalPara->m_NetWork.m_email.m_account[i] = pcsPara->m_NetWork.m_email.m_account[i];
                    needUpdate++;
                    lochemail |= 0x04;
                }
            }
            for(i = 0; i < 16; i++)
            {
                if( plocalPara->m_NetWork.m_email.m_password[i] != pcsPara->m_NetWork.m_email.m_password[i])
                {
                    plocalPara->m_NetWork.m_email.m_password[i] = pcsPara->m_NetWork.m_email.m_password[i];
                    needUpdate++;
                    lochemail |= 0x08;
                }
            }
            if( plocalPara->m_NetWork.m_email.m_mode != pcsPara->m_NetWork.m_email.m_mode)
            {
                plocalPara->m_NetWork.m_email.m_mode = pcsPara->m_NetWork.m_email.m_mode;
                needUpdate++;
                lochemail |= 0x10;
            }
            if( plocalPara->m_NetWork.m_email.m_u8Sslswitch != pcsPara->m_NetWork.m_email.m_u8Sslswitch)
            {
                plocalPara->m_NetWork.m_email.m_u8Sslswitch = pcsPara->m_NetWork.m_email.m_u8Sslswitch;
                needUpdate++;
                lochemail |= 0x20;
            }
            if( plocalPara->m_NetWork.m_email.m_u16SslPort != pcsPara->m_NetWork.m_email.m_u16SslPort)
            {
                if(pcsPara->m_NetWork.m_email.m_u16SslPort < 1) /**/
                {
                    pcsPara->m_NetWork.m_email.m_u16SslPort = 25;
                }
                plocalPara->m_NetWork.m_email.m_u16SslPort = pcsPara->m_NetWork.m_email.m_u16SslPort;
                needUpdate++;
                lochemail |= 0x40;
            }
        }

        int lochNtp = 0; // 位9 平台用户密码--直接保存
        if(pcsPara->m_NetWork.m_changeinfo & 0x0200)
        {
            if( plocalPara->m_NetWork.m_NTP.m_ntp_switch != pcsPara->m_NetWork.m_NTP.m_ntp_switch )
            {
                plocalPara->m_NetWork.m_NTP.m_ntp_switch = pcsPara->m_NetWork.m_NTP.m_ntp_switch;
                needUpdate++;
                lochNtp |= 0x04;
            }

        }
        if(pcsPara->m_NetWork.m_changeinfo & 0x0400)
        {
			if(pcsPara->m_NetWork.m_WifiConfig.WifiAddrMode.m_u8Selected == 0)
			{
				return 0;	
			}
			//#ifdef SUPPORT_WIFI
            csSetWIFI(plocalPara,pcsPara);
			/*add by hayson 2014.1.16*/
			/*When the wifi closing shut down the DHCP service*/
			if(pcsPara->m_NetWork.m_WifiConfig.WifiAddrMode.m_u8Selected == 0)
				GetNetModule()->DownDHCP((char*)WIFI_NET_NAME);
			needUpdate++;
			//#endif
		}

		if(needUpdate)
			g_cParaManage->SetSystemParameter(SYSNET_SET,&plocalPara->m_NetWork);
		
		/* 参数保存之后才能处理的部分 */
		if((lochPppoeSet !=0)&&( plocalPara->m_NetWork.m_PppoeSet.m_u8PppoeSelected))
			SetPppoeConfigFile(0, plocalPara->m_NetWork.m_PppoeSet.m_s8UserName, plocalPara->m_NetWork.m_PppoeSet.m_s32Passwd);

        if(needreboot) return needreboot;


        if((lochNtp & 0x04) && (plocalPara->m_NetWork.m_NTP.m_ntp_switch) && (plocalPara->m_NetWork.m_CenterNet.m_Switch))
           // g_ntpnow = 1;  /

        if((lochDomainConfig != 0) && (plocalPara->m_NetWork.m_DomainConfig.m_u8Selected))
        {
            GetNetModule()->Do_ddns();
        }
    }

    return needreboot;
}

int csSetMachinePara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;
    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_Machine.m_changeinfo))
    {
        g_cParaManage->GetSysParameter(SYSMACHINE_SET, &plocalPara->m_Machine);
        int needUpdate = 0;

        int lochMachine = 0;
        if(plocalPara->m_Machine.m_uMachinId != pcsPara->m_Machine.m_uMachinId)
        {
            plocalPara->m_Machine.m_uMachinId = pcsPara->m_Machine.m_uMachinId;
            needUpdate++;
            lochMachine |= 0x01; // 直接保存
        }
        if(plocalPara->m_Machine.m_uTvSystem != pcsPara->m_Machine.m_uTvSystem)
        {
            plocalPara->m_Machine.m_uTvSystem = pcsPara->m_Machine.m_uTvSystem;
            needUpdate++;
            lochMachine |= 0x02; // 重启编码
        }
        if(plocalPara->m_Machine.m_uHddOverWrite != pcsPara->m_Machine.m_uHddOverWrite)
        {
            plocalPara->m_Machine.m_uHddOverWrite = pcsPara->m_Machine.m_uHddOverWrite;
            needUpdate++;
            lochMachine |= 0x04; // 直接保存
        }
        if(plocalPara->m_Machine.m_uHddorSD != pcsPara->m_Machine.m_uHddorSD)
        {
            plocalPara->m_Machine.m_uHddorSD = pcsPara->m_Machine.m_uHddorSD;
            needUpdate++;
            lochMachine |= 0x08;
            needreboot = 1;		// 需重新启动
        }
		
        if(needUpdate)
            g_cParaManage->SetSystemParameter(SYSMACHINE_SET, &plocalPara->m_Machine);

        if((lochMachine & 0x02) && (needreboot == 0))
        {
            SetRecordCtrlThread(THREAD_PAUSE);
            RestartEncodeSystem();
            SetRecordCtrlThread(THREAD_CONTINUE);
        }
    }

    return needreboot;
}

int csSetCameraPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
	int i, chann;
	int needreboot = 0;
	int needrebootenc = 0;
#ifdef DM368
	int osdset = 0;	
	OSDPrm osdprm;	
#endif

	if((plocalPara != NULL)&&(pcsPara != NULL)&&(pcsPara->m_Camera.m_changeinfo))
	{
		g_cParaManage->GetSysParameter(SYSCAMERA_SET,&plocalPara->m_Camera);
		int needUpdate = 0;

		needrebootenc = 0;
		for(chann=0; chann<MAX_REC_CHANNEL; chann++)
		{
			if(pcsPara->m_Camera.m_changeinfo & (1<<chann))
			{
				for(i=0; i<17; i++)
				{
					if(plocalPara->m_Camera.m_ChannelPara[chann].m_Title[i] != pcsPara->m_Camera.m_ChannelPara[chann].m_Title[i])
					{
						plocalPara->m_Camera.m_ChannelPara[chann].m_Title[i] = pcsPara->m_Camera.m_ChannelPara[chann].m_Title[i];
						needUpdate++;
						needrebootenc = 1;
						#ifdef DM368
						osdset = 1;
						#endif
					}
				}
				#ifdef DM368	
                if(osdset == 1) 													
                {
                    memset(&osdprm, 0, sizeof(OSDPrm));
                    osdprm.detailedInfo = 0;
                    osdprm.dateEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch;
                    osdprm.timeEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch;
                    osdprm.logoEnable = 0;
                    osdprm.logoPos = 0;
                    osdprm.textEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TltleSwitch;
                    osdprm.textPos = 0;
                    memcpy(osdprm.text, pcsPara->m_Camera.m_ChannelPara[chann].m_Title, 17);
                
                    if((osdprm.dateEnable == 1) || (osdprm.timeEnable == 1)  || (osdprm.textEnable == 1))
                    {
                        for(i = 0; i < DM36XCHANN; i++) 									
                        {
                            VIDEO_streamOSDEnable(i, 1);							
                            VIDEO_streamOsdPrm(&osdprm, i);
                        }
                    }
                }
				#endif

                if(plocalPara->m_Camera.m_ChannelPara[chann].m_uFrameRate != pcsPara->m_Camera.m_ChannelPara[chann].m_uFrameRate)
                {
					#ifdef DM368
						upDate1080pFramerate(pcsPara->m_Camera.m_ChannelPara[chann].m_uFrameRate);
					#endif
                    plocalPara->m_Camera.m_ChannelPara[chann].m_uFrameRate = pcsPara->m_Camera.m_ChannelPara[chann].m_uFrameRate;
                    needUpdate++;
                    needrebootenc = 1;
                }               
				
				if(get_hd_resolution() == HD_1080P)
				{
	                if (4 != pcsPara->m_Camera.m_ChannelPara[chann].m_uResolution || \
	                        pcsPara->m_Camera.m_ChannelPara[chann].m_uResolution != plocalPara->m_Camera.m_ChannelPara[chann].m_uResolution)
	                {
	                    plocalPara->m_Camera.m_ChannelPara[chann].m_uResolution = 4;
	                    needUpdate++;
	                    needrebootenc = 1;
	                }
				}
				else if(get_hd_resolution() == HD_720P)
				{
					if (5 != pcsPara->m_Camera.m_ChannelPara[chann].m_uResolution || \
							pcsPara->m_Camera.m_ChannelPara[chann].m_uResolution != plocalPara->m_Camera.m_ChannelPara[chann].m_uResolution
						)
					{
						plocalPara->m_Camera.m_ChannelPara[chann].m_uResolution = 5;
						needUpdate++;
						needrebootenc = 1;
					}
				}
				else if(get_hd_resolution() == HD_720P)
				{
	                if (6 != pcsPara->m_Camera.m_ChannelPara[chann].m_uResolution || \
							pcsPara->m_Camera.m_ChannelPara[chann].m_uResolution != plocalPara->m_Camera.m_ChannelPara[chann].m_uResolution)
	                {
	                    plocalPara->m_Camera.m_ChannelPara[chann].m_uResolution = 6;
	                    needUpdate++;
	                    needrebootenc = 1;
	                }
				}

                if(plocalPara->m_Camera.m_ChannelPara[chann].m_uQuality != pcsPara->m_Camera.m_ChannelPara[chann].m_uQuality)
                {
                	#ifdef DM368
                    if(0 == chann)
                    {
                        if(0 == pcsPara->m_Camera.m_ChannelPara[chann].m_uQuality)
                        {
                              upDate1080pTargerBitrate(5000000);

                        }
                        if(1 == pcsPara->m_Camera.m_ChannelPara[chann].m_uQuality)
                        {
                            upDate1080pTargerBitrate(5000000);
                        }
                        if(2 == pcsPara->m_Camera.m_ChannelPara[chann].m_uQuality)
                        {
                            upDate1080pTargerBitrate(4000000);
                        }
                        if(3 == pcsPara->m_Camera.m_ChannelPara[chann].m_uQuality)
                        {
                            upDate1080pTargerBitrate(3000000);
                        }
                        if(4 == pcsPara->m_Camera.m_ChannelPara[chann].m_uQuality)
                        {
                            upDate1080pTargerBitrate(2000000);
                        }
                    }
					#endif
					
					plocalPara->m_Camera.m_ChannelPara[chann].m_uQuality = pcsPara->m_Camera.m_ChannelPara[chann].m_uQuality;
					needUpdate++;
					needrebootenc = 1;
				}
				if(plocalPara->m_Camera.m_ChannelPara[chann].m_uEncType != pcsPara->m_Camera.m_ChannelPara[chann].m_uEncType)
				{
					#ifdef DM368
                    if(0 == pcsPara->m_Camera.m_ChannelPara[chann].m_uEncType)
                    {
                        //配置vbr
                        upDate1080pVbrOrCbr(1);
                    }
                    if(1 == pcsPara->m_Camera.m_ChannelPara[chann].m_uEncType)
                    {
                        //配置cbr
                        upDate1080pVbrOrCbr(2);
                    }
					#endif
					plocalPara->m_Camera.m_ChannelPara[chann].m_uEncType = pcsPara->m_Camera.m_ChannelPara[chann].m_uEncType;
					needUpdate++;
					needrebootenc = 1;
				}	
				if(plocalPara->m_Camera.m_ChannelPara[chann].m_uSubEncType != pcsPara->m_Camera.m_ChannelPara[chann].m_uSubEncType)
				{
					#ifdef DM368
                    if(0 == pcsPara->m_Camera.m_ChannelPara[chann].m_uSubEncType)
                    {
                        //配置vbr
                        upDateVgaVbrOrCbr(1);
                    }
                    if(1 == pcsPara->m_Camera.m_ChannelPara[chann].m_uSubEncType)
                    {
                        //配置cbr
                        upDateVgaVbrOrCbr(2);
                    }
					#endif
					
					plocalPara->m_Camera.m_ChannelPara[chann].m_uSubEncType = pcsPara->m_Camera.m_ChannelPara[chann].m_uSubEncType;
					needUpdate++;
					needrebootenc = 1;
				}
				if(plocalPara->m_Camera.m_ChannelPara[chann].m_uSubEncSwitch != pcsPara->m_Camera.m_ChannelPara[chann].m_uSubEncSwitch)
				{
					plocalPara->m_Camera.m_ChannelPara[chann].m_uSubEncSwitch = pcsPara->m_Camera.m_ChannelPara[chann].m_uSubEncSwitch;
					needUpdate++;
					needrebootenc = 1;
				}
				if(plocalPara->m_Camera.m_ChannelPara[chann].m_uSubRes != pcsPara->m_Camera.m_ChannelPara[chann].m_uSubRes)
				{
					plocalPara->m_Camera.m_ChannelPara[chann].m_uSubRes = pcsPara->m_Camera.m_ChannelPara[chann].m_uSubRes;
					needUpdate++;
					needrebootenc = 1;
				}
				if(plocalPara->m_Camera.m_ChannelPara[chann].m_uSubQuality != pcsPara->m_Camera.m_ChannelPara[chann].m_uSubQuality)
				{
                	#ifdef DM368
									
                    if(0 == chann)
                    {
                    	#if 1
                        if(0 == pcsPara->m_Camera.m_ChannelPara[chann].m_uSubQuality)
                        {
                            upDateVgaTargerBitrate(500000);
                        }
                        if(1 == pcsPara->m_Camera.m_ChannelPara[chann].m_uSubQuality)
                        {
                            upDateVgaTargerBitrate(400000);
                        }
                        if(2 == pcsPara->m_Camera.m_ChannelPara[chann].m_uSubQuality)
                        {
                            upDateVgaTargerBitrate(300000);
                        }
                        if(3 == pcsPara->m_Camera.m_ChannelPara[chann].m_uSubQuality)
                        {
                            upDateVgaTargerBitrate(200000);
                        }
                        if(4 == pcsPara->m_Camera.m_ChannelPara[chann].m_uSubQuality)
                        {
                            upDateVgaTargerBitrate(100000);
                        }
				#else
									upDateVgaTargerBitrate(pcsPara->m_Camera.m_ChannelPara[chann].m_uSubQuality);
				#endif
					  }
			
					#endif
					plocalPara->m_Camera.m_ChannelPara[chann].m_uSubQuality = pcsPara->m_Camera.m_ChannelPara[chann].m_uSubQuality;
					needUpdate++;
					needrebootenc = 1;
				}
				if(plocalPara->m_Camera.m_ChannelPara[chann].m_uSubFrameRate != pcsPara->m_Camera.m_ChannelPara[chann].m_uSubFrameRate)
				{
					#ifdef DM368
                    upDateVgaFramerate(pcsPara->m_Camera.m_ChannelPara[chann].m_uSubFrameRate);
					#endif
                    plocalPara->m_Camera.m_ChannelPara[chann].m_uSubFrameRate = pcsPara->m_Camera.m_ChannelPara[chann].m_uSubFrameRate;
                    needUpdate++;
                    needrebootenc = 1;
                }
				
                if(plocalPara->m_Camera.m_ChannelPara[chann].m_u16RecDelay != pcsPara->m_Camera.m_ChannelPara[chann].m_u16RecDelay)
                {
                    plocalPara->m_Camera.m_ChannelPara[chann].m_u16RecDelay = pcsPara->m_Camera.m_ChannelPara[chann].m_u16RecDelay;
                    needUpdate++;
                }
                if(plocalPara->m_Camera.m_ChannelPara[chann].m_u8PreRecTime != pcsPara->m_Camera.m_ChannelPara[chann].m_u8PreRecTime)
                {
                    plocalPara->m_Camera.m_ChannelPara[chann].m_u8PreRecTime = pcsPara->m_Camera.m_ChannelPara[chann].m_u8PreRecTime;
                    needUpdate++;
                }
                if(plocalPara->m_Camera.m_ChannelPara[chann].m_TltleSwitch != pcsPara->m_Camera.m_ChannelPara[chann].m_TltleSwitch)
                {
                    plocalPara->m_Camera.m_ChannelPara[chann].m_TltleSwitch = pcsPara->m_Camera.m_ChannelPara[chann].m_TltleSwitch;
					#ifdef DM368
                    if(pcsPara->m_Camera.m_ChannelPara[chann].m_TltleSwitch == 1)
                    {
                        memset(&osdprm, 0, sizeof(OSDPrm));
                        osdprm.detailedInfo = 0;
                        osdprm.dateEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch;
                        osdprm.timeEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch;
                        osdprm.logoEnable = 0;
                        osdprm.logoPos = 0;
                        osdprm.textEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TltleSwitch;
                        osdprm.textPos = 0;
                        memcpy(osdprm.text, pcsPara->m_Camera.m_ChannelPara[chann].m_Title, 17);

                        for(i = 0; i < DM36XCHANN; i++) 								
                        {
                            VIDEO_streamOSDEnable(i, 1);
                            VIDEO_streamOsdPrm(&osdprm, i);
                        }
                    }
                    else
                    {
                        memset(&osdprm, 0, sizeof(OSDPrm));
                        osdprm.detailedInfo = 0;
                        osdprm.dateEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch;
                        osdprm.timeEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch;
                        osdprm.logoEnable = 0;
                        osdprm.logoPos = 0;
                        osdprm.textEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TltleSwitch;
                        osdprm.textPos = 0;
                        memset(&osdprm.text, 0, 17);
                        for(i = 0; i < DM36XCHANN; i++) 									
                        {
                            VIDEO_streamOSDEnable(i, 1);
                            VIDEO_streamOsdPrm(&osdprm, i);
                        }
                    }
					#endif
                    needUpdate++;
                    needrebootenc = 1;
                }
                if(plocalPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch != pcsPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch)
                {
                    plocalPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch = pcsPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch;
					#ifdef DM368
                    memset(&osdprm, 0, sizeof(OSDPrm));
                    osdprm.detailedInfo = 0;
                    osdprm.dateEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch;
                    osdprm.timeEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TimeSwitch;
                    osdprm.logoEnable = 0;
                    osdprm.logoPos = 0;
                    osdprm.textEnable = pcsPara->m_Camera.m_ChannelPara[chann].m_TltleSwitch;
                    osdprm.textPos = 0;

                    memcpy(osdprm.text, pcsPara->m_Camera.m_ChannelPara[chann].m_Title, 17);

                    if(plocalPara->m_CommPara.m_uDateMode != pcsPara->m_CommPara.m_uDateMode ||
                            plocalPara->m_CommPara.m_uTimeMode != pcsPara->m_CommPara.m_uTimeMode)
                    {
                        if(pcsPara != NULL) 
                        {
                            DatePlayStyle = pcsPara->m_CommPara.m_uDateMode;
                        }
												
                        if((pcsPara != NULL) && (pcsPara->m_CommPara.m_uTimeMode == 0))
                        {
                            TimePlayStyle = 1;
                        }
                        else
                        {
                            TimePlayStyle = 0;
                        }
                    }
										
                    for(i = 0; i < DM36XCHANN; i++) 									
                    {
                        VIDEO_streamOSDEnable(i, 1);
                        VIDEO_streamOsdPrm(&osdprm, i);
                    }	
					#endif
					
                    needUpdate++;
                    needrebootenc = 1;
                }
                if(plocalPara->m_Camera.m_ChannelPara[chann].m_uSubEncSwitch != pcsPara->m_Camera.m_ChannelPara[chann].m_uSubEncSwitch)
                {
                    plocalPara->m_Camera.m_ChannelPara[chann].m_uSubEncSwitch = pcsPara->m_Camera.m_ChannelPara[chann].m_uSubEncSwitch;
                    needUpdate++;
                    needrebootenc = 1;
                }
                if(plocalPara->m_Camera.m_ChannelPara[chann].m_uSubAudioEncSwitch != pcsPara->m_Camera.m_ChannelPara[chann].m_uSubAudioEncSwitch)
                {
                    plocalPara->m_Camera.m_ChannelPara[chann].m_uSubAudioEncSwitch = pcsPara->m_Camera.m_ChannelPara[chann].m_uSubAudioEncSwitch;
                    needUpdate++;
                    needrebootenc = 1;
                }
                if(plocalPara->m_Camera.m_ChannelPara[chann].m_uChannelValid != pcsPara->m_Camera.m_ChannelPara[chann].m_uChannelValid)
                {
                    plocalPara->m_Camera.m_ChannelPara[chann].m_uChannelValid = pcsPara->m_Camera.m_ChannelPara[chann].m_uChannelValid;
                    needUpdate++;
                    needrebootenc = 1;
                }
                if(plocalPara->m_Camera.m_ChannelPara[chann].m_uAudioSwitch != pcsPara->m_Camera.m_ChannelPara[chann].m_uAudioSwitch)
                {
                    plocalPara->m_Camera.m_ChannelPara[chann].m_uAudioSwitch = pcsPara->m_Camera.m_ChannelPara[chann].m_uAudioSwitch;
                    needUpdate++;
                    needrebootenc = 1;
                }
            }
        }
        if(needUpdate)
            g_cParaManage->SetSystemParameter(SYSCAMERA_SET, &plocalPara->m_Camera);

        if(needrebootenc)
        {
            SetRecordCtrlThread(THREAD_PAUSE);
            RestartEncodeSystem();
            SetRecordCtrlThread(THREAD_CONTINUE);
        }
    }

    return needreboot;
}

int csSetCameraVideoCodePara(CAMERA_PARA *plocalPara, CAMERA_PARA *pcsPara)
{
    int   i;
    int needreboot = 0;
    int needrebootenc = 0;
#ifdef DM368
    int osdset = 0;
    OSDPrm osdprm;
#endif

    if((plocalPara != NULL) && (pcsPara != NULL))
    {
        g_cParaManage->GetSysParameter(SYSCAMERA_SET, plocalPara);
        int needUpdate = 0;
        needrebootenc = 0;
        for(i = 0; i < 17; i++)
        {
            if(plocalPara->m_ChannelPara[0].m_Title[i] != pcsPara->m_ChannelPara[0].m_Title[i])
            {
                plocalPara->m_ChannelPara[0].m_Title[i] = pcsPara->m_ChannelPara[0].m_Title[i];
                needUpdate++;
                needrebootenc = 1;
				#ifdef DM368
				osdset = 1;
				#endif
            }
        }

		#ifdef DM368
        if(osdset == 1)
        {
            memset(&osdprm, 0, sizeof(OSDPrm));
            osdprm.detailedInfo = 0;
            osdprm.dateEnable = pcsPara->m_ChannelPara[0].m_TimeSwitch;
            osdprm.timeEnable = pcsPara->m_ChannelPara[0].m_TimeSwitch;
            osdprm.logoEnable = 0;
            osdprm.logoPos = 0;
            osdprm.textEnable = pcsPara->m_ChannelPara[0].m_TltleSwitch;
            osdprm.textPos = 0;
            memcpy(osdprm.text, pcsPara->m_ChannelPara[0].m_Title, 17);

            if((osdprm.dateEnable == 1) || (osdprm.timeEnable == 1)  || (osdprm.textEnable == 1))
            {
                for(i = 0; i < DM36XCHANN; i++)
                {
                    VIDEO_streamOSDEnable(i, 1);							
                    VIDEO_streamOsdPrm(&osdprm, i);
                }
            }
        }
		#endif
		
        if(plocalPara->m_ChannelPara[0].m_uFrameRate != pcsPara->m_ChannelPara[0].m_uFrameRate)
        {
#ifdef DM368
				upDate1080pFramerate(pcsPara->m_ChannelPara[0].m_uFrameRate);
#endif
			plocalPara->m_ChannelPara[0].m_uFrameRate = pcsPara->m_ChannelPara[0].m_uFrameRate;
			needUpdate++;
			needrebootenc = 1;
		}

			if(get_hd_resolution() == HD_1080P)
			{
	            if (4 != pcsPara->m_ChannelPara[0].m_uResolution || \
	                    pcsPara->m_ChannelPara[0].m_uResolution != plocalPara->m_ChannelPara[0].m_uResolution)
	            {
	                plocalPara->m_ChannelPara[0].m_uResolution = 4;
	                needUpdate++;
	                needrebootenc = 1;
	            }
			}
			else if(get_hd_resolution() == HD_720P)
			{
				if (5 != pcsPara->m_ChannelPara[0].m_uResolution || \
						pcsPara->m_ChannelPara[0].m_uResolution != plocalPara->m_ChannelPara[0].m_uResolution
					)
				{
					plocalPara->m_ChannelPara[0].m_uResolution = 5;
					needUpdate++;
					needrebootenc = 1;
				}
			}
			else if(get_hd_resolution() == HD_VGA)
			{
	            if (6 != pcsPara->m_ChannelPara[0].m_uResolution || \
						pcsPara->m_ChannelPara[0].m_uResolution != plocalPara->m_ChannelPara[0].m_uResolution)
	            {
	                plocalPara->m_ChannelPara[0].m_uResolution = 6;
	                needUpdate++;
	                needrebootenc = 1;
	            }
			}

			
        if(plocalPara->m_ChannelPara[0].m_uQuality != pcsPara->m_ChannelPara[0].m_uQuality)
        {
        	#ifdef DM368
            if(0 == pcsPara->m_ChannelPara[0].m_uQuality)
            {
                upDate1080pTargerBitrate(5000000);
            }
            if(1 == pcsPara->m_ChannelPara[0].m_uQuality)
            {
                upDate1080pTargerBitrate(5000000);
            }
            if(2 == pcsPara->m_ChannelPara[0].m_uQuality)
            {
                upDate1080pTargerBitrate(4000000);
            }
            if(3 == pcsPara->m_ChannelPara[0].m_uQuality)
            {
                upDate1080pTargerBitrate(3000000);
            }
            if(4 == pcsPara->m_ChannelPara[0].m_uQuality)
            {
                upDate1080pTargerBitrate(2000000);
            }
			#endif
            plocalPara->m_ChannelPara[0].m_uQuality = pcsPara->m_ChannelPara[0].m_uQuality;
            needUpdate++;
			needrebootenc = 1;
		}
		
		if(plocalPara->m_ChannelPara[0].m_uEncType != pcsPara->m_ChannelPara[0].m_uEncType)
		{
        	#ifdef DM368
            if(0 == pcsPara->m_ChannelPara[0].m_uEncType)
            {
                //配置vbr
                upDate1080pVbrOrCbr(1);
            }
            if(1 == pcsPara->m_ChannelPara[0].m_uEncType)
            {
                //配置cbr
                upDate1080pVbrOrCbr(2);
            }
			#endif
            plocalPara->m_ChannelPara[0].m_uEncType = pcsPara->m_ChannelPara[0].m_uEncType;
            needUpdate++;
            needrebootenc = 1;
        }
		
        if(plocalPara->m_ChannelPara[0].m_uSubEncType != pcsPara->m_ChannelPara[0].m_uSubEncType)
        {
        	#ifdef DM368
            if(0 == pcsPara->m_ChannelPara[0].m_uSubEncType)
            {
                //配置vbr
                upDateVgaVbrOrCbr(1);
            }
            if(1 == pcsPara->m_ChannelPara[0].m_uSubEncType)
            {
                //配置cbr
                upDateVgaVbrOrCbr(2);
            }
			#endif
            plocalPara->m_ChannelPara[0].m_uSubEncType = pcsPara->m_ChannelPara[0].m_uSubEncType;
            needUpdate++;
            needrebootenc = 1;
        }
		
        if(plocalPara->m_ChannelPara[0].m_uSubEncSwitch != pcsPara->m_ChannelPara[0].m_uSubEncSwitch)
        {
            plocalPara->m_ChannelPara[0].m_uSubEncSwitch = pcsPara->m_ChannelPara[0].m_uSubEncSwitch;
            needUpdate++;
            needrebootenc = 1;
        }
		
        if(plocalPara->m_ChannelPara[0].m_uSubRes != pcsPara->m_ChannelPara[0].m_uSubRes)
        {
            plocalPara->m_ChannelPara[0].m_uSubRes = pcsPara->m_ChannelPara[0].m_uSubRes;
            needUpdate++;
            needrebootenc = 1;
        }
		
        if(plocalPara->m_ChannelPara[0].m_uSubQuality != pcsPara->m_ChannelPara[0].m_uSubQuality)
        {
        	#ifdef DM368
			#if  1
            if(0 == pcsPara->m_ChannelPara[0].m_uSubQuality)
            {
                upDateVgaTargerBitrate(500000);
            }
            if(1 == pcsPara->m_ChannelPara[0].m_uSubQuality)
            {
                upDateVgaTargerBitrate(400000);
            }
            if(2 == pcsPara->m_ChannelPara[0].m_uSubQuality)
            {
                upDateVgaTargerBitrate(300000);
            }
            if(3 == pcsPara->m_ChannelPara[0].m_uSubQuality)
            {
                upDateVgaTargerBitrate(200000);
            }
            if(4 == pcsPara->m_ChannelPara[0].m_uSubQuality)
            {
                upDateVgaTargerBitrate(100000);
            }
			#else
					upDateVgaTargerBitrate( pcsPara->m_ChannelPara[0].m_uSubQuality);
			#endif
			#endif
			
            plocalPara->m_ChannelPara[0].m_uSubQuality = pcsPara->m_ChannelPara[0].m_uSubQuality;
            needUpdate++;
            needrebootenc = 1;
        }

        if(plocalPara->m_ChannelPara[0].m_uSubFrameRate != pcsPara->m_ChannelPara[0].m_uSubFrameRate)
        {
        	#ifdef DM368
            upDateVgaFramerate(pcsPara->m_ChannelPara[0].m_uSubFrameRate);
       		#endif
            plocalPara->m_ChannelPara[0].m_uSubFrameRate = pcsPara->m_ChannelPara[0].m_uSubFrameRate;
            needUpdate++;
            needrebootenc = 1;
        }

        if(plocalPara->m_ChannelPara[0].m_u16RecDelay != pcsPara->m_ChannelPara[0].m_u16RecDelay)
        {
            plocalPara->m_ChannelPara[0].m_u16RecDelay = pcsPara->m_ChannelPara[0].m_u16RecDelay;
            needUpdate++;
        }
		
        if(plocalPara->m_ChannelPara[0].m_u8PreRecTime != pcsPara->m_ChannelPara[0].m_u8PreRecTime)
        {
            plocalPara->m_ChannelPara[0].m_u8PreRecTime = pcsPara->m_ChannelPara[0].m_u8PreRecTime;
            needUpdate++;
        }
		
        if(plocalPara->m_ChannelPara[0].m_TltleSwitch != pcsPara->m_ChannelPara[0].m_TltleSwitch)
        {
            plocalPara->m_ChannelPara[0].m_TltleSwitch = pcsPara->m_ChannelPara[0].m_TltleSwitch;
			#ifdef DM68
            if(pcsPara->m_ChannelPara[0].m_TltleSwitch == 1)
            {
                memset(&osdprm, 0, sizeof(OSDPrm));
                osdprm.detailedInfo = 0;
                osdprm.dateEnable = pcsPara->m_ChannelPara[0].m_TimeSwitch;
                osdprm.timeEnable = pcsPara->m_ChannelPara[0].m_TimeSwitch;
                osdprm.logoEnable = 0;
                osdprm.logoPos = 0;
                osdprm.textEnable = pcsPara->m_ChannelPara[0].m_TltleSwitch;
                osdprm.textPos = 0;
                memcpy(osdprm.text, pcsPara->m_ChannelPara[0].m_Title, 17);
           
                for(i = 0; i < DM36XCHANN; i++) 								
                {
                    VIDEO_streamOSDEnable(i, 1);
                    VIDEO_streamOsdPrm(&osdprm, i);
                }
            }
            else
            {
                memset(&osdprm, 0, sizeof(OSDPrm));
                osdprm.detailedInfo = 0;
                osdprm.dateEnable = pcsPara->m_ChannelPara[0].m_TimeSwitch;
                osdprm.timeEnable = pcsPara->m_ChannelPara[0].m_TimeSwitch;
                osdprm.logoEnable = 0;
                osdprm.logoPos = 0;
                osdprm.textEnable = pcsPara->m_ChannelPara[0].m_TltleSwitch;
                osdprm.textPos = 0;
                memset(&osdprm.text, 0, 17);

                for(i = 0; i < DM36XCHANN; i++) 								
                {
                    VIDEO_streamOSDEnable(i, 1);
                    VIDEO_streamOsdPrm(&osdprm, i);
                }
			
            }
			#endif
            needUpdate++;
            needrebootenc = 1;
        }
        if(plocalPara->m_ChannelPara[0].m_TimeSwitch != pcsPara->m_ChannelPara[0].m_TimeSwitch)
        {
            plocalPara->m_ChannelPara[0].m_TimeSwitch = pcsPara->m_ChannelPara[0].m_TimeSwitch;
			#ifdef DM368
            memset(&osdprm, 0, sizeof(OSDPrm));
            osdprm.detailedInfo = 0;
            osdprm.dateEnable = pcsPara->m_ChannelPara[0].m_TimeSwitch;
            osdprm.timeEnable = pcsPara->m_ChannelPara[0].m_TimeSwitch;
            osdprm.logoEnable = 0;
            osdprm.logoPos = 0;
            osdprm.textEnable = pcsPara->m_ChannelPara[0].m_TltleSwitch;
            osdprm.textPos = 0;
            memcpy(osdprm.text, pcsPara->m_ChannelPara[0].m_Title, 17);

            for(i = 0; i < DM36XCHANN; i++) 								
            {
                VIDEO_streamOSDEnable(i, 1);
                VIDEO_streamOsdPrm(&osdprm, i);
            }
			#endif
			needUpdate++;
			needrebootenc = 1;
		}
		if(plocalPara->m_ChannelPara[0].m_uSubEncSwitch != pcsPara->m_ChannelPara[0].m_uSubEncSwitch)
		{
			plocalPara->m_ChannelPara[0].m_uSubEncSwitch = pcsPara->m_ChannelPara[0].m_uSubEncSwitch;
			needUpdate++;
			needrebootenc = 1;
		}
		if(plocalPara->m_ChannelPara[0].m_uSubAudioEncSwitch != pcsPara->m_ChannelPara[0].m_uSubAudioEncSwitch)
		{
			plocalPara->m_ChannelPara[0].m_uSubAudioEncSwitch = pcsPara->m_ChannelPara[0].m_uSubAudioEncSwitch;
			needUpdate++;
			needrebootenc = 1;
		}
		if(plocalPara->m_ChannelPara[0].m_uChannelValid != pcsPara->m_ChannelPara[0].m_uChannelValid)
		{
			plocalPara->m_ChannelPara[0].m_uChannelValid = pcsPara->m_ChannelPara[0].m_uChannelValid;
			needUpdate++;
			needrebootenc = 1;
		}
		if(plocalPara->m_ChannelPara[0].m_uAudioSwitch != pcsPara->m_ChannelPara[0].m_uAudioSwitch)
		{
			plocalPara->m_ChannelPara[0].m_uAudioSwitch = pcsPara->m_ChannelPara[0].m_uAudioSwitch;
			needUpdate++;
			needrebootenc = 1;
		}			
	
		if(needUpdate)
		{
			g_cParaManage->SetSystemParameter(SYSCAMERA_SET, plocalPara);
		}

		if(needrebootenc)
		{
			SetRecordCtrlThread(THREAD_PAUSE);
			RestartEncodeSystem();
			SetRecordCtrlThread(THREAD_CONTINUE);
		}
	}

	return needreboot;
}

int csSetAnalogPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
	int chann, lochAnalog;
	int needreboot = 0;
	
	if((plocalPara != NULL)&&(pcsPara != NULL)&&(pcsPara->m_Analog.m_changeinfo))
	{
		g_cParaManage->GetSysParameter(SYSANALOG_SET,&plocalPara->m_Analog); // 亮度，对比度，色度，饱和度
		int needUpdate = 0;

		for(chann=0; chann<16; chann++)
		{
			lochAnalog = 0;
			if(pcsPara->m_Analog.m_changeinfo & (1<<chann))
			{
				if(plocalPara->m_Analog.m_Channels[chann].m_nBrightness != pcsPara->m_Analog.m_Channels[chann].m_nBrightness)
				{
					plocalPara->m_Analog.m_Channels[chann].m_nBrightness = pcsPara->m_Analog.m_Channels[chann].m_nBrightness;
					needUpdate++;
					lochAnalog |= 0x01;
					//SetVideoAnalogValue(Type_Brightness,pcsPara->m_Analog.m_Channels[chann].m_nBrightness,chann);
				}
				if(plocalPara->m_Analog.m_Channels[chann].m_nContrast != pcsPara->m_Analog.m_Channels[chann].m_nContrast)
				{
					plocalPara->m_Analog.m_Channels[chann].m_nContrast = pcsPara->m_Analog.m_Channels[chann].m_nContrast;
					needUpdate++;
					lochAnalog |= 0x02;
					//SetVideoAnalogValue(Type_Contrast,pcsPara->m_Analog.m_Channels[chann].m_nContrast,chann);
				}
				if(plocalPara->m_Analog.m_Channels[chann].m_nHue != pcsPara->m_Analog.m_Channels[chann].m_nHue)
				{
					plocalPara->m_Analog.m_Channels[chann].m_nHue = pcsPara->m_Analog.m_Channels[chann].m_nHue;
					needUpdate++;
					lochAnalog |= 0x04;
					//SetVideoAnalogValue(Type_Hue,pcsPara->m_Analog.m_Channels[chann].m_nHue,chann);
				}
				if(plocalPara->m_Analog.m_Channels[chann].m_nSaturation != pcsPara->m_Analog.m_Channels[chann].m_nSaturation)
				{
					plocalPara->m_Analog.m_Channels[chann].m_nSaturation = pcsPara->m_Analog.m_Channels[chann].m_nSaturation;
					needUpdate++;
					lochAnalog |= 0x08;
					//SetVideoAnalogValue(Type_Saturation,pcsPara->m_Analog.m_Channels[chann].m_nSaturation,chann);
				}
			}
		}
		
		if(needUpdate)
			g_cParaManage->SetSystemParameter(SYSANALOG_SET,&plocalPara->m_Analog);
	}

	return needreboot;
}

int csSetComParaPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
	int needreboot = 0;
	
	if((plocalPara != NULL)&&(pcsPara != NULL)&&(pcsPara->m_CommPara.m_changeinfo))
	{
		g_cParaManage->GetSysParameter(SYSCOMMON_SET,&plocalPara->m_CommPara); // 机器设置:时间格式，打包时间
		plocalPara->m_CommPara.m_changeinfo = pcsPara->m_CommPara.m_changeinfo;
		if(plocalPara->m_CommPara.m_uDiaphaneity != pcsPara->m_CommPara.m_uDiaphaneity)
		{
			//SetVideoLayAlpha(pcsPara->m_CommPara.m_uDiaphaneity);
		}
		#ifdef DM368
        if((plocalPara->m_CommPara.m_uDateMode != pcsPara->m_CommPara.m_uDateMode) ||
                (plocalPara->m_CommPara.m_uTimeMode != pcsPara->m_CommPara.m_uTimeMode))
        {

            //DatePlayStyle = 0; //
            //TimePlayStyle = 0; // 0: 12小时， 1：24小时
            OsdDateTimeFormatSet(pcsPara->m_CommPara);
        }
		#endif
        if(memcmp(&plocalPara->m_CommPara, &pcsPara->m_CommPara, sizeof(COMMON_PARA)) != 0)
            g_cParaManage->SetSystemParameter(SYSCOMMON_SET, &pcsPara->m_CommPara); // 只需保存参数即可，大部分参数无功能
    }

    return needreboot;
}

int csSetCamBlindPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_CamerBlind.m_changeinfo))
    {
        g_cParaManage->SetSystemParameter(SYSBLIND_SET, &pcsPara->m_CamerBlind);
    }

    return 0;
}

int csSetCamVideoLossPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_CamerVideoLoss.m_changeinfo))
    {
        g_cParaManage->SetSystemParameter(VIDEOLOSS_SET, &pcsPara->m_CamerVideoLoss);
    }

    return 0;
}

int csSetCamAlarmPortPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_AlarmPort.m_changeinfo))
    {
        g_cParaManage->SetSystemParameter(ALARMPORT_SET, &pcsPara->m_AlarmPort);
    }

    return 0;
}

int csSetCamMdPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;
	
    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_CameraMd.m_changeinfo))
    {
        g_cParaManage->GetSysParameter(SYSMOTION_SET, &plocalPara->m_CameraMd); // 报警->移动侦测: 需更新移动侦测设置
        int needUpdate = 0;

        int lochCameraMd = 0;


        for(int ch = 0; ch < MAX_REC_CHANNEL; ch++)
        {
            if(pcsPara->m_CameraMd.m_changeinfo)
            {
                if(pcsPara->m_CameraMd.m_Channel[ch].m_uMDSensitive > 7) pcsPara->m_CameraMd.m_Channel[ch].m_uMDSensitive = 7;
                if(memcmp(&plocalPara->m_CameraMd.m_Channel[ch], &pcsPara->m_CameraMd.m_Channel[ch], sizeof(MD_SETUP)) != 0)
                {
                    if(plocalPara->m_CameraMd.m_Channel[ch].m_uMDSwitch != pcsPara->m_CameraMd.m_Channel[ch].m_uMDSwitch)
                        lochCameraMd |= (1 << ch);
                    memcpy(&plocalPara->m_CameraMd.m_Channel[ch], &pcsPara->m_CameraMd.m_Channel[ch], sizeof(MD_SETUP));
                    needUpdate++;
                    SetMotionDetetionPara(ch, plocalPara->m_CameraMd.m_Channel[ch], plocalPara->m_CameraMd.m_Channel[ch].m_uMDSensitive);
					#ifdef DM368
                    AVSERVER_setMotionPre(pcsPara->m_CameraMd.m_Channel[ch].m_uMDSwitch, 1, 1, pcsPara->m_CameraMd.m_Channel[ch].m_uMDSensitive, pcsPara->m_CameraMd.m_Channel[ch].m_MDMask);			
					#endif
				}
            }
        }

        if(needUpdate)
        {
            g_cParaManage->SetSystemParameter(SYSMOTION_SET, &pcsPara->m_CameraMd);
        }
    }

    return needreboot;
}

int csSetRecSchedPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;

    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_RecordSchedule.m_changeinfo))
    {
        g_cParaManage->GetSysParameter(SYSRECSCHEDULE_SET, &plocalPara->m_RecordSchedule); // 影像->定时录像设置: 重启录像线程
        int needUpdate = 0;

        int lochRecSched = 0;
        for(int ch = 0; ch < 16; ch++)
        {
            if(pcsPara->m_RecordSchedule.m_changeinfo & (1 << ch))
            {
                if(memcmp(&plocalPara->m_RecordSchedule.m_ChTask[ch], &pcsPara->m_RecordSchedule.m_ChTask[ch], sizeof(RECORDTASK)) != 0)
                {
                    if(plocalPara->m_RecordSchedule.m_ChTask[ch].m_uTimerSwitch != pcsPara->m_RecordSchedule.m_ChTask[ch].m_uTimerSwitch)
                        lochRecSched |= (1 << ch);
                    memcpy(&plocalPara->m_RecordSchedule.m_ChTask[ch], &pcsPara->m_RecordSchedule.m_ChTask[ch], sizeof(RECORDTASK));
                    needUpdate++;
                }
            }
        }

        if(needUpdate)
        {			
            SetRecordCtrlThread(THREAD_PAUSE);
            SetRecordCtrlThread(THREAD_CONTINUE);
            g_cParaManage->SetSystemParameter(SYSRECSCHEDULE_SET, &plocalPara->m_RecordSchedule);
        }
    }

    return needreboot;
}

int csSetPcDirPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;

    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_PcDir.m_changeinfo))
    {
        g_cParaManage->GetSysParameter(SYSPCDIR_SET, &plocalPara->m_PcDir); // PC端的数据保存目录--直接保存
        int needUpdate = 0;

        int lochPcDir = 0;
        if(pcsPara->m_PcDir.m_changeinfo & 0x01)
        {
            if(memcmp(plocalPara->m_PcDir.m_pcSnapDir, pcsPara->m_PcDir.m_pcSnapDir, 96) != 0)
            {
                memcpy(plocalPara->m_PcDir.m_pcSnapDir, pcsPara->m_PcDir.m_pcSnapDir, 96);
                needUpdate++;
                lochPcDir |= 0x01;
            }
        }
        if(pcsPara->m_PcDir.m_changeinfo & 0x02)
        {
            if(memcmp(plocalPara->m_PcDir.m_pcRecDir, pcsPara->m_PcDir.m_pcRecDir, 96) != 0)
            {
                memcpy(plocalPara->m_PcDir.m_pcRecDir, pcsPara->m_PcDir.m_pcRecDir, 96);
                needUpdate++;
                lochPcDir |= 0x02;
            }
        }
        if(pcsPara->m_PcDir.m_changeinfo & 0x04)
        {
            if(memcmp(plocalPara->m_PcDir.m_pcdLoadDir, pcsPara->m_PcDir.m_pcdLoadDir, 96) != 0)
            {
                memcpy(plocalPara->m_PcDir.m_pcdLoadDir, pcsPara->m_PcDir.m_pcdLoadDir, 96);
                needUpdate++;
                lochPcDir |= 0x04;
            }
        }

        if(needUpdate)
            g_cParaManage->SetSystemParameter(SYSPCDIR_SET, &plocalPara->m_PcDir);
    }

    return needreboot;
}

int csSetMainetancePara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;

    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_SysMainetance.m_changeinfo))
    {
        g_cParaManage->GetSysParameter(SYSMAINETANCE_SET, &plocalPara->m_SysMainetance); // 系统维护
        plocalPara->m_SysMainetance.m_changeinfo = pcsPara->m_SysMainetance.m_changeinfo;

        if(memcmp(&plocalPara->m_SysMainetance, &pcsPara->m_SysMainetance, sizeof(SYSTEMMAINETANCE)) != 0)
            g_cParaManage->SetSystemParameter(SYSMAINETANCE_SET, &pcsPara->m_SysMainetance);
    }

    return needreboot;
}

int csSetDisplayPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;

    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_DisplaySet.m_changeinfo))
    {
        g_cParaManage->GetSysParameter(SYSDISPLAY_SET, &plocalPara->m_DisplaySet); // 控制GUI本地预览显示
        int needUpdate = 0;

        int lochDisplay = 0;
        if(plocalPara->m_DisplaySet.m_u8Alpha != pcsPara->m_DisplaySet.m_u8Alpha)
        {
            plocalPara->m_DisplaySet.m_u8Alpha = pcsPara->m_DisplaySet.m_u8Alpha;
            lochDisplay |= 0x01;
            needUpdate++; // 只有GUI使用
        }
        if(plocalPara->m_DisplaySet.m_u8VGAMode != pcsPara->m_DisplaySet.m_u8VGAMode)
        {
            plocalPara->m_DisplaySet.m_u8VGAMode = pcsPara->m_DisplaySet.m_u8VGAMode;
            needreboot = 1;
            lochDisplay |= 0x02;
            needUpdate++; // 只有GUI使用
        }
        if(plocalPara->m_DisplaySet.m_u8CruiseInterval != pcsPara->m_DisplaySet.m_u8CruiseInterval)
        {
            plocalPara->m_DisplaySet.m_u8CruiseInterval = pcsPara->m_DisplaySet.m_u8CruiseInterval;
            lochDisplay |= 0x04;
            needUpdate++;  // 只有GUI使用
        }
        if(plocalPara->m_DisplaySet.m_u8AlarmCruiseInterval != pcsPara->m_DisplaySet.m_u8AlarmCruiseInterval)
        {
            plocalPara->m_DisplaySet.m_u8AlarmCruiseInterval = pcsPara->m_DisplaySet.m_u8AlarmCruiseInterval;
            lochDisplay |= 0x08;
            needUpdate++;	// 只有GUI使用
        }
        if(plocalPara->m_DisplaySet.m_u8CruiseMode != pcsPara->m_DisplaySet.m_u8CruiseMode)
        {
            plocalPara->m_DisplaySet.m_u8CruiseMode = pcsPara->m_DisplaySet.m_u8CruiseMode;
            lochDisplay |= 0x10;
            needUpdate++;	// 只有GUI使用
        }
        if(plocalPara->m_DisplaySet.m_u32SChValid != pcsPara->m_DisplaySet.m_u32SChValid)
        {
            plocalPara->m_DisplaySet.m_u32SChValid = pcsPara->m_DisplaySet.m_u32SChValid;
            lochDisplay |= 0x20;
            needUpdate++;	// 只有GUI使用
        }
        if(plocalPara->m_DisplaySet.m_u32FourValid != pcsPara->m_DisplaySet.m_u32FourValid)
        {
            plocalPara->m_DisplaySet.m_u32FourValid = pcsPara->m_DisplaySet.m_u32FourValid;
            lochDisplay |= 0x40;
            needUpdate++;	// 只有GUI使用
        }

        if(needUpdate)
            g_cParaManage->SetSystemParameter(SYSDISPLAY_SET, &plocalPara->m_DisplaySet);
    }

    return needreboot;
}

int csSetUsersPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;
    char adminstr[16] = {'a', 'd', 'm', 'i', 'n', 0, 0};

    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_Users.m_changeinfo))
    {
        g_cParaManage->GetSysParameter(SYSUSERPREMIT_SET, &plocalPara->m_Users); // 用户设置--直接保存
        int needUpdate = 0;

        for(int ch = 0; ch < 16; ch++)
        {
            if(pcsPara->m_Users.m_changeinfo & (1 << ch))
            {
                pcsPara->m_Users.m_UserSet[ch].m_cUserName[15] = 0;
                pcsPara->m_Users.m_UserSet[ch].m_s32Passwd[15] = 0;
                if(memcmp(&plocalPara->m_Users.m_UserSet[ch], &pcsPara->m_Users.m_UserSet[ch], sizeof(SINGLEUSERSET)) != 0)
                {
                    memcpy(&plocalPara->m_Users.m_UserSet[ch], &pcsPara->m_Users.m_UserSet[ch], sizeof(SINGLEUSERSET));
                    needUpdate++;
                }
            }
        }

        if(needUpdate)
        {
            memcpy(plocalPara->m_Users.m_UserSet[0].m_cUserName, adminstr, 16); // "admin" 超级用户只能改密码
            plocalPara->m_Users.m_UserSet[0].m_s32UserPermit = 0;
            plocalPara->m_Users.m_UserSet[0].m_u8UserValid = 1;
            g_cParaManage->SetSystemParameter(SYSUSERPREMIT_SET, &plocalPara->m_Users);
        }
    }

    return needreboot;
}

int csSetSysExceptPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;

    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_SysExcept.m_changeinfo))
    {
        g_cParaManage->SetSystemParameter(SYSEXCEPT_SET, &pcsPara->m_SysExcept);
    }

    return needreboot;
}

int csSetOsdInsertPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
	int i;
	int needUpdate = 0;
	#if (defined APP3518) 
	RECT_S	rect;
	#endif
	
    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_OsdInsert.m_changeinfo1 || pcsPara->m_OsdInsert.m_changeinfo2 || pcsPara->m_OsdInsert.m_changeinfo3 || pcsPara->m_OsdInsert.m_changeinfo4))
    {

        g_cParaManage->GetSysParameter(SYSOSDINSERT_SET, &plocalPara->m_OsdInsert); // 报警->视频遮挡: 更新视频遮挡
        
        for(i = 0; i < MAX_REC_CHANNEL; i++)
        {
            if(memcmp(&plocalPara->m_OsdInsert.m_CoverLay[i], &pcsPara->m_OsdInsert.m_CoverLay[i], sizeof(OVERLAYSET)) != 0)
            {
                memcpy(&plocalPara->m_OsdInsert.m_CoverLay[i], &pcsPara->m_OsdInsert.m_CoverLay[i], sizeof(OVERLAYSET));
                needUpdate++;
				#ifdef DM368
                zmd_set_shield_channel0(
                    pcsPara->m_OsdInsert.m_CoverLay[i].m_u16X,
                    pcsPara->m_OsdInsert.m_CoverLay[i].m_u16Y,
                    pcsPara->m_OsdInsert.m_CoverLay[i].m_u16Width,
                    pcsPara->m_OsdInsert.m_CoverLay[i].m_u16Height,
                    pcsPara->m_OsdInsert.m_CoverLay[i].m_u8OverValid);
				#endif

				#if (defined APP3518) 
				if(plocalPara->m_OsdInsert.m_CoverLay[i].m_u8OverValid != 0)
				{
					rect.s32X = plocalPara->m_OsdInsert.m_CoverLay[i].m_u16X;
					rect.s32Y = plocalPara->m_OsdInsert.m_CoverLay[i].m_u16Y;
					rect.u32Width = plocalPara->m_OsdInsert.m_CoverLay[i].m_u16Width;
					rect.u32Height = plocalPara->m_OsdInsert.m_CoverLay[i].m_u16Height;
					CreateFullVideoCoverRegion(i, rect,0);
				}
				else
				{
					DeleteFullVideoCoverRegion(i,0);
				}
				#endif
			}
            if(memcmp(&plocalPara->m_OsdInsert.m_CoverLay2[i], &pcsPara->m_OsdInsert.m_CoverLay2[i], sizeof(OVERLAYSET)) != 0)
            {
                memcpy(&plocalPara->m_OsdInsert.m_CoverLay2[i], &pcsPara->m_OsdInsert.m_CoverLay2[i], sizeof(OVERLAYSET));
                needUpdate++;
				#ifdef DM368
                zmd_set_shield_channel1(
                    pcsPara->m_OsdInsert.m_CoverLay2[i].m_u16X,
                    pcsPara->m_OsdInsert.m_CoverLay2[i].m_u16Y,
                    pcsPara->m_OsdInsert.m_CoverLay2[i].m_u16Width,
                    pcsPara->m_OsdInsert.m_CoverLay2[i].m_u16Height,
                    pcsPara->m_OsdInsert.m_CoverLay2[i].m_u8OverValid);
				#endif

				#if (defined APP3518)
				if(plocalPara->m_OsdInsert.m_CoverLay2[i].m_u8OverValid != 0)
				{
					rect.s32X = plocalPara->m_OsdInsert.m_CoverLay2[i].m_u16X;
					rect.s32Y = plocalPara->m_OsdInsert.m_CoverLay2[i].m_u16Y;
					rect.u32Width = plocalPara->m_OsdInsert.m_CoverLay2[i].m_u16Width;
					rect.u32Height = plocalPara->m_OsdInsert.m_CoverLay2[i].m_u16Height;
					CreateFullVideoCoverRegion(i, rect,1);	
				}
				else
				{
					DeleteFullVideoCoverRegion(i,1);
				}
				#endif
			}
            if(memcmp(&plocalPara->m_OsdInsert.m_CoverLay3[i], &pcsPara->m_OsdInsert.m_CoverLay3[i], sizeof(OVERLAYSET)) != 0)
            {

                memcpy(&plocalPara->m_OsdInsert.m_CoverLay3[i], &pcsPara->m_OsdInsert.m_CoverLay3[i], sizeof(OVERLAYSET));
                needUpdate++;
				#ifdef DM368
		        zmd_set_shield_channel2(
		            pcsPara->m_OsdInsert.m_CoverLay3[i].m_u16X,
		            pcsPara->m_OsdInsert.m_CoverLay3[i].m_u16Y,
		            pcsPara->m_OsdInsert.m_CoverLay3[i].m_u16Width,
		            pcsPara->m_OsdInsert.m_CoverLay3[i].m_u16Height,
		            pcsPara->m_OsdInsert.m_CoverLay3[i].m_u8OverValid);
				#endif

				#if (defined APP3518) 
				if(plocalPara->m_OsdInsert.m_CoverLay3[i].m_u8OverValid != 0)
				{
					rect.s32X = plocalPara->m_OsdInsert.m_CoverLay3[i].m_u16X;
					rect.s32Y = plocalPara->m_OsdInsert.m_CoverLay3[i].m_u16Y;
					rect.u32Width = plocalPara->m_OsdInsert.m_CoverLay3[i].m_u16Width;
					rect.u32Height = plocalPara->m_OsdInsert.m_CoverLay3[i].m_u16Height;
					CreateFullVideoCoverRegion(i, rect,2);	
				}
				else
				{
					DeleteFullVideoCoverRegion(i,2);
				}
				#endif
			}
            if(memcmp(&plocalPara->m_OsdInsert.m_CoverLay4[i], &pcsPara->m_OsdInsert.m_CoverLay4[i], sizeof(OVERLAYSET)) != 0)
            {
                memcpy(&plocalPara->m_OsdInsert.m_CoverLay4[i], &pcsPara->m_OsdInsert.m_CoverLay4[i], sizeof(OVERLAYSET));
                needUpdate++;    
				#ifdef DM368
                zmd_set_shield_channel3(
                    pcsPara->m_OsdInsert.m_CoverLay4[i].m_u16X,
                    pcsPara->m_OsdInsert.m_CoverLay4[i].m_u16Y,
                    pcsPara->m_OsdInsert.m_CoverLay4[i].m_u16Width,
                    pcsPara->m_OsdInsert.m_CoverLay4[i].m_u16Height,
                    pcsPara->m_OsdInsert.m_CoverLay4[i].m_u8OverValid);
				#endif				
				#if (defined APP3518) 
				if(plocalPara->m_OsdInsert.m_CoverLay4[i].m_u8OverValid != 0)
				{
					rect.s32X = plocalPara->m_OsdInsert.m_CoverLay4[i].m_u16X;
					rect.s32Y = plocalPara->m_OsdInsert.m_CoverLay4[i].m_u16Y;
					rect.u32Width = plocalPara->m_OsdInsert.m_CoverLay4[i].m_u16Width;
					rect.u32Height = plocalPara->m_OsdInsert.m_CoverLay4[i].m_u16Height;
					CreateFullVideoCoverRegion(i, rect,3);	
				}
				else
				{
					DeleteFullVideoCoverRegion(i,3);
				}
				#endif
			}
				  
        }

        if(needUpdate)
        {
            g_cParaManage->SetSystemParameter(SYSOSDINSERT_SET, &plocalPara->m_OsdInsert);
			#if (defined APP3518) || (defined DM368)
            UpdateCoverlayOsd();
			#endif
        }
    }

    return 0;
}

int csSetPTZPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;

    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_PTZ.m_changeinfo))
    {
        g_cParaManage->GetSysParameter(SYSPTZ_SET, &plocalPara->m_PTZ); // 影像->云台设置:直接保存参数，不用处理
        int needUpdate = 0;

        int lochPTZ = 0;
        for(int ch = 0; ch < MAX_REC_CHANNEL; ch++)
        {
            if(pcsPara->m_PTZ.m_changeinfo & (1 << ch))
            {
                if(memcmp(&plocalPara->m_PTZ.m_ptz_channel[ch], &pcsPara->m_PTZ.m_ptz_channel[ch], sizeof(PTZ_Info_S)) != 0)
                {
                    memcpy(&plocalPara->m_PTZ.m_ptz_channel[ch], &pcsPara->m_PTZ.m_ptz_channel[ch], sizeof(PTZ_Info_S));
                    needUpdate++;
                    lochPTZ |= (1 << ch);
                }
            }
        }
        if(needUpdate)
            g_cParaManage->SetSystemParameter(SYSPTZ_SET, &plocalPara->m_PTZ);
    }

    return needreboot;
}

int csSetPELCOPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;

    if((plocalPara != NULL) && (pcsPara != NULL))
    {
        g_cParaManage->GetSysParameter(PELCOCMD_SET, &plocalPara->m_pelcoCfg); // Pelco命令:直接保存参数，不用处理
        int needUpdate = 0;
        int lochPELCO = 0;

        for(int i = 0; i < CHANNEL_NUM; i++)
        {
            if(pcsPara->m_pelcoCfg.m_changeinfo[i] == 0) continue;
            lochPELCO = 0;
            if(pcsPara->m_pelcoCfg.m_changeinfo[i] & 0x01)
            {
                if(memcmp(plocalPara->m_pelcoCfg.pelcoP_enterMenu[i], pcsPara->m_pelcoCfg.pelcoP_enterMenu[i], 4) != 0)
                {
                    memcpy(plocalPara->m_pelcoCfg.pelcoP_enterMenu[i], pcsPara->m_pelcoCfg.pelcoP_enterMenu[i], 4);
                    needUpdate++;
                    lochPELCO |= 0x01;
                }
            }
            if(pcsPara->m_pelcoCfg.m_changeinfo[i] & 0x02)
            {
                if(memcmp(plocalPara->m_pelcoCfg.pelcoP_runpattern[i], pcsPara->m_pelcoCfg.pelcoP_runpattern[i], 4) != 0)
                {
                    memcpy(plocalPara->m_pelcoCfg.pelcoP_runpattern[i], pcsPara->m_pelcoCfg.pelcoP_runpattern[i], 4);
                    needUpdate++;
                    lochPELCO |= 0x02;
                }
            }
            if(pcsPara->m_pelcoCfg.m_changeinfo[i] & 0x04)
            {
                if(memcmp(plocalPara->m_pelcoCfg.pelcoP_cruiseOn[i], pcsPara->m_pelcoCfg.pelcoP_cruiseOn[i], 4) != 0)
                {
                    memcpy(plocalPara->m_pelcoCfg.pelcoP_cruiseOn[i], pcsPara->m_pelcoCfg.pelcoP_cruiseOn[i], 4);
                    needUpdate++;
                    lochPELCO |= 0x04;
                }
            }
            if(pcsPara->m_pelcoCfg.m_changeinfo[i] & 0x08)
            {
                if(memcmp(plocalPara->m_pelcoCfg.pelcoP_autoScan[i], pcsPara->m_pelcoCfg.pelcoP_autoScan[i], 4) != 0)
                {
                    memcpy(plocalPara->m_pelcoCfg.pelcoP_autoScan[i], pcsPara->m_pelcoCfg.pelcoP_autoScan[i], 4);
                    needUpdate++;
                    lochPELCO |= 0x08;
                }
            }
            if(pcsPara->m_pelcoCfg.m_changeinfo[i] & 0x10)
            {
                if(memcmp(plocalPara->m_pelcoCfg.pelcoP_stopScan[i], pcsPara->m_pelcoCfg.pelcoP_stopScan[i], 4) != 0)
                {
                    memcpy(plocalPara->m_pelcoCfg.pelcoP_stopScan[i], pcsPara->m_pelcoCfg.pelcoP_stopScan[i], 4);
                    needUpdate++;
                    lochPELCO |= 0x10;
                }
            }

            if(pcsPara->m_pelcoCfg.m_changeinfo[i] & 0x010000)
            {
                if(memcmp(plocalPara->m_pelcoCfg.pelcoD_enterMenu[i], pcsPara->m_pelcoCfg.pelcoD_enterMenu[i], 4) != 0)
                {
                    memcpy(plocalPara->m_pelcoCfg.pelcoD_enterMenu[i], pcsPara->m_pelcoCfg.pelcoD_enterMenu[i], 4);
                    needUpdate++;
                    lochPELCO |= 0x010000;
                }
            }
            if(pcsPara->m_pelcoCfg.m_changeinfo[i] & 0x020000)
            {
                if(memcmp(plocalPara->m_pelcoCfg.pelcoD_runpattern[i], pcsPara->m_pelcoCfg.pelcoD_runpattern[i], 4) != 0)
                {
                    memcpy(plocalPara->m_pelcoCfg.pelcoD_runpattern[i], pcsPara->m_pelcoCfg.pelcoD_runpattern[i], 4);
                    needUpdate++;
                    lochPELCO |= 0x020000;
                }
            }
            if(pcsPara->m_pelcoCfg.m_changeinfo[i] & 0x040000)
            {
                if(memcmp(plocalPara->m_pelcoCfg.pelcoD_cruiseOn[i], pcsPara->m_pelcoCfg.pelcoD_cruiseOn[i], 4) != 0)
                {
                    memcpy(plocalPara->m_pelcoCfg.pelcoD_cruiseOn[i], pcsPara->m_pelcoCfg.pelcoD_cruiseOn[i], 4);
                    needUpdate++;
                    lochPELCO |= 0x040000;
                }
            }
            if(pcsPara->m_pelcoCfg.m_changeinfo[i] & 0x080000)
            {
                if(memcmp(plocalPara->m_pelcoCfg.pelcoD_autoScan[i], pcsPara->m_pelcoCfg.pelcoD_autoScan[i], 4) != 0)
                {
                    memcpy(plocalPara->m_pelcoCfg.pelcoD_autoScan[i], pcsPara->m_pelcoCfg.pelcoD_autoScan[i], 4);
                    needUpdate++;
                    lochPELCO |= 0x080000;
                }
            }
            if(pcsPara->m_pelcoCfg.m_changeinfo[i] & 0x100000)
            {
                if(memcmp(plocalPara->m_pelcoCfg.pelcoD_stopScan[i], pcsPara->m_pelcoCfg.pelcoD_stopScan[i], 4) != 0)
                {
                    memcpy(plocalPara->m_pelcoCfg.pelcoD_stopScan[i], pcsPara->m_pelcoCfg.pelcoD_stopScan[i], 4);
                    needUpdate++;
                    lochPELCO |= 0x100000;
                }
            }
        }

        if(needUpdate)
            g_cParaManage->SetSystemParameter(PELCOCMD_SET, &plocalPara->m_pelcoCfg);
    }

    return needreboot;
}

int csSetPicTimerPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int needreboot = 0;

    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_picTimer.m_changeinfo))
    {
        g_cParaManage->GetSysParameter(PICTIMER_SET, &plocalPara->m_picTimer);
        int needUpdate = 0;

        if(pcsPara->m_picTimer.m_changeinfo & 0x01)
        {
            memcpy(&plocalPara->m_picTimer.m_picChn[0], &pcsPara->m_picTimer.m_picChn[0], sizeof(ANALOG_CHANNEL));
            needUpdate |= 0x01;
        }

        if(pcsPara->m_picTimer.m_changeinfo & 0x02)
        {
            memcpy(&plocalPara->m_picTimer.m_picChn[1], &pcsPara->m_picTimer.m_picChn[1], sizeof(ANALOG_CHANNEL));
            needUpdate |= 0x02;
        }

        if(pcsPara->m_picTimer.m_changeinfo & 0x04)
        {
            memcpy(&plocalPara->m_picTimer.m_picChn[2], &pcsPara->m_picTimer.m_picChn[2], sizeof(ANALOG_CHANNEL));
            needUpdate |= 0x04;
        }

        if(pcsPara->m_picTimer.m_changeinfo & 0x08)
        {
            memcpy(&plocalPara->m_picTimer.m_picTmr[0], &pcsPara->m_picTimer.m_picTmr[0], sizeof(TIMETBLSECTION));
            needUpdate |= 0x08;
        }

        if(pcsPara->m_picTimer.m_changeinfo & 0x10)
        {
            memcpy(&plocalPara->m_picTimer.m_picTmr[1], &pcsPara->m_picTimer.m_picTmr[1], sizeof(TIMETBLSECTION));
            needUpdate |= 0x10;
        }

        if(pcsPara->m_picTimer.m_changeinfo & 0x20)
        {
            memcpy(&plocalPara->m_picTimer.m_picTmr[2], &pcsPara->m_picTimer.m_picTmr[2], sizeof(TIMETBLSECTION));
            needUpdate |= 0x20;
        }

        if(needUpdate)
            g_cParaManage->SetSystemParameter(PICTIMER_SET, &plocalPara->m_picTimer);
    }

    return needreboot;
}

int csSetZoneGroupPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
    int i, needreboot = 0;

    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_ZoneGroup.m_changeinfo1 ))
    {
        g_cParaManage->GetSysParameter(SYSALARMZONE_SET, &plocalPara->m_ZoneGroup); //  报警->视频抓拍联动:直接保存参数，不用处理
        int needUpdate = 0;

        if(pcsPara->m_ZoneGroup.m_changeinfo1)
        {
            for(i = 0; i < MAX_ALARM_ZONE; i++)
            {
                if(pcsPara->m_ZoneGroup.m_changeinfo1)
                {
                    if(memcmp(&plocalPara->m_ZoneGroup.m_AlarmZone[i], &pcsPara->m_ZoneGroup.m_AlarmZone[i], sizeof(AlarmZoneSet)) != 0)
                    {
                        memcpy(&plocalPara->m_ZoneGroup.m_AlarmZone[i], &pcsPara->m_ZoneGroup.m_AlarmZone[i], sizeof(AlarmZoneSet));
                        needUpdate++;
                    }
                }
            }
        }
        if(needUpdate)
            g_cParaManage->SetSystemParameter(SYSALARMZONE_SET, &plocalPara->m_ZoneGroup);
    }

    return needreboot;
}



int csSetDefSchedPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
	printf("mail %s %s\n", pcsPara->m_ParaExtend.m_MailExt.m_addr1, pcsPara->m_ParaExtend.m_MailExt.m_addr2);
	
	
    if((plocalPara != NULL) && (pcsPara != NULL))
    {
    	 g_cParaManage->GetSysParameter(EXTEND_SET, &plocalPara->m_ParaExtend);

		if( memcmp(&plocalPara->m_ParaExtend.m_MailExt, &pcsPara->m_ParaExtend.m_MailExt, sizeof(MailExt)) != 0 )
        {
            memcpy(&plocalPara->m_ParaExtend.m_MailExt, &pcsPara->m_ParaExtend.m_MailExt, sizeof(MailExt));

            g_cParaManage->SetSystemParameter(EXTEND_SET, &plocalPara->m_ParaExtend);
        }		
    }
	
	return 0;
}



int csSetPtzLinkPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
	
#if 0
    int i, needreboot = 0;

    if((plocalPara != NULL) && (pcsPara != NULL) && (pcsPara->m_PtzLink.m_changeinfo1))
    {
        g_cParaManage->GetSysParameter(SYSPTZLINK_SET, &plocalPara->m_PtzLink); // 报警->云台联动:直接保存参数，不用处理
        int needUpdate = 0;

        if(pcsPara->m_PtzLink.m_changeinfo1)
        {
            for(i = 0; i < MAX_ALARM_ZONE; i++)
            {
                if(memcmp(&plocalPara->m_PtzLink.m_Zone[i], &pcsPara->m_PtzLink.m_Zone[i], sizeof(FangZonePtzLinkSet)) != 0)
                {
                    memcpy(&plocalPara->m_PtzLink.m_Zone[i], &pcsPara->m_PtzLink.m_Zone[i], sizeof(FangZonePtzLinkSet));
                    needUpdate++;
                }
            }
        }

        if(needUpdate)
            g_cParaManage->SetSystemParameter(SYSPTZLINK_SET, &plocalPara->m_PtzLink);
    }

    return needreboot;
#endif
	return 0;
}

int csSet3GPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara)
{
	#if 0
	int i, needreboot = 0;
	
	if((plocalPara != NULL)&&(pcsPara != NULL))
	{
		if(pcsPara->m_3g.m_changeinfo)
		{
			g_cParaManage->GetSysParameter(SYS3G_SET,&plocalPara->m_3g);
			int needUpdate = 0;

			if(pcsPara->m_3g.m_changeinfo & 0x01)
			{
				if( plocalPara->m_3g.g3modenable != pcsPara->m_3g.g3modenable )
				{
					plocalPara->m_3g.g3modenable = pcsPara->m_3g.g3modenable;
					needUpdate++;
					needreboot++;
				}
			}
			if(pcsPara->m_3g.m_changeinfo & 0x02)
			{
				if( plocalPara->m_3g.g3conmethd != pcsPara->m_3g.g3conmethd )
				{
					plocalPara->m_3g.g3conmethd = pcsPara->m_3g.g3conmethd;
					needUpdate++;
				}
			}
			if(pcsPara->m_3g.m_changeinfo & 0x04)
			{
				for(i=0; i<15; i++)
				{
					if( plocalPara->m_3g.simcard[i] != pcsPara->m_3g.simcard[i] )
					{
						plocalPara->m_3g.simcard[i] = pcsPara->m_3g.simcard[i];
						needUpdate++;
					}
				}
				plocalPara->m_3g.simcard[15] = 0;
			}
			if(pcsPara->m_3g.m_changeinfo & 0x08)
			{
				for(i=0; i<15; i++)
				{
					if( plocalPara->m_3g.ctlpwd[i] != pcsPara->m_3g.ctlpwd[i] )
					{
						plocalPara->m_3g.ctlpwd[i] = pcsPara->m_3g.ctlpwd[i];
						needUpdate++;
					}
				}
				plocalPara->m_3g.ctlpwd[15] = 0;
			}
			if(pcsPara->m_3g.m_changeinfo & 0x10)
			{
				if( plocalPara->m_3g.g3card != pcsPara->m_3g.g3card )
				{
					plocalPara->m_3g.g3card = pcsPara->m_3g.g3card;
					needUpdate++;
				}
			}
			if(pcsPara->m_3g.m_changeinfo & 0x20)
			{
				for(i=0; i<15; i++)
				{
					if( plocalPara->m_3g.phonenum[i] != pcsPara->m_3g.phonenum[i] )
					{
						plocalPara->m_3g.phonenum[i] = pcsPara->m_3g.phonenum[i];
						needUpdate++;
					}
				}
				plocalPara->m_3g.phonenum[15] = 0;
			}
			if(pcsPara->m_3g.m_changeinfo & 0x40)
			{
				if( plocalPara->m_3g.g3TypeSelCtl != pcsPara->m_3g.g3TypeSelCtl )
				{
					plocalPara->m_3g.g3TypeSelCtl = pcsPara->m_3g.g3TypeSelCtl;
					needUpdate++;
				}
			}

			if(needUpdate)
			{
				memcpy(&g3cfg, &plocalPara->m_3g, sizeof(G3G_CONFIG));
				g_cParaManage->SetSystemParameter(SYS3G_SET,&plocalPara->m_3g);
			}			
		}	
		
		if(pcsPara->m_3gDial.m_changeinfo)
		{
			g_cParaManage->GetSysParameter(SYS3G_DIAL_SET,&plocalPara->m_3gDial);
			int needUpdate = 0;

			if(pcsPara->m_3gDial.m_changeinfo & 0x01)
			{
				if( plocalPara->m_3gDial.g3mandialEnable != pcsPara->m_3gDial.g3mandialEnable)
				{
					plocalPara->m_3gDial.g3mandialEnable = pcsPara->m_3gDial.g3mandialEnable;
					needUpdate++;
				}
			}
			if(pcsPara->m_3gDial.m_changeinfo & 0x02)
			{
				for(i=0; i<16; i++)
				{
					if( plocalPara->m_3gDial.g3UserName[i] != pcsPara->m_3gDial.g3UserName[i] )
					{
						plocalPara->m_3gDial.g3UserName[i] = pcsPara->m_3gDial.g3UserName[i];
						needUpdate++;
					}
				}
				plocalPara->m_3gDial.g3UserName[16] = 0;
			}
			if(pcsPara->m_3gDial.m_changeinfo & 0x04)
			{
				for(i=0; i<16; i++)
				{
					if( plocalPara->m_3gDial.g3Passwrd[i] != pcsPara->m_3gDial.g3Passwrd[i] )
					{
						plocalPara->m_3gDial.g3Passwrd[i] = pcsPara->m_3gDial.g3Passwrd[i];
						needUpdate++;
					}
				}
				plocalPara->m_3gDial.g3Passwrd[16] = 0;
			}
			if(pcsPara->m_3gDial.m_changeinfo & 0x08)
			{
				for(i=0; i<16; i++)
				{
					if( plocalPara->m_3gDial.g3CountryCode[i] != pcsPara->m_3gDial.g3CountryCode[i] )
					{
						plocalPara->m_3gDial.g3CountryCode[i] = pcsPara->m_3gDial.g3CountryCode[i];
						needUpdate++;
					}
				}
				plocalPara->m_3gDial.g3CountryCode[16] = 0;
			}

			if(needUpdate)
			{
				memcpy(&g3dcfg, &plocalPara->m_3gDial, sizeof(G3G_DIAL_CONFIG));
				g_cParaManage->SetSystemParameter(SYS3G_DIAL_SET,&plocalPara->m_3gDial);
			}	
		}
		
	}

	return needreboot;
	#endif
	return 0;
}


