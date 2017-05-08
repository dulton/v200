/*********************************************************************
 * Copyright (C), ZMODO Technology Corp., Ltd.
 *********************************************************************
 * File Name    : nvtApp.cpp
 * Description  :
 * Author       : hulz <danielhu@zmodo.cn>
 * Create Date  : 2012-11-02
 ********************************************************************/

#ifdef SUPPORTONVIF
#include "nvtApp.h"
#include "nvtLib.h"
#include "rtspLib.h"
#include "CommonDefine.h"
#include "common.h"
#include "systemparameterdefine.h"
#include "parametermanage.h"
#include "ov7725.h"
#include "ModuleFuncInterface.h"
#include "upgrademodule.h"
#include "netserver.h"
#include "mediamgr.h"
#include "zmdnetlib.h"

extern PARAMETER_MANAGE *g_cParaManage;
extern DeviceConfigInfo 	ConfigInfo;


typedef struct frame_head_s 
{
	int frame_type;
	int frame_size;
	int frame_rate;
	venc_stream_t venc_stream;
} FRAME_HEAD;

int rtsp_start_strm(void *usr_data)
{
	NDB("rtsp start strm");
    return 0;
}

int rtsp_stop_strm(void *usr_data)
{
    int usr_id = *(int *)usr_data;
    NDB("stop stream[usr_id = %d]!\n", usr_id);
	
    GetMediaMgr()->freeMediaSession( 0 , 2 , usr_id ) ;
	
    NDB( "free media session %d\r\n" , usr_id ) ;
	
    return 0;
}
static int frame_rate;

int rtsp_get_strm(char local_chn, char chn_type, void *buf, void *usr_data)
{
    int ret = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int use = 0;
    int usr_id = *(int *)usr_data;

    unsigned char *frmp = NULL;         /* Frame pointer. */
    VideoFrameHeader *frm_hdrp = NULL;  /* Frame header. */
	AudioFrameHeader *fm_audio = NULL; /*audio header*/
    FrameInfo info;
	FRAME_HEAD head;
	#ifdef SUPPORT_AUDIO
	AudioParm* param;
	#endif
	memset(&head, 0, sizeof(FRAME_HEAD));


	
	
	if (!usr_id)
	{
		CAMERA_PARA  encode_para;	
		memset(&encode_para, 0, sizeof(CAMERA_PARA));
		if(chn_type == 0)
		{
			g_cParaManage->GetSysParameter(SYSCAMERA_SET, (void *) &encode_para);
			frame_rate =  (encode_para.m_ChannelPara[0].m_uFrameRate);
			usr_id = GetMediaMgr()->getUnuseMediaSession( 0 , D720P_CHN_TYPE) ;	
		}
		else if(chn_type == 1)
		{
			usr_id = GetMediaMgr()->getUnuseMediaSession( 0 , VGA_CHN_TYPE) ;	
			g_cParaManage->GetSysParameter(SYSCAMERA_SET, (void *) &encode_para);
			frame_rate =  encode_para.m_ChannelPara[0].m_uSubFrameRate;
		}
	    	
	    
	    NDB( "get media session %d\r\n" , usr_id ) ;
	    if( usr_id == -1 )
		{
	    	printf( "get media session %d\r\n" , usr_id ) ;
	    	return -1 ;
		}
	    *(int *)usr_data = usr_id ;
		
	    ResetUserData2IFrame(0,chn_type , usr_id );
    }

    while (1) 
    {
		ret = GetSendNetFrame(local_chn, chn_type , usr_id , &frmp, &info);    	
		if(ret == -2)/*返回值-2 表示需要跳帧*/
			continue;

        if (ret == -1 ||
            info.FrmLength >= MAX_FRAME_SIZE ||
            info.FrmLength <= 0)
        {
            usleep(10 * 1000);
            continue;
        } 
		else  
        {
        	if(info.Flag == 1 || info.Flag == 2)
        	{
	        	frm_hdrp = (VideoFrameHeader *)frmp;
				
	            head.frame_type = info.Flag;
	            head.frame_size = info.FrmLength - sizeof(VideoFrameHeader);
				head.frame_rate = frame_rate;
				
				memcpy(&head.venc_stream, &info.venc_stream, sizeof(info.venc_stream));
	            memcpy((unsigned char *)buf, &head, sizeof(head));
	            memcpy((unsigned char *)buf + sizeof(head), frmp + sizeof(VideoFrameHeader), info.FrmLength - sizeof(VideoFrameHeader));
	            break;
        	}
			else if (info.Flag == 3)
			{
				#ifdef SUPPORT_AUDIO
				fm_audio = (AudioFrameHeader *)frmp;
				head.frame_type = info.Flag;
	            head.frame_size = info.FrmLength - sizeof(AudioFrameHeader) -4;
				
	            memcpy((unsigned char *)buf, &head, sizeof(head));
	            memcpy((unsigned char *)buf + sizeof(head), frmp + sizeof(AudioFrameHeader)+4, info.FrmLength - sizeof(AudioFrameHeader)-4);
				break;
				#endif
			} 
			else
			{
				//printf("=============================info.Flag = %d\n", info.Flag);
				return -1;
			}
        }
    }
    return 0;
}

int nvt_ctrl_dev(int type, void *buf, int buf_sz)
{
	NDB("type:%d\n",type);
    switch (type) {
    case NVT_CTRL_REBOOT_SYSTEM:
        NDB("Debug Mode: Do not reboot system, but give a notice.\n");
		//system("reboot");
        break;
    case NVT_CTRL_UPGRADE_SYSTEM:
	{
		#if 0
        UPGRADEINFO upgrade_info;
        upgrade_info_t *infop = (upgrade_info_t *)buf;

        if (infop == NULL || infop->file_name == NULL || infop->file_buf == NULL)
		{
            printf("Upgrade system failed!\n");
            return -1;
        }
        memset(&upgrade_info, 0, sizeof(upgrade_info));
        strncpy(upgrade_info.m_filename, infop->file_name, sizeof(upgrade_info.m_filename));
        upgrade_info.buffer = infop->file_buf;
        upgrade_info.BufferLen = infop->file_sz;
        CreateNetUpgradeSoftwareThread(&upgrade_info);
        while(upgrade_info.m_percent != 100)
        {
            printf("\033[32;1m[%s, %s, %d]====> m_upgrade_status = %d\033[0m\n", __FILE__, __FUNCTION__, __LINE__, upgrade_info.m_upgrade_status);
            if((upgrade_info.m_upgrade_status == PROGRAM_UPGRADEFILEEEOR)
               || (upgrade_info.m_upgrade_status == PROGRAM_UPGRADERROR))
            {
                printf("upgrade system failed!\n");
                return -1;
            }
            else if(upgrade_info.m_upgrade_status == PROGRAM_UPGRADEALREADY)
            {
                return -1;
            }
            sleep(2);
        }
        printf("upgrade system success!\n");
		#endif
        break;
    }
	case NVT_CTRL_SNAP_IMAGE:
	{
		SnapImageArg  Snap;
		memset(&Snap,0x0,sizeof(SnapImageArg));
		sprintf(Snap.FileName, "%s", "snaponvif.jpg");
		int snap_ret = -1;
		snap_ret  = SnapOneChannelImage(0, &Snap);
		if(snap_ret == 0)
		{
			NDB("path:%s\r\n", Snap.FullFileName);
		}
		break;
	}
	case NVT_CTRL_RE_IFRERAME:
	{
		#ifndef IPC720P
        //RequestIFrame(0);
		#endif
		// RequestIFrame(1);
		break;
	}
    default:
        ERR("Unknown NVT control type: %d\n", type);
        return -1;
    }
    return 0;
}

int nvt_set_param(int type, void *buf, int buf_sz)
{
	NDB("type:%d\n", type);
    switch (type)
    {
    case NVT_PARAM_IMG_CFG:
    {
        CAMERA_ANALOG tmp;
        img_cfg_t *img_cfgp = (img_cfg_t *)buf;

        g_cParaManage->GetSysParameter(SYSANALOG_SET, &tmp);
        tmp.m_Channels[0].m_nContrast = img_cfgp->contrast;
        tmp.m_Channels[0].m_nBrightness = img_cfgp->brightness;
        tmp.m_Channels[0].m_nSaturation = img_cfgp->saturation;
        tmp.m_Channels[0].m_nHue = img_cfgp->hue;
#ifdef APP3511
#ifdef IPCVGA
        Setov7725Reg(DC_SET_BRIGHT, tmp.m_Channels[0].m_nBrightness);
        Setov7725Reg(DC_SET_CONTRACT, tmp.m_Channels[0].m_nContrast);
        Setov7725Reg(DC_SET_SATURATION, tmp.m_Channels[0].m_nSaturation);
        //Setov7725Reg(DC_SET_HUE, tmp.m_Channels[0].m_nHue);
#ifdefine IPC720P
        SetMt9d131Reg(MT_DC_SET_BRIGHT, tmp.m_Channels[0].m_nBrightness);
#endif
#endif
        g_cParaManage->SetSystemParameter(SYSANALOG_SET, &tmp);
        break;
    }
	case NVT_PARAM_SET_NETWORK:
	{
		nvt_net_cfg_t *net = (nvt_net_cfg_t*)buf;
		unsigned char ipaddr[4] = {0};
		unsigned char dns[4] = {0};
		int tmp1,tmp2,tmp3,tmp4;
		
		if(net != NULL && buf_sz == sizeof(nvt_net_cfg_t))
		{
			NETWORK_PARA NetWork;
			g_cParaManage->GetSysParameter(SYSNET_SET, &NetWork);

			if(net->ipaddr[0] != '\0')
			{
				sscanf(net->ipaddr, "%d.%d.%d.%d", &tmp1, &tmp2, &tmp3, &tmp4);
				ipaddr[0] = tmp1;
				ipaddr[1] = tmp2;
				ipaddr[2] = tmp3;
				ipaddr[3] = tmp4;
				for(int i = 0; i < 4; i++)
				{
					NetWork.m_Eth0Config.m_uLocalIp[i] = ipaddr[i];
				}
				CNetServer::getInstance()->SetNetAttrib(&NetWork, 0);
			}
			
			if(net->mDNSIp[0] != '\0')
			{
				sscanf(net->mDNSIp, "%d.%d.%d.%d", &tmp1, &tmp2, &tmp3, &tmp4);
				dns[0] = tmp1;
				dns[1] = tmp2;
				dns[2] = tmp3;
				dns[3] = tmp4;
				for(int i = 0; i < 4; i++)
				{
					if(NetWork.m_DNS.m_umDNSIp[i] != dns[i])
					{
						NetWork.m_DNS.m_umDNSIp[i] = dns[i];
					}
				}
			}
			
			if(NetWork.m_Eth0Config.m_dhcp != net->dhcp)
			{
				NetWork.m_Eth0Config.m_dhcp = net->dhcp;
				if(GetNetModule()->SetDHCP((char*)"eth0") < 0)
					NDB("######### faild to set dhcp  ########\r\n");
				else
					NDB("######### set dhcp success  ########\r\n");
			}
			
			g_cParaManage->SetSystemParameter(SYSNET_SET, &NetWork);
		}
		break;
	}
	case NVT_PARAM_SET_DEV :
	{
		nvt_dev_cfg_t*dev = (nvt_dev_cfg_t*)buf;
		NETWORK_PARA tmp;
		if(dev != NULL && buf_sz == sizeof(nvt_dev_cfg_t))
		{
			g_cParaManage->GetSysParameter(SYSNET_SET, &tmp);
			tmp.m_NatConfig.m_u8NatIpValid = dev->discv_mode;
			//g_cParaManage->SetSystemParameter(SYSNET_SET, &tmp);  /* 发布版本不打开搜索开关*/
		}
		break;
	}
    default:
        ERR("Unknown parameter type which nvt sets: %d\n", type);
        return -1;
    }
    return 0;
}

int nvt_get_param(int type, void *buf, int *buf_sz)
{
	//printf("----------------------------------function:%s,line:%d,type:%d\n",__FUNCTION__,__LINE__,type);
    switch (type) 
	{
    case NVT_PARAM_IMG_CFG:
		{
	        CAMERA_ANALOG tmp;
	        img_cfg_t *img_cfgp = (img_cfg_t *)buf;

	        if (buf_sz) 
			{
	        *buf_sz = sizeof(img_cfg_t);
	  		}

	        g_cParaManage->GetSysParameter(SYSANALOG_SET, &tmp);
	        img_cfgp->contrast = tmp.m_Channels[0].m_nContrast;
	        img_cfgp->brightness = tmp.m_Channels[0].m_nBrightness;
	        img_cfgp->saturation = tmp.m_Channels[0].m_nSaturation;
	        img_cfgp->hue = tmp.m_Channels[0].m_nHue;
	        break;
   		 }
    case NVT_PARAM_NET_STATUS: 
		{
        int *status = (int *)buf;
        
        if (buf_sz) 
		{
            *buf_sz = sizeof(int);
        }

        *status = CNetServer::getInstance()->JudgeNetworkStatus();
        break;
    }
    case NVT_PARAM_IPC_TYPE: 
	{
        int *type = (int *)buf;
        
        if (buf_sz) 
		{
            *buf_sz = sizeof(int);
        }

#ifdef IPC720P
        *type = 1;
#else
        *type = 0;
#endif
        break;
    }
	case NVT_PARAM_GET_NETWORK:
	{
		nvt_net_cfg_t*net = (nvt_net_cfg_t*)buf;
		NETWORK_PARA	tmp;
		
		if (buf_sz) 
		{
            *buf_sz = sizeof(nvt_net_cfg_t);
        }
		g_cParaManage->GetSysParameter(SYSNET_SET, &tmp);
		net->dhcp = tmp.m_Eth0Config.m_dhcp;

		sprintf(net->mDNSIp, "%d.%d.%d.%d", tmp.m_DNS.m_umDNSIp[0], tmp.m_DNS.m_umDNSIp[1], tmp.m_DNS.m_umDNSIp[2], tmp.m_DNS.m_umDNSIp[3]);
		sprintf(net->ipaddr, "%d.%d.%d.%d", tmp.m_Eth0Config.m_uLocalIp[0], tmp.m_Eth0Config.m_uLocalIp[1], 
											tmp.m_Eth0Config.m_uLocalIp[2], tmp.m_Eth0Config.m_uLocalIp[3]);
		break;
	}
	case NVT_PARAM_GET_DEV:
	{
		nvt_dev_cfg_t*dev = (nvt_dev_cfg_t*)buf;
		NETWORK_PARA	tmp;
		MACHINE_PARA	machine;
		
		if (buf_sz) 
		{
            *buf_sz = sizeof(nvt_dev_cfg_t);
        }
		g_cParaManage->GetSysParameter(SYSNET_SET, &tmp);
		dev->discv_mode = 0;//tmp.m_NatConfig.m_u8NatIpValid;/* 发布版强制打开搜索开关*/
		
		g_cParaManage->GetSysParameter(SYSMACHINE_SET, &machine);
		dev->serialId = machine.m_uMachinId;
		snprintf(dev->hardware_id, sizeof(dev->hardware_id), "%s", HARDWAREVERSION);
		snprintf(dev->firmware_version, sizeof(dev->firmware_version), "%s", (char*)ConfigInfo.AppVersion);
		
		break;
	}
	case NVT_PARAM_GET_USER:
	{
		int i;
		int user_num = 0;
		nvt_dev_user *user = (nvt_dev_user *)buf;
		if (buf_sz)
		{
            *buf_sz = sizeof(nvt_dev_user);
        }
		USERGROUPSET m_user;
		g_cParaManage->GetSysParameter(SYSUSERPREMIT_SET, &m_user);
		
		for(i = 0; i < 16; i++)
		{
			if(m_user.m_UserSet[i].m_cUserName[0] > 0)
			{
				strncpy(user->UserSet[i].Username, m_user.m_UserSet[i].m_cUserName, sizeof(user->UserSet[i].Username));
				user_num++;
			}
			if(m_user.m_UserSet[i].m_s32Passwd[0] > 0)
			{
				strncpy(user->UserSet[i].Password, m_user.m_UserSet[i].m_s32Passwd, sizeof(user->UserSet[i].Password));
			}
			user->UserSet[i].UserLevel = m_user.m_UserSet[i].m_s32UserPermit;
		}
		user->changeinfo = m_user.m_changeinfo;
		user->UserNum = user_num;

		break;
	}
    default:
        ERR("Unknown parameter type which nvt gets: %d\n", type);
        return -1;
    }
    return 0;
}

#endif
