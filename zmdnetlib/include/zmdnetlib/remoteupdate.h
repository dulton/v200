#ifndef __REMOTE_UPDATE_H__
#define __REMOTE_UPDATE_H__

/**
 * @file remoteupdate.h
 *
 * @brief  
 *
 * @auth mike
 *
 * @date 2013-12-19
 *
**/

/**
 * 内置的ZMODO远程升级服务器地址
 */
#define ZMD_UPDATE_SERVER	"upgrade.meshare.com"
//#define ZMD_UPDATE_SERVER	"172.18.33.6"

/** 升级服务器使用的HTTP端口*/
#ifndef RU_HTTP_PORT
#define RU_HTTP_PORT (80)
#endif

#ifndef ROK
#define ROK (0)
#endif

#ifndef RFAILED
#define RFAILED (-1)
#endif

/** 因为网络异常进行断点续传的次数*/
#ifndef RU_RETRY_DOWNLOAD_COUNT
#define RU_RETRY_DOWNLOAD_COUNT (3)
#endif

#define MAX_HTTP_REQ_LEN (2048)

#ifndef NET_CONNECT_TIME_OUT
#define NET_CONNECT_TIME_OUT (5)
#endif

/**
 * 定义了自动升级过程的可能状态
 */
typedef enum
{
	UPDATE_STAT_IDEL = 0,	/** 没有在进行远程升级*/
	UPDATE_STAT_IN_DOWNLOAD, /** 正在进行文件下载*/
	UPDATE_STAT_PAUSED,		/** 远程升级被暂停*/
	UPDATE_STAT_IN_UPDATE,	/** 文件下载完成，在升级*/
	UPDATE_STAT_ERROR,		/** 升级错误，比如服务器连接不上*/
	
}UPDATE_STAT_E;

/**
 * 定义了控制下载的指令
 */
typedef enum
{
	DOWNLOAD_CMD_IDEL = 0,	/** 没有下载*/
	DOWNLOAD_CMD_START, 	/** 开始下载*/
	DOWNLOAD_CMD_PAUSE, 	/** 暂停下载*/
	DOWNLOAD_CMD_RESUME, 	/** 恢复下载*/
	DOWNLOAD_CMD_CANCEL,	/** 取消下载*/
	
}DOWNLOAD_CMD_E;

/**
 * @brief 向远程升级服务器获取软件版本更新信息
 *
 * @param version 	返回的新版本号
 * @param updateflag 	是否需要更新，1:是，0:否
 * @param description 	新版本描述信息，仅当updateflag为1时有意义
 * @param des_len 	描述信息的长度
 * @param ubootversion 	当前系统UBOOT版本
 * @param kernelversion 	当前系统内核版本
 * @param fsversion 	当前系统文件系统版本
 * @param appversion 	当前系统APP版本
 *
 * @return 0:检测更新信息成功，-1:检测更新信息失败
 */
int ru_check_update_info
( 
	char* version,
	int* updateflag, 
	char* description, 
	int* des_len,
	const char* ubootversion,
	const char* kernelversion,
	const char* fsversion,
	const char* appversion
);

/**
 * @brief 开始系统升级
 *
 * @param savefile 下载升级文件后写入的文件
 */
void ru_start_update( const char* savefile);

/**
 * @brief 获取升级状态
 *
 * @param update_stat 升级状态
 * @param process 下载进度
 */
void ru_get_update_stat( int* update_stat, int* process );

/**
 * @brief 取消升级
 *
 * @return 0:取消成功，1:无法取消
 */
int ru_cancel_update();

/**
 * @brief 暂停升级
 *
 * @return 0:暂停成功，1:无法暂停
 */
int ru_pause_update();

/**
 * @brief 恢复升级
 *
 * @return 0:恢复成功，-1:无法恢复
 */
int ru_resume_update();

#endif /** __REMOTE_UPDATE_H__*/
