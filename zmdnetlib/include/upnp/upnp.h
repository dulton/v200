
#ifndef _JIANGHM_UPNP_HEADER_329874328974329873420432
#define _JIANGHM_UPNP_HEADER_329874328974329873420432

int upnp(int v_port,
			int http_port,
			int plat_alarm_port,
			unsigned short m_uHttpListenPt,
			unsigned short m_uVideoListenPt,
			unsigned short m_uPhoneListenPt,
			unsigned char	 ddnsFlag, 
			unsigned short o_uHttpListenPt, 
			unsigned short o_uVideoListenPt, 
			unsigned short o_uPhoneListenPt);
			
bool IsChangeExtIP( );
const char* GetInternetIp();
#endif



