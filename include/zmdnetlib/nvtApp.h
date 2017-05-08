/*********************************************************************
 * Copyright (C), ZMODO Technology Corp., Ltd.
 *********************************************************************
 * File Name    : nvtApp.h
 * Description  : 
 * Author       : hulz <danielhu@zmodo.cn>
 * Create Date  : 2012-11-02
 ********************************************************************/

#ifndef __NVTAPP_H__
#define __NVTAPP_H__

#if defined (__cplusplus) || defined (_cplusplus)
extern "C" {
#endif
int rtsp_start_strm(void *usr_data);
int rtsp_stop_strm(void *usr_data);
int rtsp_get_strm(char local_chn, char chn_type, void *buf, void *usr_data);
int nvt_ctrl_dev(int type, void *buf, int buf_sz);
int nvt_set_param(int type, void *buf, int buf_sz);
int nvt_get_param(int type, void *buf, int *buf_sz);

#if defined (__cplusplus) || defined (_cplusplus)
}
#endif
#endif /* __NVTAPP_H__ */

