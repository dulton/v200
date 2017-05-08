#ifndef P2P_INTERFACE_H

#define P2P_INTERFACE_H

#include <time.h>

enum p2p_error_code
{
	P2P_ERR_UNKNOW = -1,
		
	P2P_OK = 0,
	P2P_ERR_OFFLINE = 1	
};

typedef struct _p2p_find_playback_file_t
{
    short  file_type;
    short  alarm_level;
    int  channel;
    int  file_size;
    int  is_finished;
    char create_date[32];
    char create_time[32];
    char finish_time[32];
    char file_path[96];
} p2p_find_playback_file_t;

typedef struct 
{
	/* 最多16位 */
	char physical_id[20];	
	/* 最多16个utf-8字符  */
	char device_name[50];	
	unsigned char  device_type;
	unsigned char  channel_id;
	/* md5 32位 */
	char local_pwd[34];	
	/* 使能开关 1开启，0 关闭*/
	int  use_on;
	char lan_ip[32];
    /* 报警器持续时间， 单位秒*/
    int  buzzer_last; 
    /* 是否启用报警器 */
    int  buzzer_trigger;

    /* 被动设备在哪种模式下触发报警器，此值是一个按位或的值，
     * 默认值为-1，此时表示所有模式下都触发，
     * 否则有0x1表示模式0下触发报警器，
     * 0x2表示模式1下触发报警器, 
     * 0x4表示模式2下触发报警器,依此类推;
     * 模式是指用户模式*/
    int  mode_buzzer;

    /* 小温控的温度 */
    int  temperature; 
    /* 通风孔的开关，小温控使用，0表示关，1表示开，2表示自动 */
    int  vent_switch;
}sub_device_info; 

/* 正在被添加的被动设备的信息 */
typedef struct 
{
	/* 最多16位 */
	char physical_id[32];
    /* 设备类型， 门磁 9*/
	int  dev_type;
}adding_device_info;

struct p2p_pt_preset_info_t
{
	char trigger_id[16]; // 被动设备的ID
	char preset_name[48]; // 预置位的名称: 唯一
	int  preset_on;		// 预置位的开关  0: 关  1： 开	
};
typedef struct 
{
	int 				schedule_id;
	int 				flag; 			// 0: 此schedule 为控制设备开关 ; 1: 控制告警推送
	int 				repeat_day; 	//按位或字段, bit0 表示星期天，以此类推
	int 				off_at;		//关闭的起始时刻
	int			 		on_at;		//关闭的结束时刻说明：如果off_at 与on_at 都为0, 则表示全天关闭
}p2p_schedule_t;

typedef struct _p2p_md_region_t {
    float x;      //横坐标比例
    float y;      //纵坐标比例
    float width;  //宽比例
    float height; //高比例
#ifdef _BUILD_FOR_NVR_
    int   channel;  //通道号
    char  physical_id[16];  //通道号对应的设备ID
#endif
} p2p_md_region_t;

typedef struct _p2p_timezonelist_t {
    char timezone[40];
    char desc_zh[200];
    char desc_en[48];
    char offset[8];
} p2p_timezonelist_t;

typedef struct _p2p_thermostatinfo_t {
    unsigned int air_switch;    // 空调开关，0为关，1为开
    unsigned int work_mode;     // 工作模式，0为制冷，1为制热，2为除湿，3为通风
    unsigned int cool_temp;     // 制冷模式下的温度设置
    unsigned int heat_temp;     // 制热模式下的温度设置
    unsigned int dry_temp;      // 除湿模式下的温度设置
    unsigned int ven_temp;      // 通风模式下的温度设置
} p2p_thermostatinfo_t;

typedef struct _p2p_thermostat_stat_t{
    char physical_id[16];// 被动设备或自己的ID（以下参数至少带一个）
    int temperature;// 温度设置
    int vent_switch;// 通风孔设置
    int current_temp;// 当前温度状态
    //表示通风孔状态，0为关，1为开
    //当physical_id为小温控且其通风孔开关设置为自动时才需要填此值，否则请填负值
    int vent_status; 
} p2p_thermostat_stat_t;

typedef struct _p2p_watering_ctrl_t {
    int op_type;   // 1 开始浇水； 2 停止浇水 
    char physical_id[32];   //浇水设备的ID
    int hole_id;    //出水口的序号，送0开始
    int time;   //浇水时长，单位分钟
} p2p_watering_ctrl_t;

typedef struct _p2p_push_condition_t {
    char delta[16];  // 10-20 单位百分比，告警时面积变化范围
    char push_time[32]; // 07:00-20:00, 24小时制
    int  alarm_interval;    //两次有效告警触发的时间间隔，单位秒
    int  after_nightview;   //夜视切换后多久移动侦测生效，单位秒
    char width_height_div[16];  // 10-20 单位是百分比，宽高变化范围
    char speed[16];             // 10-20 单位是百分比，目标移动速度范围
    int  position_x;        // 侦测区域，单位百分比
    int  position_y;
    int  position_w;
    int  position_h;
    int  push_type;         // 推送方式， 1 图片，2 视频，3 文字
    int  picture_count;     // 图片数量， push_type 为1时有用
    int  video_last;        // 告警视频时长，单位秒， push_type 为 2时有用
    int  video_bitrate;     // 视频码率级别， 1-5
    int  change_bitrate_interval;   //切换码率间隔, 单位秒
    int  if_push;           // 是否要推送
} p2p_push_condition_t;

typedef struct
{
	/* 
	设备类型		
	0-普通摄像头
	1-NVR
	2-DVR
	3-智能门铃
	4-智能温控
	5-智能开关
	6-智能门灯
	7-Pivot
	8-简易门铃
	9-门磁(被动设备)
	10-PIR(被动设备)
	11-烟感(被动设备)
	12-气感(被动设备)
	13-紧急按钮(被动设备)
	14-花园灯
    15-中继设备
    16-简易中继设备
    17-声光报警器(被动设备)
    18-遥控器(被动设备)
    19-大温控
    20-小温控(被动设备)
    21-通风孔(被动设备)
    22-简易PT
    23-复杂PT
	24-水位报警器(被动设备)
	25-灌溉设备(被动设备)
	26-全景相机
	27-鱼眼灯泡
	28-窗帘设备

	*/
	unsigned int device_type;
	/* 设备类型 DEV_TYPE_INFO */
	unsigned int device_capacity;
	/* 	
	新增设备能力（0：不支持，1：支持）
	bit0: 是否支持设备开关与告警schedule的设置功能（目前设备此字段要bit1支持，才可以设置）
	bit1: 设备是否支持sync接口
	bit2: 设备是否支持异常声音检测
	bit3: 表示spoe一拖二套包NVR，需要引导添加ipc
	bit4: 是否支持告警云录像
	bit5: 是否支持普通云录像，即支持新流程协议的云存储录像
	bit6: 设备是否支持时区设置能力.
	8-15位定义同ZSP协议CMD_DECIVE_TYPE 获取设备型号命令DeviceType字段
	bit16: 是否支持设备静音，
	bit17: 是否支持sd卡管理(查询和格式化)；
	bit18: 是否支持敏感度设置和告警邮箱列表设置,及推送间隔设置
	bit19：是否支持夜视设置
	bit20：是否支持图像翻转
	bit21：是否支持SD卡远程回放
	bit22：是否支持360度侦测
	bit23：是否支持被动设备
	bit24：是否支持h265
	bit25：是否支持预置位设置
	bit26: 是否支持bluetooth speak
	bit27: 是否支持全双工对讲
	bit28：是否支持夜灯
	bit29: 是否支持SD卡格式化
	bit30：是否支持zink
	*/
	unsigned int device_extend_capacity;

	/*  再新设备能力扩展
	bit0: 是否支持设备唤醒功能
	bit1: 是否支持区域侦测
    bit2: 是否支持光敏设置
    bit3: 是否支持电池续航
    bit4: 是否支持鱼眼能力
    bit5: 是否支持非局域网设置预置位
    bit6: 是否支持服务器下发的告警配置
    bit7: 是否支持音量设置
    bit8: 是否支持自动显示wifi列表

	*/
	unsigned long long device_supply_capacity;

	/* 报警输入通道数 */
	unsigned int device_alarm_in_num;
	/* 视频通道数 */
	unsigned int device_video_num;
		
	/* 设备是否支持有线联网, 1是，0否 */
	unsigned int use_wired_network;

	/* 设备的ID */
	const char *device_id;
	/* 配置目录 eg:/config */
	const char *config_dir;
	
	const char *uboot_version;
	
	const char *kernel_version;
	
	const char *rootfs_version;
	
	const char *app_version;
	
	const char *device_name;
	/* 网卡名称 wlan0, ra0, eth0 ... */
	const char *network_interface;

	/* 主码流(高清)的分辨率, 格式 1280*720 */
	const char *high_resolution;

	/* 次码流(普清)的分辨率, 格式 1280*720 */
	const char *secondary_resolution;
	
	/* 低码流(低清)的分辨率, 格式 1280*720 */
	const char *low_resolution;
	
	/* 语音留言文件的全路径 */
	const char *voice_message_path[5];

    /* 加密的key */
    const char *aes_key;
	/*
	设置客户端接听设备来电事件的回调函数, 
	当客户端通过远程触发接听事件时调用此回调,
	由视频流线程触发，请不要在回调内阻塞!!!
	*/
	void (*answer_call_callback)();

	/* 	
		此函数将服务器上的被动设备列表的全部内容同步到本地
		参数 int num, 列表中被动设备个数
    	注意: 当 num=0时，意思是清空列表
	*/

	int (*set_sub_device_list_callback)(int, const sub_device_info *);


	/* 
		此函数将服务器上的预置点列表的全部内容同步到本地
		参数 int num, 列表中预置点个数
		注意: 当 num=0时，意思是清空列表 
	*/
	void (*set_preset_list_callback)(int , const p2p_pt_preset_info_t *);

	/*
		播放留言回调函数, APP选择一条留言，让设备即时播放
		参数是音频文件路径
	*/
	void (*play_audio_file_callback)(const char*);
	
	/*
		查询本地回放列表回调函数, 支持远程本地回放的设备请赋值
		@date: 查询的日期, 格式 '2014-07-16'
		@channel: 查询的通道
		@list: 查询到的回放列表, 内部用malloc申请内容，调用后释放
		@num: 查询到的list里的录像个数
		return 0 on success
		if success and num is not 0, user must free info
	*/
	int (*find_playback_list_callback)(const char* date, int channel, 
			p2p_find_playback_file_t** list ,int* num);

	/* 新的校时方式,利用服务器返回的时间戳校时 */
	void (*timing_callback)(time_t timestamp);
	
	/* 
		刷新设备的状态回调函数,当app调用刷新封面接口时，调用此回调, 临时给pivot用于上报温湿度;
		返回值0或者再次尝试的毫秒数，当返回值不等于0时，内部会等待返回的毫秒数后再次调用此回调函数
	*/
	int (*on_refresh_device_callback)();

	/*
		设备上线后会调用此回调函数，@status 参数目前无用
	*/
	void (*on_device_online_callback)(int status);

	/*
		使设备进入ap 模式的回调函数
		@mode  1：表示进入软AP模式; 2:表示进入添加被动设备的监听模式
		@device_count 要连接的设备数量
	*/
	void (*on_ap_mode_callback)(int mode, int device_count);

    /*
     * 配置schedule回调
     * @num schedule的条数
     */
    void (*on_set_schedule)(p2p_schedule_t *sch, int num);

    /*
     * 测试报警器
     * @op_type 0停止，1触发
     */
    void (*on_test_buzzer)(int op_type);

    /*
     * 区域侦测的配置或者获取
     * @op_type 1 为设置， 2为获取
     */
    void (*on_md_region_ctrl)(int op_type, p2p_md_region_t* reg);

    /*
     * 温控设置回调函数
     */
    void (*on_thermostat_set)(p2p_thermostatinfo_t * info);

    /*
     * 灌溉设备控制
     */
    void (*on_watering_ctrl)(p2p_watering_ctrl_t * wc);

    /*
     * 窗帘状态查询
     * @curtain_status 窗帘状态 0:禁止状态， 1：正在打开，2：正在关闭
     * @screen_status  窗纱状态 0:禁止状态， 1：正在打开，2：正在关闭
     */
    void (*on_curtain_get_state)(int *curtain_status, int *screen_status);

    /*
     * 窗帘控制
     * @op_type 0:停止运动，1：打开，2：关闭
     * @is_curtain 1:窗帘， 2：窗纱
     */
    void (*on_curtain_ctrl)(int op_type, int is_curtain);

    /*
     * 预置点配置
     * @op_type 0: 添加， 1：修改
     * @physical_id 被动设备ID
     * @preset_name 预置点名称
     * @new_name 新的预置点名称(修改的时候才有效，添加时为空)
     * @image_url, 既p2p_upload_pt_preset 返回的 image_url, 回调结束后内部会调用free释放
     * 返回值 0 成功，其他失败
     *
     */
    int  (*on_preset_ctrl)(int op_type, const char* physical_id, const char* preset_name, const char* new_name, char** image_url);

    /*
     * 设置报警推送策略
     * @pc_count p2p_push_condition_t 的个数
     * */
    void (*on_set_push_condition)(p2p_push_condition_t* pc, int pc_count);

}p2p_init_info_t;

/* 
判断设备是否在线
*/
bool p2p_is_online();


struct p2p_report_value_t
{
    int  channel; // 设备相关的通道号， 视频，音频，IO等
    int  type;   // 1：温度 2：湿度 
    char value[128];
};

struct p2p_report_battery_t
{
	char physical_id[32]; // 对应的物理设备ID, 如门磁ID
	int  device_type;	  // 设备类型，门磁是9
	//int  battery_level;	  // 电量级别， 0是满电，1是充足，2是不足 
	float  battery_voltage;	//具体电压值，与battery_level二选一
};

/*
上报实时变化的参数到服务器上
@v, 上报的参数
@num, v的个数
return 0 成功
*/
int p2p_report_value(p2p_report_value_t * v, int num);


/*
上报指定设备的电量到服务器上
@v, 上报的参数
@num, v的个数
return 0 成功
*/
int p2p_report_battery(p2p_report_battery_t * v, int num);
/*
 * 异步上传电池电量，会尝试3次
 */
int p2p_report_battery_a(p2p_report_battery_t * v, int num);


int p2p_upload_sub_device_list(int num, const sub_device_info *sd_list);

/*
	将增加或者修改的预置点信息同步到服务器
	@flag  		// 0: 添加 ，1：修改
	@trigger_id  // 被动设备的ID
	@preset_name  // 预置位的名称: 唯一
	@preset_on 	// 预置位的开关  0: 关  1： 开
	@image_name  // 预置点的抓图	
	@original_name	//修改预置点名称时，提供原始名字
	@image_url	// 返回的抓图url，必须 free !!!
	return p2p_error_code
*/
int p2p_upload_pt_preset(int flag, const char* trigger_id,
	const char* preset_name, int  preset_on, 
	const char* image_name, const char* original_name,
	char** image_url);


int p2p_init(p2p_init_info_t* info, unsigned int info_len);

/*
	返回从服务器获取的utc时间戳，用于校时 
	return -1 当设备没有连接到服务器时，返回失败
*/
int p2p_get_server_timestamp();

/*
 *  返回非 0 表示有人在看视频
 *
 * */
int p2p_if_somebody_watching_video();

/*
 * 上报正在添加的被动设备信息 
 * 返回0表示成功，其他返回值需要重新调用此函数
 * @num 是被动设备的个数    
 */
int p2p_report_adding_devices(adding_device_info *ad, int num);


int p2p_sync_value(const char* name, const char* value);

/*	
	判断是否正在升级
	返回1是，0否
*/
int p2p_if_upgrading();

typedef struct
{
	char server_ip[32];
	int server_port;
	char auth_code[64];
}sleep_server_info;

/*
	获取休眠服务器信息
*/
int p2p_get_sleep_server_info(sleep_server_info *info);

/*
	请求下线，异步调用。通过bool p2p_is_online() 来判断是否成功。
*/
int p2p_request_offline();

/* 
 * 获取时区列表
 * @list 保存列表的空间，个数由num指定
 * @num  输入时为可保存的时区个数，输出为获取到的时区个数
 * return 0 成功， -1 为网络错误，-2为输入的列表个数太小, 1为设备离线
 * */
int p2p_get_timezonelist(p2p_timezonelist_t *list, int *num);

/*
 * 上报温控的状态
 * return 0 成功，1为设备离线
 */
int p2p_report_thermostat_stat(p2p_thermostat_stat_t * st);

/*
 * 上报灌溉口的状态
 * @physical_id 灌溉设备的ID
 * @hole_id, 灌溉口序号， 从0开始
 * return 0 成功，1为设备离线, -1 为网络错误
 * */
int p2p_report_waterholeinfo(const char* physical_id, int hole_id, int watering);

#endif /* end of include guard: P2P_INTERFACE_H */
