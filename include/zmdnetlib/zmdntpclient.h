#ifndef ZMD_NTPCLIENT_H
#define ZMD_NTPCLIENT_H
#ifdef __cplusplus
extern "C"
{
#endif
void ntpclient_set_callback(void(*fun)(time_t));
/**************************************************************************
functional: start a ntp client to adjust local time with ntp server
@timezone: time zone,in seconds base UTC, [-12*3600, 13*3600]  ex: UTC+8:00 is 8*3600, UTC-5:30 is -5.5*3600
@interval: sync interval in seconds, 0 means sync once and stop automatically
@ntp_server: ntp server, if NULL, use "pool.ntp.org"
@auto_flag:auto timezone,1 means open
return: 0 on success,
       -1 means ntpclient already running,
       -2 means a previous requestion of stop is processing, please wait
**************************************************************************/
int ntpclient_start(int timezone, int interval, const char* ntp_server, int auto_flag);

/*************************************************************************
functionality: get timezone(country) name and timezone
@tzname: country name
@tzname_len: buffer length of tzname
@timezone: timezone, seconds base UTC, [-12*3600, 13*3600]  ex: UTC+8:00 is 8*3600, UTC-5:30 is -5.5*3600
return: 0 on success
************************************************************************/
int ntpclient_gettimezone(char* tzname, int tzname_len,  int* timezone);

/***************************************************************************
functional: stop ntpclient
this function use pthread_kill to send SIGUSR1 to stop ntpclient thread
***************************************************************************/
void ntpclient_stop();

/***************************************************************************
funcional: check if ntpclient is running
return : 1 if running, 0 if stop
***************************************************************************/
int ntpclient_is_running();

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: ZMD_NTPCLIENT_H */
