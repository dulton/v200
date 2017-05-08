#include <sys/types.h>    
#include <sys/socket.h>    
#include <netinet/in.h>    
#include <net/if.h>    
#include <sys/ioctl.h>    
#include <errno.h>    
#include <string.h>    
#include <net/route.h>    
#include <unistd.h>    
#include <stdio.h>    
#include <arpa/inet.h>

#include "gateway.h"


const char *path_arp_ignore = "/proc/sys/net/ipv4/conf/all/arp_ignore";
const char *path_arp_arp_announce = "/proc/sys/net/ipv4/conf/all/arp_announce";

int interface_arp_setting(const char *path, int data)
{
    FILE *arpFile;
	arpFile = fopen (path, "r+");
	if (!arpFile) 
	{
		printf ("can't create arp config file 1 \n" );
		return -1;
	}
	fprintf (arpFile, "%d\n",data);
	fclose(arpFile);
	return 0 ;
}

int set_gateway(unsigned long gw, char * devname )    
{    
  int skfd;    
  struct rtentry rt;    
  int err;    
  
  skfd = socket(PF_INET, SOCK_DGRAM, 0);    
  if (skfd < 0)    
    return -1;    
  
  
  /* Set default gateway */    
  memset(&rt, 0, sizeof(rt));    
  
  rt.rt_dst.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;    
  
  rt.rt_gateway.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = gw;    
  
  rt.rt_genmask.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;    
  
  rt.rt_flags = RTF_UP | RTF_GATEWAY;    
  rt.rt_dev = devname;
  
  err = ioctl(skfd, SIOCADDRT, &rt);    

  
  close(skfd);    
  
  return err;    
}    

int del_net(unsigned long net, unsigned long  ifnet_mask)    
{    
  int skfd;    
  struct rtentry rt;    
  int err;    
  
  skfd = socket(PF_INET, SOCK_DGRAM, 0);    
  if (skfd < 0)    
    return -1;    
    
  /* del default gateway */    
  memset(&rt, 0, sizeof(rt));    
  
  rt.rt_dst.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = net;    
  
  rt.rt_gateway.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = 0;    
  
  rt.rt_genmask.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = ifnet_mask;    
  
  rt.rt_flags = RTF_UP ;    
  
  err = ioctl(skfd, SIOCDELRT, &rt);    
  
  close(skfd);    
  
  return err;    
}    


int add_net(unsigned long net, unsigned long  ifnet_mask, char * devname )    
{    
  int skfd;    
  struct rtentry rt;    
  int err;    
  
  skfd = socket(AF_INET, SOCK_DGRAM, 0);    
  if (skfd < 0)    
    return -1;    
    	
  /* del default gateway */    
  memset(&rt, 0, sizeof(rt));    
  
  rt.rt_dst.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = net;    
  
  rt.rt_gateway.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = 0;    
  
  rt.rt_genmask.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = ifnet_mask;    
  
  rt.rt_flags = RTF_UP;  
  
  rt.rt_dev = devname;
   
  err = ioctl(skfd, SIOCADDRT, &rt);    
  
  close(skfd);    
  
  return err;    
}    


int del_gateway(unsigned long gw)    
{    
  int skfd;    
  struct rtentry rt;    
  int err;    
  
  skfd = socket(PF_INET, SOCK_DGRAM, 0);    
  if (skfd < 0)    
    return -1;    
    
  /* del default gateway */    
  memset(&rt, 0, sizeof(rt));    
  
  rt.rt_dst.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;    
  
  rt.rt_gateway.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = gw;    
  
  rt.rt_genmask.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;    
  
  rt.rt_flags = RTF_UP | RTF_GATEWAY;    
  
  err = ioctl(skfd, SIOCDELRT, &rt);    
  
  close(skfd);    
  
  return err;    
}    


int GetLocalNetInfo( 
    const char* lpszEth, 
    unsigned long *ip, 
    unsigned long *netmask, 
    char* szMacAddr 
) 
{ 
    int ret = 0; 
 
    struct ifreq req; 
    struct sockaddr_in* host = NULL; 
 
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if ( -1 == sockfd ) 
    { 
        return -1; 
    } 
 
    bzero(&req, sizeof(struct ifreq)); 
    strcpy(req.ifr_name, lpszEth); 
    if ( ioctl(sockfd, SIOCGIFADDR, &req) >= 0 ) 
    { 
        host = (struct sockaddr_in*)&req.ifr_addr; 
		*ip = host->sin_addr.s_addr;
        //strcpy(szIpAddr, inet_ntoa(host->sin_addr)); 
    } 
    else 
    { 
        ret = -1; 
    } 
 
    bzero(&req, sizeof(struct ifreq)); 
    strcpy(req.ifr_name, lpszEth); 
    if ( ioctl(sockfd, SIOCGIFNETMASK, &req) >= 0 ) 
    { 
        host = (struct sockaddr_in*)&req.ifr_addr; 
		*netmask = host->sin_addr.s_addr;
        //strcpy(szNetmask, inet_ntoa(host->sin_addr)); 
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

/**
 * @brief 删除多余网关
 * @netcard  0:有线网卡  1:无线网卡
 * @sznetcard 有线网口名称
 * @wifi_name 无线网口名称
 * @pGw 添加的网关
 */

int delnet_gateway(NET_CARD netcard, char *sznetcard, char* wifi_name, char* pGw)    
{    
  FILE *fp;    
  char buf[256]; // 128 is enough for linux    
  char iface[16];   
  char firstiface[16];   
  char szIpAddr[20] = {0}; 
  char szNetmask[20] = {0}; 
  char szMacAddr[20] = {0}; 
  char szDefaultGateway[20] = {0}; 
  unsigned long dest_addr, dest_addr1, gate_addr;  
  unsigned long ifnet_mask,ifnet_ip;
  unsigned long ifnet_mask1,ifnet_ip1;
  
  unsigned long ifnet_mask1_tmp = 0,ifnet_ip1_tmp = 0,dest_addr1_tmp = 0;
  unsigned long ifnet_mask0_tmp = 0,ifnet_ip0_tmp = 0,dest_addr0_tmp = 0;
  
  static unsigned char arpchangedflag = 0x00;  //保证arp Setting 只设置一次
  
  unsigned long temp_ip = 0;
  unsigned char flag = 0x00;
  GetLocalNetInfo(sznetcard,&ifnet_ip,&ifnet_mask,szMacAddr);
  GetLocalNetInfo(wifi_name,&ifnet_ip1,&ifnet_mask1,szMacAddr);
  
  if (arpchangedflag == 0x00)
  {
       interface_arp_setting(path_arp_ignore,1);
	   interface_arp_setting(path_arp_arp_announce,2);
       arpchangedflag = 0x01;
  }

  fp = fopen("/proc/net/route", "r");    
  if (fp == NULL)    
    return -1;    
    
  /* Skip title line */    
  fgets(buf, sizeof(buf), fp);    
  while (fgets(buf, sizeof(buf), fp)) {    
    if (sscanf(buf, "%s\t%lX\t%lX", iface,       &dest_addr, &gate_addr) != 3 )     
        continue;  
	if (dest_addr == 0)
	{
	  del_gateway(gate_addr); 
	}
	else
	{
	     del_net(dest_addr,ifnet_mask); 
	}
  }    
  
  fclose(fp);  
  
  if (netcard == NET_CARD_LOCAL)
  {
     ifnet_ip0_tmp =  ntohl(ifnet_ip);
	 ifnet_mask0_tmp =  ntohl(ifnet_mask);
	 dest_addr0_tmp = ifnet_ip0_tmp & ifnet_mask0_tmp;
	 dest_addr = htonl(dest_addr0_tmp);
	 
     //如果eth0 存在的，并且ra0链接成功，应该添加ra0 所在的网络    			 
		ifnet_ip1_tmp =  ntohl(ifnet_ip1);
		ifnet_mask1_tmp =  ntohl(ifnet_mask1);
		dest_addr1_tmp = ifnet_ip1_tmp & ifnet_mask1_tmp;
	    dest_addr1 = htonl(dest_addr1_tmp);
	    
	  	if (dest_addr1 != 0)
			add_net(dest_addr1,ifnet_mask1, wifi_name);
	
          //必须添加ra0，网络在先，eth0 网络在后
     add_net(dest_addr,ifnet_mask, sznetcard); 
	 temp_ip = inet_addr(pGw);
     set_gateway((unsigned long)temp_ip, sznetcard);
  }
  else
  {
	ifnet_ip1_tmp =  ntohl(ifnet_ip1);
	ifnet_mask1_tmp =  ntohl(ifnet_mask1);
	dest_addr1_tmp = ifnet_ip1_tmp & ifnet_mask1_tmp;
	dest_addr1 = htonl(dest_addr1_tmp);

     if (dest_addr1 != 0)
     	add_net(dest_addr1,ifnet_mask1, wifi_name); 

	 temp_ip = inet_addr(pGw);
     set_gateway((unsigned long)temp_ip, wifi_name);
  }
   
  return temp_ip;    
}   
