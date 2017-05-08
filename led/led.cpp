#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<errno.h>
#include<sys/msg.h>
#include<string.h>
#include <pthread.h>

#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "led.h"
#include "common.h"
#include "3518c_mini_ipc_gpio.h"  


static int		led_fd=-1;
bool WifiProflag=false;
pthread_t WifiLedpid;
void OpenLedGpio()
{
	led_fd = open("/dev/gpio_dev",O_RDONLY);
	if(led_fd<0)
	{
		printf("open gpio erro!\n");
	}
	printf("\n\n led_fd=%d\n",led_fd);
}
void CloseLedGpio()
{
	if(led_fd > 0)
	{
		close(led_fd);
		led_fd=-1;
	}
}

bool WIFI_LED_ON(int Color)
{
	struct LED_STATUS led_status;
    	int ret;
	
	led_status.led = (enum LED)Color;
	led_status.on = (bool)0;
	//printf("----------------------->[ON:%d]\n",led_status.on);
	ret=ioctl(led_fd, GPIO_LED_CTRL, &led_status);
	if(ret < 0)
	{
		//printf("led on ioctl erro\n");
		return false;
	}
	return true;

}

bool WIFI_LED_OFF(int Color)
{
	
	struct LED_STATUS led_status;
    int ret;
	
	led_status.led = (enum LED)Color;
	led_status.on = (bool)1;
	//printf("----------------------->[OFF:%d]\n",led_status.on);
	ret=ioctl(led_fd, GPIO_LED_CTRL, &led_status);
	if(ret < 0)
	{
		//printf("led off ioctl erro\n");
		return false;
	}
	return true;
	
}


void InitStartLed()
{
	WIFI_LED_OFF(LED_R);
	WIFI_LED_OFF(LED_B);
	WIFI_LED_ON(LED_G);	
}

void ConnectLed()
{
	WIFI_LED_OFF(LED_R);		
	WIFI_LED_OFF(LED_G);
	WIFI_LED_ON(LED_B);
}

void *WifiLedProcess_Thread(void *para)
{
	printf("########WifiLedProcess_Thread#####\n");
	OpenLedGpio();
	WifiProflag = true;


	LED_Get_Wifi_Status *LED_for_WifiStatus;
    if (NULL == (LED_for_WifiStatus=(LED_Get_Wifi_Status *)malloc(sizeof(LED_Get_Wifi_Status))))
    	{
			printf("__LINE:%d__Malloc  LED_for_WifiStatus error\n",__LINE__);
			return NULL;	
    	}
	LED_for_WifiStatus = (LED_Get_Wifi_Status*) para;
	
	while(1)
	{
		
		while(WifiProflag && (!((unsigned int)(*LED_for_WifiStatus).Conn_SAP)))
		{
				if((int)*(*LED_for_WifiStatus).wifi_state!=WPA_COMPLETED)
					printf("#######wifi status=%d#######\n",(int)*(*LED_for_WifiStatus).wifi_state);
			
				switch((int)*(*LED_for_WifiStatus).wifi_state)
				{	
					case WPA_AUTHENTICATING://UNCONNECTED://蓝闪烁 jam 
					case WPA_GROUP_HANDSHAKE://CONNECTING://蓝色闪烁 
					case UNCONNECTED:
					case CONNECTING:
					case WPA_ASSOCIATING:
					case WPA_ASSOCIATED:
					case WPA_4WAY_HANDSHAKE:

						 WIFI_LED_OFF(LED_G);
						 WIFI_LED_ON(LED_B);
						 usleep(400 * 1000);
						 WIFI_LED_OFF(LED_B);			 
						 break; 				
					 case WPA_COMPLETED://CONNECTED://蓝色常亮jam 
					 case CONNECTED:

						//WIFI_LED_OFF(LED_R);	
						WIFI_LED_OFF(LED_G);
						WIFI_LED_ON(LED_B);
						usleep(400 * 1000);
						break;			
					case WPA_INACTIVE://WIFI_PASSWD_ERROR://绿色闪烁jam 
					case WIFI_ERROR:
					case WPA_DISCONNECTED:
					case WPA_IDLE:
					case WPA_SCANNING: 
					case WIFI_MONITOR://9271
					case MONITOR_WIFI://8188
					case WIFI_PASSWD_ERROR://9271

						WIFI_LED_OFF(LED_B);
						WIFI_LED_ON(LED_G); 		
						usleep(400 * 1000);
						WIFI_LED_OFF(LED_G);				
						break;			
					case SYS_RESET://系统复位后熄灭
						WIFI_LED_OFF(LED_B);
						WIFI_LED_OFF(LED_R);
						sleep(3);	
						break;	
						
					default:
						sleep(1);
						printf("unkown WIFI_ConnectStatus=%d\n",(int)*(*LED_for_WifiStatus).wifi_state);
						break;		
				}
				usleep(500*1000);
			}
		while(WifiProflag && ((unsigned int)(*LED_for_WifiStatus).Conn_SAP))
			{
				printf("#######wifi Conn_SAP=%d#######\n",(unsigned int)(*LED_for_WifiStatus).Conn_SAP);
				WIFI_LED_OFF(LED_G);
				WIFI_LED_ON(LED_B);
				usleep(400 * 1000);
				WIFI_LED_OFF(LED_B);
				usleep(500*1000);

			}
	}

	free(LED_for_WifiStatus);
	return NULL; 
}
void CreateWifiLedProcess(void *status)
{
	
	if(pthread_create(&WifiLedpid, NULL, WifiLedProcess_Thread, status) < 0)//创建线程
	{
		printf("CreateWifiLedProcess failed \n");
	}

}
void CloseWifiLedProcess()
{
	WifiProflag=false;
	if(WifiLedpid > 0)
	{
		pthread_join(WifiLedpid,NULL);
		
	}
	CloseLedGpio();
}

//复位后熄灭WIFI LED
void SetSysRestLED()
{
    //WIFI_ConnectStatus = SYS_RESET;
	WIFI_LED_OFF(LED_B);
	WIFI_LED_OFF(LED_R);
	sleep(3);		
}


