#ifndef __WIFICONFIG__H
#define __WIFICONFIG__H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "util.h"
#include "systemparameterdefine.h"

#define WIFI_INTERNAL_VERSION "v2.0"
#define WIFI_MAC_CFG_FILE "/config/wifi_mac" 

//#define WIFI_DEBUG //调试开关


#if defined(HI3518A_PT) || defined(HI3518C_MINI_IPC) ||defined(HI3518C_MININVR_IPC)  
//#define ATHEROS  //wifi 方案厂商
#define WPA_SUPPLICANT  //wifi控制类型
#endif


#define NO_PASSWORD      0x00
#define OPEN_WEP         0x01
#define SHARED_WEP       0x03

#define WPAPSK_          0x02
#define WPA2PSK_         0x04
#define WPAPSK_WPA2PSK   0x06
#define MAXAPBUFFER		1024*8





#define ETH_INTERFACE "eth0"
#if defined(AR9271)
#define WIFI_INTERFACE "wlan0"
#elif defined(RT3070) || defined(MT7601)
#define WIFI_INTERFACE "ra0"
#endif




#ifdef WPA_SUPPLICANT
#define WIFI_IC "AR9271,MT7601"

#define WIFI_CONFIG "/config/wpa_supplicant.conf"

#define WPA_SUPPALICANT_RUN_PATH  "/app/wifi/tools/wpa_supplicant"

struct wpa_ctrl
{
    int s;
    struct sockaddr_un local;
    struct sockaddr_un dest;
};

struct wpa_ctrl *wpa_ctrl_open( void);
void wpa_ctrl_close(struct wpa_ctrl *ctrl);
int GetWifiAPList( char *buf , int bufLen,char  *listbuf,int listlen );
void GetAP_Lists(char *sendBuf,unsigned int  *apCount);
int get_status(TYPE_WIFI_LOGIN *pwifidev);
int CreateTimer(int seconds,int id,timer_t *tid,void TimerHandler(int v) ) ;  

void CreatWIFI_ScanTimer();
void DeleteWIFI_ScanTimer();
int StartWPS_Demon();
int StartUSB_Demon();
int WIFI_ConnectedDemon();
int WIFI_LedDemon();


#else
#define WIFI_IC "rt3070"

#define SCAN_RESUILT "/tmp/scan_result.txt"
#define SSID  "ESSID"
#define SIGNALLEVEL  "Quality"
#define WPAPSK  "WPA Version 1"
#define WPA2PSK  "IEEE 802.11i/WPA2 Version 1"
#define AES "CCMP"
#define TKIP "TKIP"
#define MIX_ENCRYPT "CCMP TKIP"

#define ENCRY_ON "Encryption key=on"
#define ENCRY_OFF "Encryption key=off"

#define CONNSTATUS "/proc/rt5350/wifi_status"
#define STRING_CONNECTED "Connected"
#define DISCONNECTED  "Disconnected"

#define GC_TKIP "Group Cipher=TKIP"
#define PC_TKIP "Pairwise Ciphers (1) :TKIP"

#define GC_CCMP "Group Cipher=CCMP"
#define PC_CCMP "Pairwise Ciphers (1) :CCMP"
#define PC_CCMPTKIP "Pairwise Ciphers (2) :CCMPTKIP"

#define WIFI_DISCONNECTED  0x00 
#define WIFI_CONNECTED     0x01 
#define WIFI_CONNECTING    0x02
//#define WIFI_PASSWD_ERROR  0x03


//extern unsigned char  WIFI_ConnectStatus;

void DoScan();
void GetWIFI_ConnectStatus();
void ParseFromScanResults(char *send_buff,unsigned int *k);
void SortByHighSignal( char *send_buff,unsigned int total);
int GetWIFI_Status(TYPE_WIFI_LOGIN *pwifidev);



int NoPasswd(TYPE_WIFI_LOGIN wifidev);
int OPEN_WEP_Process(TYPE_WIFI_LOGIN wifidev);
int SHARED_WEP_Process(TYPE_WIFI_LOGIN wifidev);
int WPAPSK_Process(TYPE_WIFI_LOGIN wifidev);
int WPA2PSK_Process(TYPE_WIFI_LOGIN wifidev);

int DHCP_Ra0();
int StartWifiLedControl();

extern "C" 
{
	int iwpriv(int	argc,char **argv);	
	int iwlist(int	argc,char **argv);
}

#endif //end ifdef WPA_SUPPLICANT


//通用
#define CONNECT_COUNT       5
extern WIFI_Connect_Status WIFI_ConnectStatus;

int  CheckValidAP(const TYPE_WIFI_LOGIN *pAP_Info);
void UpWifi();
void DownWifi();
int GetWIFI_Status(TYPE_WIFI_LOGIN *pwifidev);
int WIFI_TryConnect(TYPE_WIFI_LOGIN wifidev,int first);
int  GetWifiMac(unsigned char *mac);
int ZMD_SmartLink();
int WIFI_ConnectedDemon();


#endif

