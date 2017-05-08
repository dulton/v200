#define	 IMPROVE_PIC
#ifdef IMPROVE_PIC
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
 
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mount.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>     /* gethostbyname */
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <net/route.h>
#include <net/if_arp.h>

#include "ModuleFuncInterface.h"

#include "EncodeManage.h"
#include "ViVo_Api.h"
#include "HiIRApi.h"
#include "mpi_isp.h"
#include "hi_comm_isp.h"

//#include "httpd.h"
#include "AudioManage.h"
#include "ping.h"

//#include "wificonfig.h"
//#include "GuiHandle.h"
#include "FeedDog.h"

#include "InputDevice.h"

#include "TW2865.h"

#include "nvp5000.h"
#include "WatchDog.h"
#include "McuCommunication.h"
#include "GpioApi.h"
#include "CATsha204.h"
//#include "thttpd.h"


//md127
#include"md127.h" //新添加
//#define PC_KEYBOARD

//#include "netserver.h"

//add by table : 网络层接口
#include "zmdnetlib.h"

 
#include "rtspLib.h"
#include "nvtLib.h"
#include "nvtApp.h"
#include "wificonfig.h" 
#include "IR_Cut.h"


int		cursorfd = 0; 
int		SystemEncodeIsReboot = 0;
extern 	int		NeedRebootEncode;
extern	int startupdate;
extern	int	EncodeRunFlg;
extern    int change_flag1; 
extern    int change_flag2; //保证设置网络地址的时候，能删除掉原来的网关
extern Audio 		*paudio;
extern int zsip_main();

int IdleProcess()
{ 

	#if !defined(SPOE_IPC)&& !defined(H42SPOE_IPC)
	unsigned char gpioflag = 1;
	int count = 0;	
	int cnt=0;
	#endif
	unsigned char flag;
	//NightMode(false);//false  白天 true夜视
	bool hasUpdate=true;
	Get_IRCutStatus(flag);
	while(1)	
	{	
		#ifdef OPENSOUND
		if(p2p_is_online()&&hasUpdate)    
		{        
			p2p_sync_value("device_volume", "50");       
			hasUpdate=false;
			break;  
		}	
		#endif
		if(flag==0){
			//夜晚不进行图像参数的重新设置
			cnt=11;
		}
		if(cnt==10){
			CAMERA_ANALOG  lchaPara;	
			g_cParaManage->GetSysParameter(SYSANALOG_SET,&lchaPara);
			SetVideoChnAnaLog(
				lchaPara.m_Channels[0].m_nBrightness,
				lchaPara.m_Channels[0].m_nContrast,
				lchaPara.m_Channels[0].m_nSaturation);
			cnt++;
		}
		else if(cnt<10){
			cnt++;
			printf("cnt=%d\n",cnt);
		}
		
		Get_IRCutStatus(flag);
		//printf("**************flag=%d\n",flag);
		if(flag==1)
		{
			NightMode(false);
		}
		else
		{
			NightMode(true);
		}	
		#if !defined(SPOE_IPC)&& !defined(H42SPOE_IPC)
		//检测复位	
	
		HiGetResetGpio(&gpioflag);
		
		if(0==gpioflag) 
		{	
			printf("---------------gpioflag:%d,count:%d\n",gpioflag,count);
			count ++;
			if(count ==5)
			{
				printf("Restore Default\n");
				RestoreDefault((SYSTEM_PARAMETER *)PubGetParameterPtr());
				RebootSystem();
			}
		}
		else
		{
			count = 0;
		}		
		
		FeedWatchDog();
		DeviceSwtich();
		#endif
		//usleep(200000);
		sleep(1);
	}
	
	
	
	return 0;
	
}




void testUpgrade(){
	int i;
	for(i=0;i<30;i++){
		sleep(1);
		printf("i=%d\n",i);
	}
	system("wget -O /tmp/UpdateFile  http://192.168.1.104/app.img");
	
	if(0== access((char *)"/tmp/UpdateFile",F_OK))
	{		
		T_MSGBUF	Msg;
		int 		QuenceMsgCmd=CMD_UPGRADE;
		Msg.mtype = 1;
		memcpy(Msg.mtext,&QuenceMsgCmd,sizeof(QuenceMsgCmd));
		Msg.mtext[4]=4;
		OSPQueueSendMessage(GetOSPQueueSendQuenceMsgID(),&Msg,IPC_NOWAIT);
		printf("------------------------- UpdateToFlash filetype:%d\r\n",4);		
			
	}
}
#define TESTSOUND "/app/sound"
int  playmusic(){
	int fd=-1;
	unsigned char decodebuf[164]={0x0};
	decodebuf[0]=0x0;
	decodebuf[1]=0x01;
	decodebuf[2]=0x50;
	decodebuf[3]=0x00;
	fd = open(TESTSOUND,O_RDONLY);
	if(fd<0){
		printf("open error\n");
		return -1;
	}
	//AudioOutPutOnOff(1);
	while(1)
	{
		int readlen;
		int decodecount  = 0;
		readlen= read(fd,decodebuf+4,160);
		if(readlen<=0)
		{
			printf("read error\n");
			break;
		}		
		SendAudioStreamToDecode(decodebuf,164,1);
		if(decodecount++>50)
		{
			sleep(1);
			decodecount =0;
		}
	}
}
int main(int argc, char *argv[])
{
	printf("SYSTEM_PARAMETER size:%d,Create Pthread:%s line:%d,time:%s, pid:%d  \n",sizeof(SYSTEM_PARAMETER),__FILE__,__LINE__,__TIME__,(unsigned)pthread_self());
	//system("killall -9 telnetd");
	//system("killall -9 wpa_supplicant");
	
	Init_IRCut();
	InitSysParameter();
	
	SignalRegister();
	WaitForSystemTime();
	InitPTZ(9600);
	InitEncodeSystem(); 
	
	
	
	GetNetModule()->StartNetDevice( );		
	GetNetModule()->StartNetModule( );
        P2P_Start();

	InitBlockDevice();
	InitRecordManage();
	StartRecordSystem();	
	//testUpgrade();
  	/*循环检测复位*/
	IdleProcess();	
}


