#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "upgrademodule.h"
#include "md5File.h"
#include "WatchDog.h"
#include "ModuleFuncInterface.h"
#include "AdapterMessage.h"


extern DeviceConfigInfo ConfigInfo;

//Endian convert: big <-> little
unsigned int conv(unsigned int a)
{
	unsigned int b=0;
	b|=(a<<24)&0xff000000;
	b|=(a<<8)&0x00ff0000;
	b|=(a>>8)&0x0000ff00;
	b|=(a>>24)&0x000000ff;
	return b;
}


/*
 * 
 */
 int string_check(unsigned char *str_src,  unsigned char *str_dst, unsigned char filetype)
 {
	int i = 0;
	unsigned char tmp1[20];
	unsigned char tmp2[20];
	
	char *token1 = NULL;
	char *token2 = NULL;
	char *tmp_ptr1,*tmp_ptr2;

	
	memcpy(tmp1, str_src, VERSION_LEN);
	memcpy(tmp2, str_dst, VERSION_LEN);
	
	tmp_ptr1 = (char *)tmp1;
	tmp_ptr2 = (char *)tmp2;
	
	/*Uboot , Kernel, Rootfs version compare*/
    if ( filetype < 4)
	{	
		for( i = 0; i< 3; i++)
		{
			token1  = strsep (&tmp_ptr1, ".");
			token2  = strsep (&tmp_ptr2, ".");
			if (strcmp(token1,token2) != 0) 
				return -1;
		}

		if (strcmp(tmp_ptr1,tmp_ptr2) == 0)
			return -2;
		else
			return 0;
		
	}
	/*App version compare*/
	if ( filetype ==  4) 
	{
		/*first two tokens compare */
		for( i = 0; i< 2; i++)
		{
			token1  = strsep (&tmp_ptr1, ".");
			token2  = strsep (&tmp_ptr2, ".");
			if (strcmp(token1,token2) != 0) 
				return -1;
		}
		/*the third token compare */
		token1  = strsep (&tmp_ptr1, ".");
		token2  = strsep (&tmp_ptr2, ".");
		if (strcmp(token1,token2) != 0) 
			return 0;
		else {
			if (strcmp(tmp_ptr1,tmp_ptr2) != 0)
				return 0;
			else
			    return -2;
		}
		
	}
	
	return 0;

 }
 
 
 /*
 * return value:
 * -1:
 * Can't Update the File to the Flash
 * -2:
 * Version is same 
 * 0 :
 * Can Update the File to the Flash 
 */
int check_update_file(UPGRADECHECKSTRUCT *ptr_check, DeviceConfigInfo *ptr_devinfo)
{
	int ret = -1;
	
	ret = string_check(ptr_check->m_uboot_version,ptr_devinfo->UbootVersion,1);
	if (ret != 0)
    {	
		if (  ptr_check->m_filetype <= 4 && ptr_check->m_filetype == 1)
			goto out;
	}
	else 
	{
		printf("net uboot:%s,local uboot:%s\n",ptr_check->m_uboot_version,ptr_devinfo->UbootVersion);
		return 0;
	}

	ret = string_check(ptr_check->m_kernel_version,ptr_devinfo->KernelVersion,2);
	if ( ret != 0)
    {	
		if (  ptr_check->m_filetype <= 4 && ptr_check->m_filetype == 2)
			goto out;
	} else {
		printf("net kernel:%s,local kernel:%s\n",ptr_check->m_kernel_version,ptr_devinfo->KernelVersion);
		return 0;
	}

	ret = string_check(ptr_check->m_rootfs_version,ptr_devinfo->RootfsVersion,3);
	
	if ( ret != 0)
    {	
		if (  ptr_check->m_filetype <= 4 && ptr_check->m_filetype == 3)
			goto out;
	} else {
		printf("net rootfs:%s,local rootfs:%s\n",ptr_check->m_rootfs_version,ptr_devinfo->RootfsVersion);
		return 0;
	}

	ret = string_check(ptr_check->m_app_version,ptr_devinfo->AppVersion,4);
	if ( ret != 0)
    {	
		if (  ptr_check->m_filetype <= 4 && ptr_check->m_filetype == 4)
			goto out;
	} else {
		printf("net app:%s,local app:%s\n",ptr_check->m_app_version,ptr_devinfo->AppVersion);
		return 0;
	}
	
out:
	return ret;
}


 
 /*
 * return value:
 * -1:
 * Can't Update the File to the Flash
  * -2:
 * Version is same 
 * 0 :
 * Can Update the File to the Flash 
 */


 int CheckUpdateVersion(UPGRADECHECKSTRUCT *ptr_check)
 {
	int fd = -1;
	
 	fd = open(DEVICE_CONFIG_BIN, O_RDONLY);
	/*不存在跳过版本验证*/
	if(fd == S_FAILURE)
	{
		return 0;
	}
	else
	{
		close(fd);	
	}
	return check_update_file(ptr_check, &ConfigInfo);
 }


int CheckUpdateFileMD5(const UPGRADECHECKSTRUCT *ptr, FILE * fp)
{
	MD5VAL val;
	if(fp == NULL || ptr == NULL)
	{
		printf("CheckUpdateFileMD5 fp is NULL\r\n");
		return -1;
	}
	val = md5File(fp);

	if(ptr->m_md5[0] == conv(val.a) &&
		ptr->m_md5[1] == conv(val.b) &&
		ptr->m_md5[2] == conv(val.c) &&
		ptr->m_md5[3] == conv(val.d) )
	{
		return 0;
	}
		
	return -1;
}


void UpdateToFlash(unsigned  int filetype)
{
	char arg[2] = {0};
	char path[64] = "/tmp/upgrade";
	char cmd[64] = "upgrade";
	char  type;
	type = (char)filetype;

	CWatchDog  *pWDT = CWatchDog::Instance();
	sleep(1);
	pWDT->Dogclose();
	wifi_disable();


	T_MSGBUF	Msg;
	int 		QuenceMsgCmd=CMD_UPGRADE;
	Msg.mtype = 1;
	memcpy(Msg.mtext,&QuenceMsgCmd,sizeof(QuenceMsgCmd));
	Msg.mtext[4]=filetype;
	OSPQueueSendMessage(GetOSPQueueSendQuenceMsgID(),&Msg,IPC_NOWAIT);
	printf("------------------------- UpdateToFlash filetype:%d\r\n",filetype);

	return ;





	if (fork() == 0) //child process 
	{
		printf("####################start run child process!\n");

//18e 分区 uboot config roofs+kernel app
		snprintf(arg, sizeof(arg), "%d", type);
		
		printf("UpdateToFlash():the upgrade path : %s\n", path);
		printf("UpdateToFlash():the upgrade filetype: %s\n", arg);
	   	execl(path, cmd, arg, (char*)0);
	}
	else
	{
		printf("####################continue parent !!!!!\n");
	}
	
	
}

