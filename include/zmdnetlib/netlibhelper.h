

#ifndef _JIANGHM_NETLIB_HELPER_FUNCTION_HEADER_3249874328432
#define _JIANGHM_NETLIB_HELPER_FUNCTION_HEADER_3249874328432

#ifndef _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <unistd.h>

/*
* ftpgetput
*
* Mini implementation of FTP to retrieve a remote file.
*
* Copyright (C) 2010kxzhong, <zkx_2000_1982@163.com>
*
* Based on wget.c by Chip Rosenthal Covad Communications
*
*/
int		ftpupload(char *username, char *passwd, char *serverip, char *targetpath, char *sourcepath);

/*RFC 2045
* 0---25	A---Z
* 26---51	a---z
* 52---61	0---9
* 62		+
* 63		/
* (pad)	=
*/

/* This function encrypt src to dest by Base64 */
//int encrypt_b64(char* dest,char* src,size_t size);

int		encrypt_b64_file_to_buf (char *en64_buf, const char *s_file, int len);
int		encrypt_b64(char *dest, char *src, int size);

void base_encode64(char *outbuf, char *string, int outlen);

/*输入任意一个字符串，经过md5算法处理后，返回结果：一个定长（32个字符）
字符串 */
char	*MDString (char *);

/*输入任意一个文件名，文件内容经过md5算法处理后，返回结果：一个定长
（32个字符）字符串 */
char	*MDFile (char *);

/*输入任意一个字符串text,和一个用做密钥的字符串key,经过hmac_md5算法处
理，返回处理结果：一个定长字符串（32个字符）*/
char	*hmac_md5(char *text, char *key);

extern "C" int sys_reboot();

#define DHCP_IP_PRIFIX		"Lease of"
#define DHCP_IP_SUFFIX		"obtained"
#define DHCP_DNS_PRIFIX	"adding dns"
#define PPPOE_IP_PRIFIX		"remote IP address"
int		SetPppoeConfigFile(unsigned char eth_id, char *username, char *password);
int		PppoeConnect(int pppoeswitch);
void	DhcpConfig(int flag);


#endif

#endif

