#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include 	"ptz.h"

#include	"gpio_motor.h"
int 	g_motorfd = -1;
int		g_autoscan = 0;
bool	g_init = false;
bool	g_stop = true;
int nFlag = 0;
static pthread_mutex_t motor_mutex = PTHREAD_MUTEX_INITIALIZER;  

static pthread_cond_t motor_cond = PTHREAD_COND_INITIALIZER;  

static	pthread_t ThreadMotorPID;	
static  motor_info  motor_data;	
static  motor_info  motor_scan;	
bool isStop = false;
static int isRuning = 0;
void *thread_Motor(void *arg);

int InitMotorPT()
{
	if(g_init)
		return -1;
	g_motorfd = open("/dev/gpio_motor",O_RDWR); //阻塞模式
	if(g_motorfd<0)
	{
		printf("open gpio_motor erro!\n");
		return -1;
	}	
	pthread_mutex_init(&motor_mutex, NULL);
	pthread_create(&ThreadMotorPID, NULL, &thread_Motor,NULL);
	pthread_detach(ThreadMotorPID);
	g_init = true;

	return 0;
}
int UnInitMotorPT()
{
	g_init = false;
	pthread_join(ThreadMotorPID, NULL);
	pthread_mutex_destroy(&motor_mutex);
	return 0;
}
int MotorPT_Operate(PTZ_CMD_E enCmd, int  s32Spd,int other)
{
	switch (enCmd)
	{
	 	case CMD_STOP:
			g_stop =true;
			g_autoscan = 0;
			break;
		case CMD_RIGHT:
			
			motor_data.type = MOTOR_H;
			motor_data.direct = FORWORD;
			motor_data.speed = 1;
			g_stop =false;
			g_autoscan = 0;
			break;	
		case CMD_LEFT:	
			motor_data.type = MOTOR_H;
			motor_data.direct = REVERSE;
			motor_data.speed = 1;
			g_stop =false;
			g_autoscan = 0;
			break;	
		case CMD_UP:
			motor_data.type = MOTOR_V;
			motor_data.direct = FORWORD;
			motor_data.speed = 8;
			g_stop =false;
			g_autoscan = 0;
			break;	
		case CMD_DOWN:
			motor_data.type = MOTOR_V;
			motor_data.direct = REVERSE;
			motor_data.speed = 8;
			g_stop =false;
			g_autoscan = 0;
			break;
		case CMD_AUTOSCAN:
			motor_scan.type = MOTOR_H;
			motor_scan.direct = FORWORD;
			motor_scan.speed = 1;
			g_stop =false;
			g_autoscan = 1;
			break;
		case CMD_V_SCAN:
			motor_scan.type = MOTOR_V;
			motor_scan.direct = FORWORD;
			motor_scan.speed = 8;
			g_stop =false;
			g_autoscan = 2;
			break;

		default:
			break;
	}	
	if(g_stop == false)
	{
		pthread_cond_broadcast(&motor_cond);
	}
	
	
	
	return 0;
}


int MotorTurnAroundTest(int flag)
{
	switch (flag)
	{
		 case 0:
		    MotorPT_Operate(CMD_STOP,0,0);
		    isStop = true;
		    isRuning = 0;
		    break;
		 case 1:
		    isStop = false;
		    if(isRuning == 0)
		    Motor_Ctrl();
		    else
		    break;
			
		    break;
		 case 2:
		    MotorPT_Operate(CMD_LEFT,0,0);
		    break;
		 case 3:
		    MotorPT_Operate(CMD_RIGHT,0,0);
		    break;
		 case 4:
		    MotorPT_Operate(CMD_UP,0,0);
		 case 5:
		    MotorPT_Operate(CMD_DOWN,0,0);
		    break;
		 default:
		    break;
	}
	return 0;
}



void AutoScan(bool sustain)
{
	
	while(g_autoscan)
	{
		if(g_autoscan ==1)//水平
		{
						
			if((motor_scan.direct == FORWORD)&&(motor_scan.type==MOTOR_H)&&(motor_scan.status&0x01))
			{
				printf("=====================MOTOR_H FORWORD LIMIT\n");	
				motor_scan.direct = REVERSE;
				
			}

			if((motor_scan.direct == REVERSE)&&(motor_scan.type==MOTOR_H)&&(motor_scan.status&0x02))
			{
				printf("=====================MOTOR_H REVERSE LIMIT\n");				
				motor_scan.direct = FORWORD;
				if(!sustain)
					return ;

			}			

		
			write(g_motorfd, &motor_scan, sizeof(motor_scan));
		}
		else if(g_autoscan ==2)//垂直
		{
			if((motor_scan.direct == FORWORD)&&(motor_scan.type==MOTOR_V)&&(motor_scan.status&0x08))
			{
				printf("=====================MOTOR_V FORWORD LIMIT\n");
				motor_scan.direct = REVERSE;
			}

			if((motor_scan.direct == REVERSE)&&(motor_scan.type==MOTOR_V)&&(motor_scan.status&0x04))
			{
				printf("=====================MOTOR_V REVERSE LIMIT\n");
				motor_scan.direct = FORWORD;
				if(!sustain)
					return ;
			}	
			
			write(g_motorfd, &motor_scan, sizeof(motor_scan));
		}
	}		
}

int Motor_Ctrl()
{
	motor_info  motor_ctrl_h;
	motor_info  motor_ctrl_v;
	motor_ctrl_h.direct = FORWORD;
	motor_ctrl_h.speed= 1;
	motor_ctrl_h.type= MOTOR_H;
	motor_ctrl_v.direct = FORWORD;
	motor_ctrl_v.speed= 8;
	motor_ctrl_v.type= MOTOR_V;
	isRuning = 1;

	while(!isStop)
	{
		do
		{
			write(g_motorfd, &motor_ctrl_h, sizeof(motor_ctrl_h));
		//	printf("\n-----[%d]--------------->motor_ctrl_h.status==[%d]\n",motor_ctrl_h.direct,motor_ctrl_h.status&0x01);
			if(isStop)
			return -1;
		}while(!(motor_ctrl_h.status&0x01));
			
		motor_ctrl_h.direct = REVERSE;
		do
		{
			write(g_motorfd, &motor_ctrl_h, sizeof(motor_ctrl_h));
	//		printf("\n-----[%d]--------------->motor_ctrl_h.status==[%d]\n",motor_ctrl_h.direct,motor_ctrl_h.status&0x02);
			if(isStop)
			return -1;
		}while(!(motor_ctrl_h.status&0x02));

		motor_ctrl_h.direct = FORWORD;

		do
		{
		        write(g_motorfd, &motor_ctrl_v, sizeof(motor_ctrl_v));
	//	        printf("\n-----[%d]--------------->motor_ctrl_v.status==[%d]\n",motor_ctrl_v.direct,motor_ctrl_v.status&0x08);
   			 if(isStop)
   			return -1;
		}while(!(motor_ctrl_v.status&0x08 ));
			
		motor_ctrl_v.direct = REVERSE;

		do
		{
			 write(g_motorfd, &motor_ctrl_v, sizeof(motor_ctrl_v));
	//printf("\n----[%d]---------------->motor_ctrl_v.status==[%d]\n",motor_ctrl_v.direct,motor_ctrl_v.status&0x04);
			 if(isStop)
			return -1;
		}while(!(motor_ctrl_v.status&0x04 ));

		motor_ctrl_v.direct = FORWORD;
			
		sleep(1);
	} 
	return 0;
}

void *thread_Motor(void *arg)
{
	motor_data.type = MOTOR_H;
	//motor_data.direct = FORWORD;
	motor_data.direct = FORWORD;
	motor_data.speed = 10;


	motor_scan.type = MOTOR_H;
	motor_scan.direct = FORWORD;
	motor_scan.speed = 1;
	g_autoscan = 1;
	AutoScan(false);

	motor_scan.type = MOTOR_V;
	motor_scan.direct = FORWORD;
	motor_scan.speed = 8;
	g_autoscan = 2;
	AutoScan(false);
	/*回到中点*/
	int step=40;
	motor_scan.direct = FORWORD;
	motor_scan.type=MOTOR_V;
	while(g_autoscan&&step--)
	{
		write(g_motorfd, &motor_scan, sizeof(motor_scan));
	}
	
	while(1)
	{
			
		if(g_stop==true)
		{
			pthread_mutex_lock(&motor_mutex);			
			pthread_cond_wait(&motor_cond,&motor_mutex); 			
			pthread_mutex_unlock(&motor_mutex);  
		}	
		if((motor_data.direct == FORWORD)&&(motor_data.type==MOTOR_H)&&\
			(motor_data.status&0x01))
		{
			printf("=====================MOTOR_H FORWORD LIMIT\n");
			g_stop=true;

		}
		if((motor_data.direct == REVERSE)&&(motor_data.type==MOTOR_H)&&\
			(motor_data.status&0x02))
		{
			printf("=====================MOTOR_H REVERSE LIMIT\n");
			g_stop=true;

		}
		
		if((motor_data.direct == REVERSE)&&(motor_data.type==MOTOR_V)&&\
			(motor_data.status&0x04))
		{
			printf("=====================MOTOR_V REVERSE LIMIT\n");
			g_stop=true;

		}

		if((motor_data.direct == FORWORD)&&(motor_data.type==MOTOR_V)&&\
			(motor_data.status&0x08))
		{
			printf("=====================MOTOR_V FORWORD LIMIT\n");
			g_stop=true;
		}
		if(g_autoscan>0)
			AutoScan(true);
		else
			write(g_motorfd, &motor_data, sizeof(motor_data));
		
		//printf("direct:%d,type:%d,status:%d,speed:%d\n",motor_data.direct,motor_data.type,motor_data.status,motor_data.speed);
		//usleep(10000);
		//pthread_mutex_unlock(&motor_mutex); 
		
	}
	return NULL;
}



