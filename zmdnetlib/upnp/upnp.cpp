#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "miniwget.h"
#include "miniupnpc.h"
#include "upnpcommands.h"
#include "upnperrors.h"
//#include "ddnsfunction.h"
int	upnpcount = 0;
char lastexternalIPAddress[20] ="";
#define     itoa( intdata, str, dec){sprintf(str,"%d",intdata);}
//void ListRedirections(struct UPNPUrls * urls , struct IGDdatas * data)  ;


//========================================
//查询公网ip是否变更
bool IsChangeExtIP( )
{
	int ret;
	char lanaddr[16] ="";
	char externalIPAddress[20] = "";
	struct UPNPDev *devlist = NULL;
	
	memset( lanaddr , 0 , sizeof( lanaddr ) ) ;
	memset( externalIPAddress , 0 , sizeof( externalIPAddress ) ) ;
	
	if( (devlist = upnpDiscover(2000, NULL, NULL)))
	{
		int RetVal = 0;


		struct UPNPUrls urls;
		struct IGDdatas data;
		memset( &urls , 0 , sizeof( urls ) ) ;
		memset(&data , 0 , sizeof( data ) ) ;
		/*		
		struct UPNPDev * device = NULL;
		
		for( device=devlist; NULL != device; device=device->pNext)
		{
			//printf(" [UPNP]:desc: %s\n st: %s",device->descURL, device->st);		   
		}
		*/
		
		if((RetVal = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))))
		{
            if( !urls.controlURL )
            {
				if( devlist )
					freeUPNPDevlist(devlist);
				return false ;
			}
						
			switch(RetVal) 
			{
			case 1:
				{
					//printf(" [UPNP]:Found valid IGD : %s\r\n", urls.controlURL);
					break;
				}
			case 2:
				{
					//printf(" [UPNP]:Found a (not connected?) IGD : %s\r\n", urls.controlURL);
					break;
				}
			case 3:
				{
					//printf(" [UPNP]:UPnP device found. Is it an IGD ? : %s\r\n", urls.controlURL);
					break;
				}
			default:
				{
				//	printf(" [UPNP]:Found device (igd ?) : %s\r\n", urls.controlURL);
					break;
				}
			}
		}
		
		if( !urls.controlURL )
		{
			printf( "urls.controlURL is NULL ,IsChangeExtIP() goto UPNPSET_FAULT\r\n" ) ;
			if( devlist )
				freeUPNPDevlist(devlist);
			
			return false ;
		}
		//ListRedirections( &urls , &data ) ;
		//printf("UPNP_GetExternalIPAddress () start!!!!!\r\n" ) ;
		
		ret = UPNP_GetExternalIPAddress(urls.controlURL,data.servicetype, externalIPAddress);
		//sleep( 10 ) ;
		if(ret!=UPNPCOMMAND_SUCCESS)
		{
		//	printf("IsChangeExtIP () return false!!!!!\r\n" ) ;
			FreeUPNPUrls(&urls);
		
			if( devlist )
				freeUPNPDevlist(devlist);
			return false ;
		}
		
		if((externalIPAddress[0])&&(externalIPAddress[0] != '0'))
		{
			//printf( "lastexternalIPAddress = %s , externalIPAddress = %s\r\n" , lastexternalIPAddress , externalIPAddress ) ;
			if(strcmp(lastexternalIPAddress,externalIPAddress)!=0)
			{
				FreeUPNPUrls(&urls);
				
				if( devlist )
					freeUPNPDevlist(devlist);
				
				return true ;
			}
		}

		FreeUPNPUrls(&urls);
	}
	if( devlist )
		freeUPNPDevlist(devlist);
		
	return false ;
}
const char* GetInternetIp()
{
	return lastexternalIPAddress;
}

int upnp(int v_port, 
			int http_port, 
			int phone_port, 
			unsigned short m_uHttpListenPt, 
			unsigned short m_uVideoListenPt, 
			unsigned short m_uPhoneListenPt,
			unsigned char 	ddnsFlag, 
			unsigned short o_uHttpListenPt, 
			unsigned short o_uVideoListenPt, 
			unsigned short o_uPhoneListenPt)
{
	int ret;
	char lanaddr[16]={0};
	char externalIPAddress[20] = "";
	struct UPNPDev *devlist = NULL;
	
	
	
	if((v_port>65535||v_port<1024)||(http_port>65535||http_port<1024)||(phone_port>65535||phone_port<1024))
	{
		printf(" [UPNP]:v_port %d http_port %d plat_alarm_port %d \r\n",v_port, http_port,phone_port);
		goto UPNPSET_FAULT;
	}
	if( (devlist = upnpDiscover(2000, NULL, NULL)))
	{
		int RetVal = 0;
		struct UPNPUrls urls;
		struct IGDdatas data;
		struct UPNPDev * device = NULL;
		
		memset( &urls , 0 , sizeof( urls ) ) ;
		memset(&data , 0 , sizeof( data ) ) ;

		
		if((RetVal = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))))
		{
			
            if( !urls.controlURL ) 									
            {
			//	printf( "urls.controlURL is NULL , goto UPNPSET_FAULT\r\n" ) ;
				goto UPNPSET_FAULT;
			}
			
			switch(RetVal) 
			{
			case 1:
				{
					//printf(" [UPNP]:Found valid IGD : %s\r\n", urls.controlURL);
					break;
				}
			case 2:
				{
					//printf(" [UPNP]:Found a (not connected?) IGD : %s\r\n", urls.controlURL);
					break;
				}
			case 3:
				{
					//printf(" [UPNP]:UPnP device found. Is it an IGD ? : %s\r\n", urls.controlURL);
					break;
				}
			default:
				{
					//printf(" [UPNP]:Found device (igd ?) : %s\r\n", urls.controlURL);
					break;
				}
			}
		}
		
        if( !urls.controlURL ) 								
        {
			//printf( "urls.controlURL is NULL , goto UPNPSET_FAULT\r\n" ) ;
			goto UPNPSET_FAULT;
		}
	
		ret = UPNP_GetExternalIPAddress(urls.controlURL,data.servicetype, externalIPAddress);

		if(ret!=UPNPCOMMAND_SUCCESS)
		{
			//printf("Faild to UPNP_GetExternalIPAddress goto UPNPSET_FAULT\r\n");
			FreeUPNPUrls(&urls);
			goto UPNPSET_FAULT;
		}
		
		
		if((externalIPAddress[0])&&(externalIPAddress[0] != '0'))
		{
			char inport[20];
			char outport[20];
			if(upnpcount == 0)
				memcpy(lastexternalIPAddress,externalIPAddress,sizeof(externalIPAddress));
			upnpcount ++;
			if(strcmp(lastexternalIPAddress,externalIPAddress)!=0)
			{
				//if(ddnsFlag)
					//Do_ddns();
			}
			//映射WEB 端口
			/*add by hayson begin 20131025*/
			itoa(o_uHttpListenPt, inport ,10);
			UPNP_DeletePortMapping(urls.controlURL, data.servicetype, inport, "TCP");
			/*add by hayson end 20131025*/
			itoa(http_port,outport,10);
			itoa(m_uHttpListenPt,inport ,10);

			UPNP_DeletePortMapping(urls.controlURL, data.servicetype, outport, "TCP");
			ret = UPNP_AddPortMapping(urls.controlURL, data.servicetype, outport, inport, lanaddr, 0, "TCP");//设置外网web端口固定为设备好(devNo),IP地址指向本机(lanaddr)   
			if(ret!=UPNPCOMMAND_SUCCESS)
			{
				printf(" [UPNP]:AddPortMapping(%s, %s, %s) failed with code %d \r\n",outport, inport, lanaddr, ret);
				FreeUPNPUrls(&urls);
				goto UPNPSET_FAULT;
			}
			
			//映射手机监听端口
			/*add by hayson begin 20131025*/
			itoa(o_uPhoneListenPt, inport ,10);
			UPNP_DeletePortMapping(urls.controlURL, data.servicetype, inport, "TCP");
			/*add by hayson end 20131025*/
			itoa(phone_port,outport,10);
			itoa(m_uPhoneListenPt ,inport ,10);
			UPNP_DeletePortMapping(urls.controlURL, data.servicetype, outport, "TCP");
			ret = UPNP_AddPortMapping(urls.controlURL, data.servicetype, outport, inport, lanaddr, 0, "TCP");//设置外网系统服务端口固定为设备好(devNo),IP地址指向本机(lanaddr)
			if(ret!=UPNPCOMMAND_SUCCESS)
			{
				printf(" [UPNP]:AddPortMapping(%s, %s, %s) failed with code %d \r\n",outport, inport, lanaddr, ret);
				FreeUPNPUrls(&urls);
				goto UPNPSET_FAULT;
			}

			//映射视频服务端口
			/*add by hayson begin 20131025*/
			itoa(o_uVideoListenPt, inport ,10);
			UPNP_DeletePortMapping(urls.controlURL, data.servicetype, inport, "TCP");
			/*add by hayson end 20131025*/
			itoa(v_port,outport,10);
			itoa(m_uVideoListenPt,inport ,10);
			UPNP_DeletePortMapping(urls.controlURL, data.servicetype, outport, "TCP");
			ret = UPNP_AddPortMapping(urls.controlURL, data.servicetype, outport, inport, lanaddr, 0, "TCP");//设置外网系统服务端口固定为设备好(devNo),IP地址指向本机(lanaddr)
			if(ret!=UPNPCOMMAND_SUCCESS)
			{
				printf(" [UPNP]:AddPortMapping(%s, %s, %s) failed with code %d \r\n",outport, inport, lanaddr, ret);
				FreeUPNPUrls(&urls);
				goto UPNPSET_FAULT;
			}
			
			UPNP_DeletePortMapping(urls.controlURL, data.servicetype, outport, "TCP");
			ret = UPNP_AddPortMapping(urls.controlURL, data.servicetype, outport, inport, lanaddr, 0, "TCP");//设置外网系统服务端口固定为设备好(devNo),IP地址指向本机(lanaddr)
			if(ret!=UPNPCOMMAND_SUCCESS)
			{
				printf(" [UPNP]:AddPortMapping(%s, %s, %s) failed with code %d \r\n",outport, inport, lanaddr, ret);
				FreeUPNPUrls(&urls);
				goto UPNPSET_FAULT;
			}
			
			memcpy(lastexternalIPAddress,externalIPAddress,sizeof(externalIPAddress));
			printf(" [UPNP]:UPNPSET OK\r\n");

			FreeUPNPUrls(&urls);
				
			if( devlist )
				freeUPNPDevlist(devlist);
			
			return 0 ;
		}
		else
		{
			goto UPNPSET_FAULT;
		}
	}
	else
	{
		//printf(" [UPNP]:not fond upnp device\n");
		//sleep(60);
		if( devlist )
			freeUPNPDevlist(devlist);
		return 1;
	}
	
UPNPSET_FAULT:

	if( devlist )
		freeUPNPDevlist(devlist);
	
		
//	printf(" [UPNP]:UPNPSET FALUT!!\n");
	return 1;
}

#if 0
void ListRedirections(struct UPNPUrls * urls , struct IGDdatas * data)  
{  
	int r;
	int i = 0;
	struct PortMappingParserData pdata;
	struct PortMapping * pm;

	memset(&pdata, 0, sizeof(struct PortMappingParserData));
	r = UPNP_GetListOfPortMappings(urls->controlURL,
                                   data->first.servicetype,
	                               "0",
	                               "65535",
	                               "TCP",
	                               "1000",
	                               &pdata);
	if(r == UPNPCOMMAND_SUCCESS)
	{
		printf(" i protocol exPort->inAddr:inPort description remoteHost leaseTime\n");
		for(pm = pdata.head.lh_first; pm != NULL; pm = pm->entries.le_next)
		{
			printf("%2d %s %5hu->%s:%-5hu '%s' '%s' %u\n",
			       i, pm->protocol, pm->externalPort, pm->internalClient,
			       pm->internalPort,
			       pm->description, pm->remoteHost,
			       (unsigned)pm->leaseTime);
			i++;
		}
		FreePortListing(&pdata);
	}
	else
	{
		printf("GetListOfPortMappings() returned %d (%s)\n",
		       r, strupnperror(r));
	}
}  
#endif
