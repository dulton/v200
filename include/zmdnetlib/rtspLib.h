/************************************************************************
 *
 * Copyright (C), Zmodo Technology Corp., Ltd.
 *
 * File Name : rtspLib.h
 *
 * Description : Provide the initialization, de-initialization
 *               and stream data callback function interfaces
 *               to the application level.
 *
 * Author : hulz <lizhen.hu@santachi.com.cn>
 *
 * Create Date : 2012-08-01
 *
 ***********************************************************************/


#ifndef __RTSPLIB_H__
#define __RTSPLIB_H__

#if defined (__cplusplus) || defined (_cplusplus)
extern "C" {
#endif

#define RTSP_DEBUG 0
/* Not recommended: Look up NALU start code byte by byte. */
#ifdef DM368
	#define BYTE_BY_BYTE 1
#else
	#define BYTE_BY_BYTE 0
#endif

#define DFL_RTSP_PORT       10554 /* Default RTSP port. */

#define MAX_FRAME_SIZE      (1024 * 1024)


typedef int (*start_strm_t)(void *usr_data);
typedef int (*stop_strm_t)(void *usr_data);
typedef int (*get_strm_t)(char local_chn, char chn_type, void *buf, void *usr_data);


int zmd_rtsp_initLib(start_strm_t start, stop_strm_t stop, get_strm_t get);
int zmd_rtsp_deinitLib(void);
int zmd_rtsp_startService(void);
int zmd_rtsp_stopService(void);

#if defined (__cplusplus) || defined (_cplusplus)
}
#endif

#endif
