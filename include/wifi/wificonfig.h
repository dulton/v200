#ifndef __WIFICONFIG__H
#define __WIFICONFIG__H

//初始化wifi
int InitWifi();

int StartConnectWifi(TYPE_WIFI_LOGIN* wifilogin);
//读取热点列表
void GetAP_Lists(char *sendBuf,unsigned int  *apCount);

//IE配置wifi网络
bool csSetWIFI(SYSTEM_PARAMETER *plocalPara,SYSTEM_PARAMETER *pcsPara);

//读取wifi连接状态
int GetWIFI_Status(TYPE_WIFI_LOGIN *pwifidev);

//设置wifi MAC地址
int SetWifiMac(unsigned char *mac);

//读取wifi MAC地址
int GetWifiMac(unsigned char *mac);

//status:0--关闭wifi led;1--开启wifi led
bool SetWIFI_LED(bool status);

//返回值:0--wifi led关闭;1--wifi led开启
bool GetWIFI_LED();

//设置系统复位状态LED标志
void SetSysRestLED();

void wifi_disable(void);

#endif


