#ifndef  _ABNORMAL_H_
#define  _ABNORMAL_H_
#ifdef __cplusplus ////main.cpp文件调用Abnormal.c文件，需要显示声明，否则链接会出错
extern "C" {
#endif

#include "BabyCry.h"
typedef void* ZMD_AbHandle;

//control的返回值
#define ZMD_ABNORMALVOICE_CONTROL_RETURN_FAILED   0   // 失败
#define ZMD_ABNORMALVOICE_CONTROL_RETURN_SUCCESS  1   // 成功
//process的返回值
#define ZMD_ABNORMAL_VOICE		-1		// 异常声音
#define ZMD_NORMAL_VOICE		1		// 正常声音
#define ZMD_TRAIN_VOICE			0		// 学习过程中
#define ZMD_BABYCRY_VOICE       -2      //婴儿哭声

//灵敏度
typedef enum tagZMD_ABNORMALVOICE_CONTROL
{
	SetVerySensitive      ,//特别灵敏,最高
	SetGenSensitive       ,//一般灵敏，高
	SetSensitive          ,//灵敏，中
	SetUnSensitive        ,//不灵敏，低
	//SetVeryUnSensitive    ,//很不灵敏
}ZMD_ABNORMALVOICE_CONTROL;


    /**
    * @brief                初始化操作
    */
	ZMD_AbHandle ZMD_AbnormalVoice_init();

	/**
    * @brief                控制函数
    * @param controlType    控制类型，详见ZMD_ABNORMALVOICE_CONTROL
    * @param buffer         控制参数，详见ZMD_ABNORMALVOICE_CONTROL
    * @return               返回值，详见ZMD_ABNORMALVOICE_CONTROL_RETURN_*
    */
    int ZMD_AbnormalVoice_control(ZMD_AbHandle handle, ZMD_ABNORMALVOICE_CONTROL controlType_Ab,ZMD_BABYCRY_CONTROL controlType_BC);
	/**
    * @brief                处理函数，连续的n次视频流为异常声音才输出异常声音
    * @param handle         init函数的返回值，包含申请的内存空间
    * @param buffer         从IPC获取的PCM未压缩格式数据流
    * @param bufferLen      数据流的长度
    * @return               返回值
    * @note                 n具有默认值ZMD_MIN_LENGTH_DEFAULT，可以通过Control函数重新设置,
    */
	int ZMD_AbnormalVoice_process(ZMD_AbHandle handle,short* pcmBuffer,int bufferLen,double *db_ret);

    //输入的音频数据是海思g711格式数据，包含头部；其他参数与ZMD_AbnormalVoice_process一致
    int ZMD_AbnormalVoice_G711_process(ZMD_AbHandle handle,short* g711Buffer,int bufferLen,double *db_ret);
	
    /**
    * @brief                释放申请的内存空间
    */
	void     ZMD_AbnormalVoice_free(ZMD_AbHandle *handle);

#ifdef __cplusplus

};
#endif


#endif
