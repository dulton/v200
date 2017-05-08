
#ifndef __VIDEO_AD_CTRL_H__

#define __VIDEO_AD_CTRL_H__



#define ADV7180_SET_CON					0x01		//对比度

#define ADV7180_SET_BRT					0x02		//亮度

#define ADV7180_SET_HUE					0x03		// 色度

#define ADV7180_SET_SAT					0x04		// 饱和度


#define ADV7180_VIDEOSTATUS				0x10		// 看视频是否丢失

void VideoADInit();

void GetVideoADStatus(int *status);

void VideoADExit();

#endif 

