
#ifndef __UPGRADE_MODULE_H__
#define __UPGRADE_MODULE_H__

#include "common.h"



#define VERSION_LEN 16
#define HARDWARE_LEN 32

typedef struct
{
	unsigned  int 		        m_magic;
	unsigned  char				m_uboot_version[VERSION_LEN];
	unsigned  char				m_kernel_version[VERSION_LEN];
	unsigned  char				m_rootfs_version[VERSION_LEN];
	unsigned  char				m_app_version[VERSION_LEN];
	unsigned  char				m_hardware_version[HARDWARE_LEN];
    unsigned  char		        m_date[VERSION_LEN];
	unsigned  int               m_md5[4];
	unsigned  int				m_file_len;	
	unsigned  int 		        m_filetype;	
	unsigned  char				m_reserved[116];
}UPGRADECHECKSTRUCT;



/*
* return value:
* -1:
* Can't Update the File to the Flash
* 0 :
* Can Update the File to the Flash 
*/

int CheckUpdateVersion(UPGRADECHECKSTRUCT *ptr_check);

int CheckUpdateFileMD5(const UPGRADECHECKSTRUCT *ptr, FILE * fp);

void UpdateToFlash(unsigned  int filetype);




#endif 

