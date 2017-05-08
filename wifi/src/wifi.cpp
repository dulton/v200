#include <net/if.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<signal.h>
#include<unistd.h>

#include <malloc.h>
#include "GpioApi.h"
#include "common.h"
#include "pthread.h"

#include "wifi.h"
#include "ModuleFuncInterface.h"
#include "interfacedef.h"
#include "zmdnetlib.h"
#include "Video_MD.h"
#include "DevSmartLink.h"
//#include "3518c_mini_ipc_gpio.h"  
#include "wificonfig.h"
#include "led.h"
#define iw_path "/app/wifi/tools/iw"
typedef enum WIFI_Connect_Time 
{ 
	START_CONNECT=0, 
	SD_FIRST_LINK , 
	RECONNECT_LINK 

}WIFI_Connect_Time;

/* from libwifi.a */
WIFI_Connect_Time WIFI_connectTime; 
char get_router_mac[32];


#define		COMPLETED 		0
#define		PASSWORDERR 	1
#define		SACNNING 		2
#define		MODESTA			3


char	net_name[32];//eg:ra0 ,wlan0
char	src_sun_path[32];//
char	dest_sun_path[32];//
static int 	ctrl_socket=-1;
static bool Smartlinkrun = false;
static bool SmartlinkPlayVoice = false;
WIFI_Connect_Status WIFI_ConnectStatus =UNCONNECTED;

static	pthread_t	WifiConnect_pid;
extern Audio *paudio;
typedef struct tagWifiSearchInfo
{
	char	ssid[32];	
	int		Encrypt;
}S_WIFI_INFO;
#define MAX_AP	50
S_WIFI_INFO	Search_ap_info[MAX_AP];
int		search_ap_count=0;

void *WifiProceess_Thread(void *para);
int  Wifi_wpa_ctrl_open();
int Wifi_wpa_ctrl_request(int socket , const char *cmd, size_t cmd_len, char *reply, int *reply_len);
int ZMD_Connect_Wifi(int EnCryPt,char *ssid,char *passwd);
int Smartlinkcallback(char *ssid ,char *password, int lang);
void UpdateWIFI_Mac();
int SetWifiMac(unsigned char *mac);
void RetartWPA();
int	GetConnectStatus();
int GetWifiStatus();


int InitWifi()
{
	SetWifiMac(NULL);
	RetartWPA();
	strcpy(src_sun_path,"/config/wlan0");
	strcpy(dest_sun_path,"/config/run/wpa_supplicant/wlan0");	
	ctrl_socket=Wifi_wpa_ctrl_open();
	WIFI_ConnectStatus = WIFI_MONITOR;
	CreateWifiLedProcess(&WIFI_ConnectStatus);
	ZMD_StartQRScan(true,Smartlinkcallback,GetConnectStatus);	
	printf("\n\n\n tuyanlin============================InitWifi ctrl_socket:%d\n",ctrl_socket );
	if(pthread_create(&WifiConnect_pid, NULL, WifiProceess_Thread, NULL) < 0)//创建线程
	{
		printf("Fastlink_Thread failed \n");
		return -1;
	}	
	return 0;
}

void ParseOneLineScanResults(char *line,TYPE_WIFI_LOGIN *pwifi)
{
    int space_count = 0;
    char data[255] = {0};
    unsigned char index = 0;

    while(*line != '\0')
    {
        if (*line != '\t' &&  *(line + 1) != '\0')
        {
            data[index] = *line;
            index++;
        }
        else
        {
            if (space_count == 0)   //signal Level
            {
                unsigned char i ;
                for (i = 0; i < index; i++)
            	{
	            //	printf("ivan:SignalLevel=%s\n",data);
					
					//wifilogin.SignalLevel = ParseAtherosSig(data);
					pwifi->SignalLevel = atoi(data);
					
	               // wifilogin.SignalLevel = wifilogin.SignalLevel * 10  + data[i] - '0';
	            // printf("%s wifilogin.SignalLevel ::%d\n",__FUNCTION__,wifilogin.SignalLevel);
            	}
            }
            else  if (space_count == 1)
            {
                unsigned char i ;
                for(i = 0; i < index; i++)
                {
                    if (strncmp(&data[i], "WEP", strlen("WEP")) == 0)
                    {
                        pwifi->EncryptionProtocol |= 0x01;
                    }
                    if ((strncmp(&data[i], "WPA", strlen("WPA")) == 0 )
                            && (data[i + 3] != '2'))
                    {
                        pwifi->EncryptionProtocol |= 0x02;
                    }
                    if (strncmp(&data[i], "WPA2", strlen("WPA2")) == 0)
                    {
                        pwifi->EncryptionProtocol |= 0x04;
                    }
                }
				Search_ap_info[search_ap_count].Encrypt = pwifi->EncryptionProtocol;
                //printf("%s wifilogin.EncryptionProtocol ::%d\n",__FUNCTION__,wifilogin.EncryptionProtocol);
            }
            else  if ( space_count == 2)        //SSID name
            {
                data[index] = *line;
                index++;
                memcpy(pwifi->RouteDeviceName, data, index );
				strcpy(Search_ap_info[search_ap_count].ssid,pwifi->RouteDeviceName);
                //printf("%s wifilogin.RouteDeviceName ::%s\n",__FUNCTION__,wifilogin.RouteDeviceName);
            }

            index = 0;
            space_count++;
        }
        line++;
    }
}

void GetAP_Lists(char *sendBuf,unsigned int  *apCount)
{
	int reply_len =0;
	char *data = (char *)malloc(MAXAPBUFFER);
	memset(data,0x0,MAXAPBUFFER);
	reply_len = MAXAPBUFFER;
	printf("===================================GetAP_Lists\n");
	if(Wifi_wpa_ctrl_request(ctrl_socket, "SCAN", strlen("SCAN"), data, &reply_len)<0)
	{	
		if(data)
			free(data);
		return ;
	}
	printf("=====================SCAN reply_len:%d\n%s\n",reply_len,data);
	sleep(2);
	memset(data,0x0,MAXAPBUFFER);
	reply_len = MAXAPBUFFER;
	if(Wifi_wpa_ctrl_request(ctrl_socket, "SCAN_RESULTS", strlen("SCAN_RESULTS"), data , &reply_len)<0)
	{	
		if(data)
			free(data);
		return ;
	}
	printf("=====================SCAN_RESULTS reply_len:%d\n%s\n",reply_len,data);
	char *line;
	char *outer_ptr = NULL;
	TYPE_WIFI_LOGIN localwifilogin;
	line = strtok_r(data, "\n", &outer_ptr); //delete the first line //strtok_r(buf,",",&outer_ptr))!=NULL
	line = strtok_r(NULL, "\n", &outer_ptr);

	NETWORK_PARA netset;
	g_cParaManage->GetSysParameter(SYSNET_SET, &netset);
	memset(&localwifilogin, 0, sizeof(localwifilogin));
	GetWIFI_Status(&localwifilogin);
	int index = sizeof(Cmd_Header);
	TYPE_WIFI_LOGIN wifi_info;

	search_ap_count = 0;
	memset(Search_ap_info,0x0,sizeof(Search_ap_info));
	while  (line != NULL)
	{
		line = line + 23; //delet the first 33 char of BSSID ;

		memset(&wifi_info, 0, sizeof(wifi_info)); //clear zero
		ParseOneLineScanResults(line,&wifi_info);	
		search_ap_count++;
		if (strlen(&wifi_info.RouteDeviceName[0]) !=  0x00)  //ssid  非空的情况
		{

			*apCount = *apCount + 1;
			if (strncmp(&localwifilogin.RouteDeviceName[0], &wifi_info.RouteDeviceName[0], 32) == 0)
			{
				printf("k==== %d ,%s\n", *apCount, &wifi_info.RouteDeviceName[0]);
				wifi_info.ConnectStatus = localwifilogin.ConnectStatus;
			}			
			//printf("%s %d %d\n",wifilogin.RouteDeviceName,wifilogin.EncryptionProtocol,wifilogin.SignalLevel);
			memcpy((sendBuf + index), &wifi_info, sizeof(TYPE_WIFI_LOGIN));
			index += sizeof(TYPE_WIFI_LOGIN);

		}
		line =	strtok_r(NULL, "\n", &outer_ptr);
		if ( *apCount > 39 )
			break;

	}
	if(data)
		free(data);


   
	return  ;

}

int GetWIFI_Status(TYPE_WIFI_LOGIN *pwifidev)
{
	char data[1024] = {0};
    int reply_len;    
    reply_len = sizeof(data) - 1;
    if(Wifi_wpa_ctrl_request(ctrl_socket, "STATUS", strlen("STATUS"), data, &reply_len)<0)
	{
		printf("send cmd STATUS error\n");
		return 0;
	}
	if(strlen(data)==0)
		return 0;
	int retvalue = 0;
	// check if  passwd error 
	//printf("=================================\n");
	//printf("%s\n",data);
	//printf("=================================\n");
	if(strstr(data, "wpa_state=PASSWORD ERROR") != NULL)
	{
		pwifidev->ConnectStatus = 0x02;
		retvalue|=(1<<PASSWORDERR);
		printf("device is connected passwd error %s  %s %d\n", __FILE__, __FUNCTION__, __LINE__);  
		return retvalue;
	}
	else
	{
		char *ptemp = NULL;
		if(strstr(data, "bssid") != NULL)
			ptemp=&data[5];
		else
			ptemp=data;
		if(strstr(ptemp, "wpa_state=COMPLETED") != NULL)
		{
			char *pssid = strstr(ptemp, "ssid=");
			if(pssid!=NULL)
			{
				pwifidev->ConnectStatus = 0x01;
				pssid+=strlen("ssid=");
				int i=0;				
				while(*pssid != 0 )
				{
					pwifidev->RouteDeviceName[i++]=*pssid;
					pssid++;
					if(*pssid=='\n')
						break;
				}
				//printf("=====ssid:%s\n",pwifidev->RouteDeviceName);
			}
			retvalue|=(1<<COMPLETED);
			retvalue|=(1<<MODESTA);
			return retvalue;
		}	
		if(strstr(ptemp, "wpa_state=SCANNING") != NULL)
		{
			retvalue|=(1<<SACNNING);
		}

		if(strstr(ptemp, "mode=station") != NULL)
		{
			retvalue|=(1<<MODESTA);
		}
		
	}
	
	return retvalue;	
}




void RetartWPA()
{
	char cmd[256] ={0};
	
	system("killall -9 wpa_supplicant");
	sprintf(cmd,"%s -B -Dwext -i%s -c%s&",WPA_SUPPALICANT_RUN_PATH,WIFI_INTERFACE,WIFI_CONFIG);
	system(cmd);
	sleep(2);
	printf("%s:%s\n",__FUNCTION__,cmd);
	
}

//设置wifi MAC地址
int SetWifiMac(unsigned char *mac)
{	
	char cmd[64]={0x0};
	char strMac[32]={0};
	char tmp[6]={0x0};
	int fd=-1;
	if(mac == NULL)
	{
		fd = open(WIFI_MAC_CFG_FILE, O_RDONLY);
		if(fd < 0)//不存在就创建mac地址文件并写入mac
		{
		
			
			struct timeval tv_now ;
			tmp[0] =0x04;
			tmp[1] =0x5c;
			tmp[2] =0x06;
			int j =3;
			for(;j<6;j++)
			{
				gettimeofday( &tv_now , NULL ) ;
				unsigned char  mac = tv_now.tv_usec%256 ;	
				tmp[j] =mac;
				usleep(200);
			}			
			sprintf(strMac,"%02x:%02x:%02x:%02x:%02x:%02x",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
			fd = open(WIFI_MAC_CFG_FILE,  O_CREAT|O_RDWR,0777);
			if(fd>0)
			{
				write(fd , tmp , 6);
				
			}				

		}
		else//存在，直接读取mac地址
		{
			read(fd , tmp , 6);
			sprintf(strMac,"%02x:%02x:%02x:%02x:%02x:%02x",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
		}
		if(fd>0)
			close(fd);

	}
	else
	{
		
		sprintf(strMac,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		fd = open(WIFI_MAC_CFG_FILE,  O_CREAT|O_RDWR,0777);	
		
		if(fd>0)
		{
			read(fd , tmp , 6);
			lseek(fd, 0, SEEK_SET);
			if(memcmp(tmp,mac,6)==0)
			{
				printf("================================= mac is same!\n");
				close(fd);
				return 0;
			}
			write(fd , mac , 6);
		}
		else
		{
			return 0;
		}
		printf("====net:%s|local:%02x,%02x,:%02x,%02x,%02x,%02x\n",strMac,tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
		if(fd>0)
			close(fd);
		NETWORK_PARA netset;
		g_cParaManage->GetSysParameter(SYSNET_SET, &netset);
		memcpy( netset.m_WifiConfig.WifiAddrMode.m_uMac , mac , 6 ) ;
		g_cParaManage->SetSystemParameter(SYSNET_SET,&netset);
		RebootSystem();
		return 0;

	}
	
	sprintf(cmd,"ifconfig %s down",WIFI_INTERFACE);
	system(cmd);	
	printf("============================cmd:%s\n",cmd);
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"ifconfig %s hw ether %s",WIFI_INTERFACE,strMac);
	//sprintf(cmd,"ifconfig %s down hw ether %s",WIFI_INTERFACE,strMac);	
	system(cmd);
	sleep(1);
	printf("==============================cmd:%s\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",cmd);
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"ifconfig %s up",WIFI_INTERFACE);
	system(cmd);
	sleep(1);	
	
	return 0;
}

unsigned char ASCIItohex(unsigned char data)
{
   unsigned res = 0x00;
   switch(data)
   {
     case '0':
	 	res = 0;
	 break;
	 case '1':
	 	res = 1;
	 break;
	 case '2':
	 	res = 2;
	 break;
	 case '3':
	 	res = 3;
	 break;
	 case '4':
	 	res = 4;
	 break;
	 case '5':
	 	res = 5;
	 break;
	 case '6':
	 	res = 6;
	 break;
	 case '7':
	 	res = 7;
	 break;
	 case '8':
	 	res = 8;
	 break;
	 case '9':
	 	res = 9;
	 break;
	 case 'A':
	 case 'a':
	 	res = 10;
	 break;
	 case 'B':
	 case 'b':
	 	res = 11;
	 break;
	 case 'C':
	 case 'c':
	 	res = 12;
	 break;
	 case 'D':
	 case 'd':
	 	res = 13;
	 break;
	 case 'E':
	 case 'e':
	 	res = 14;
	 break;
	 case 'F':
	 case 'f':
	 	res = 15;
	 break;
	 default :
		res = 0x00;
	 	break;
	 	  
   }
   return (res);
}

//读取wifi MAC地址
int GetWifiMac(unsigned char *mac)
{



	int fd = open(WIFI_MAC_CFG_FILE, O_RDONLY);
	if(fd > 0)
	{
		read(fd , mac , 6);
		close(fd);
		return 0;
	}
	else //不存在
	{
		int ret = 0; 
		size_t i,j;
		unsigned char strMac[32] = {0};
		char line[30] = {0};
		char netname[]= WIFI_INTERFACE;
		struct ifreq req; 
	 
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
		if ( -1 == sockfd ) 
		{ 
			return false; 
		} 
	
		bzero(&req, sizeof(struct ifreq)); 
		strcpy(req.ifr_name, netname); 
		if ( ioctl(sockfd, SIOCGIFHWADDR, &req) >= 0 ) 
		{ 
				sprintf( 
				line, "%02x:%02x:%02x:%02x:%02x:%02x", 
				(unsigned char)req.ifr_hwaddr.sa_data[0], 
				(unsigned char)req.ifr_hwaddr.sa_data[1], 
				(unsigned char)req.ifr_hwaddr.sa_data[2], 
				(unsigned char)req.ifr_hwaddr.sa_data[3], 
				(unsigned char)req.ifr_hwaddr.sa_data[4], 
				(unsigned char)req.ifr_hwaddr.sa_data[5] 
			); 
			printf("%s mac=[%s]\n",netname,line);
		} 
		else 
		{ 
			perror(__FUNCTION__);
			ret = false; 
		} 
		
		for(i = 0 ,j = 0;i < 27 ; i++)
		{
		 if (line[i] != ':')
			 {	   
			strMac[j] = ASCIItohex(line[i]);
			j++;
			 }
				
	   }
	   
	   for(i = 0; i < 6; i++) 
	   {
		  mac[i] = (strMac[i*2] * 16 + strMac[i*2 + 1]);
		  printf("%02x ",mac[i]);
	   }
	   
		if ( sockfd != -1 ) 
		{ 
			close(sockfd); 
			sockfd = -1; 
		}	

	}
	
	
	
	return 0;
}


int StartConnectWifi(TYPE_WIFI_LOGIN* wifilogin)
{
	printf("%s wifilogin.RouteDeviceName ::%s\n",__FUNCTION__,wifilogin->RouteDeviceName);	
	printf("%s wifilogin.AuthenticationMode ::%d\n",__FUNCTION__,wifilogin->AuthenticationMode);
	printf("%s wifilogin.Passwd ::%s\n",__FUNCTION__,wifilogin->Passwd);
	printf("%s wifilogin.EncryptionProtocol ::%d\n",__FUNCTION__,wifilogin->EncryptionProtocol);
	printf("%s wifilogin.WepKeyMode ::%d\n",__FUNCTION__,wifilogin->WepKeyMode);
	printf("%s wifilogin.m_Reserved ::%d\n",__FUNCTION__,wifilogin->m_Reserved);
	printf("ZMD_Connect_Wifi====================FUNC:%s,LINE:%d\n",__FUNCTION__,__LINE__);

	if(0==ZMD_Connect_Wifi(wifilogin->EncryptionProtocol,wifilogin->RouteDeviceName,wifilogin->Passwd))
		WIFI_ConnectStatus = CONNECTING;
	NETWORK_PARA netset;
	memset(&netset, 0x0, sizeof(NETWORK_PARA));
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);	
	netset.m_WifiConfig.LoginWifiDev.EncryptionProtocol = wifilogin->EncryptionProtocol;
	strcpy(netset.m_WifiConfig.LoginWifiDev.RouteDeviceName,wifilogin->RouteDeviceName);
	strcpy(netset.m_WifiConfig.LoginWifiDev.Passwd,wifilogin->Passwd);	
	g_cParaManage->SetSystemParameter(SYSNET_SET,&netset);
	
	return 0;
}

bool csSetWIFI(SYSTEM_PARAMETER *plocalPara,SYSTEM_PARAMETER *pcsPara)
{



	StartConnectWifi(&(plocalPara->m_NetWork.m_WifiConfig.LoginWifiDev));
	if(pcsPara->m_NetWork.m_WifiConfig.WifiAddrMode.m_dhcp == 0)
	{
		GetNetModule()->SetNetAttrib(&pcsPara->m_NetWork,2);
	}
	else if(pcsPara->m_NetWork.m_WifiConfig.WifiAddrMode.m_dhcp == 1 )
	{	
		printf("%s: %s\n",__FUNCTION__,(char*)WIFI_INTERFACE);
		if(GetNetModule()->SetDHCP((char*)WIFI_INTERFACE) < 0)
			
			printf("######### faild to set dhcp  (func:%s line:%d)	########\r\n",__func__, __LINE__);
		else
			printf("######### set dhcp success(func:%s line:%d)  ########\r\n",__func__, __LINE__);
	}
	memcpy(&plocalPara->m_NetWork.m_WifiConfig.WifiAddrMode,&pcsPara->m_NetWork.m_WifiConfig.WifiAddrMode,sizeof(TYPE_WIFI_ADDR) );
	return true;
}

int Wifi_wpa_ctrl_open()
{
	struct sockaddr_un local;
    struct sockaddr_un dest;
	int socketfd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if(socketfd<=0)
	{
		perror("creat socket");
		return -1;
	}
	unlink(local.sun_path);
	int tries=0;
	local.sun_family = AF_UNIX;
	strcpy(local.sun_path,src_sun_path );
try_again:	
	if (bind(socketfd, (struct sockaddr *) &local,sizeof(local)) < 0)
	{
		if (errno == EADDRINUSE && tries < 2)
		{
			/*
			 * getpid() returns unique identifier for this instance
			 * of wpa_ctrl, so the existing socket file must have
			 * been left by unclean termination of an earlier run.
			 * Remove the file and try again.
			 */
			unlink(local.sun_path);
			goto try_again;
		}
		perror("bind socket");
		close(socketfd);
		return -1;
	}
	dest.sun_family = AF_UNIX;
	strcpy(dest.sun_path, dest_sun_path);
    if (connect(socketfd, (struct sockaddr *) &dest, sizeof(dest)) < 0)
    {
        close(socketfd);
        unlink(local.sun_path);
		perror("connect socket");
        return -1;
    }
	return socketfd;

}
int Wifi_wpa_ctrl_request(int socket , const char *cmd, size_t cmd_len, char *reply, int *reply_len)
{
	struct timeval tv;
	int res;
	fd_set rfds;

   //	printf("command : %s %s %d ::ctrl == %d\n",cmd, __FUNCTION__,__LINE__,socket);

	if (send(socket, cmd, cmd_len, 0) < 0)
	{
		printf("send the cmd : %s ERROR\n", cmd);
		return -1;
	}
	memset(reply, 0, *reply_len);
	for (;;)
	{
		tv.tv_sec = 8;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(socket, &rfds);
		res = select(socket + 1, &rfds, NULL, NULL, &tv);
		if (FD_ISSET(socket, &rfds))
		{
			res = recv(socket, reply, *reply_len, 0);
			if (res < 0)
			{
				
				return res;
			}
			*reply_len = res;
			break;
		}
		else
		{
			return -2;
		}
	}
	return 0;
}
int Smartlinkcallback(char *ssid ,char *password, int lang)
{	// 1 chinese  2  english  3 spanish
	printf("####### ---ssid:%s,passwd:%s,lang:%d\n",ssid,password,lang);	
	int EnCryPt = 4;
	NETWORK_PARA netset;
	memset(&netset, 0x0, sizeof(NETWORK_PARA));
	g_cParaManage->GetSysParameter(SYSNET_SET,&netset);	
	if(lang == 1)
	{
		netset.m_WifiConfig.LoginWifiDev.lang = 1;
		if(paudio) paudio->StartPlayFile(ZBAR_WIFI_START_CH,NULL);//开始连接wifi
	}
	else if(lang == 2)
	{
		netset.m_WifiConfig.LoginWifiDev.lang = 0;		
		if(paudio) paudio->StartPlayFile(ZBAR_WIFI_START_EN,NULL);//开始连接wifi
	}
	else if(lang == 3)
	{
		netset.m_WifiConfig.LoginWifiDev.lang = 2;		
		if(paudio) paudio->StartPlayFile(ZBAR_WIFI_START_SP,NULL);//开始连接wifi
	}
	else if(lang == -1)
	{
		netset.m_WifiConfig.LoginWifiDev.lang = 0;		
		if(paudio) paudio->StartPlayFile(ZBAR_SCAN,NULL);//开始连接wifi
	}
	platform_wifi_smartlink_cleanup();	
	WIFI_ConnectStatus = CONNECTING;
	SmartlinkPlayVoice = true;
/*************************************/
	char *sendBuf = (char *)malloc(MAXAPBUFFER);
	unsigned int apCount =0;
	int iCount =0;
	while(iCount++<4)
	{
		GetAP_Lists(sendBuf,&apCount);
		if(apCount>1)
			break;
	}
	for(iCount=0;iCount<search_ap_count;iCount++)
	{
		printf("@@@@@@@@apCount:%d ssid:%s,Encrypt:%d\n",apCount,Search_ap_info[iCount].ssid,Search_ap_info[iCount].Encrypt);
		if(strcmp(Search_ap_info[iCount].ssid,ssid)==0)
		{
			EnCryPt=Search_ap_info[iCount].Encrypt;
			break;
		}		
	}
	free(sendBuf);
/*************************************/	
	printf("ZMD_Connect_Wifi====================FUNC:%s,LINE:%d\n",__FUNCTION__,__LINE__);
	ZMD_Connect_Wifi(EnCryPt,ssid,password);
	netset.m_WifiConfig.LoginWifiDev.Smartlink = 0;//open:1   close:0
	strcpy(netset.m_WifiConfig.LoginWifiDev.RouteDeviceName,ssid);
	strcpy(netset.m_WifiConfig.LoginWifiDev.Passwd,password);
	netset.m_WifiConfig.LoginWifiDev.EncryptionProtocol = EnCryPt;

	g_cParaManage->SetSystemParameter(SYSNET_SET,&netset);
	printf("---------------------------------------stop smartlink:%d\n",netset.m_WifiConfig.LoginWifiDev.EncryptionProtocol);
	
	Smartlinkrun = false;
	
	return 0;	
}

int ZMD_SmartLinkProcess(NETWORK_PARA *pNet)
{
  	if(WIFI_ConnectStatus == CONNECTED)
	{
		if(Smartlinkrun)
		{
			platform_wifi_smartlink_cleanup();
			WIFI_connectTime = SD_FIRST_LINK;
			Smartlinkrun = false;
		}
		return -1;
	}
	
	if(WIFI_ConnectStatus == WIFI_PASSWD_ERROR&&Smartlinkrun==false)
	{
		platform_wifi_smartlink_startup(Smartlinkcallback, (char *)iw_path);
		
		if(pNet->m_WifiConfig.LoginWifiDev.lang==1)
		{
			if(paudio) paudio->StartPlayFile(ZBAR_WIFI_WAIT_CH,NULL);
		}
		else if(pNet->m_WifiConfig.LoginWifiDev.lang==0)
		{
			if(paudio) paudio->StartPlayFile(ZBAR_WIFI_WAIT_EN,NULL);
		}
		else if(pNet->m_WifiConfig.LoginWifiDev.lang==2)
		{
			if(paudio) paudio->StartPlayFile(ZBAR_WIFI_WAIT_SP,NULL);
		}

		Smartlinkrun = true;		
		WIFI_ConnectStatus = WIFI_MONITOR;
		printf("######################### WIFI_PASSWD_ERROR start smartlink!\n");
		return 0;
	}
	if(pNet->m_WifiConfig.LoginWifiDev.Smartlink==0)
		return -1;
	if(Smartlinkrun==false)
	{
		printf("######################### Reset start smartlink!\n");

		if(pNet->m_WifiConfig.LoginWifiDev.lang==1)
		{
			if(paudio) paudio->StartPlayFile(ZBAR_WIFI_WAIT_CH,NULL);
		}
		else if(pNet->m_WifiConfig.LoginWifiDev.lang==0)
		{
			if(paudio) paudio->StartPlayFile(ZBAR_WIFI_WAIT_EN,NULL);
		}	
		else if(pNet->m_WifiConfig.LoginWifiDev.lang==2)
		{
			if(paudio) paudio->StartPlayFile(ZBAR_WIFI_WAIT_SP,NULL);
		}	

		platform_wifi_smartlink_startup(Smartlinkcallback, (char *)iw_path);
		Smartlinkrun = true;		
		WIFI_ConnectStatus = WIFI_MONITOR;
	}
	return 0;
}
int	GetConnectStatus()
{
	printf("-----------------------------------GetConnectStatus:%d\n",WIFI_ConnectStatus);	
	if(WIFI_ConnectStatus == CONNECTED)
	{
		return 1;
	}
	else if(WIFI_ConnectStatus == CONNECTING)
	{
		return 0;
	}
	else
	{
		return -1;
	}

}
int GetWifiStatus()
{
	return WIFI_ConnectStatus;
}

int ConnectStatusProcess(NETWORK_PARA *pNet)
{
	static int timebak = time(NULL);
	static int	scanncount = 0;
	if(abs(time(NULL)-timebak)<3)
	{
		return 0;
	}
	timebak = time(NULL);
	int ret = GetWIFI_Status(&(pNet->m_WifiConfig.LoginWifiDev));
	//printf("===========================WIFI_ConnectStatus:%d,(%02x)\n",WIFI_ConnectStatus,ret);

	/*连接成功*/
	if((ret&(1<<COMPLETED))&&(WIFI_ConnectStatus != CONNECTED))
	{
		WIFI_ConnectStatus = CONNECTED;
		g_cParaManage->GetSysParameter(SYSNET_SET, pNet);
		if(SmartlinkPlayVoice)
		{	
			if(pNet->m_WifiConfig.LoginWifiDev.lang == 1)
			{
				if(paudio) paudio->StartPlayFile(ZBAR_WIFI_SUCCESS_CH,NULL);//连接wifi成功
			}
			else  if(pNet->m_WifiConfig.LoginWifiDev.lang == 0)
			{
				if(paudio) paudio->StartPlayFile(ZBAR_WIFI_SUCCESS_EN,NULL);//连接wifi成功
			}
			else  if(pNet->m_WifiConfig.LoginWifiDev.lang == 2)
			{
				if(paudio) paudio->StartPlayFile(ZBAR_WIFI_SUCCESS_SP,NULL);//连接wifi成功
			}
			SmartlinkPlayVoice = false;
		}
		if(pNet->m_WifiConfig.WifiAddrMode.m_dhcp)	//dhcp 启用			
		{
			//system("killall -9	udhcpc ");
			if(GetNetModule()->SetDHCP((char*)WIFI_INTERFACE) < 0)
				printf("######### faild to set dhcp  ########\r\n");
			else
				printf("######### set dhcp success ########\r\n");
			
	    }
		else
	    {
			//system("killall -9	udhcpc ");
			GetNetModule()->SetNetAttrib(pNet,2);
		}		
		
		/************************/
		printf("....................wait for 3s \n");
		sleep(3);//等待3s,p2p注册再发送广播包
		/************************/
		int i=0;
		for( i =0 ; i< 10; i++)
		{
			GetNetModule()->BroadcastDeviceInfo();
			usleep(1000*500);
		}
	}
	/*未连接成功且返回值为未连接*/
	if( 0==((1<<COMPLETED)&ret) && WIFI_ConnectStatus != CONNECTED)
	{
		g_cParaManage->GetSysParameter(SYSNET_SET, pNet);
		//printf("ZMD_Connect_Wifi====================FUNC:%s,LINE:%d\n",__FUNCTION__,__LINE__);
		ZMD_Connect_Wifi(pNet->m_WifiConfig.LoginWifiDev.EncryptionProtocol,
			pNet->m_WifiConfig.LoginWifiDev.RouteDeviceName,
			pNet->m_WifiConfig.LoginWifiDev.Passwd);

	}
	/*连接成功但返回值为未连接则表示掉线*/
	if((ret&(1<<COMPLETED))==0&&(WIFI_ConnectStatus == CONNECTED))
	{
		WIFI_ConnectStatus = UNCONNECTED;
	}
	/*密码错误*/
	if( (ret&(1<<PASSWORDERR)) && WIFI_ConnectStatus != WIFI_PASSWD_ERROR)
	{
		WIFI_ConnectStatus = WIFI_PASSWD_ERROR;
	}
	if(ret == (1<<SACNNING))
	{
		if((scanncount++==5)&&(WIFI_ConnectStatus != WIFI_PASSWD_ERROR))
		{
			WIFI_ConnectStatus = WIFI_PASSWD_ERROR;
			printf("\n\n\n\n\n--------------scanning time out passwd error\n");
			
		}
		printf("-----------------------scanning %d\n",scanncount);
		
	}
	else
	{
		scanncount =0;
	}

	
	return 0;
}



int ZMD_Connect_Wifi(int EnCryPt,char *ssid,char *passwd)
{
	static char connectap[32]={0};
	static char connectpwd[32]={0};
	static char connectEP =0;
	static int	net_id_back=0;
	char reply[64] = {0};
	char cmd[256] = {0};
	int ret =-1;
	int reply_len;
	int net_id ;
	bool	ReConnect = false;
	reply_len = sizeof(reply) - 1;	
	
	if(strlen(ssid)==0)
		return -1;
	if(EnCryPt==0xff)
		return -1;

	if(strcmp(connectap,ssid)!=0)
	{
		
		ReConnect = true;
	}
	if(strcmp(connectpwd,passwd)!=0)
	{
		
		ReConnect = true;
	}
	if(connectEP != EnCryPt)
	{
		connectEP  = EnCryPt;
		ReConnect = true;
	}
	if(ReConnect ==false)
		return -1;
	
	printf("=============================ZMD_Connect_Wifi EnCryPt:%d, ssid:%s,pwd:%s\n",EnCryPt,ssid,passwd);
	ret = Wifi_wpa_ctrl_request(ctrl_socket, "ADD_NETWORK", strlen("ADD_NETWORK"), reply, &reply_len);
	if(ret == -2)
	{
		printf("ADD_NETWORK time out !!!\n");
		return -1;
	}
	else if(ret == -1)
	{
		printf("ADD_NETWORK error !!!\n");
		return -1;
	}
	if (reply[0] == 'F') {
		printf("Fail Add Network to wpa_suplicant\n");
		return -1;
	}
	net_id = atoi(reply);
	printf("--------------------------net_id:%d\n",net_id);

	//set network ssid
	memset(reply,0x0,sizeof(reply));
	memset(cmd,0x0,sizeof(cmd));
	reply_len = sizeof(reply) - 1;
	sprintf(cmd,"SET_NETWORK %d %s \"%s\"",net_id, "ssid",ssid);
	ret = Wifi_wpa_ctrl_request(ctrl_socket, cmd, strlen(cmd), reply, &reply_len);
	printf("set network ssid:%s\n",cmd);
	if(ret == -2)
	{
		printf("set network time out !!!\n");
		return -1;
	}
	else if(ret == -1)
	{
		printf("set network error !!!\n");
		return -1;
	}
	reply[reply_len]=0;
	printf("set network ssid return len:%d,%s\n",reply_len,reply);	

	/*set network scan_ssid 1 ,解决有些ssid是隐藏起来的，禁止广播的*/
	memset(reply,0x0,sizeof(reply));
	memset(cmd,0x0,sizeof(cmd));
	reply_len = sizeof(reply) - 1;
	sprintf(cmd,"SET_NETWORK %d %s %d",net_id, "scan_ssid",1);
	ret = Wifi_wpa_ctrl_request(ctrl_socket, cmd, strlen(cmd), reply, &reply_len);
	printf("set network ssid:%s\n",cmd);
	if(ret == -2)
	{
		printf("set network time out !!!\n");
		return -1;
	}
	else if(ret == -1)
	{
		printf("set network error !!!\n");
		return -1;
	}
	reply[reply_len]=0;
	printf("set network ssid return len:%d,%s\n",reply_len,reply);	


	if(EnCryPt == 0)/*无密码模式*/
	{
		memset(reply,0x0,sizeof(reply));
		memset(cmd,0x0,sizeof(cmd));
		reply_len = sizeof(reply) - 1;
		
		sprintf(cmd,"SET_NETWORK %d %s",net_id, "key_mgmt NONE");
		ret = Wifi_wpa_ctrl_request(ctrl_socket, cmd, strlen(cmd), reply, &reply_len);
		printf("set network key_mgmt NONE:%s\n",cmd);
		if(ret == -2)
		{
			printf("set network NONE time out !!!\n");
			return -1;
		}
		else if(ret == -1)
		{
			printf("set network  NONE error !!!\n");
			return -1;
		}
		reply[reply_len]=0;
		printf("set key_mgmt NONE return len:%d,%s\n",reply_len,reply);
	 }
	 else if((EnCryPt == 2)||(EnCryPt == 4)||(EnCryPt == 6))/*WPAPSK WPA2PSK WPAPSK_WPA2PSK模式*/
	 {

		memset(reply,0x0,sizeof(reply));		
		memset(cmd,0x0,sizeof(cmd));		
		reply_len = sizeof(reply) - 1;		

		sprintf(cmd,"SET_NETWORK %d %s \"%s\"",net_id, "psk",passwd);		
		ret = Wifi_wpa_ctrl_request(ctrl_socket, cmd, strlen(cmd), reply, &reply_len);		
		printf("set network psk:%s\n",cmd);		
		if(ret == -2)		
		{			
			printf("set network psk time out !!!\n");			
			return -1;		
		}		
		else if(ret == -1)		
		{			
			printf("set network  psk error !!!\n");			
			return -1;		
		}		
		reply[reply_len]=0;		
		printf("set network psk return len:%d,%s\n",reply_len,reply);
		
	 }
	 else if((EnCryPt == 1)||(EnCryPt == 3))/*WEP和OPEN模式*/
	 {
		/*1 set_network 1 key_mgmt NONE*/
		memset(reply,0x0,sizeof(reply));
		memset(cmd,0x0,sizeof(cmd));
		reply_len = sizeof(reply) - 1;
		
		sprintf(cmd,"SET_NETWORK %d %s",net_id, "key_mgmt NONE");
		ret = Wifi_wpa_ctrl_request(ctrl_socket, cmd, strlen(cmd), reply, &reply_len);
		printf("set network key_mgmt NONE:%s\n",cmd);
		if(ret == -2)
		{
			printf("set network NONE time out !!!\n");
			return -1;
		}
		else if(ret == -1)
		{
			printf("set network  NONE error !!!\n");
			return -1;
		}
		reply[reply_len]=0;
		printf("set key_mgmt NONE return len:%d,%s\n",reply_len,reply);

		/*2 set_network 1 wep_key0 “your ap passwork”*/
		memset(reply,0x0,sizeof(reply));
		memset(cmd,0x0,sizeof(cmd));
		reply_len = sizeof(reply) - 1;
		
		sprintf(cmd,"SET_NETWORK %d %s %s",net_id, "wep_key0",passwd);
		ret = Wifi_wpa_ctrl_request(ctrl_socket, cmd, strlen(cmd), reply, &reply_len);
		printf("set network wep_key0:%s\n",cmd);
		if(ret == -2)
		{
			printf("set network wep_key0 time out !!!\n");
			return -1;
		}
		else if(ret == -1)
		{
			printf("set network  wep_key0 error !!!\n");
			return -1;
		}
		reply[reply_len]=0;
		printf("set wep_key0 return len:%d,%s\n",reply_len,reply);

		/*3  set_network 1 wep_tx_keyidx 0*/
		memset(reply,0x0,sizeof(reply));
		memset(cmd,0x0,sizeof(cmd));
		reply_len = sizeof(reply) - 1;
		
		sprintf(cmd,"SET_NETWORK %d %s",net_id, "wep_tx_keyidx 0");
		ret = Wifi_wpa_ctrl_request(ctrl_socket, cmd, strlen(cmd), reply, &reply_len);
		printf("set network wep_tx_keyidx:%s\n",cmd);
		if(ret == -2)
		{
			printf("set network wep_tx_keyidx time out !!!\n");
			return -1;
		}
		else if(ret == -1)
		{
			printf("set network  wep_tx_keyidx error !!!\n");
			return -1;
		}
		reply[reply_len]=0;
		printf("set wep_tx_keyidx return len:%d,%s\n",reply_len,reply);
		
	 }
		  

	
	memset(reply,0x0,sizeof(reply));
	memset(cmd,0x0,sizeof(cmd));
	reply_len = sizeof(reply) - 1;
	sprintf(cmd, "SELECT_NETWORK %d", net_id);	
	Wifi_wpa_ctrl_request(ctrl_socket, cmd, strlen(cmd), reply, &reply_len);
	reply[reply_len]=0;
	printf("set SELECT_NETWORK return len:%d,%s\n",reply_len,reply);

	memset(reply,0x0,sizeof(reply));
	memset(cmd,0x0,sizeof(cmd));
	reply_len = sizeof(reply) - 1;
	sprintf(cmd, "ENABLE_NETWORK %d", net_id);
	ret = Wifi_wpa_ctrl_request(ctrl_socket, cmd, strlen(cmd), reply, &reply_len);
	printf("set ENABLE_NETWORK:%s\n",cmd);
	if(ret == -2)
	{
		printf("set ENABLE_NETWORK time out !!!\n");
		return -1;
	}
	else if(ret == -1)
	{
		printf("set ENABLE_NETWORK error !!!\n");
		return -1;
	}
	reply[reply_len]=0;
	net_id_back =net_id;
	strcpy(connectpwd,passwd);
	strcpy(connectap,ssid);
	printf("set ENABLE_NETWORK return len:%d,%s\n",reply_len,reply);	

	return 0;
}
void *WifiProceess_Thread(void *para)
{
	
	NETWORK_PARA netset;
	g_cParaManage->GetSysParameter(SYSNET_SET, &netset);
	printf("ZMD_Connect_Wifi====================FUNC:%s,LINE:%d\n",__FUNCTION__,__LINE__);
	int ret = ZMD_Connect_Wifi(netset.m_WifiConfig.LoginWifiDev.EncryptionProtocol,
		netset.m_WifiConfig.LoginWifiDev.RouteDeviceName,
		netset.m_WifiConfig.LoginWifiDev.Passwd);
	if(ret == 0)
		WIFI_ConnectStatus = CONNECTING;
	WIFI_connectTime = START_CONNECT;
	while(1)
	{
		g_cParaManage->GetSysParameter(SYSNET_SET, &netset);
		ConnectStatusProcess(&netset);
		ZMD_SmartLinkProcess(&netset);				
		sleep(1);
	}
	return NULL;
}





