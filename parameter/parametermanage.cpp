
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/dir.h>

#include "parametermanage.h"
#include "ModuleFuncInterface.h"

extern int	startupdate;
extern DeviceConfigInfo 	ConfigInfo;

PARAMETER_MANAGE *PARAMETER_MANAGE::m_pInstance = NULL;


PARAMETER_MANAGE *PARAMETER_MANAGE::Instance()
{
	if(NULL == m_pInstance)
	{	
		m_pInstance = new PARAMETER_MANAGE();
	}

	return m_pInstance;
}

int PARAMETER_MANAGE::GetMacAddr(SYSTEM_PARAMETER *para)
{
/*
	int retval = -1;
	int fd = -1;
	char mac[6] ={0};

	fd = open("/var/devicemac", O_RDONLY);
	if(fd == S_FAILURE)
	{
		printf("Mac file open failure use default\n");
		return S_FAILURE;
	}
	retval = read(fd, mac, 6);
	printf("%d %0x:%0x:%0x:%0x:%0x:%0x\n",retval,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	if(retval == 6)
	{
		memcpy(m_syspara->m_NetWork.m_Eth0Config.m_uMac,mac,6);
	}
	else
	{
		printf("MAC Format err\n");
	}
	return retval;
*/
	int retval = -1;
	int fd = -1;
	char mac[6] = {0} ;
	char deviceid[16] = {0} ;

	fd = open(MAC_ADDR_FILE, O_RDONLY);
	if(fd == S_FAILURE)
	{
		fd = open(MAC_ADDR_FILE,  O_CREAT|O_RDWR,0777);		
		//写入mac地址
		if( fd >= 0 )
		{
			memcpy(mac,m_syspara->m_NetWork.m_Eth0Config.m_uMac,6);
			retval = write(fd , mac , 6);
			#ifdef ID15_EXPANDTO
			retval = write(fd , m_syspara->m_NetWork.m_CenterNet.deviceid, 15);
			#endif
			if( retval > 0 )
			{
				sync();
			}
			close(fd);
			fd = -1;
		}
		printf("Mac file open failure use default\n");
		printf("######## %0x:%0x:%0x:%0x:%0x:%0x\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		return S_SUCCESS;
	}
	retval = read(fd, mac, 6);
	printf("%d %0x:%0x:%0x:%0x:%0x:%0x\n",retval,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	if(retval == 6)
	{
		memcpy(m_syspara->m_NetWork.m_Eth0Config.m_uMac,mac,6);
	}
	else
	{
		printf("MAC Format err\n");
	}
	
#ifdef ID15_EXPANDTO
	retval = read( fd , deviceid , 15 ) ;
	close( fd ) ;
	printf( "deviceid = %s\r\n" , deviceid ) ;
	LoadDeviceID(&(m_syspara->m_NetWork));
#else

	retval = read( fd , deviceid , 15 ) ;
	printf( "deviceid = %s\r\n" , deviceid ) ;
	if(retval == 15)
	{
		memcpy(m_syspara->m_NetWork.m_CenterNet.deviceid , deviceid , 15 );
	}
	else
	{
		memset(m_syspara->m_NetWork.m_CenterNet.deviceid , 0 , 15 );
		printf("Device Format err\n");
	}
	
	//add by table close the file
	close( fd ) ;
#endif	
	return retval;
	
}

PARAMETER_MANAGE::PARAMETER_MANAGE()
{
	m_config_fd = -1;
	m_syspara = NULL;
	InitWriteLock();
	m_syspara = (SYSTEM_PARAMETER *)malloc(sizeof(SYSTEM_PARAMETER));
	if(m_syspara == NULL)
	{
		printf(" malloc system para space null \n");
	}
	else 
	{
		memset(m_syspara, 0, sizeof(SYSTEM_PARAMETER));
		LoadParameterFromFile(m_syspara);
	}
	//读取MAC地址并设置
	GetMacAddr(m_syspara);
	if(get_network_support()!=NET_WORK_CARD_LOCAL)
		GetWifiMac(m_syspara->m_NetWork.m_WifiConfig.WifiAddrMode.m_uMac);
		
	InitDeviceConfigInfo();
	InitDeviceAesKey();
}

PARAMETER_MANAGE::~PARAMETER_MANAGE()
{
	if(m_syspara )
	{
		free(m_syspara);
		m_syspara = NULL;
	}
}

int  PARAMETER_MANAGE::SaveSystemParameter(SYSTEM_PARAMETER *para)
{
	int retval = S_FAILURE;
	
	if(para)
	{
		AddWriteLock();
		
		memcpy(m_syspara, para, sizeof(SYSTEM_PARAMETER));
		retval = SaveParameter2File(para);
		
		ReleaseWriteLock();
	}

	return retval;
}

int PARAMETER_MANAGE::ReadSystemParameter(SYSTEM_PARAMETER *para)
{
	int retval = S_FAILURE;
	
	if(para)
	{
		memcpy(para, m_syspara, sizeof(SYSTEM_PARAMETER));
		retval = S_SUCCESS;
	}
	
	return retval;
	
}

int PARAMETER_MANAGE::SaveParameter2File(SYSTEM_PARAMETER *para)
{
	int retval = -1;
	
	if(para == NULL)
	{
		printf("parameter pionter null \n");
		return S_FAILURE;
	}

	m_config_fd = open(PARA_FILENAME, O_RDWR|O_CREAT, 0777);
	if(m_config_fd == S_FAILURE)
	{
		printf("config file open failure \n");
		return S_FAILURE;
	}
	retval = write(m_config_fd, para, sizeof(SYSTEM_PARAMETER));
	if(retval == -1)
		printf("save para to file errno[%d] :%s \n", errno, strerror(errno));
	fsync(m_config_fd);
	printf("Save Parameter\n");
	close(m_config_fd);
	m_config_fd = -1;
	
	return retval;
	
}

SYSTEM_PARAMETER *PARAMETER_MANAGE::GetParameterPtr()
{
	
	return m_syspara;
}

int PARAMETER_MANAGE::LoadParameterFromFile(SYSTEM_PARAMETER *para)
{
	int retval = -1;

	if(para == NULL)
	{
		printf("%s parameter pionter null\n", __FUNCTION__);
		return S_FAILURE;
	}

	printf(" parameter size is : %d \n", sizeof(SYSTEM_PARAMETER));
	
	m_config_fd = open(PARA_FILENAME, O_RDONLY);
	if(m_config_fd == -1)
	{
		printf("LoadParameterDefault\n");
		retval = LoadParameterDefault(para); 
	}
	else 
	{
		
		retval = read(m_config_fd, para, sizeof(SYSTEM_PARAMETER));
		if(retval != S_FAILURE && 0 != retval) //added by panjy 0 != retval 如果读到的数据长度为0
		{
			#if 0
				if((para->m_NetWork.m_WifiConfig.LoginWifiDev.Smartlink) == 0 )
				{
					(para->m_NetWork.m_WifiConfig.LoginWifiDev.Smartlink) = 1;
				}
				else
				{
					//do nothing
				}
			
			#endif
			CheckSystemParameter(para);
		}
		else 
		{
			LoadParameterDefault(para);
		}
		close(m_config_fd);
		m_config_fd = -1;	
	}
	
	printf( "upnp port =%d     %d      %d \r\n" , para->m_NetWork.m_Eth0Config.m_http_port ,
		para->m_NetWork.m_Eth0Config.m_v_port ,para->m_NetWork.m_Eth0Config.m_phone_port ) ;
	
	if( para->m_NetWork.m_Eth0Config.m_http_port < 1024 )
	{
		struct timeval tv_now ;
		gettimeofday( &tv_now , NULL ) ;

		int upnpPort = tv_now.tv_usec % 60000 + 1024 ;
		para->m_NetWork.m_Eth0Config.m_http_port = upnpPort ;
		para->m_NetWork.m_Eth0Config.m_v_port = upnpPort + 1;
		para->m_NetWork.m_Eth0Config.m_phone_port = upnpPort + 2 ;

		printf( "**************change upnp port !!***********\r\n") ;
		SaveParameter2File( para ) ;
	}
	
	return retval;
}


int PARAMETER_MANAGE::LoadParameterDefault(SYSTEM_PARAMETER *para)
{

	LoadNetWorkDefault(&(para->m_NetWork));

	LoadAnalogDefault(&(para->m_Analog));

	LoadCameraBlindDefault(&(para->m_CamerBlind));

	LoadDefaultMDParameter(&(para->m_CameraMd));

	LoadCameraSetDefault(&(para->m_Camera));

	LoadMachineParaDefault(&(para->m_Machine));

	LoadNormalParaDefault(&(para->m_CommPara));
	
	LoadDefaultUserParameter(&para->m_Users);

	LoadDefaultSysMainetanceParaMeter(&(para->m_SysMainetance));

	LoadDefaultDisplaySetParameter(&(para->m_DisplaySet));

	LoadDefaultExceptHandleParameter(&(para->m_SysExcept));

	LoadDefaultPcDirParameter(&(para->m_PcDir));

	LoadRecordTaskDefaultParameter(&(para->m_RecordSchedule));

	LoadPTZParameter(&(para->m_PTZ));

	LoadAlarmZoneParameter(&(para->m_ZoneGroup));

	LoadDefaultPELCOParameter(&(para->m_pelcoCfg));
	
	LoadDefaultPicTimerParameter(&(para->m_picTimer));
	
	LoadDefaultVoPicParameter(&(para->m_picVo));
	
	//LoadPtzLinkSet(&(para->m_PtzLink));

	//added by panjy
	LoadDefaultOsdInsertParameter(&(para->m_OsdInsert));
	//end
	LoadCamSensorParameter(&(para->m_Sensor));

	LoadExtendApSettings(&(para->m_ParaExtend));	
	LoadWebSetDefault(&(para->m_web));
	LoadMdSetDefault(&(para->m_mdset));
	return S_SUCCESS;
	
}
int PARAMETER_MANAGE::LoadWebSetDefault(web_sync_param_t *pweb)
{
	if(pweb != NULL)
	{
		pweb->sync_key[0]='0';
		strcpy(pweb->time_zone,"UTC");
		pweb->mute =1;
		pweb->alarm_interval =900;

		pweb->device_on =1;
		pweb->device_schedule =0;
		
		pweb->nightvision_switch = 1;
		pweb->imageflip_switch = 0;

		pweb->cvr_on =0;
		pweb->sensitivity =1;

		#ifdef PT_IPC
		pweb->ircut_admax=45;
		pweb->ircut_admin=138;
		#endif 
		return S_SUCCESS;
	}
	return S_FAILURE;
}
int PARAMETER_MANAGE::LoadMdSetDefault(P2P_MD_REGION_CHANNEL *pmd)
{
	if(pmd != NULL)
	{
		pmd->x = 0;
		pmd->y = 0;
		pmd->width= 1;
		pmd->height = 1;
		return S_SUCCESS;
	}
	return S_FAILURE;
}

int PARAMETER_MANAGE::LoadExtendApSettings(PARAMETEREXTEND *pApSettings)
{
	if(pApSettings != NULL)
	{
		pApSettings->m_ntp.m_daylight_switch = 0;
#ifdef WIFI_18E_IPC

		pApSettings->m_ntp.m_ntp_switch = 1;  //miniIPC无RTC 需ntp校正时间
#else
        pApSettings->m_ntp.m_ntp_switch = 0;
#endif
		pApSettings->m_ntp.m_idx_tzname = 35;
		pApSettings->m_ntp.m_diff_timezone = 0;		
		memset(pApSettings->m_MailExt.m_addr1,0x0,sizeof(pApSettings->m_MailExt.m_addr1));
		memset(pApSettings->m_MailExt.m_addr2,0x0,sizeof(pApSettings->m_MailExt.m_addr2));

		return S_SUCCESS;
	}
	return S_FAILURE;
}
int PARAMETER_MANAGE::LoadDeviceID(NETWORK_PARA *network)
{
	struct timeval tv_now ;
	char tmpid[16]={0x0};	
	char  id[32]={0x0};
	int retval=-1;
	int fd =-1;
	int i =0;
	int j=0;

	while(j<6)
	{
		if((network->m_Eth0Config.m_uMac[j]>>4)>9)
		{
			
			tmpid[i++]=(network->m_Eth0Config.m_uMac[j]>>4)+'A'-10;
		}				
		else
		{
			
			tmpid[i++]=(network->m_Eth0Config.m_uMac[j]>>4)+'0';
			
		}
				
			
		if((network->m_Eth0Config.m_uMac[j]&0x0f)>9)
		{	
			
			tmpid[i++]=(network->m_Eth0Config.m_uMac[j]&0x0f)+'A'-10;
			
		}
		else
		{
			
			tmpid[i++]=(network->m_Eth0Config.m_uMac[j]&0x0f)+'0';	
			
		}
		
		j++;
	}

	fd = open(MAC_ADDR_FILE, O_RDWR);
	if(fd == S_FAILURE)
	{
		fd = open(MAC_ADDR_FILE,  O_CREAT|O_RDWR,0777);	
		//memcpy(mac,m_syspara->m_NetWork.m_Eth0Config.m_uMac,6);		 
		for(i=12;i<15;i++)
		{
			gettimeofday( &tv_now , NULL ) ;
			tmpid[i]=tv_now.tv_usec%25+'A' ;				
			usleep(200);
		}
		memcpy(network->m_CenterNet.deviceid,tmpid,15);
		write(fd , m_syspara->m_NetWork.m_Eth0Config.m_uMac , 6);
		write(fd , network->m_CenterNet.deviceid , 15);
		printf("---------------------------------\n");
		printf("ID:%s\n",tmpid);
		printf("MAC:%02x:%02x:%02x:%02x:%02x:%02x\n",network->m_Eth0Config.m_uMac[0],network->m_Eth0Config.m_uMac[1],network->m_Eth0Config.m_uMac[2],network->m_Eth0Config.m_uMac[3],network->m_Eth0Config.m_uMac[4],network->m_Eth0Config.m_uMac[5]);
		printf("---------------------------------");
		return 0;
	}
	else
	{	
		
		/*6 mac + 12 id +3 id*/
		retval = read(fd,id,21);//id里面包含了设备ID+MAC地址
		//后三位是否合法
		bool	available  = false;
		if((id[18]>='A')&&(id[18]<='Z')\
			&&(id[19]>='A')&&(id[19]<='Z')\
			&&(id[20]>='A')&&(id[20]<='Z')\
			&&retval==21)
		{
			//memcpy(tmpid,&id[6],15);
			//memcpy(network->m_CenterNet.deviceid,tmpid,15);
			available = true;
			//int i=0;
			for(i=0;i<21;i++)
			printf("%02x \n",id[i]);
			printf("\n-----ID:%s is   available\n",tmpid);
		}
		
		if((memcmp(tmpid,id+6,12)!=0)||(available==false))	//从第7位起是ID，前6位是MAC
		{
			
			for(i=12;i<15;i++)
			{
				gettimeofday( &tv_now , NULL ) ;
				tmpid[i]=tv_now.tv_usec%25+'A' ;				
				usleep(200);
			}
			
			memcpy(network->m_CenterNet.deviceid,tmpid,15);			
	        if(-1 == lseek(fd, 0, SEEK_SET)) 
            { 
              //  close(fd); 
                printf("----- %s,lseek error \n",__FUNCTION__); 
              //  return -1; 
            }
			retval=write(fd , m_syspara->m_NetWork.m_Eth0Config.m_uMac , 6);
			retval=write(fd , network->m_CenterNet.deviceid , 15);			
			printf("----ID is error need Reset :%s,retval:%d,fd:%d\n",tmpid,retval,fd);
			if( retval > 0 )
			{
				sync();
			}
			
		}
		else
		{
			memcpy(network->m_CenterNet.deviceid,id+6,15);	
		}
		
		printf("----ID is:%s\n",network->m_CenterNet.deviceid);

	}
	close(fd);	
	return 0;

}

int PARAMETER_MANAGE::LoadNetWorkDefault(NETWORK_PARA *network)
{
	int i = 0;

	if(network !=NULL)
	{
		memset(network, 0, sizeof(NETWORK_PARA));
#if 1
		network->m_Eth0Config.m_uGateWay[0] = 192;//192;// 172;//172;//192;
		network->m_Eth0Config.m_uGateWay[1] = 168;//168;//18;//168;
		network->m_Eth0Config.m_uGateWay[2] = 1;//0;//16;
		network->m_Eth0Config.m_uGateWay[3] = 1;
		network->m_Eth0Config.m_uLocalIp[0] = 192;//192;//172;//192;
		network->m_Eth0Config.m_uLocalIp[1] = 168;//168;//168;
		network->m_Eth0Config.m_uLocalIp[2] = 1;//0;//0;
		network->m_Eth0Config.m_uLocalIp[3] = 156;//100;// 112;//100;

#else
		network->m_Eth0Config.m_uGateWay[0] = 172;//192;// 172;//172;//192;
		network->m_Eth0Config.m_uGateWay[1] = 18;//168;//18;//168;
		network->m_Eth0Config.m_uGateWay[2] = 16;//0;//16;
		network->m_Eth0Config.m_uGateWay[3] = 1;
		network->m_Eth0Config.m_uLocalIp[0] = 172;//192;//172;//192;
		network->m_Eth0Config.m_uLocalIp[1] = 18;//168;//168;
		network->m_Eth0Config.m_uLocalIp[2] = 16;//0;//0;
		network->m_Eth0Config.m_uLocalIp[3] = 178;//100;// 112;//100;
#endif
		
		network->m_Eth0Config.m_uMask[0] = 255;
		network->m_Eth0Config.m_uMask[1] = 255;
		network->m_Eth0Config.m_uMask[2] = 255;
		network->m_Eth0Config.m_uMask[3] = 0;

		network->m_WifiConfig.WifiAddrMode.m_dhcp = OPEN_DHCP;// OPEN_DHCP;
		printf("WifiAddrMode m_dhcp:%d\n",network->m_WifiConfig.WifiAddrMode.m_dhcp);
		network->m_WifiConfig.WifiAddrMode.m_u8Selected = 1;
		network->m_WifiConfig.WifiAddrMode.m_uGateWay[0] = 192;
		network->m_WifiConfig.WifiAddrMode.m_uGateWay[1] = 168;
		network->m_WifiConfig.WifiAddrMode.m_uGateWay[2] = 1;
		network->m_WifiConfig.WifiAddrMode.m_uGateWay[3] = 1;
		network->m_WifiConfig.WifiAddrMode.m_uLocalIp[0] = 192;
		network->m_WifiConfig.WifiAddrMode.m_uLocalIp[1] = 168;
		network->m_WifiConfig.WifiAddrMode.m_uLocalIp[2] = 1;
		network->m_WifiConfig.WifiAddrMode.m_uLocalIp[3] = 157;
		network->m_WifiConfig.WifiAddrMode.m_uMask[0] = 255;
		network->m_WifiConfig.WifiAddrMode.m_uMask[1] = 255;
		network->m_WifiConfig.WifiAddrMode.m_uMask[2] = 255;
		network->m_WifiConfig.WifiAddrMode.m_uMask[3] = 0;
		network->m_WifiConfig.WifiAddrMode.m_umDNSIp[0] = 202;
		network->m_WifiConfig.WifiAddrMode.m_umDNSIp[1] = 96;
		network->m_WifiConfig.WifiAddrMode.m_umDNSIp[2] = 134;
		network->m_WifiConfig.WifiAddrMode.m_umDNSIp[3] = 133;
		network->m_WifiConfig.WifiAddrMode.m_usDNSIp[0] = 8;
		network->m_WifiConfig.WifiAddrMode.m_usDNSIp[1] = 8;
		network->m_WifiConfig.WifiAddrMode.m_usDNSIp[2] = 8;
		network->m_WifiConfig.WifiAddrMode.m_usDNSIp[3] = 8;
		network->m_WifiConfig.LoginWifiDev.m_Reserved =1;
		#if 1
		//strcpy(network->m_WifiConfig.LoginWifiDev.RouteDeviceName,"zmodo_nvr" );
		//strcpy(network->m_WifiConfig.LoginWifiDev.Passwd,"12345678" );
		//strcpy(network->m_WifiConfig.LoginWifiDev.RouteDeviceName,"ZMDTEST-D-NOBLE" );
		//strcpy(network->m_WifiConfig.LoginWifiDev.Passwd,"noble1234" );
		strcpy(network->m_WifiConfig.LoginWifiDev.RouteDeviceName,"" );
		strcpy(network->m_WifiConfig.LoginWifiDev.Passwd,"" );	
		network->m_WifiConfig.LoginWifiDev.AuthenticationMode = 0;
		network->m_WifiConfig.LoginWifiDev.EncryptionProtocol = 2;
		//network->m_WifiConfig.LoginWifiDev.smart_flag = 0;
		network->m_WifiConfig.LoginWifiDev.lang =0;
		network->m_WifiConfig.LoginWifiDev.Smartlink = 1;
		#endif	
		//for test
		network->m_Eth0Config.m_dhcp= OPEN_DHCP;
		network->m_Eth0Config.m_upnp = OPEN_DHCP;
		network->m_Eth0Config.m_http_port = 0;
		network->m_Eth0Config.m_v_port = 0;
		network->m_Eth0Config.m_plat_port = 0;
		network->m_Eth0Config.m_phone_port = 0;
		
		/*mac 地址烧片时前两位位固定后三位随机 */
		if(access(MAC_ADDR_FILE,0)==-1)
		{
			struct timeval tv_now ;
			network->m_Eth0Config.m_uMac[0] =0x04;
			network->m_Eth0Config.m_uMac[1] =0x5c;
			network->m_Eth0Config.m_uMac[2] =0x06;
			int j =3;
			for(;j<6;j++)
			{
				gettimeofday( &tv_now , NULL ) ;
				unsigned char  mac = tv_now.tv_usec%256 ;	
				network->m_Eth0Config.m_uMac[j] =mac;
				usleep(200);
			}
		}


		//for(i = 0; i < 6; i++)
		//{
		//	network->m_Eth0Config.m_uMac[i] = i;
		//	if(i!=0)network->m_Eth0Config.m_uMac[5] += 2;
		//}
		
		network->m_Eth1Config.m_uGateWay[0] = 192;
		network->m_Eth1Config.m_uGateWay[1] = 168;
		network->m_Eth1Config.m_uGateWay[2] = 27;
		network->m_Eth1Config.m_uGateWay[3] = 1;
		network->m_Eth1Config.m_uLocalIp[0] = 192;
		network->m_Eth1Config.m_uLocalIp[1] = 168;
		network->m_Eth1Config.m_uLocalIp[2] = 27;
		network->m_Eth1Config.m_uLocalIp[3] = 82;
		
		network->m_Eth1Config.m_uMask[0] = 255;
		network->m_Eth1Config.m_uMask[1] = 255;
		network->m_Eth1Config.m_uMask[2] = 255;
		network->m_Eth1Config.m_uMask[3] = 0;
		for(i = 0; i < 6; i++)
		{
			network->m_Eth1Config.m_uMac[i] = i*2;
		}

		network->m_CenterNet.m_uPhoneListenPt = 9000;
		network->m_CenterNet.m_uVideoListenPt = 8000;
		network->m_CenterNet.m_uHttpListenPt = 80;

		network->m_CenterNet.m_heartbeat = 60*30;

		network->m_DNS.m_umDNSIp[0] = 8;
		network->m_DNS.m_umDNSIp[1] = 8;
		network->m_DNS.m_umDNSIp[2] = 8;
		network->m_DNS.m_umDNSIp[3] = 8;

		network->m_DNS.m_usDNSIp[0] = 202;
		network->m_DNS.m_usDNSIp[1] = 96;
		network->m_DNS.m_usDNSIp[2] = 134;
		network->m_DNS.m_usDNSIp[3] = 133;

		network->m_DomainConfig.m_server = 5;// 1:表示3322.org, 2:表示dynDDNS.org 3:88ip 4:no-ip 5:zmododdns
						
		network->m_email.m_u8Sslswitch =1;
		network->m_email.m_u16SslPort =465;
		
	}

	return S_SUCCESS;
	
}

int PARAMETER_MANAGE::LoadSecurityDefault(PASSWORD_PARA *security)
{
	if(security == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	security->m_uAdminPassword= 888888;
	security->m_uOperatePassword= 222222;
	security->m_uPwdEnable = 0;// 密码关闭

	return S_SUCCESS;

}

int PARAMETER_MANAGE::LoadRecTaskDefault(REC_TASK_PARA *task)
{
#if 1
	int i = 0;
	
	if(task == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	memset(task, 0, sizeof(REC_TASK_PARA));
	for(i = 0; i < 8; i++)
	{
		task->m_TimerTask[i].m_uWeekDay= 8;
		task->m_TimerTask[i].m_uWeekDay = 8;
		task->m_TimerTask[i].m_uTimeTbl[0] = 60*23+59;
		task->m_TimerTask[i].m_uTrigeType = 0;

	}


#endif 	
	return S_SUCCESS;

}

int  PARAMETER_MANAGE::LoadMachineParaDefault(MACHINE_PARA *machine)
{
	if(machine == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}	

	memset(machine, 0, sizeof(MACHINE_PARA));

	machine->m_uTvSystem = 1; //pal 制式
	machine->m_uHddOverWrite = 1; // 硬盘覆盖打开

	#ifdef IPC720P
	machine->m_uHddorSD = 1;//IPC 支持SD卡 panjy
	#else
	machine->m_uHddorSD = 0;//默认硬盘录像
	#endif

	return S_SUCCESS;
}

int PARAMETER_MANAGE::LoadCameraSetDefault(CAMERA_PARA *cameraset)
{
	int i = 0;
	char string[10] = {0};
	
	if(cameraset == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	memset(cameraset, 0, sizeof(CAMERA_PARA));

	for(i = 0; i < CHANNEL_MAX; i++)
	{
		memset(string, 0, 10);	
		sprintf(string, "CH%d", i+1);
		strncpy(cameraset->m_ChannelPara[i].m_Title, string, 16);
		cameraset->m_ChannelPara[i].m_uChannelValid = 1;
		cameraset->m_ChannelPara[i].m_uAudioSwitch = 1;
		cameraset->m_ChannelPara[i].m_uEncType = 0; //*可变码率
		cameraset->m_ChannelPara[i].m_uFrameRate = 20; //*full frame rate ;
		cameraset->m_ChannelPara[i].m_uQuality = 1; //*best quality;
		#ifdef HI1080P_IPC
        cameraset->m_ChannelPara[i].m_uResolution = 4;
        #else
    		#ifdef IPCVGA
    		cameraset->m_ChannelPara[i].m_uResolution = 6;  // 清晰度  0:D1, 1: HD1, 2:CIF 3:QCIF 4:1080P 5:720P 6:VGA 7:QVGA
    		#endif
    		#ifdef IPC720P
    		cameraset->m_ChannelPara[i].m_uResolution = 5;  // 清晰度  0:D1, 1: HD1, 2:CIF 3:QCIF 4:1080P 5:720P 6:VGA 7:QVGA
    		#endif
        #endif
		cameraset->m_ChannelPara[i].m_TimeSwitch = 1;
		cameraset->m_ChannelPara[i].m_TltleSwitch = 0;
		cameraset->m_ChannelPara[i].m_u8PreRecTime = 10;
		cameraset->m_ChannelPara[i].m_u16RecDelay = 20;
		cameraset->m_ChannelPara[i].m_uSubEncSwitch = 1;
		cameraset->m_ChannelPara[i].m_uSubAudioEncSwitch = 0;
		cameraset->m_ChannelPara[i].m_uSubFrameRate = 12;
		cameraset->m_ChannelPara[i].m_uSubQuality = 1;
		cameraset->m_ChannelPara[i].m_uSubRes = 7;
	}


	return S_SUCCESS;

}

int PARAMETER_MANAGE::LoadAnalogDefault(CAMERA_ANALOG *analog)
{
	int i = 0;

	if(analog == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}	

	for(i = 0; i < CHANNEL_MAX; i++)
	{
		analog->m_Channels[i].m_nBrightness= Def_Brightness;
		analog->m_Channels[i].m_nContrast= Def_Contrast;
		analog->m_Channels[i].m_nHue= Def_Hue;
		analog->m_Channels[i].m_nSaturation= Def_Saturation;

	}

	return S_SUCCESS;
	
}

int PARAMETER_MANAGE::LoadAlarmSetDefault(ALARM_PARA *alarm)
{
#if 0
	if(alarm == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	memset(alarm, 0, sizeof(ALARM_PARA));

	alarm->m_nAlarmOutDelay = 30;

	alarm->m_nAlarmRecDelay = 30;

	alarm->m_nPreRecTime = 10;

	alarm->m_nBuzzeDelay = 0;
#endif 

	return S_SUCCESS;
}

int PARAMETER_MANAGE::LoadSysRunInfoDefault(SYSTEM_RUNINFO *systeminfo)
{

	if(systeminfo == NULL)
	{

		return S_FAILURE;
	}

	memset(systeminfo, 0, sizeof(SYSTEM_RUNINFO));

	return S_SUCCESS;

}

int PARAMETER_MANAGE::LoadSensorSetDefault(SENSOR_PARA *sensor_alarm)
{

#if 0
	int i = 0;
	char string[10] = {0};
	
	if(sensor_alarm == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	memset(sensor_alarm, 0, sizeof(SENSOR_PARA));
	
	for(i = 0; i < 9; i++)
	{
		memset(string, 0, 10);
		sprintf(string, "S%02d", i+1);
		strcpy(sensor_alarm->m_SensorIO.m_cTitle[i].m_cTitle, string);
	}
#endif 
	//sensor_alarm->s_speed.m_max_limit = 200;
//	sensor_alarm->s_speed.m_min_limit = 200;

	return S_SUCCESS;
	
}

#if 0
int PARAMETER_MANAGE::LoadVechileSetDefault(VECHIL_SETTING *vechile)
{
	if(vechile == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	memset(vechile, 0, sizeof(VECHIL_SETTING));
	for(int i = 0; i < 3; i++)
	{
		switch(i)
		{
			case 0:
				strcpy(vechile->m_companyname.m_title, "AAAAAAAAAAAAAAAA");
				break;
			case 1:
				strcpy(vechile->m_drivername.m_title, "BBBBBBBBBBBBBBBB");
				break;
			case 2:
				strcpy(vechile->m_vechil_number.m_title, "CCCCCCCCCCCCCCCC");
				break;
		}
	}
	
	return S_SUCCESS;
	
}

#endif 

int  PARAMETER_MANAGE::LoadPowerManageDefault(POWER_MANAGE *pw_manage)
{
	if(pw_manage == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	memset(pw_manage, 0, sizeof(POWER_MANAGE));

	pw_manage->m_uShutDelay = 10;
	
	return S_SUCCESS;
}

int PARAMETER_MANAGE::LoadNormalParaDefault(COMMON_PARA *common_para)
{
	int lan = 0;
	if(common_para == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}
	lan = common_para->m_language;
	memset(common_para, 0, sizeof(COMMON_PARA));
	common_para->m_uDateMode = 2;

	common_para->m_uTimeMode =1;

	common_para->m_uFilePacketLen = 3;
	
	common_para->m_uIdleTime = 1;

	common_para->m_uWeekDayStart = 1;
	
	common_para->m_uWeekDayEnd = 5;
	
	common_para->m_uTimeInSert = 1;

	common_para->m_language = lan;

	common_para->m_uDiaphaneity= 7;
	
	return S_SUCCESS;
}

int PARAMETER_MANAGE::LoadCameraBlindDefault(CAMERA_BLIND *bcd)
{
	if(bcd == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}
	
	memset(bcd, 0, sizeof(CAMERA_BLIND));

	return S_SUCCESS;
	
}
// add by HY
int PARAMETER_MANAGE::LoadAlarmInParameter(ALARMZONEGROUPSET *ai_set)
{
	int i = 0;
	
	if(ai_set == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	memset(ai_set, 0, sizeof(ALARMZONEGROUPSET));

	for(i=0; i < MAX_ALARM_ZONE; i++)
	{
	    ai_set->m_AlarmZone[i].m_OutputTime = 2;
	    ai_set->m_AlarmZone[i].m_DetectTime = 2;
	}
	    
	return S_SUCCESS;
}

int PARAMETER_MANAGE::LoadDefaultVideoLossParameter( CAMERA_VideoLoss *vl_set)
{	
	if(vl_set == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	memset(vl_set, 0, sizeof(CAMERA_MD));
		
	return S_SUCCESS;
}

int PARAMETER_MANAGE::LoadDefaultBDParameter( CAMERA_BLIND *bd_set)
{
	int i = 0;
	
	if(bd_set == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	memset(bd_set, 0, sizeof(CAMERA_BLIND));

	for(i = 0; i < CHANNEL_MAX; i++)
	{
		bd_set->m_Channel[i].m_uBlindSensitive = 2;
	}
		
	return S_SUCCESS;
}
// add by HY end
//#define MDDEBUG
int PARAMETER_MANAGE::LoadDefaultMDParameter( CAMERA_MD *md_set)
{
	int i = 0;
	
	if(md_set == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	memset(md_set, 0, sizeof(CAMERA_MD));

	for(i = 0; i < CHANNEL_MAX; i++)
	{
#if 0
		md_set->m_Channel[i].m_uMDSwitch = 1;
		for(int j = 0;j < 100;j++)
			md_set->m_Channel[i].m_MDMask[j] = 1;
		for(int k = 0;k< 8;k++)
		{
			md_set->m_Channel[i].m_TimeTblSet[k].m_TBLSection[0].m_u16StartTime = 0;
			md_set->m_Channel[i].m_TimeTblSet[k].m_TBLSection[0].m_u16EndTime = 60*23+59;
			md_set->m_Channel[i].m_TimeTblSet[k].m_TBLSection[0].m_u8Valid = 1;
		}
		md_set->m_Channel[i].m_uAlarmInterval =5;
		md_set->m_Channel[i].m_uOutputDelay =1;
		md_set->m_Channel[i].m_uMDSensitive =1;
		md_set->m_Channel[i].m_uAalarmOutMode = md_set->m_Channel[i].m_uAalarmOutMode|0x48;
#endif

	}
		
	return S_SUCCESS;
}

int PARAMETER_MANAGE::LoadDefaultAlarmInParameter(GROUPALARMINSET *AlarmIn)
{
	int i = 0;

	if(AlarmIn == NULL)
	{

		return S_FAILURE;
	}

	for(i = 0; i < 4; i++)
	{
		memset(&(AlarmIn->m_AlarmIn[i]), 0, sizeof(ALARMINSET));
		AlarmIn->m_AlarmIn[i].m_u8AlarmValid = 1;
		AlarmIn->m_AlarmIn[i].m_u32RecSel = CHANNEL_MASK;
		AlarmIn->m_AlarmIn[i].m_u32AlarmOutSel = 0x0f;
		AlarmIn->m_AlarmIn[i].m_u32AlarmHandle = 0x01;
	}

	return S_SUCCESS;
	
}


int PARAMETER_MANAGE::LoadDefaultUserParameter(USERGROUPSET  *UserSet)
{
	int Id = 0;

	if(UserSet == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}


	/*所有的用户无效*/
	for(Id = 0; Id < 16; Id++)
	{
		memset(&UserSet->m_UserSet[Id], 0, sizeof(SINGLEUSERSET));
	}

	strcpy(UserSet->m_UserSet[0].m_cUserName, "admin");
	strcpy(UserSet->m_UserSet[0].m_s32Passwd, "111111");
	
	return S_SUCCESS;	

}

int PARAMETER_MANAGE::LoadDefaultSysMainetanceParaMeter(SYSTEMMAINETANCE *Mainetance)
{

	if(Mainetance == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	// 从不进行系统维护
	memset(Mainetance, 0, sizeof(SYSTEMMAINETANCE));

	return S_SUCCESS;
	
}

int PARAMETER_MANAGE::LoadDefaultDisplaySetParameter(VIDEODISPLAYSET *DisplaySet)
{

	if(DisplaySet == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	// 默认不录像，显示为最大通道。
	memset(DisplaySet, 0, sizeof(VIDEODISPLAYSET));
	DisplaySet->m_u8VGAMode = 1;

	return S_SUCCESS;
	
}


int  PARAMETER_MANAGE::LoadDefaultExceptHandleParameter(GROUPEXCEPTHANDLE *ExceptHandle)
{
	int i = 0;

	if(ExceptHandle == NULL)
	{
		printf(" failure %s, %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	// 系统异常处理
	for(i = 0; i < 8; i++)
	{
		ExceptHandle->m_ExceptHandle[i].m_u16AlarmValid = 1;
		ExceptHandle->m_ExceptHandle[i].m_u16AlarmMode = 0x2;
	}

	
	return S_SUCCESS;
	
}


int PARAMETER_MANAGE::LoadDefaultPcDirParameter(PCDIR_PARA *pcdir)
{

	memset(pcdir, 0, sizeof(PCDIR_PARA));
	strcpy(pcdir->m_pcSnapDir,"D:\\Snap");
	strcpy(pcdir->m_pcRecDir,"D:\\Recfiles");
	strcpy(pcdir->m_pcdLoadDir,"D:\\Downloads");
	
	return S_SUCCESS;
}

int PARAMETER_MANAGE::LoadDefaultOsdInsertParameter(VIDEOOSDINSERT *OsdInsert)
{
	int  ch = 0;

	if(OsdInsert == NULL)
	{
		return S_FAILURE;
	} 


	for(ch = 0; ch < CHANNEL_MAX; ch++)
	{		
		memset(&(OsdInsert->m_CoverLay[ch]), 0, sizeof(CHARINSERTSET));
		memset(&(OsdInsert->m_CoverLay2[ch]), 0, sizeof(CHARINSERTSET));
		memset(&(OsdInsert->m_CoverLay3[ch]), 0, sizeof(CHARINSERTSET));
		memset(&(OsdInsert->m_CoverLay4[ch]), 0, sizeof(CHARINSERTSET));
		
	}
	

	return S_SUCCESS;
	
}


void PARAMETER_MANAGE::LoadRecordTaskDefaultParameter(GROUPRECORDTASK *RecTask)
{

	int week = 0;
	for(week = 0; week < 8; week++)
	{
		//memset(RecTask,0x1,sizeof(GROUPRECORDTASK));
		RecTask->m_ChTask[0].m_TimerTask[week].m_uTimeTbl[0].m_u16StartTime = 0;
		RecTask->m_ChTask[0].m_TimerTask[week].m_uTimeTbl[0].m_u16EndTime = 23*60+59;
		RecTask->m_ChTask[0].m_TimerTask[week].m_uTimeTbl[0].m_u8Valid =0x01;
		RecTask->m_ChTask[0].m_TimerTask[week].m_uTimeTbl[0].m_validChannel =0x01;
		RecTask->m_ChTask[0].m_uTimerSwitch =0x01;
		RecTask->m_ChTask[0].m_uPowerRecEnable =0x01;
		
		RecTask->m_ChTask[0].m_TimerTask[week].m_uMdTbl[0].m_u16StartTime = 0;
		RecTask->m_ChTask[0].m_TimerTask[week].m_uMdTbl[0].m_u16EndTime = 23*60+59;
		RecTask->m_ChTask[0].m_TimerTask[week].m_uMdTbl[0].m_u8Valid =0x01;
		RecTask->m_ChTask[0].m_TimerTask[week].m_uMdTbl[0].m_validChannel =0x01;
		
		RecTask->m_ChTask[0].m_TimerTask[week].m_uAlarmTbl[0].m_u16StartTime = 0;
		RecTask->m_ChTask[0].m_TimerTask[week].m_uAlarmTbl[0].m_u16EndTime = 23*60+59;
		RecTask->m_ChTask[0].m_TimerTask[week].m_uAlarmTbl[0].m_u8Valid =0x01;		
		RecTask->m_ChTask[0].m_TimerTask[week].m_uAlarmTbl[0].m_validChannel =0x01;
		
		
	}
}


void PARAMETER_MANAGE::LoadPTZParameter(PTZ_PARA  *ptzpara)
{
	if(ptzpara == NULL) return;
	memset(ptzpara, 0, sizeof(PTZ_PARA));

	for(int i = 0; i < CHANNEL_MAX; i++)
	{
		ptzpara->m_ptz_channel[i].m_u8Addr = 1;
		ptzpara->m_ptz_channel[i].m_u8Speed = 31;
		ptzpara->m_ptz_channel[i].m_u8Protocol = 0;
		ptzpara->m_ptz_channel[i].m_u8BautRate = 2;
		ptzpara->m_ptz_channel[i].m_u8Databit = 8;
		ptzpara->m_ptz_channel[i].m_u8Stopbit = 1;
		ptzpara->m_ptz_channel[i].m_s32Chn = i;
		ptzpara->m_ptz_channel[i].m_u8Parity = 0;
	}

}


void PARAMETER_MANAGE::LoadDefaultPELCOParameter(PELCO_CmdCfg *pelcopara)
{
	if(pelcopara == NULL) return;
	memset(pelcopara, 0, sizeof(PELCO_CmdCfg));

	for(int i = 0; i < CHANNEL_NUM; i++)
	{
		pelcopara->pelcoP_enterMenu[i][0] = 0x00;
		pelcopara->pelcoP_enterMenu[i][1] = 0x07;		
		pelcopara->pelcoP_enterMenu[i][2] = 0x00;
		pelcopara->pelcoP_enterMenu[i][3] = 0x5f;
		pelcopara->pelcoP_default1[0][0] = 0x00; //*pelcoP_enterMenu 默认值
		pelcopara->pelcoP_default1[0][1] = 0x07;
		pelcopara->pelcoP_default1[0][2] = 0x00;
		pelcopara->pelcoP_default1[0][3] = 0x5f;
		pelcopara->pelcoP_default2[0][0] = 0x00;
		pelcopara->pelcoP_default2[0][1] = 0x07;
		pelcopara->pelcoP_default2[0][2] = 0x00;
		pelcopara->pelcoP_default2[0][3] = 0x5f;
		pelcopara->pelcoP_default3[0][0] = 0x00;
		pelcopara->pelcoP_default3[0][1] = 0x07;
		pelcopara->pelcoP_default3[0][2] = 0x00;
		pelcopara->pelcoP_default3[0][3] = 0x5f;

		pelcopara->pelcoD_enterMenu[i][0] = 0x00;
		pelcopara->pelcoD_enterMenu[i][1] = 0x07;
		pelcopara->pelcoD_enterMenu[i][2] = 0x00;
		pelcopara->pelcoD_enterMenu[i][3] = 0x5f;
		pelcopara->pelcoD_default1[0][0] = 0x00; //*pelcoD_enterMenu 默认值
		pelcopara->pelcoD_default1[0][1] = 0x07;
		pelcopara->pelcoD_default1[0][2] = 0x00;
		pelcopara->pelcoD_default1[0][3] = 0x5f;
		pelcopara->pelcoD_default2[0][0] = 0x00;
		pelcopara->pelcoD_default2[0][1] = 0x07;
		pelcopara->pelcoD_default2[0][2] = 0x00;
		pelcopara->pelcoD_default2[0][3] = 0x5f;
		pelcopara->pelcoD_default3[0][0] = 0x00;
		pelcopara->pelcoD_default3[0][1] = 0x07;
		pelcopara->pelcoD_default3[0][2] = 0x00;
		pelcopara->pelcoD_default3[0][3] = 0x5f;

		pelcopara->pelcoP_runpattern[i][0] = 0x00;	
		pelcopara->pelcoP_runpattern[i][1] = 0x23;	
		pelcopara->pelcoP_runpattern[i][2] = 0x00;	
		pelcopara->pelcoP_runpattern[i][3] = 0x00;
		pelcopara->pelcoP_default1[1][0] = 0x00; //*pelcoP_runpattern 默认值
		pelcopara->pelcoP_default1[1][1] = 0x23;
		pelcopara->pelcoP_default1[1][2] = 0x00;
		pelcopara->pelcoP_default1[1][3] = 0x00;
		pelcopara->pelcoP_default2[1][0] = 0x00;
		pelcopara->pelcoP_default2[1][1] = 0x23;
		pelcopara->pelcoP_default2[1][2] = 0x00;
		pelcopara->pelcoP_default2[1][3] = 0x00;
		pelcopara->pelcoP_default3[1][0] = 0x00;
		pelcopara->pelcoP_default3[1][1] = 0x23;
		pelcopara->pelcoP_default3[1][2] = 0x00;
		pelcopara->pelcoP_default3[1][3] = 0x00;
		
		pelcopara->pelcoD_runpattern[i][0] = 0x00;
		pelcopara->pelcoD_runpattern[i][1] = 0x23;
		pelcopara->pelcoD_runpattern[i][2] = 0x00;
		pelcopara->pelcoD_runpattern[i][3] = 0x00;
		pelcopara->pelcoD_default1[1][0] = 0x00; //*pelcoD_runpattern 默认值
		pelcopara->pelcoD_default1[1][1] = 0x23;
		pelcopara->pelcoD_default1[1][2] = 0x00;
		pelcopara->pelcoD_default1[1][3] = 0x00;
		pelcopara->pelcoD_default2[1][0] = 0x00;
		pelcopara->pelcoD_default2[1][1] = 0x23;
		pelcopara->pelcoD_default2[1][2] = 0x00;
		pelcopara->pelcoD_default2[1][3] = 0x00;
		pelcopara->pelcoD_default3[1][0] = 0x00;
		pelcopara->pelcoD_default3[1][1] = 0x23;
		pelcopara->pelcoD_default3[1][2] = 0x00;
		pelcopara->pelcoD_default3[1][3] = 0x00;
		
		pelcopara->pelcoP_cruiseOn[i][0] = 0x01;
		pelcopara->pelcoP_cruiseOn[i][1] = 0x81;
		pelcopara->pelcoP_cruiseOn[i][2] = 0x00;
		pelcopara->pelcoP_cruiseOn[i][3] = 0x00;
		pelcopara->pelcoP_default1[2][0] = 0x01; //*pelcoP_cruiseOn 默认值
		pelcopara->pelcoP_default1[2][1] = 0x81;
		pelcopara->pelcoP_default1[2][2] = 0x00;
		pelcopara->pelcoP_default1[2][3] = 0x00;
		pelcopara->pelcoP_default2[2][0] = 0x01;
		pelcopara->pelcoP_default2[2][1] = 0x81;
		pelcopara->pelcoP_default2[2][2] = 0x00;
		pelcopara->pelcoP_default2[2][3] = 0x00;
		pelcopara->pelcoP_default3[2][0] = 0x01;
		pelcopara->pelcoP_default3[2][1] = 0x81;
		pelcopara->pelcoP_default3[2][2] = 0x00;
		pelcopara->pelcoP_default3[2][3] = 0x00;
		
		pelcopara->pelcoD_cruiseOn[i][0] = 0x01;
		pelcopara->pelcoD_cruiseOn[i][1] = 0x81;
		pelcopara->pelcoD_cruiseOn[i][2] = 0x00;
		pelcopara->pelcoD_cruiseOn[i][3] = 0x00;
		pelcopara->pelcoD_default1[2][0] = 0x01; //*pelcoD_cruiseOn 默认值
		pelcopara->pelcoD_default1[2][1] = 0x81;
		pelcopara->pelcoD_default1[2][2] = 0x00;
		pelcopara->pelcoD_default1[2][3] = 0x00;
		pelcopara->pelcoD_default2[2][0] = 0x01;
		pelcopara->pelcoD_default2[2][1] = 0x81;
		pelcopara->pelcoD_default2[2][2] = 0x00;
		pelcopara->pelcoD_default2[2][3] = 0x00;
		pelcopara->pelcoD_default3[2][0] = 0x01;
		pelcopara->pelcoD_default3[2][1] = 0x81;
		pelcopara->pelcoD_default3[2][2] = 0x00;
		pelcopara->pelcoD_default3[2][3] = 0x00;
		
		pelcopara->pelcoP_autoScan[i][0] = 0x00;	
		pelcopara->pelcoP_autoScan[i][1] = 0x07;	
		pelcopara->pelcoP_autoScan[i][2] = 0x00;	
		pelcopara->pelcoP_autoScan[i][3] = 0x63;	
		pelcopara->pelcoP_default1[3][0] = 0x00; //*pelcoP_autoScan 默认值
		pelcopara->pelcoP_default1[3][1] = 0x07;
		pelcopara->pelcoP_default1[3][2] = 0x00;
		pelcopara->pelcoP_default1[3][3] = 0x63;
		pelcopara->pelcoP_default2[3][0] = 0x00;
		pelcopara->pelcoP_default2[3][1] = 0x07;
		pelcopara->pelcoP_default2[3][2] = 0x00;
		pelcopara->pelcoP_default2[3][3] = 0x63;
		pelcopara->pelcoP_default3[3][0] = 0x00;
		pelcopara->pelcoP_default3[3][1] = 0x07;
		pelcopara->pelcoP_default3[3][2] = 0x00;
		pelcopara->pelcoP_default3[3][3] = 0x63;
		
		pelcopara->pelcoD_autoScan[i][0] = 0x00;
		pelcopara->pelcoD_autoScan[i][1] = 0x07;
		pelcopara->pelcoD_autoScan[i][2] = 0x00;
		pelcopara->pelcoD_autoScan[i][3] = 0x63;
		pelcopara->pelcoD_default1[3][0] = 0x00; //*pelcoD_autoScan 默认值
		pelcopara->pelcoD_default1[3][1] = 0x07;
		pelcopara->pelcoD_default1[3][2] = 0x00;
		pelcopara->pelcoD_default1[3][3] = 0x63;
		pelcopara->pelcoD_default2[3][0] = 0x00;
		pelcopara->pelcoD_default2[3][1] = 0x07;
		pelcopara->pelcoD_default2[3][2] = 0x00;
		pelcopara->pelcoD_default2[3][3] = 0x63;
		pelcopara->pelcoD_default3[3][0] = 0x00;
		pelcopara->pelcoD_default3[3][1] = 0x07;
		pelcopara->pelcoD_default3[3][2] = 0x00;
		pelcopara->pelcoD_default3[3][3] = 0x63;
		
		pelcopara->pelcoP_stopScan[i][0] = 0x00;
		pelcopara->pelcoP_stopScan[i][1] = 0x07;
		pelcopara->pelcoP_stopScan[i][2] = 0x00;
		pelcopara->pelcoP_stopScan[i][3] = 0x60;
		pelcopara->pelcoP_default1[4][0] = 0x00; //*pelcoP_stopScan 默认值
		pelcopara->pelcoP_default1[4][1] = 0x07;
		pelcopara->pelcoP_default1[4][2] = 0x60;
		pelcopara->pelcoP_default1[4][3] = 0x00;
		pelcopara->pelcoP_default2[4][0] = 0x00;
		pelcopara->pelcoP_default2[4][1] = 0x07;
		pelcopara->pelcoP_default2[4][2] = 0x60;
		pelcopara->pelcoP_default2[4][3] = 0x00;
		pelcopara->pelcoP_default3[4][0] = 0x00;
		pelcopara->pelcoP_default3[4][1] = 0x07;
		pelcopara->pelcoP_default3[4][2] = 0x60;
		pelcopara->pelcoP_default3[4][3] = 0x00;
		
		pelcopara->pelcoD_stopScan[i][0] = 0x00;
		pelcopara->pelcoD_stopScan[i][1] = 0x07;
		pelcopara->pelcoD_stopScan[i][2] = 0x00;
		pelcopara->pelcoD_stopScan[i][3] = 0x60;
		pelcopara->pelcoD_default1[4][0] = 0x00; //*pelcoD_stopScan 默认值
		pelcopara->pelcoD_default1[4][1] = 0x07;
		pelcopara->pelcoD_default1[4][2] = 0x00;
		pelcopara->pelcoD_default1[4][3] = 0x60;
		pelcopara->pelcoD_default2[4][0] = 0x00;
		pelcopara->pelcoD_default2[4][1] = 0x07;
		pelcopara->pelcoD_default2[4][2] = 0x00;
		pelcopara->pelcoD_default2[4][3] = 0x60;
		pelcopara->pelcoD_default3[4][0] = 0x00;
		pelcopara->pelcoD_default3[4][1] = 0x07;
		pelcopara->pelcoD_default3[4][2] = 0x00;
		pelcopara->pelcoD_default3[4][3] = 0x60;
	}
	
}


void PARAMETER_MANAGE::LoadDefaultPicTimerParameter(PICTURE_TIMER *picTmrpara)
{
	if(picTmrpara == NULL) return;
	memset(picTmrpara, 0, sizeof(PICTURE_TIMER));
	
	picTmrpara->m_picChn[0].m_nBrightness = 0;
	picTmrpara->m_picChn[0].m_nContrast = 0;
	picTmrpara->m_picChn[0].m_nHue = 0;
	picTmrpara->m_picChn[0].m_nSaturation = 0;
	picTmrpara->m_picTmr[0].m_u16StartTime = 0;	  //*早0:00:00
	picTmrpara->m_picTmr[0].m_u16EndTime = 6*60;   //*早6:00:00
	//*默认值
	picTmrpara->m_picChnDefault[0][0].m_nBrightness = 0; 
	picTmrpara->m_picChnDefault[0][0].m_nContrast = 0;
	picTmrpara->m_picChnDefault[0][0].m_nHue = 0;
	picTmrpara->m_picChnDefault[0][0].m_nSaturation = 0;
	picTmrpara->m_picTmrDefault[0][0].m_u16StartTime = 0;
	picTmrpara->m_picTmrDefault[0][0].m_u16EndTime = 6*60;

	picTmrpara->m_picChnDefault[1][0].m_nBrightness = 0;
	picTmrpara->m_picChnDefault[1][0].m_nContrast = 0;
	picTmrpara->m_picChnDefault[1][0].m_nHue = 0;
	picTmrpara->m_picChnDefault[1][0].m_nSaturation = 0;
	picTmrpara->m_picTmrDefault[1][0].m_u16StartTime = 0;
	picTmrpara->m_picTmrDefault[1][0].m_u16EndTime = 6*60;

	picTmrpara->m_picChnDefault[2][0].m_nBrightness = 0;
	picTmrpara->m_picChnDefault[2][0].m_nContrast = 0;
	picTmrpara->m_picChnDefault[2][0].m_nHue = 0;
	picTmrpara->m_picChnDefault[2][0].m_nSaturation = 0;
	picTmrpara->m_picTmrDefault[2][0].m_u16StartTime = 0;
	picTmrpara->m_picTmrDefault[2][0].m_u16EndTime = 6*60;

	picTmrpara->m_picChn[1].m_nBrightness = 0;
	picTmrpara->m_picChn[1].m_nContrast = 0;
	picTmrpara->m_picChn[1].m_nHue = 0;
	picTmrpara->m_picChn[1].m_nSaturation = 0;
	picTmrpara->m_picTmr[1].m_u16StartTime = 6*60; //*早6:00:00
	picTmrpara->m_picTmr[1].m_u16EndTime = 18*60;   //*晚6:00:00
	//*默认值
	picTmrpara->m_picChnDefault[0][1].m_nBrightness = 0; 
	picTmrpara->m_picChnDefault[0][1].m_nContrast = 0;
	picTmrpara->m_picChnDefault[0][1].m_nHue = 0;
	picTmrpara->m_picChnDefault[0][1].m_nSaturation = 0;
	picTmrpara->m_picTmrDefault[0][1].m_u16StartTime = 6*60;
	picTmrpara->m_picTmrDefault[0][1].m_u16EndTime = 18*60;

	picTmrpara->m_picChnDefault[1][1].m_nBrightness = 0;
	picTmrpara->m_picChnDefault[1][1].m_nContrast = 0;
	picTmrpara->m_picChnDefault[1][1].m_nHue = 0;
	picTmrpara->m_picChnDefault[1][1].m_nSaturation = 0;
	picTmrpara->m_picTmrDefault[1][1].m_u16StartTime = 6*60;
	picTmrpara->m_picTmrDefault[1][1].m_u16EndTime = 18*60;

	picTmrpara->m_picChnDefault[2][1].m_nBrightness = 0;
	picTmrpara->m_picChnDefault[2][1].m_nContrast = 0;
	picTmrpara->m_picChnDefault[2][1].m_nHue = 0;
	picTmrpara->m_picChnDefault[2][1].m_nSaturation = 0;
	picTmrpara->m_picTmrDefault[2][1].m_u16StartTime = 6*60;
	picTmrpara->m_picTmrDefault[2][1].m_u16EndTime = 18*60;

	picTmrpara->m_picChn[2].m_nBrightness = 0;
	picTmrpara->m_picChn[2].m_nContrast = 0;
	picTmrpara->m_picChn[2].m_nHue = 0;
	picTmrpara->m_picChn[2].m_nSaturation = 0;
	picTmrpara->m_picTmr[2].m_u16StartTime = 18*60; //*晚6:00:00
	picTmrpara->m_picTmr[2].m_u16EndTime = 23*60+59;   //*晚23:59
	//*默认值
	picTmrpara->m_picChnDefault[0][2].m_nBrightness = 0; 
	picTmrpara->m_picChnDefault[0][2].m_nContrast = 0;
	picTmrpara->m_picChnDefault[0][2].m_nHue = 0;
	picTmrpara->m_picChnDefault[0][2].m_nSaturation = 0;
	picTmrpara->m_picTmrDefault[0][2].m_u16StartTime = 18*60;
	picTmrpara->m_picTmrDefault[0][2].m_u16EndTime = 23*60+59;

	picTmrpara->m_picChnDefault[1][2].m_nBrightness = 0;
	picTmrpara->m_picChnDefault[1][2].m_nContrast = 0;
	picTmrpara->m_picChnDefault[1][2].m_nHue = 0;
	picTmrpara->m_picChnDefault[1][2].m_nSaturation = 0;
	picTmrpara->m_picTmrDefault[1][2].m_u16StartTime = 18*60;
	picTmrpara->m_picTmrDefault[1][2].m_u16EndTime = 23*60+59;

	picTmrpara->m_picChnDefault[2][2].m_nBrightness = 0;
	picTmrpara->m_picChnDefault[2][2].m_nContrast = 0;
	picTmrpara->m_picChnDefault[2][2].m_nHue = 0;
	picTmrpara->m_picChnDefault[2][2].m_nSaturation = 0;
	picTmrpara->m_picTmrDefault[2][2].m_u16StartTime = 18*60;
	picTmrpara->m_picTmrDefault[2][2].m_u16EndTime = 23*60+59;
	
}


void PARAMETER_MANAGE::LoadDefaultVoPicParameter(VODEV_ANALOG *voPicpara)
{
	if(voPicpara == NULL) return;
	memset(voPicpara, 0, sizeof(VODEV_ANALOG));

	voPicpara->m_SDPic.m_nBrightness = 50;
	voPicpara->m_SDPic.m_nContrast = 54;
	voPicpara->m_SDPic.m_nHue = 49;
	voPicpara->m_SDPic.m_nSaturation = 55;

	voPicpara->m_HDPic.m_nBrightness = 50;
	voPicpara->m_HDPic.m_nContrast = 55;
	voPicpara->m_HDPic.m_nHue = 50;
	voPicpara->m_HDPic.m_nSaturation = 49;
}


void PARAMETER_MANAGE::LoadPtzLinkSet(GroupZonePtzLinkSet *PtzLink)
{

	memset(PtzLink, 0, sizeof(GroupZonePtzLinkSet));

}


void PARAMETER_MANAGE::LoadAlarmZoneParameter(ALARMZONEGROUPSET *AlarmZone)
{

	int  i = 0;

	for(i = 0 ; i < MAX_ALARM_ZONE; i++)
	{
		memset(&(AlarmZone->m_AlarmZone[i]), 0, sizeof(AlarmZoneSet));

		AlarmZone->m_AlarmZone[i].m_u8RpCenter = 0;

		AlarmZone->m_AlarmZone[i].m_u8ZoneEnable = 1;

	}

}





void PARAMETER_MANAGE::LoadMegaSetDefault(MegaEyes_PARA *MegaSet)
{
	if(MegaSet == NULL)
	{
		return;
	}
	
	memset(&(MegaSet->m_netWorkMegaEyes), 0, sizeof(NetWorkMegaEyes));
	memset(&(MegaSet->m_netWorkMegaEcho), 0, sizeof(NetWorkMegaEcho));

	strcpy(MegaSet->m_netWorkMegaEyes.m_uAccDomain,"113.105.146.146" ); // 我们自己的平台
	MegaSet->m_netWorkMegaEyes.m_MsgPort = 4062;

	// 单路ipcid测试用ID：321706F346CC6495196B 
	memcpy(MegaSet->m_netWorkMegaEyes.m_deviceid,"321706F346CC6495196B",20);

	MegaSet->m_netWorkMegaEcho.r_HeartCycle = 120;
	
}
void PARAMETER_MANAGE::LoadCamSensorParameter(CAMERASENSOR_PARA		*pSensor)
{
	if(pSensor == NULL)
	{
		return ;
	}
	pSensor->m_colorMode = 1;
	pSensor->m_picMode = 4;	
	pSensor->m_PowerFreq = 3;
	
	return ;
}

void PARAMETER_MANAGE::CheckNetWorkParameter(NETWORK_PARA *network)
{
	if(network == NULL)
	{
		return;
	}

	//added by panjy 如果为全0, 则给加载默认配置, 没有判断子网掩码/网关
	if ( 0 == network->m_Eth0Config.m_uLocalIp[0]
		&& 0 == network->m_Eth0Config.m_uLocalIp[1]
		&& 0 == network->m_Eth0Config.m_uLocalIp[2]
		&& 0 == network->m_Eth0Config.m_uLocalIp[3]
		)
	{
		LoadNetWorkDefault(network);
	}
	//end

	if((network->m_CenterNet.m_uPhoneListenPt < 1024))
		network->m_CenterNet.m_uPhoneListenPt = 9000;
	if((network->m_CenterNet.m_uVideoListenPt < 1024))
		network->m_CenterNet.m_uVideoListenPt = 8000;
	if((network->m_CenterNet.m_uHttpListenPt < 1024))
		network->m_CenterNet.m_uHttpListenPt = 80;

}

void PARAMETER_MANAGE::CheckSecurityParameter(PASSWORD_PARA *pswd)
{
	int flag = 0;
	
	if(pswd == NULL)
	{
		return;
	}

	if(pswd->m_uPwdEnable> 0x01)
	{
		pswd->m_uPwdEnable = 0;
		flag =1;
	}

	if(flag > 0)
	{
		printf("密码设置数据有误  \n");
		LoadSecurityDefault(pswd);
	}
	
	DPRINTF("admin pwd : %x oper pwd : %x enable : %d \n", pswd->m_uAdminPassword,
		pswd->m_uOperatePassword, pswd->m_uPwdEnable);

}

void PARAMETER_MANAGE::CheckRecTaskParameter(REC_TASK_PARA *Task_Para)
{

#if 0
	int i = 0;
	int flag = 0;

	if(Task_Para == NULL)
	{
		return;
	}

	if(Task_Para->m_uPowerRecEnable> 0x01)
	{
		Task_Para->m_uPowerRecEnable = 0;
	}

	if(Task_Para->m_uTimerSwitch> 0x01)
	{
		Task_Para->m_uTimerSwitch = 0;
	}

	for( i = 0; i < 8; i++)
	{
		
		if(Task_Para->m_TimerTask[i].m_uTrigeType> 2)
		{
			Task_Para->m_TimerTask[i].m_uTrigeType = 0;
			flag = 1;
		}

		if(Task_Para->m_TimerTask[i].m_uWeekDay> 8)
		{
			Task_Para->m_TimerTask[i].m_uWeekDay = 8;
			flag = 1;
		}
		
		for(int j = 0; j < 4; j++)
		{
			if(Task_Para->m_TimerTask[i].m_uTimeTbl[j] >= 1440)
			{
				Task_Para->m_TimerTask[i].m_uTimeTbl[j] = 0;
				flag = 1;
			}
		}
	}

	if(flag > 0)
	{
		printf("录象时间设置数据有误  \n");
		LoadRecTaskDefault(Task_Para);
	}
#endif 
}

void PARAMETER_MANAGE::CheckMachineParameter(MACHINE_PARA *machine)
{
	int flag = 0;

	if(machine == NULL)
	{
		return;
	}

	if(machine->m_uHddOverWrite > 0x01)
	{
		machine->m_uHddOverWrite = 0x01;
		flag = 1;
	}

	if(machine->m_uTvSystem > 0x01)
	{
		machine->m_uTvSystem = 0x00;
		flag = 1;
		
	}

	if((machine->m_uMachinId > 99999))
	{
		machine->m_uMachinId = 0;
		flag = 1;
	}

	if(flag > 0)
	{
		printf("机器参数设置数据有误  \n");
		LoadMachineParaDefault(machine);
	}
	
	DPRINTF("overtime :%d, tv system: %d  m_unit_id \n", machine->m_uHddOverWrite,
		machine->m_uTvSystem, machine->m_uMachinId);

	
}

void PARAMETER_MANAGE::CheckCameraSetParameter(CAMERA_PARA *camera)
{
	int i = 0, j = 0;
	int flag = 0;
	unsigned char tmp = 0;
	
	if(camera == NULL) return;

	for(i = 0; i < CHANNEL_MAX; i++)
	{
		if(camera->m_ChannelPara[i].m_uQuality > 4)
		{
			printf("画质错误:%d\n", camera->m_ChannelPara[i].m_uQuality);
			camera->m_ChannelPara[i].m_uQuality = 4;
			flag = 1;
		}
		if((camera->m_ChannelPara[i].m_uFrameRate > 25))
		{
			printf("帧率错误:%d\n", camera->m_ChannelPara[i].m_uFrameRate);
			camera->m_ChannelPara[i].m_uFrameRate = 25;
			flag = 1;
		}
		if(camera->m_ChannelPara[i].m_TimeSwitch > 1)
		{
			camera->m_ChannelPara[i].m_TimeSwitch = 0x01;
			flag = 1;
		}
		if(camera->m_ChannelPara[i].m_TltleSwitch > 1)
		flag = 1;

		if(camera->m_ChannelPara[i].m_uEncType > 0x01)
		{
			camera->m_ChannelPara[i].m_uEncType = 0x01;
			flag = 1;
		}

		//added by panjy 检测分辨率
		#ifdef HI1080P_IPC
            if (4 != camera->m_ChannelPara[i].m_uResolution)
    		{
    			camera->m_ChannelPara[i].m_uResolution = 4;
    		}
        #else
    		#ifdef IPC720P
        		if (5 != camera->m_ChannelPara[i].m_uResolution)
        		{
        			camera->m_ChannelPara[i].m_uResolution = 5;
        		}
        		#endif

        		#ifdef IPCVGA
        		if (6 != camera->m_ChannelPara[i].m_uResolution)
        		{
        			camera->m_ChannelPara[i].m_uResolution = 5;
        		}
    		#endif
        #endif
		//end

		for(j = 0; j < 17; j++)
		{
			tmp = camera->m_ChannelPara[i].m_Title[j];
			#if 0
			if(((tmp < 0x30) || ((tmp > 0x7a) && (tmp < 0xa0)) || (tmp > 0xfe)) && ( tmp != 0x20))
			{
				camera->m_ChannelPara[i].m_Title[j] = '\0';
			}
			#endif
		}
		camera->m_ChannelPara[i].m_Title[16] = '\0';
		
	}

	if(flag > 0)
	{
		printf("编码参数设置数据有误\n");
		LoadCameraSetDefault(camera);
	}

}


int PARAMETER_MANAGE::CheckAnalogParameter(CAMERA_ANALOG *analog)
{
	int ch = 0;

	if(analog == NULL)
	{

		return S_FAILURE;
	}

	for(ch = 0; ch < CHANNEL_MAX; ch++)
	{
		if(analog->m_Channels[ch].m_nBrightness>=255)
		{
			analog->m_Channels[ch].m_nBrightness = Def_Brightness;
		}

		if(analog->m_Channels[ch].m_nContrast>= 255)
		{
			analog->m_Channels[ch].m_nContrast =Def_Contrast;
		}
		if(analog->m_Channels[ch].m_nSaturation>= 255)
		{
			analog->m_Channels[ch].m_nSaturation =Def_Saturation;
		}
		if(analog->m_Channels[ch].m_nHue>= 127)
		{
			analog->m_Channels[ch].m_nHue = 0;
		}
	}

	return S_SUCCESS;
	
}


void PARAMETER_MANAGE::CheckAlarmOutParameter(ALARM_PARA *alarm_out)
{

#if 0
	if(alarm_out == NULL)
	{
		return;
	}

	if((alarm_out->m_nAlarmOutDelay> 300) || (alarm_out->m_nAlarmOutDelay < 30))
	{
		alarm_out->m_nAlarmOutDelay = 30;
	}

	if((alarm_out->m_nAlarmRecDelay> 900) 
		|| (alarm_out->m_nAlarmRecDelay < 30))
	{
		alarm_out->m_nAlarmRecDelay = 30;
	}

	if((alarm_out->m_nPreRecTime > 20) || (alarm_out->m_nPreRecTime < 10))
	{
		alarm_out->m_nPreRecTime = 10;
	}

	if(alarm_out->m_nBuzzeDelay > 300)
	{
		alarm_out->m_nBuzzeDelay = 0;
	}

#endif 	
	
}

void  PARAMETER_MANAGE::CheckSensorParameter(SENSOR_PARA *sensor)
{
#if 0
	int i = 0, j = 0;
	int flag = 0;
	unsigned char tmp = 0;

	if(sensor == NULL)
	{
		return;
	}

#if 0
	if(sensor->s_speed.m_max_limit > 200)
	{
		sensor->s_speed.m_max_limit = 200;
		flag = 1;
	}
	
	if(sensor->s_speed.m_min_limit > 200)
	{
		sensor->s_speed.m_min_limit = 200;
		flag = 1;
	}	

	sensor->s_speed.m_speed_unit &=0x01;
#endif 

	for( i = 0; i < 9 ; i++)
	{
		for(j = 0; j < 8; j++)
		{

		/*
			if((!isalpha(sensor->s_io.m_title[i].title[j]))
				&& (!isdigit(sensor->s_io.m_title[i].title[j])))
		*/
		
			tmp = sensor->m_SensorIO.m_cTitle[i].m_cTitle[j];
		
			if(((tmp < 0x30) || ((tmp > 0x7a) && (tmp < 0xa0)) || (tmp > 0xfe)) && ( tmp != 0x20))				
			{
			//	printf(" %x ", tmp);
				sensor->m_SensorIO.m_cTitle[i].m_cTitle[j] = ' ';
			}
		}

	//	printf(" \n");
		
		sensor->m_SensorIO.m_cTitle[i].m_cTitle[8] = '\0';

	}

	if(flag > 0)
	{
		printf("传感器设置数据有误  \n");
		LoadSensorSetDefault(sensor);
	}

#endif 

}

#if 0
void PARAMETER_MANAGE::CheckVechileParameter(VECHIL_SETTING *vechile)
{
	int i = 0, j = 0;
	
	if(vechile == NULL)
	{
		return;
	}

	for(i = 0; i < 3; i++)
	{
		switch(i)
		{
			case 0:
				for(j = 0; j < 16; j++)
				{
					if((!isdigit(vechile->m_companyname.m_title[j]))
						&&(!isalpha(vechile->m_companyname.m_title[j])))
					{
						vechile->m_companyname.m_title[j] = ' ';
					}
				}
				vechile->m_companyname.m_title[16] = '\0';
				dprintf("para company  : %s \n", vechile->m_companyname.m_title);

			break;
			
			case 1:
				for(j = 0; j < 16; j++)
				{
					if((!isdigit(vechile->m_drivername.m_title[j]))
						&&(!isalpha(vechile->m_drivername.m_title[j])))
					{
						vechile->m_drivername.m_title[j] = ' ';
					}
				}
				vechile->m_drivername.m_title[16] = '\0';
				dprintf("para driver : %s \n", vechile->m_drivername.m_title);
			
			break;

			case 2:
				for(j = 0; j < 16; j++)
				{
					if((!isdigit(vechile->m_vechil_number.m_title[j]))
						&&(!isalpha(vechile->m_vechil_number.m_title[j])))
					{
						vechile->m_vechil_number.m_title[j] = ' ';
					}
				}
				vechile->m_vechil_number.m_title[16] = '\0';
				dprintf("para vechi : %s \n", vechile->m_vechil_number.m_title);
			break;			
		}
		
	}
	
}
#endif 

void PARAMETER_MANAGE::CheckPowerManageParameter(POWER_MANAGE *pw_manage)
{
	if(pw_manage == NULL)
	{
		return;
	}

	if(pw_manage->m_uStartupMode > 2)
	{
		pw_manage->m_uStartupMode = 0;
	}

	if(pw_manage->m_uShutDownHour > 23)
	{
		pw_manage->m_uShutDownHour = 0;
	}

	if(pw_manage->m_uShutDownMin > 59)
	{
		pw_manage->m_uShutDownMin = 0;
	}

	if(pw_manage->m_uShutDownSec > 59)
	{
		pw_manage->m_uShutDownSec = 0;
	}
	if(pw_manage->m_uStartupHour > 23)
	{
		pw_manage->m_uStartupHour = 0;
	}

	if(pw_manage->m_uStartupMin> 59)
	{
		pw_manage->m_uStartupMin = 0;
	}

	if(pw_manage->m_uStartupSec > 59)
	{
		pw_manage->m_uStartupSec = 0;
	}

	if(pw_manage->m_uShutDelay > 999)
	{
		pw_manage->m_uShutDelay = 999;
	}
	
}

int PARAMETER_MANAGE::CheckCommonParameter(COMMON_PARA *comm_para)
{
	if(comm_para == NULL)
	{
		return S_FAILURE;
	}
	if(comm_para->m_uDateMode>2)
		comm_para->m_uDateMode =2;

	if(comm_para->m_uTimeMode>1)
		comm_para->m_uTimeMode =1;
	

	
	if(comm_para->m_uWeekDayEnd > 6)
	{
		comm_para->m_uWeekDayEnd = 0;
	}

	if(comm_para->m_uWeekDayStart > 6)
	{
		comm_para->m_uWeekDayStart = 0;
	}

	if(comm_para->m_uTimeInSert > 1)
	{
		comm_para->m_uTimeInSert =0x01;
	}
	
	if(comm_para->m_uIdleTime > 99)
	{
		comm_para->m_uIdleTime = 1;
	}
	
	comm_para->m_uTimeSyncMode&=0x03;
	
	DPRINTF("time inset : %d , syncmode : %d \n",
		comm_para->m_uTimeInSert, comm_para->m_uTimeSyncMode);

	
	return S_SUCCESS;
	
}


int PARAMETER_MANAGE::CheckSystemRunInfo(SYSTEM_RUNINFO *runinfo)
{

	if(runinfo == NULL)
	{

		return S_FAILURE;
	}

	return S_SUCCESS;
	
}

int PARAMETER_MANAGE::CheckCamerBlind(CAMERA_BLIND *bcd)
{
	int i = 0;

	if(bcd == NULL)
	{
		return S_FAILURE;
	}
	
	for( i = 0; i < CHANNEL_MAX; i++)
	{
		if((bcd->m_Channel[i].m_uBlindSensitive > 4) || (bcd->m_Channel[i].m_uBlindSensitive <= 0))
		{
			bcd->m_Channel[i].m_uBlindSensitive = 0; // 高
		}
	}

	return S_SUCCESS;
	
}

int PARAMETER_MANAGE::CheckCameraMotionDetection(CAMERA_MD *md_set)
{
	int i = 0;

	if(md_set== NULL)
	{
		return S_FAILURE;
	}



	for(i = 0; i < CHANNEL_MAX; i++)
	{
		
		if(md_set->m_Channel[i].m_uMDSensitive < 0)
		{
			md_set->m_Channel[i].m_uMDSensitive = 2;
		}
	}

	return S_SUCCESS;
	
}

int  PARAMETER_MANAGE::CheckAlarmZoneParameter(ALARMZONEGROUPSET *AlarmZone)
{
	int i = 0;

	if(AlarmZone == NULL)
	{
		return -1;
	}

	for(i = 0; i < MAX_ALARM_ZONE; i++)
	{
		AlarmZone->m_AlarmZone[i].m_u32RecEnable &=CHANNEL_MASK;
		AlarmZone->m_AlarmZone[i].m_u32UionChannel &=CHANNEL_MASK;
		AlarmZone->m_AlarmZone[i].m_u8OSDEnable &=0x01;
		AlarmZone->m_AlarmZone[i].m_u8ZoneEnable &=0x01;
	}

	return 0;
	
}



int PARAMETER_MANAGE::CheckPTZParameter(PTZ_PARA  *ptzpara)
{
	int i = 0;

	if(ptzpara == NULL)
	{
		return S_FAILURE;
	}

	for(i = 0; i < CHANNEL_MAX; i++)
	{
		if(ptzpara->m_ptz_channel[i].m_u8Speed != 0x40)
		{
			if(ptzpara->m_ptz_channel[i].m_u8Speed > 0x3f)
				ptzpara->m_ptz_channel[i].m_u8Speed = 0x3f;
		}

		#if 0
		if(ptzpara->m_ptz_channel[i].m_u8Protocol> 1)
		#else	//zlz add ptz protocol
		if(ptzpara->m_ptz_channel[i].m_u8Protocol> 6)
		#endif	
		{
			ptzpara->m_ptz_channel[i].m_u8Protocol = 0;
		}

		if(ptzpara->m_ptz_channel[i].m_u8BautRate > 5)
		{
			ptzpara->m_ptz_channel[i].m_u8BautRate= 0;
		}

		ptzpara->m_ptz_channel[i].m_s32Chn = i;
	}

	return S_SUCCESS;
}

int PARAMETER_MANAGE::CheckPELCOParameter(PELCO_CmdCfg *pelcopara)
{
	if(pelcopara == NULL)return S_FAILURE;

	return S_SUCCESS;
}

int PARAMETER_MANAGE::CheckPicTimerParameter(PICTURE_TIMER *picTmrpara)
{
	if(picTmrpara == NULL)return S_FAILURE;

	return S_SUCCESS;
}

int PARAMETER_MANAGE::CheckVoPicParameter(VODEV_ANALOG *voPicpara)
{
	if(voPicpara == NULL)return S_FAILURE;

	if((voPicpara->m_SDPic.m_nBrightness == 0)||(voPicpara->m_SDPic.m_nBrightness > 100))
		voPicpara->m_SDPic.m_nBrightness = 50;

	if((voPicpara->m_SDPic.m_nContrast == 0)||(voPicpara->m_SDPic.m_nContrast > 100))
		voPicpara->m_SDPic.m_nContrast = 55;

	if((voPicpara->m_SDPic.m_nHue == 0)||(voPicpara->m_SDPic.m_nHue > 100))
		voPicpara->m_SDPic.m_nHue = 50;

	if((voPicpara->m_SDPic.m_nSaturation == 0)||(voPicpara->m_SDPic.m_nSaturation > 100))
		voPicpara->m_SDPic.m_nSaturation = 49;

	if((voPicpara->m_HDPic.m_nBrightness == 0)||(voPicpara->m_HDPic.m_nBrightness > 100))
		voPicpara->m_HDPic.m_nBrightness = 50;

	if((voPicpara->m_HDPic.m_nContrast == 0)||(voPicpara->m_HDPic.m_nContrast > 100))
		voPicpara->m_HDPic.m_nContrast = 54;

	if((voPicpara->m_HDPic.m_nHue == 0)||(voPicpara->m_HDPic.m_nHue > 100))
		voPicpara->m_HDPic.m_nHue = 49;

	if((voPicpara->m_HDPic.m_nSaturation == 0)||(voPicpara->m_HDPic.m_nSaturation > 100))
		voPicpara->m_HDPic.m_nSaturation = 55;
	
	return S_SUCCESS;
}

int PARAMETER_MANAGE::CheckPtzLinkParameter(GroupZonePtzLinkSet  *PtzLink)
{

	int ch = 0;
	int zone = 0;

	for(zone = 0; zone < MAX_ALARM_ZONE; zone++)
	{

		for(ch = 0; ch < CHANNEL_MAX; ch++)
		{

			PtzLink->m_Zone[zone].m_Channel[ch].m_u8CruiseEn &=0x01;
			PtzLink->m_Zone[zone].m_Channel[ch].m_u8PresetEn &=0x01;
			PtzLink->m_Zone[zone].m_Channel[ch].m_u8TrackEn &=0x01;
			
		}
		
	}
	
	return S_SUCCESS;
	
}
int PARAMETER_MANAGE::CheckExtendApSettings(PARAMETEREXTEND *pExtendApSettings)
{
	printf("%s %d %s......\n",__FILE__,__LINE__,__FUNCTION__);
	if(pExtendApSettings->m_ntp.m_daylight_switch > 1)
	{
		pExtendApSettings->m_ntp.m_daylight_switch = 1;
	}
	if(pExtendApSettings->m_ntp.m_idx_tzname > 90)
	{
		pExtendApSettings->m_ntp.m_idx_tzname =0;
	}	
	if(pExtendApSettings->m_ntp.m_daylight_switch > 1)
	{
		pExtendApSettings->m_ntp.m_daylight_switch =1;
	}	

	return S_SUCCESS;
}
int PARAMETER_MANAGE::CheckWebSettings(web_sync_param_t *pweb)
{
	printf("%s %d %s......\n",__FILE__,__LINE__,__FUNCTION__);
	if((strlen(pweb->sync_key)==0)||(strlen(pweb->time_zone)<3))
		
	{
		strcpy(pweb->sync_key, "0");
		strcpy(pweb->time_zone,"UTC");
		pweb->mute =1;
		pweb->alarm_interval =900;

		pweb->device_on =1;
		pweb->device_schedule =0;

		pweb->nightvision_switch = 1;
		pweb->imageflip_switch = 0;
	}
	
	if(pweb->nightvision_switch == 0)
		pweb->nightvision_switch = 1;
		
	return S_SUCCESS;
}
int PARAMETER_MANAGE::CheckMdSettings(P2P_MD_REGION_CHANNEL *pmd)
{
	
	if(pmd->x>1)
		pmd->x =0;
	if(pmd->y>1)
		pmd->y =0;
	if(pmd->width>1)
		pmd->width =1;
	if(pmd->height>1)
		pmd->height =1;
	if(pmd->x==0&&pmd->y==0&&pmd->width==0&&pmd->height==0)
	{
		pmd->x =0;
		pmd->y =0;
		pmd->height =1;
		pmd->width =1;
	}
	
	printf("%s %d %s......[%f,%f,%f,%f]\n",__FILE__,__LINE__,__FUNCTION__,pmd->x,pmd->y,pmd->width,pmd->height);

		
	return S_SUCCESS;
}

int PARAMETER_MANAGE::CheckCamSensorParameter(CAMERASENSOR_PARA		*pSensor)
{
	if(pSensor == NULL)
	{
		return -1;
	}
	if((pSensor->m_colorMode > 2)||(pSensor->m_colorMode < 1))
	{
		pSensor->m_colorMode = 1;
	}
	if((pSensor->m_picMode > 5)||(pSensor->m_picMode < 0))
	{
		pSensor->m_picMode = 4;
	}
	if((pSensor->m_PowerFreq > 3)||(pSensor->m_PowerFreq < 0))
	{
		pSensor->m_PowerFreq = 3;
	}
	return S_SUCCESS;
}
int PARAMETER_MANAGE::CheckSystemParameter(SYSTEM_PARAMETER *para)
{

	CheckNetWorkParameter(&(para->m_NetWork));
	
	//CheckAnalogParameter(&(para->m_Analog));

	CheckCameraSetParameter(&(para->m_Camera));

	CheckCamerBlind(&(para->m_CamerBlind));

	CheckCommonParameter(&(para->m_CommPara));

	CheckMachineParameter(&(para->m_Machine));

	CheckCameraMotionDetection(&(para->m_CameraMd));

	CheckUserGroupParameter(&(para->m_Users));

	CheckSysMainetanceParameter(&(para->m_SysMainetance));

	CheckDisplaySetParameter(&(para->m_DisplaySet));

	CheckSysExceptParameter(&(para->m_SysExcept));

	CheckPcDirParameter(&(para->m_PcDir));

	CheckPTZParameter(&(para->m_PTZ));

	CheckPELCOParameter(&(para->m_pelcoCfg));
	
	CheckPicTimerParameter(&(para->m_picTimer));
	
	CheckVoPicParameter(&(para->m_picVo));	

	//CheckPtzLinkParameter(&para->m_PtzLink);
	
	CheckCamSensorParameter(&para->m_Sensor);

	CheckExtendApSettings(&para->m_ParaExtend);

	CheckWebSettings(&para->m_web);
	CheckMdSettings(&para->m_mdset);
	return S_SUCCESS;

}


#if 0
void  PARAMETER_MANAGE::ConvertTrainEncodeSet( TRAIN_VIDEOENCODE_SET *encode_set)
{
	int i = 0;
	int maxframe = 0;

	if(encode_set == NULL)
	{
		return;
	}

	if(encode_set->m_framerat > 4)
	{
		encode_set->m_framerat = 4;
	}

	if(encode_set->m_resolution > 2)
	{
		dprintf(" ******train res : %d *****\n", encode_set->m_resolution);
		encode_set->m_resolution = 2;
		
	}

	if(encode_set->m_resolution == 0)
	{
		maxframe = 2;
	}
	else if(encode_set->m_resolution == 1)
	{
		maxframe = 1;
	}
	else 
	{
		maxframe = 0;
	}

	if(maxframe > encode_set->m_framerat)
	{
		encode_set->m_framerat = maxframe;
	}

	for(i = 0; i < CHANNEL_MAX; i++)
	{
//		m_syspara->m_camera.m_channel_para[i].m_resolution = encode_set->m_resolution;
//		m_syspara->m_camera.m_channel_para[i].m_frame = encode_set->m_framerat;
	}	
	
}
#endif 


void PARAMETER_MANAGE::CheckUserGroupParameter(USERGROUPSET  *UserSet)
{
	int id = 0;	

	for(id = 0; id < 16; id++)
	{
		UserSet->m_UserSet[id].m_u8UserValid = 1; //有效性检测 

	}

}

void PARAMETER_MANAGE::CheckSysMainetanceParameter(SYSTEMMAINETANCE *mainetance)
{

	if(mainetance->m_u8DateModeHour > 23)
	{
		mainetance->m_u8DateModeHour = 0;
	}

	if(mainetance->m_u8DateModeMinute > 59)
	{
		mainetance->m_u8DateModeMinute = 0;
	}

	if(mainetance->m_u8DayInterval > 99)
	{
		mainetance->m_u8DayInterval = 1;
	}

	if(mainetance->m_u8Mode > 2)
	{
		mainetance->m_u8Mode = 0;
	}	

	if(mainetance->m_u8WeekHour > 23)
	{
		mainetance->m_u8WeekHour = 0;
	}

	if(mainetance->m_u8WeekMinute > 59)
	{
		mainetance->m_u8WeekMinute = 0;
	}

	mainetance->m_u8WeekDayValid &=0x7f;

	
	
}


void PARAMETER_MANAGE::CheckDisplaySetParameter(VIDEODISPLAYSET *DisplaySet)
{

	DisplaySet->m_u8VGAMode = (DisplaySet->m_u8VGAMode <3)?DisplaySet->m_u8VGAMode:2;

	DisplaySet->m_u8Alpha = (DisplaySet->m_u8Alpha < 3)?DisplaySet->m_u8Alpha:0;
	
	DisplaySet->m_u32FourValid &=0xf;
	DisplaySet->m_u32SChValid &=0xffff;

	if((DisplaySet->m_u8AlarmCruiseInterval < 5) && (DisplaySet->m_u8AlarmCruiseInterval > 99))
	{
		DisplaySet->m_u8AlarmCruiseInterval = 5;
	}

	if((DisplaySet->m_u8CruiseInterval < 5) && (DisplaySet->m_u8CruiseInterval  >99))
	{
		DisplaySet->m_u8CruiseInterval  = 5;
	}

}

void PARAMETER_MANAGE::CheckSysExceptParameter(GROUPEXCEPTHANDLE *ExceptHandle)
{

	int i = 0;

	for(i = 0; i < 8; i++)
	{
		//ExceptHandle->m_ExceptHandle[i].m_u16AlarmMode &=0x7;
		//ExceptHandle->m_ExceptHandle[i].m_u16AlarmOutSel &=0x0f;
	//	ExceptHandle->m_ExceptHandle[i].m_u16AlarmValid &=0x01;
	}

	for(i = 0; i < CHANNEL_MAX; i++)
	{
//		ExceptHandle->m_VideoLoss[i].m_u16AlarmMode &=0x07;
//		ExceptHandle->m_VideoLoss[i].m_u16AlarmOutSel &=0x0f;
//		ExceptHandle->m_VideoLoss[i].m_u16AlarmValid &=0x01;
	}

}

void PARAMETER_MANAGE::CheckPcDirParameter(PCDIR_PARA *pcdir)
{
	if(((pcdir->m_pcSnapDir[0]>='A') && (pcdir->m_pcSnapDir[0]<='Z'))||
		((pcdir->m_pcSnapDir[0]>='a') && (pcdir->m_pcSnapDir[0]<='z')))
	{
		if(pcdir->m_pcSnapDir[1] != ':')
			strcpy(pcdir->m_pcSnapDir,"D:\\E-AVS_snap");
	}
	else
	{
		strcpy(pcdir->m_pcSnapDir,"D:\\E-AVS_snap");
	}

	if(((pcdir->m_pcRecDir[0]>='A') && (pcdir->m_pcRecDir[0]<='Z'))||
		((pcdir->m_pcRecDir[0]>='a') && (pcdir->m_pcRecDir[0]<='z')))
	{
		if(pcdir->m_pcRecDir[1] != ':')
			strcpy(pcdir->m_pcRecDir,"D:\\E-AVS_recfiles");
	}
	else
	{
		strcpy(pcdir->m_pcRecDir,"D:\\E-AVS_recfiles");
	}

	if(((pcdir->m_pcdLoadDir[0]>='A') && (pcdir->m_pcdLoadDir[0]<='Z'))||
		((pcdir->m_pcdLoadDir[0]>='a') && (pcdir->m_pcdLoadDir[0]<='z')))
	{
		if(pcdir->m_pcdLoadDir[1] != ':')
			strcpy(pcdir->m_pcdLoadDir,"D:\\E-AVS_downloads");
	}
	else
	{
		strcpy(pcdir->m_pcdLoadDir,"D:\\E-AVS_downloads");
	}	
}


void PARAMETER_MANAGE::CheckAlarmInParameter(GROUPALARMINSET *AlarmIn)
{
	int i = 0, j = 0, m = 0;

	for(i = 0; i < 4; i++)
	{
		AlarmIn->m_AlarmIn[i].m_u8AlarmValid = 1;
		AlarmIn->m_AlarmIn[i].m_u32RecSel &= CHANNEL_MASK;
		AlarmIn->m_AlarmIn[i].m_u32AlarmOutSel &= 0x0f;
		AlarmIn->m_AlarmIn[i].m_u32AlarmHandle &= 0x01;
		AlarmIn->m_AlarmIn[i].m_u8TblValid &=0x01;
		for(j = 0; j < 8; j++)
		{
			AlarmIn->m_AlarmIn[i].m_TimeTblSet[j].m_u8WeekDay&=0x07;
			for(m = 0; m < 4; m++)
			{
				AlarmIn->m_AlarmIn[i].m_TimeTblSet[j].m_TBLSection[m].m_u8Valid&=0x01;

				if(AlarmIn->m_AlarmIn[i].m_TimeTblSet[j].m_TBLSection[m].m_u16EndTime > 1439)
				{
					AlarmIn->m_AlarmIn[i].m_TimeTblSet[j].m_TBLSection[m].m_u16EndTime= 1439;
				}
				
				if(AlarmIn->m_AlarmIn[i].m_TimeTblSet[j].m_TBLSection[m].m_u16StartTime > 1439)
				{
					AlarmIn->m_AlarmIn[i].m_TimeTblSet[j].m_TBLSection[m].m_u16StartTime = 1439;
				}
				
			}
		}
		
	}

}

void PARAMETER_MANAGE::CheckRecordTaskParameter(GROUPRECORDTASK *RecordTask)
{
	int  ch = 0;
	int  sect = 0;
	int  week = 0;

	for(ch = 0; ch < CHANNEL_MAX; ch++)
	{
		if(RecordTask->m_ChTask[ch].m_u8PreRecordTime > 20)
		{
			RecordTask->m_ChTask[ch].m_u8PreRecordTime = 20;
		}

		if(RecordTask->m_ChTask[ch].m_u8RecordDelay > 180)
		{
			RecordTask->m_ChTask[ch].m_u8RecordDelay = 180;
		}

		for(week = 0; week < 8; week++)
		{

			for(sect = 0; sect < 4; sect++)
			{
				if(RecordTask->m_ChTask[ch].m_TimerTask[week].m_uTimeTbl[sect].m_u16StartTime > (23*60+59))	
				{
					RecordTask->m_ChTask[ch].m_TimerTask[week].m_uTimeTbl[sect].m_u16StartTime = 23*60+59;
				}

				if(RecordTask->m_ChTask[ch].m_TimerTask[week].m_uTimeTbl[sect].m_u16EndTime > (23*60+59))	
				{
					RecordTask->m_ChTask[ch].m_TimerTask[week].m_uTimeTbl[sect].m_u16EndTime = 23*60+59;
				}

				RecordTask->m_ChTask[ch].m_TimerTask[week].m_uTimeTbl[sect].m_u8Valid &=0x01;


				if(RecordTask->m_ChTask[ch].m_TimerTask[week].m_uMdTbl[sect].m_u16StartTime > (23*60+59))	
				{
					RecordTask->m_ChTask[ch].m_TimerTask[week].m_uMdTbl[sect].m_u16StartTime = 23*60+59;
				}

				if(RecordTask->m_ChTask[ch].m_TimerTask[week].m_uMdTbl[sect].m_u16EndTime > (23*60+59))	
				{
					RecordTask->m_ChTask[ch].m_TimerTask[week].m_uMdTbl[sect].m_u16EndTime = 23*60+59;
				}

				RecordTask->m_ChTask[ch].m_TimerTask[week].m_uMdTbl[sect].m_u8Valid &=0x01;


				if(RecordTask->m_ChTask[ch].m_TimerTask[week].m_uAlarmTbl[sect].m_u16StartTime > (23*60+59))	
				{
					RecordTask->m_ChTask[ch].m_TimerTask[week].m_uAlarmTbl[sect].m_u16StartTime = 23*60+59;
				}

				if(RecordTask->m_ChTask[ch].m_TimerTask[week].m_uAlarmTbl[sect].m_u16EndTime > (23*60+59))	
				{
					RecordTask->m_ChTask[ch].m_TimerTask[week].m_uAlarmTbl[sect].m_u16EndTime = 23*60+59;
				}

				RecordTask->m_ChTask[ch].m_TimerTask[week].m_uAlarmTbl[sect].m_u8Valid &=0x01;				
				
				
			}
			
		}
		
		//RecordTask->m_ChTask[ch].m_TimerTask[week].m_uTimeTbl[sect].m_u8Valid = 1;
	}



}


int PARAMETER_MANAGE::CheckMegaSet(MegaEyes_PARA *MegaSet)
{
	if(MegaSet == NULL) return S_FAILURE;
	
	if(strlen(MegaSet->m_netWorkMegaEyes.m_uAccDomain) == 0)
//		strcpy(MegaSet->m_netWorkMegaEyes.m_uAccDomain,"117.25.223.22" );  // 互信互通
		strcpy(MegaSet->m_netWorkMegaEyes.m_uAccDomain,"113.105.146.146" ); // 我们自己的平台

	if(MegaSet->m_netWorkMegaEyes.m_MsgPort == 0)
		MegaSet->m_netWorkMegaEyes.m_MsgPort = 4062;

	// 单路ipcid测试用ID：321706F346CC6495196B 
	if(strlen((char*)MegaSet->m_netWorkMegaEyes.m_deviceid) == 0)
		memcpy(MegaSet->m_netWorkMegaEyes.m_deviceid,"321706F346CC6495196B",20);

	if(MegaSet->m_netWorkMegaEcho.r_HeartCycle == 0)
		MegaSet->m_netWorkMegaEcho.r_HeartCycle = 120;

	return S_SUCCESS;
}

int PARAMETER_MANAGE::ExportOutAllPara()
{
	DIR *dir = NULL;
	int retval = -1;
	int fd = -1;
	char filename[64] = {0};
	PARACONFFILEHEADER	header;

	
	if((dir = opendir(EXTERN_PARACONFIG_PATH)) !=NULL)
	{
		closedir(dir);	
	}
	else 
	{
		retval = mkdir(EXTERN_PARACONFIG_PATH, 0777);
		if(retval < 0)
		{
			printf("open para config path failure: %s \n", strerror(errno));
			return S_FAILURE;
		}
	}
	
	strcpy(filename, EXTERN_PARACONFIG_PATH);
	strcat(filename, PARACONFIG_BASENAME);
	memset(&header, 0, sizeof(PARACONFFILEHEADER));
	header.m_magic = PARA_MAGIC_NUM;
	
	fd = open(filename, O_RDWR|O_CREAT);
	if(fd < 0)
	{
		printf("creat config file failure \n");
		return S_FAILURE;
	}
	retval = write(fd, &header, sizeof(PARACONFFILEHEADER));
	if(retval < 0)
	{

		return S_FAILURE;
	}

	retval = write(fd, m_syspara, sizeof(SYSTEM_PARAMETER));
	if(retval < 0)
	{
		return S_FAILURE;
	}
	
	sync();	
	sleep(2);
	close(fd);

	
	return S_SUCCESS;
	
}

int PARAMETER_MANAGE::ExportInAllPara(SYSTEM_PARAMETER *para)
{
	char filename[64] = {0};
	PARACONFFILEHEADER	header;
	int fd = -1;
	int retval = -1;
	SYSTEM_PARAMETER sys_para;
	
	strcpy(filename, EXTERN_PARACONFIG_PATH);
	strcat(filename, PARACONFIG_BASENAME);
	memset(&header, 0, sizeof(PARACONFFILEHEADER));
	header.m_magic = PARA_MAGIC_NUM;
	
	fd = open(filename, O_RDONLY);
	if(fd < 0)
	{
		printf("creat config file failure \n");
		return S_FAILURE;     
	}

	retval = read(fd, &header, sizeof(PARACONFFILEHEADER));
	if(retval < 0)
	{
		return S_FAILURE;
	}

	if(header.m_magic != PARA_MAGIC_NUM)
	{
		printf("error file magic : %d \n", header.m_magic);
		return S_FAILURE;
	}

	retval = read(fd, &sys_para, sizeof(SYSTEM_PARAMETER));
	if(retval != sizeof(SYSTEM_PARAMETER))
	{
		printf("error para config size \n");
		return S_FAILURE;
	}

	memcpy(m_syspara, &sys_para, sizeof(SYSTEM_PARAMETER));
	
	retval = SaveParameter2File(&sys_para);

	if(fd > 0)
	{
	 	close(fd);
	}
	
	return retval;
	
}


int PARAMETER_MANAGE::LoadDefaultParaToFile(SYSTEM_PARAMETER *para)
{
	int retval = S_FAILURE;
	
	if(para == NULL)
	{
		DPRINTF(" %s : %d \n", __FUNCTION__, __LINE__);
		return S_FAILURE;
	}

	LoadParameterDefault(para);
	if(m_syspara != para)
		memcpy(m_syspara, para, sizeof(SYSTEM_PARAMETER));
	retval = SaveParameter2File( para);

	return retval;
}

int PARAMETER_MANAGE::GetSysParameter(int type,  void* para)
{
	int ch = 0;

	if(startupdate == 0)
	{
		sleep(10000);
		return S_FAILURE;
	}
	if(para == NULL)
	{
		printf(" para pointer null \n");
		return S_FAILURE;
	}

	switch(type)
	{
		case SYSNET_SET:
			memcpy(para, &m_syspara->m_NetWork, sizeof(NETWORK_PARA));
			break;


		case SYSRECTASK_SET:
//			memcpy(para, &m_syspara->m_RecordTask, sizeof(REC_TASK_PARA));
			break;

		case SYSMACHINE_SET:
			memcpy(para, &m_syspara->m_Machine, sizeof(MACHINE_PARA));
			break;

		case SYSCAMERA_SET:
			memcpy(para, &m_syspara->m_Camera, sizeof(CAMERA_PARA));
			break;

		case SYSANALOG_SET:
			memcpy(para, &m_syspara->m_Analog, sizeof(CAMERA_ANALOG));
			break;

		case SYSPCDIR_SET:
			memcpy(para, &m_syspara->m_PcDir, sizeof(PCDIR_PARA));
			break;


		case SYSCOMMON_SET:
			memcpy(para, &m_syspara->m_CommPara, sizeof(COMMON_PARA));
			break;

		case SYSBLIND_SET:
			memcpy(para, &m_syspara->m_CamerBlind, sizeof(CAMERA_BLIND));
			break;

		case SYSMOTION_SET:
			memcpy(para, &m_syspara->m_CameraMd, sizeof(CAMERA_MD));
			break;

		case SYSPTZ_SET:
			memcpy(para, &m_syspara->m_PTZ, sizeof(PTZ_PARA));
			break;
			
		case SYSREC00TASK_SET:
		case SYSREC01TASK_SET:
		case SYSREC02TASK_SET:
		case SYSREC03TASK_SET:
		case SYSREC04TASK_SET:
		case SYSREC05TASK_SET:
		case SYSREC06TASK_SET:
		case SYSREC07TASK_SET:
			ch = type - SYSREC00TASK_SET;
			memcpy(para, &m_syspara->m_RecordSchedule.m_ChTask[ch], sizeof(RECORDTASK));
			break;
			
		case SYSRECSCHEDULE_SET:
			memcpy(para, &m_syspara->m_RecordSchedule, sizeof(GROUPRECORDTASK));
			break;

		case  SYSMAINETANCE_SET:  // 系统维护
			memcpy(para, &m_syspara->m_SysMainetance, sizeof(SYSTEMMAINETANCE));
			break;
			
		case SYSDISPLAY_SET:
			memcpy(para, &m_syspara->m_DisplaySet, sizeof(VIDEODISPLAYSET));
			break;
				
		case SYSUSERPREMIT_SET:
			memcpy(para, &m_syspara->m_Users, sizeof(USERGROUPSET));
			break;
				
		case SYSEXCEPT_SET:
			memcpy(para, &m_syspara->m_SysExcept, sizeof(GROUPEXCEPTHANDLE));
			break;
				
		case SYSOSDINSERT_SET:
			memcpy(para, &m_syspara->m_OsdInsert, sizeof(VIDEOOSDINSERT));
			break;
			
		case SYSALARMZONE_SET:
			memcpy( para, &m_syspara->m_ZoneGroup, sizeof(ALARMZONEGROUPSET));
			break;

		case SYSDEFSCHEDULE_SET:
			//memcpy(para, &m_syspara->m_DefSchedule, sizeof(DefenceScheduleSet));
			break;

		case SYSPTZLINK_SET:
			//memcpy(para, &m_syspara->m_PtzLink, sizeof(GroupZonePtzLinkSet));
			break;

		case SYS3G_SET:
			//memcpy(para, &m_syspara->m_3g, sizeof(G3G_CONFIG));
			break;

		case SYS3G_DIAL_SET:
			memcpy(para, &m_syspara->m_3gDial, sizeof(G3G_DIAL_CONFIG));
			break;
			
		case PELCOCMD_SET:
			memcpy(para, &m_syspara->m_pelcoCfg, sizeof(PELCO_CmdCfg));
			break;
			
		case PICTIMER_SET:
			memcpy(para, &m_syspara->m_picTimer, sizeof(PICTURE_TIMER));
			break;
			
		case VOPIC_SET:
			memcpy(para, &m_syspara->m_picVo, sizeof(VODEV_ANALOG));
			break;

		case NETDECODER_SET:
			memcpy(para, &m_syspara->m_Netdecoder, sizeof(NETDECODER_PARA));
			break;
		case ALARMPORT_SET:
			memcpy(para, &m_syspara->m_AlarmPort, sizeof(AlarmOutPort));
			break;
		case VIDEOLOSS_SET:
			memcpy(para, &m_syspara->m_CamerVideoLoss, sizeof(CAMERA_VideoLoss));
			break;
		case SENSOR_SET:
			memcpy(para, &m_syspara->m_Sensor, sizeof(CAMERASENSOR_PARA));
			break;
		case EXTEND_SET:
			memcpy(para, &m_syspara->m_ParaExtend, sizeof(PARAMETEREXTEND));
			break;
		case WEB_SET:
			memcpy(para, &m_syspara->m_web, sizeof(web_sync_param_t));
			break;
		case MD_SET:
			memcpy(para, &m_syspara->m_mdset, sizeof(P2P_MD_REGION_CHANNEL));
			break;

		default:      
			return S_FAILURE;

	}

	return S_SUCCESS;

}

int PARAMETER_MANAGE::SetSystemParameter(int type,  void* para)
{
	int ch = 0;

	if(para == NULL)
	{
		printf(" para pointer null \n");
		return S_FAILURE;
	}

	switch(type)
	{
		case SYSNET_SET:
			
			memcpy( &m_syspara->m_NetWork, para, sizeof(NETWORK_PARA));
			//m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.m_Reserved = 1;
			printf("###### LoginWifiDev.AuthenticationMode = %d\n",m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.AuthenticationMode);
			printf("###### LoginWifiDev.lang = %d\n",m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.lang);
			printf("###### LoginWifiDev.ConnectStatus = %d\n",m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.ConnectStatus);
			printf("###### LoginWifiDev.EncryptionProtocol = %d\n",m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.EncryptionProtocol);
			printf("###### LoginWifiDev.Index = %d\n",m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.Index);
			printf("###### LoginWifiDev.m_Reserved = %d\n",m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.m_Reserved);
			printf("###### LoginWifiDev.Passwd = %s\n",m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.Passwd);
			printf("###### LoginWifiDev.RouteDeviceName = %s\n",m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.RouteDeviceName);
			printf("###### LoginWifiDev.SignalLevel = %d\n",m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.SignalLevel);
			printf("###### LoginWifiDev.WepKeyMode = %d\n",m_syspara->m_NetWork.m_WifiConfig.LoginWifiDev.WepKeyMode);		
			break;

		case SYSRECTASK_SET:
//			memcpy(para, &m_syspara->m_RecordTask, sizeof(REC_TASK_PARA));
			break;

		case SYSMACHINE_SET:
			memcpy(&m_syspara->m_Machine, para,  sizeof(MACHINE_PARA));
			break;

		case SYSCAMERA_SET:
			memcpy( &m_syspara->m_Camera, para, sizeof(CAMERA_PARA));
			
			break;

		case SYSANALOG_SET:
			memcpy(&m_syspara->m_Analog, para, sizeof(CAMERA_ANALOG));
			//printf("******LN need to %d%d%d\n",m_syspara->m_Analog.m_Channels[0].m_nBrightness,m_syspara->m_Analog.m_Channels[0].m_nContrast,m_syspara->m_Analog.m_Channels[0].m_nSaturation);
			
			break;

		case SYSPCDIR_SET: 
			memcpy(&m_syspara->m_PcDir, para, sizeof(PCDIR_PARA));
			break;

		case SYSCOMMON_SET:
			memcpy(&m_syspara->m_CommPara, para, sizeof(COMMON_PARA));
			break;

		case SYSBLIND_SET:
			memcpy(&m_syspara->m_CamerBlind, para, sizeof(CAMERA_BLIND));
			break;

		case SYSMOTION_SET:
			memcpy(&m_syspara->m_CameraMd, para, sizeof(CAMERA_MD));
			break;

		case SYSPTZ_SET:
			memcpy(&m_syspara->m_PTZ, para, sizeof(PTZ_PARA));
			break;
			
		case SYSREC00TASK_SET:
		case SYSREC01TASK_SET:
		case SYSREC02TASK_SET:
		case SYSREC03TASK_SET:
		case SYSREC04TASK_SET:
		case SYSREC05TASK_SET:
		case SYSREC06TASK_SET:
		case SYSREC07TASK_SET:
			ch = type - SYSREC00TASK_SET;
			memcpy(&m_syspara->m_RecordSchedule.m_ChTask[ch], para, sizeof(RECORDTASK));
			
			break;
			
		case SYSRECSCHEDULE_SET:
			memcpy(&m_syspara->m_RecordSchedule, para, sizeof(GROUPRECORDTASK));
			printf("获得时间表信息  \n");
			printf("ch 0   ehour %d  , eminute %d   \n", m_syspara->m_RecordSchedule.m_ChTask[0].m_TimerTask[0].m_uTimeTbl[0].m_u16StartTime, 
				m_syspara->m_RecordSchedule.m_ChTask[0].m_TimerTask[0].m_uTimeTbl[0].m_u16EndTime);
			break;

		case  SYSMAINETANCE_SET:  // 系统维护
			memcpy(&m_syspara->m_SysMainetance, para, sizeof(SYSTEMMAINETANCE));
			break;
			
		case SYSDISPLAY_SET:
			memcpy(&m_syspara->m_DisplaySet, para, sizeof(VIDEODISPLAYSET));
			break;
				
		case SYSUSERPREMIT_SET:
			memcpy(&m_syspara->m_Users, para, sizeof(USERGROUPSET));
			break;
				
		case SYSEXCEPT_SET:
			memcpy(&m_syspara->m_SysExcept, para, sizeof(GROUPEXCEPTHANDLE));
			break;
				
		case SYSOSDINSERT_SET:
			memcpy( &m_syspara->m_OsdInsert, para,sizeof(VIDEOOSDINSERT));
			break;

		case SYSALARMZONE_SET:
			memcpy(&m_syspara->m_ZoneGroup, para, sizeof(ALARMZONEGROUPSET));
			break;
			
		case SYSDEFSCHEDULE_SET:
			//memcpy(&m_syspara->m_DefSchedule, para, sizeof(DefenceScheduleSet));
			break;
		
		case SYSPTZLINK_SET:
			//memcpy(&m_syspara->m_PtzLink,para, sizeof(GroupZonePtzLinkSet));
			break;

		case SYS3G_SET:
			//memcpy(&m_syspara->m_3g, para, sizeof(G3G_CONFIG));
			break;

		case SYS3G_DIAL_SET:
			memcpy(&m_syspara->m_3gDial, para, sizeof(G3G_DIAL_CONFIG));
			break;
			
		case PELCOCMD_SET:
			memcpy(&m_syspara->m_pelcoCfg, para, sizeof(PELCO_CmdCfg));
			break;

		case PICTIMER_SET:
			memcpy(&m_syspara->m_picTimer, para, sizeof(PICTURE_TIMER));
			break;

		case VOPIC_SET:
			memcpy(&m_syspara->m_picVo, para, sizeof(VODEV_ANALOG));
			break;

		case NETDECODER_SET:
			memcpy(&m_syspara->m_Netdecoder, para, sizeof(NETDECODER_PARA));
			break;
		case ALARMPORT_SET:
			memcpy(&m_syspara->m_AlarmPort, para,  sizeof(AlarmOutPort));
			break;
		case VIDEOLOSS_SET:
			memcpy(&m_syspara->m_CamerVideoLoss, para, sizeof(CAMERA_VideoLoss));
			break;
		case SENSOR_SET:
			memcpy(&m_syspara->m_Sensor, para, sizeof(CAMERASENSOR_PARA));
			break;
		case EXTEND_SET:
			memcpy(&m_syspara->m_ParaExtend, para, sizeof(PARAMETEREXTEND));
			break;
		case WEB_SET:
			memcpy(&m_syspara->m_web, para, sizeof(web_sync_param_t));
			break;
		case MD_SET:
			memcpy(&m_syspara->m_mdset, para, sizeof(P2P_MD_REGION_CHANNEL));
			break;

		default:      
			printf("noexist para type  \n");
			return S_FAILURE;

	}

	SaveSystemParameter(m_syspara);

	return S_SUCCESS;

}

int PARAMETER_MANAGE::GetSingleParameter(int type, void * value)
{
	switch(type)
	{
		case TIMEINSERT_SET:
			memcpy(value, &m_syspara->m_CommPara.m_uTimeInSert, sizeof(unsigned char));		
			break;

		default:
			return S_FAILURE;
	}

	return S_SUCCESS;
}

