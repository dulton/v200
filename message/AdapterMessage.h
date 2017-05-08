
#ifndef  _ADAPTERMESSAGE_H
#define _ADAPTERMESSAGE_H

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <signal.h>
#include <sys/ipc.h>

#include <sys/msg.h>

/*信号量等待模式 */
#define     NO_WAIT                     (0)
#define     WAIT_FOREVER                (1)
#define     MAXLEN_MSGBUF               (256)
#define 	MAX_QUEUEMSGNUM         (16)                /*消息队列的消息数*/
#define 	MAX_QUEUEMSGSIZE        (256)                /*消息队列的消息大小*/


#define 	MESSAGENAME			"DHCP"


#define 	CMD_SETDHCP			0x1
#define 	CMD_UPGRADE			0x2

typedef struct tagDHCP_STR
{
	int		MsgCmd;		//hdcp 命令字为1
	int   	Aciton;		//0:关闭dhcp  1:开启dhcp                      
	char    NetName[16];//网卡名称
}T_CMDDHCP;



typedef struct tagMSGBUFFER_STR
{
	int   	mtype;                      /*消息类型必须大于0*/
	char    mtext[MAXLEN_MSGBUF];
}T_MSGBUF;
/***********************************/
/*消息队列*/
/***********************************/

/*获取创建消息队列的keyID*/
int OSPGetKeyID(char *szName);
/*创建消息队列*/
int OSPQueueCreate(char *szName,int *udwQueueID);

/*发送消息*/
int OSPQueueSendMessage(int udwQueueID,T_MSGBUF *msg,int dwFlag);

/*接收消息*/
int OSPQueueRcvMessage(int udwQueueID,int msgtype,T_MSGBUF *msg,int dwFlag);

/*删除消息队列*/
int OSPQueueDelete(int udwQueueID);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*_SYSADAPTER_H*/
