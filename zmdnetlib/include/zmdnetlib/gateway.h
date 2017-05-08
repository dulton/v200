#ifndef _GATEWAY_H_
#define _GATEWAY_H_


typedef enum
{
	NET_CARD_LOCAL, 	/*有线网卡*/
	NET_CARD_WIFI,		/*无线网卡*/
}NET_CARD;


/**
 * @brief 删除多余网关
 * @netcard  0:有线网卡  1:无线网卡
 * @sznetcard 有线网口名称
 * @wifi_name 无线网口名称
 * @pGw 添加的网关
 */

int delnet_gateway(NET_CARD netcard, char *sznetcard, char* wifi_name, char* pGw);

#endif

