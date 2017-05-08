#ifndef DEVSMARTLINK_H
#define DEVSMARTLINK_H

//#define DEBUG  //调试开关
#define WIFI_INTERFACE             "wlan0" //wifi物理网卡
#define WIFI_MONITER_INTERFACE     "wlan.m"  //wifi虚拟网卡，用于抓取无线数据
#define CHANNEL_CHANGE_STEP        500000U //信道切换时间间隔
#define IEEE80211_DATA             0x08 //IEEE802.11 数据帧类型值,4bits(type+version)
#define IEEE80211_MAC_HEADER       32   //IEEE802.11帧头(MAC Header)固定长度为32bytes
#define IEEE80211_PROTOCOL_VERSION 0x00 
#define IP_HEADER                  20   //IP头部长度
#define DEST_PORT                  7773 //UDP数据包的目的地地址
#define MAX_UPD_LEN                1472 //UDP数据包有效最长数据
#define IEEE80211BG_RADIO_LEN      26   //在80211BG的速度下，射频信息长度为26bytes
#define IEEE80211N_RADIO_LEN       29   //在80211N的速度下，射频信息长度为29bytes
#define MAX_CHANNEL_NUMBER         13   //支持的信道数
#define SOCK_BUFFER_SIZE           256*1024 //256KB,默认只有108KB

/* AP连接信息结构体 */
typedef struct {
	char dev_id[16];
	char ap_ssid[32];
	char ap_pwd[32];
}AP_CONNECT_INFO_T;

typedef struct{
	unsigned short SourcePort;
	unsigned short DestPort;
	unsigned short len;
	unsigned short CheckSum;
}UDP_HEADER_T;

/* Exit from The Distribution System,IEEE802.11帧头*/ 
typedef struct{
	unsigned short	FrameControl;
	unsigned short	duration;
	unsigned char   DestAddr[6];
	unsigned char   BssidAddr[6];
	unsigned char   SourceAddr[6];
	unsigned short	sequence;
	unsigned short	qos;
}IEEE80211_MAC_HEADER_EXIT_T;

/* To The Distribution System,IEEE802.11帧头*/ 
typedef struct{
	unsigned short	FrameControl;
	unsigned short	duration;
	unsigned char   BssidAddr[6];
	unsigned char   SourceAddr[6];
	unsigned char   DestAddr[6];
	unsigned short	sequence;
	unsigned short	qos;
}IEEE80211_MAC_HEADER_TO_T;

#ifdef __cplusplus
extern "C"{
#endif

typedef int(*Dev_SmartLink_Callback_Func)(char *ap_ssid, char *ap_pwd, int lang);

int platform_wifi_smartlink_startup(Dev_SmartLink_Callback_Func pCbFunc, char *iw_path);

int platform_wifi_smartlink_cleanup();

#ifdef __cplusplus
}
#endif

#endif
