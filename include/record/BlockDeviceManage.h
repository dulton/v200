
#ifndef __BLOCK_DEVICE_MANAGE_H__

#define __BLOCK_DEVICE_MANAGE_H__

#include "common.h"

typedef enum
{
	BLK_DEV_ID_HDD00 = 0,			//固定的存储设备，HDD,SD,TF等
	BLK_DEV_ID_HDD01,
	BLK_DEV_ID_HDD02,
	BLK_DEV_ID_HDD03,
	BLK_DEV_ID_UDISK0,				//可插拔的存储设备，u盘，暂不支持
	BLK_DEV_ID_UDISK1,
	BLK_DEV_ID_MAX_NUM			//最大支持块设备的数量
}BLK_DEV_ID;

typedef enum
{
	BLK_DEV_STATUS_NOT_EXIST = 0,	 //该块设备不存在
	BLK_DEV_STATUS_NOT_MOUNTE,	 //该块设备格式化完成但未挂载	
	BLK_DEV_STATUS_MOUNTED,		 //该块设备已经挂载	
	BLK_DEV_STATUS_FORMATING	 //该块设备正在格式化	
}BLK_DEV_STATUS;

typedef struct
{
	BLK_DEV_STATUS enBlkDevStatus;	//块设备状态
	char 	strDeviceNode[32];		//设备节点,格式:/dev/sda1
	char		strMountPoint[32];		//挂载点，格式:/hdd00/p01
	unsigned long u32TotalCapacityMB;	//总容量,	单位:MB
	unsigned long u32FreeSpaceMB;	//剩余空间,单位:MB
}BLK_DEV_INFO;


class BlockDevManage
{

public:
	static BlockDevManage * m_pInstance;
	int					s32BlkDevNum;							//发现的快设备数量	
	BLK_DEV_INFO		stBlkDevInfo[BLK_DEV_ID_MAX_NUM];		//块设备信息
	int ProbeSDCard();
	
public:	
	BlockDevManage();	
	~BlockDevManage();
	static BlockDevManage * GetInstance();	
	int GetBlockDeviceNum();	
	int Fat32Format(BLK_DEV_ID enBlkDevId);	
	int MountBlockDevice(BLK_DEV_ID enBlkDevId);
	int UmountBlockDevice(BLK_DEV_ID enBlkDevId);
	int GetBlockDeviceInfo(BLK_DEV_ID enBlkDevId, BLK_DEV_INFO *pstBlkDevInfo);	
};

#endif 


