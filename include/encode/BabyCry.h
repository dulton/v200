#ifndef  _BABYCRY_H_
#define  _BABYCRY_H_
#ifdef __cplusplus ////main.cpp文件调用Abnormal.c文件，需要显示声明，否则链接会出错
extern "C" {
#endif

typedef void* ZMD_BABYCRY_HANDLE;

//control的返回值
#define ZMD_BABYCRY_CONTROL_RETURN_FAILED   0   // 失败
#define ZMD_BABYCRY_CONTROL_RETURN_SUCCESS  1   // 成功

//process的返回值,一个segment返回
#define ZMD_BABY_CRY		             1		// 婴儿哭声 10s输出一次
#define ZMD_BABY_NOTCRY		             0		// 正常声音
#define ZMD_BABY_TRAIN		        	-1		// 学习过程中
#define ZMD_BABY_TIME_UNENOUGH           2      //不够一个segment



//测试过程中可以调节的参数
typedef enum tagZMD_BABYCRY_CONTROL_PARA
{
	SetEmp        ,//
	SetHanming    ,//
	SetFFTC       ,//
	SetFilterNum  ,//
	SetDCTN       ,//
	SetK          ,
	SetActivityTh ,

	Setf0th1      ,
	Setf0th2      ,
	SetHFth       ,
	SetHAPRth     ,

	SetSecCntTh   ,
	SetFrameCntTh ,

}ZMD_BABYCRY_CONTROL_PARA;
//灵敏度
typedef enum tagZMD_BABYCRY_CONTROL
{
	BC_SetVerySensitive      ,//特别灵敏
	BC_SetGenSensitive       ,//一般灵敏
	BC_SetSensitive          ,//灵敏
	BC_SetUnSensitive        ,//不灵敏
	//BC_SetVeryUnSensitive    ,//很不灵敏
}ZMD_BABYCRY_CONTROL;


    /**
    * @brief                初始化操作
    */
	ZMD_BABYCRY_HANDLE ZMD_BabyCry_init();

	/**
    * @brief                控制函数
    * @param controlType    控制类型，详见ZMD_BABYCRY_CONTROL
    * @param buffer         控制参数，详见ZMD_BABYCRY_CONTROL
    * @return               返回值，详见ZMD_BABYCRY_CONTROL_RETURN_*
    */
    int ZMD_BabyCry_control_para(ZMD_BABYCRY_HANDLE handle, ZMD_BABYCRY_CONTROL controlType);
    //int ZMD_BabyCry_control(ZMD_BABYCRY_HANDLE handle, ZMD_BABYCRY_CONTROL controlType);
	/**
    * @brief                处理函数，连续的n次视频流为异常声音才输出异常声音
    * @param handle         init函数的返回值，包含申请的内存空间
    * @param buffer         从IPC获取的数据流
    * @param bufferLen      数据流的长度
    * @return               返回值
    * @note                 n具有默认值ZMD_MIN_LENGTH_DEFAULT，可以通过Control函数重新设置
    */
	int      ZMD_BabyCry_process(ZMD_BABYCRY_HANDLE handle,short* tmpBuffer,int bufferLen);
	/**
    * @brief                释放申请的内存空间
    */
	void     ZMD_BabyCry_free(ZMD_BABYCRY_HANDLE *handle);

#ifdef __cplusplus

};
#endif


#endif