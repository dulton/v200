#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/vfs.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <scsi/scsi_ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>

#include "common.h"
#include "ModuleFuncInterface.h"
#include "BlockDeviceManage.h"

extern PARAMETER_MANAGE		*g_cParaManage ;
BlockDevManage * BlockDevManage::m_pInstance = NULL;

BlockDevManage*BlockDevManage::GetInstance()
{
	if(m_pInstance == NULL)
	{
		m_pInstance = new  BlockDevManage();
	}

	return m_pInstance;
}

#define PARTATION_PATH "/proc/partitions"
#ifdef PIVOT_3516C_1_5
	#define KEY_WORDS "mmcblk0"
	#define DEV_NODE "/dev/mmcblk0"
#elif defined V200
	#define KEY_WORDS "mmcblk0"
	#define DEV_NODE "/dev/mmcblk0p1"
	
#else
	#define KEY_WORDS "mmcblk0p1"
	#define DEV_NODE "/dev/mmcblk0p1"
#endif
#define MOUNT_POINT "/hdd00/p01"
#define BUF_SIZE (64)
int BlockDevManage::ProbeSDCard()
{
    int s32Ret = S_FAILURE;
    int size = 0;
    int fd = -1;
    char buffer[BUF_SIZE] = {0};
    
    fd = open(PARTATION_PATH, O_RDONLY);
    if (fd < 0)
    {
        printf("Open check sd card(%s) error!!!!!\n", PARTATION_PATH);
        return S_FAILURE;
    }

    while ((size = read(fd, buffer, (sizeof(buffer)-1))) > 0)
    {
        if (strstr(buffer, KEY_WORDS) != NULL)
        {
            s32Ret = S_SUCCESS;
            break;
        }
        memset(buffer, 0, sizeof(buffer));
    }
    if (-1 != fd)
    {
        close(fd);
    }

    printf("ProbeSDCard,s32Ret=%d\n", s32Ret);
    return s32Ret;
}

BlockDevManage::BlockDevManage()
{
	for(int i = 0; i < BLK_DEV_ID_MAX_NUM; i++)
	{
		memset(&stBlkDevInfo[i], 0, sizeof(BLK_DEV_INFO));
	}

	s32BlkDevNum = 0;

	int count = 10;
	while(ProbeSDCard() == S_FAILURE && (count--) > 0){
		usleep(100*1000);
	}
	
	if(ProbeSDCard() == S_SUCCESS)
	{
		s32BlkDevNum++;
		stBlkDevInfo[BLK_DEV_ID_HDD00].enBlkDevStatus = BLK_DEV_STATUS_NOT_MOUNTE;
		strcpy(stBlkDevInfo[BLK_DEV_ID_HDD00].strDeviceNode,DEV_NODE);
		strcpy(stBlkDevInfo[BLK_DEV_ID_HDD00].strMountPoint,MOUNT_POINT);
		UmountBlockDevice(BLK_DEV_ID_HDD00);
		if(MountBlockDevice(BLK_DEV_ID_HDD00) == S_SUCCESS)
			stBlkDevInfo[BLK_DEV_ID_HDD00].enBlkDevStatus = BLK_DEV_STATUS_MOUNTED;
		
		#ifdef PIVOT_3516C
			#ifndef PIVOT_3516C_1_5
				COMMON_PARA  common_para;
				g_cParaManage->GetSysParameter(SYSCOMMON_SET,&common_para);	
				common_para.m_nRebootCount = 0;
				g_cParaManage->SetSystemParameter(SYSCOMMON_SET,&common_para);
			#endif
		#endif
	}
	else
	{
		/*找不到节点重启pivot*/
		#ifdef PIVOT_3516C		
			#ifndef PIVOT_3516C_1_5
				COMMON_PARA  common_para;
				g_cParaManage->GetSysParameter(SYSCOMMON_SET,&common_para);
				if(common_para.m_nRebootCount  < 3)
				{
					common_para.m_nRebootCount++;
					g_cParaManage->SetSystemParameter(SYSCOMMON_SET,&common_para);					
					system("reboot");
				} 			
			#endif
		#endif
	}
}

BlockDevManage::~BlockDevManage()
{

}

int BlockDevManage::UmountBlockDevice(BLK_DEV_ID enBlkDevId)
{	
	int s32Ret = umount(stBlkDevInfo[enBlkDevId].strMountPoint);
	if(s32Ret >= 0)
	{
		stBlkDevInfo[enBlkDevId].enBlkDevStatus = BLK_DEV_STATUS_NOT_MOUNTE;
		printf("UmountBlockDevice S_SUCCESS \n");
		return S_SUCCESS;
	}
	
	if(stBlkDevInfo[enBlkDevId].enBlkDevStatus == BLK_DEV_STATUS_MOUNTED)
	{
		printf("UmountBlockDevice S_FAILURE\n");						
		return S_FAILURE;	
	}
	
	return S_SUCCESS;
}

int BlockDevManage::MountBlockDevice(BLK_DEV_ID enBlkDevId)
{
	if(stBlkDevInfo[enBlkDevId].enBlkDevStatus != BLK_DEV_STATUS_NOT_MOUNTE)
		return S_FAILURE;
	
//	int s32Ret = mount(stBlkDevInfo[enBlkDevId].strDeviceNode,stBlkDevInfo[enBlkDevId].strMountPoint, "vfat", MS_NOSUID|MS_NODEV, NULL);
	int s32Ret = mount(stBlkDevInfo[enBlkDevId].strDeviceNode,stBlkDevInfo[enBlkDevId].strMountPoint, "vfat", MS_NOSUID|MS_NODEV, "codepage=936,iocharset=cp936,errors=continue");

	if (s32Ret == 0)
	{
		stBlkDevInfo[enBlkDevId].enBlkDevStatus = BLK_DEV_STATUS_MOUNTED;
		printf("MountBlockDevice S_SUCCESS\n");
		return S_SUCCESS;
	}
	
	printf("MountBlockDevice  S_FAILURE\n");			
	return S_FAILURE;
}

int BlockDevManage::GetBlockDeviceInfo(BLK_DEV_ID enBlkDevId, BLK_DEV_INFO *pstBlkDevInfo)
{
	unsigned long capacity = 0;
	unsigned long freespace = 0;
	unsigned long BlockSize = 0;
	unsigned long FreeBlock = 0;
	unsigned long TotalBlks = 0;
	struct statfs stbuf;
	
	if(pstBlkDevInfo == NULL) 
		return S_FAILURE;

	if(stBlkDevInfo[enBlkDevId].enBlkDevStatus != BLK_DEV_STATUS_MOUNTED)
	{
		*pstBlkDevInfo = stBlkDevInfo[enBlkDevId];
		return S_FAILURE;
	}

	int s32Ret = statfs(stBlkDevInfo[enBlkDevId].strMountPoint, &stbuf);
	if(s32Ret >= 0)
	{
		if(stbuf.f_bsize > 0) BlockSize = stbuf.f_bsize;
		if(stbuf.f_bfree > 0) FreeBlock = stbuf.f_bfree;
		if(stbuf.f_blocks> 0) TotalBlks = stbuf.f_blocks;

		//调试信息
//		plog("BlockSize = %ld\n",BlockSize);
//		plog("FreeBlock = %ld\n",FreeBlock);
//		plog("TotalBlks = %ld\n",TotalBlks);
		
		//把Byte转为MByte，一共右移20位
		//用原来的方式，((TotalBlks>>2)*(BlockSize>>8))>>10，当TotalBlks:1464675785 BlockSize:4096，进行乘法时发生溢出
		capacity = (TotalBlks>>12)*(BlockSize>>8);		
		freespace = (FreeBlock>>12)*(BlockSize>>8);		
		
//		plog("capacity = %ld\n",capacity);
//		plog("freespace = %ld\n",freespace);
		
	}
	else
	{
		printf("err,statfs,s32Ret = %d,errno = %d\n",s32Ret,errno);
	}

	stBlkDevInfo[enBlkDevId].u32TotalCapacityMB = capacity;
	stBlkDevInfo[enBlkDevId].u32FreeSpaceMB = freespace;
	*pstBlkDevInfo = stBlkDevInfo[enBlkDevId];
	
	return S_SUCCESS;	
}

int BlockDevManage::GetBlockDeviceNum()
{
	return s32BlkDevNum;
}

int BlockDevManage::Fat32Format(BLK_DEV_ID enBlkDevId)
{
///待处理
/*
	char strCmd[BUF_SIZE] = "";
	if(stBlkDevInfo[enBlkDevId].enBlkDevStatus != BLK_DEV_STATUS_NOT_MOUNTE)
	{
		printf("err,enBlkDevStatus = %d\n",stBlkDevInfo[enBlkDevId].enBlkDevStatus);	
		return S_FAILURE;
	}	
	stBlkDevInfo[enBlkDevId].enBlkDevStatus = BLK_DEV_STATUS_FORMATING;
		
	printf("----------fat32 format--------------\n");	
	sprintf(strCmd,"mkfs.vfat  %s",stBlkDevInfo[enBlkDevId].strDeviceNode);
	printf("strCmd:%s\n",strCmd);	
	SystemByChildProcess(strCmd,BLOCK);
	printf("----------fat32 format completed--------------\n");	
	
	stBlkDevInfo[enBlkDevId].enBlkDevStatus = BLK_DEV_STATUS_NOT_MOUNTE;
*/	
	return S_SUCCESS;
}
