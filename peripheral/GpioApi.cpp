#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "GpioApi.h"

static signed int gpiofd=-1;
static const char* gpio_dev_name="/dev/gpio_dev";

int HI_SetGpio_Open(void)
{
	if(gpiofd <= 0)
    {
		gpiofd = open(gpio_dev_name,O_RDWR);
		if(gpiofd <= 0)
		{
	    	printf("/dev/hi_gpio opened failed!\n");
	    	return -1;
		}
    }

    return 0;
}

int  Hi_SetGpio_Close(void)
{
    if(gpiofd <= 0)
    {
		printf("file closed already!\n");
		return 0;
    }

    close(gpiofd);
    gpiofd = -1;
    return 0;
}


int  Hi_SetGpio_SetDir(unsigned int gpio_groupnum,unsigned int gpio_bitnum,unsigned int gpio_dir )
{
	int ret = 0;
	gpio_groupbit_info gpio_status;

    gpio_status.groupnumber = gpio_groupnum;
    gpio_status.bitnumber = gpio_bitnum;
    gpio_status.value = gpio_dir;

    if(gpiofd <= 0)
    {
		printf("file not opened yet!\n");
		return -1;
    }

    ret = ioctl(gpiofd,GPIO_SET_BIT_DIR,&gpio_status);
    if(ret != 0)
    {
		printf("set dir failed!\n");
		return -1;
    }

    return 0;
}

int  Hi_SetGpio_GetDir(unsigned int gpio_groupnum,unsigned int gpio_bitnum,unsigned int* gpio_dir)
{
    int ret = 0;
    gpio_groupbit_info gpio_status;

    gpio_status.groupnumber = gpio_groupnum;
    gpio_status.bitnumber = gpio_bitnum;
    gpio_status.value = 0xffff;

    if(gpiofd <= 0)
    {
		printf("file not opened yet!\n");
		return -1;
    }

    ret = ioctl(gpiofd,GPIO_GET_BIT_DIR, &gpio_status);
    if(ret != 0)
    {
		printf("get dir failed!\n");
		return -1;
    }

    *gpio_dir = gpio_status.value & 0xff;

    return 0;
}

int  Hi_SetGpio_SetBit(unsigned int gpio_groupnum,unsigned int gpio_bitnum, unsigned int bitvalue)
{
    int ret = 0;
    gpio_groupbit_info gpio_status;

    gpio_status.groupnumber = gpio_groupnum;
    gpio_status.bitnumber = gpio_bitnum;
    gpio_status.value = bitvalue;

    if(gpiofd <= 0)
    {
		printf("file not opened yet!\n");
		return -1;
    }

    ret = ioctl(gpiofd, GPIO_SET_BIT, &gpio_status);
    if(ret != 0)
    {
		printf("set bit failed!\n");
		return -1;
    }

    return 0;
}

int  Hi_SetGpio_GetBit(unsigned int gpio_groupnum,unsigned int gpio_bitnum,unsigned int* bitvalue)
{
	int ret = 0;
    gpio_groupbit_info gpio_status;

    gpio_status.groupnumber = gpio_groupnum;
    gpio_status.bitnumber = gpio_bitnum;
    gpio_status.value = 0xffff;
	
    if(gpiofd <= 0)
    {
		printf("file not opened yet!\n");
		return -1;
    }

    ret = ioctl(gpiofd,GPIO_GET_BIT,&gpio_status);
    if(ret != 0)
    {
		printf("get bit failed!\n");
		return -1;
    }

    *bitvalue = gpio_status.value & 0xff;

    return 0;

}
int HiGetResetGpio(unsigned char *value)
{
	int ret =-1;
	ret=ioctl(gpiofd,GPIO_FLAG,value);
	if(ret<0)
	{
		//printf("ioctl erro\n");
	}
	return ret;
}


int  HiGpioInit()
{
	if(HI_SetGpio_Open())
	{
		return -1;
	}

	return 0;
}

void HiGpioExit()
{
	Hi_SetGpio_Close();
}

