#ifndef P2P_ALARM_H_38J7D8AKDFA987ADJAIOQYFJBA
#define P2P_ALARM_H_38J7D8AKDFA987ADJAIOQYFJBA

#include <time.h>
#include <stdint.h>

enum P2pAlarmType
{
	P2P_ALARM_MD = 0,
	P2P_ALARM_IO = 4,
	P2P_ALARM_VIDEOLOST = 5,
	P2P_ALARM_SD_EXCEPTION = 6,
	P2P_ALARM_ANSWER = 7,
	P2P_ALARM_AUDIO_EXCEPTION = 8,
	P2P_ALARM_BABY_CRY = 9,
	P2P_ALARM_OLD_BELL_RING = 11,
	P2P_ALARM_DOORMEGNET_OPEN = 12,
	P2P_ALARM_DOORMEGNET_CLOSE = 13,
	P2P_ALARM_MD_CLOUD_RECORD = 14,
	P2P_ALARM_BELL_RING = 15,
    P2P_ALARM_MICROWAVE = 16,
    P2P_ALARM_SMOG = 17,    /*烟雾报警*/
    P2P_ALARM_COMBUSTIBLE_GAS = 18, /*可燃气体报警*/
    P2P_ALARM_EMERGENCY_BUTTON = 19, /*紧急按钮触发*/
    P2P_ALARM_HOME_MODE = 20, /* 在家模式 */   
    P2P_ALARM_OUT_MODE = 21,  /* 离家模式 */
    P2P_ALARM_SLEEP_MODE = 22, /* 睡眠模式 */
    P2P_ALARM_CUSTOM_MODE = 23, /* 自定义模式 */
    P2P_ALARM_BUZZER_TRIGGER = 24, /*报警器触发*/
    P2P_ALARM_BUZZER_HALT = 25, /*报警器停止*/
    P2P_ALARM_PIR = 26, /*PIR报警*/
    P2P_ALARM_REMOTE_CONTROL = 27, /*遥控器报警*/

    P2P_ALARM_NEW_MD = 99,		// malloc alarm_info: check_type/position/delta/need_capture_pic_count
    P2P_ALARM_NEW_MD_CLOUD = 100, // malloc alarm_info: check_type/position/delta
};

class AlarmPoster
{
 public:
	//channel 是视频或者IO通道号
	static AlarmPoster* CreateAlarm(P2pAlarmType type, time_t happen_time = time(NULL), int channel = 0);
	
	//如果是子设备(被动设备)触发的报警，请附上其ID
	static void AddSubDeviceID(AlarmPoster *poster, const char* id);
	
	//channel 是视频通道号
	static void AddPicture(AlarmPoster *poster, const char* file_path, int channel = 0, int need_delete = 1);

	static void AddRecord(AlarmPoster *poster, const char* file_path, uint32_t file_size);

	static void AddString(AlarmPoster *poster, const char* name, const char* value);

	//异步模式上传报警，调用之后， 内部自动销毁poster
	static void PostAlarm(AlarmPoster *poster);
	
	//同步模式上传报警，会阻塞, 函数返回时内部自动销毁poster
	static int  SendAlarm(AlarmPoster *poster);
};

#endif
