#ifndef _HDALG_H_
#define _HDALG_H_
/************************************************************************/
/*                            BG method                                 */
/************************************************************************/
typedef unsigned char uchar;
typedef void* HumanDetHandle;

//UpdateAndHD返回值定义（返回值从bool改为int）
#define NOT_ENOUGH_CHANGE 0 // 画面基本无变化，不需要录像
#define ENOUGH_CHANGE     1 // 画面有变化，需要录像，但不需要上报移动侦测
#define MOTION_DETECTED   3 // 需要录像且需要上报移动侦测

// 以下参数用不上，为多检测区域时参数。
// 与返回值按位求与，ret&MOTION_DETECTED_AREA1 大于零表示区域一发生移动，以此类推
//#define MOTION_DETECTED_AREA1   2 // 需要录像且需要上报移动侦测
//#define MOTION_DETECTED_AREA2   4 // 需要录像且需要上报移动侦测
//#define MOTION_DETECTED_AREA3   8 // 需要录像且需要上报移动侦测

HumanDetHandle CreateBackModel();

/**
*  @brief SetROI            设置侦测区域模块
*
*  @param x,y,w,h           区域的左上坐标和宽高
                            设备收到的区域为float类型，转化方法(设视频图像宽高为width,height)：
                            x = x_float * width;
                            y = x_float * height;
                            w = w_float * width;
                            h = h_float * height;
*  @param type              0- 删除之前的ROI，添加新的ROI
                            1- 保留之前的ROI，添加新的ROI
                            （单区域不要传1，会导致此次设置无效，ROI不变；
                              多区域时，如果区域个已满，也会导致此次设置无效）
                            2- 删除所有的ROI，侦测全图
*  @param phdHandle         实例
*
*  @note  
*/
bool SetROI(HumanDetHandle hdHandle, const int x, const int y, const int w, const int h, const int type);

/**
*  @brief UpdateAndHD       移动侦测处理模块
*
*  @param ptr_gray          灰度图像
*  @param width,height,step 灰度图的宽、高和step
*  @param sensitive         灵敏度参数，取值0、1、2、3，灵敏度依次降低
*  @param phdHandle         实例
*
*  @note  
*/
int UpdateAndHD(HumanDetHandle hdHandle, const uchar* ptr_gray, const int width, const int height, const int step, unsigned int sensitive);
bool ReleaseBackModel( HumanDetHandle* phdHandle );

#endif	//#define _HDALG_H_
