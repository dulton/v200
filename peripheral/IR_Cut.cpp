#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "IR_Cut.h"

static signed int  ircutfd=-1;
static const char* ircut_dev_name="/dev/ap1511a";

int Init_IRCut(void)
{
	if(ircutfd <= 0)
    {
		ircutfd = open(ircut_dev_name,O_RDWR);
		if(ircutfd <= 0)
		{
	    	printf("/dev/hi_gpio opened failed!\n");
	    	return -1;
		}
    }

    return 0;
}

int  Config_IRCutAD(unsigned int max,unsigned int min,unsigned char filp)
{
    if(ircutfd <= 0)
    {
		printf("ircutfd error\n");
		return -1;
    }
	if(max==0||min==0)
	{
		return 0;
	}
	
	if(-1==ioctl(ircutfd,IRCUT_ADMAX,&max))
	{
		printf("********************ioctl IRCUT_ADMAX\n");
		return -1;

	}

	if(-1==ioctl(ircutfd,IRCUT_ADMIN,&min)) 
	{
		printf("********************ioctl IRCUT_ADMIN\n");
		return -1;

	}
	if(-1==ioctl(ircutfd,IRCUT_ADFILP,&filp)) 
	{
		printf("********************ioctl IRCUT_ADMIN\n");
		return -1;

	}
	printf("*************Config_IRCutAD  SUCCESS  max:%d,min:%d\n",max,min);

    return 0;
}
int  Get_IRCutStatus(unsigned char &status)
{
    if(ircutfd <= 0)
    {
		//printf("ircutfd error\n");
		return 0;
    }
	return ioctl(ircutfd,IRCUT_FLAG,&status);   
}
int  Set_NightSwtich(unsigned char status)
{
    if(ircutfd <= 0)
    {
		printf("ircutfd error\n");
		return 0;
    }
	printf("#####Set_NightSwtich status=%d####\n",status);
	return ioctl(ircutfd,IRCUT_NIGHTSWITCH,status);   
}


