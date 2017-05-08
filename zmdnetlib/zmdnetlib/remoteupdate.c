/**
 * @file remote.c
 *
 * @brief  处理远程升级相关逻辑业务
 *
 * @auth mike
 *
 * @date 2013-12-19
 *
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "remoteupdate.h"
#include "upgrademodule.h"


/** 全局变量声明区*/
extern int errno;
static int g_retry_download_count;		/** 进行断点续传的次数*/
static char g_download_server[128];		/** 下载软件服务器地址*/
static int g_download_port;				/** 下载软件服务器端口*/
static char g_download_page[512];		/** 下载软件对应HTTP 页面*/
UPDATE_STAT_E g_update_stat;			/** 自动更新状态*/
int g_update_file_len;					/** 要下载的升级文件的总长*/
int g_download_file_len;				/** 已下载的升级文件的长度*/
UPGRADECHECKSTRUCT g_UpdateFileHeader;  /** 已下载升级文件的头*/
char g_save_file_name[128];				/** 升级文件存储的文件名*/
DOWNLOAD_CMD_E g_download_cmd;			/** 下载控制指令*/

/** 函数声明区*/
static void* ru_do_start_update( void* arg );

static void ru_set_io_timeout( int socket, int seconds )
{
    struct timeval tv;

    if( seconds > 0 )
    {
        tv.tv_sec  = seconds;
        tv.tv_usec = 0;
        
        (void)setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv) );
        (void)setsockopt( socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv) );
    }
}

static int ru_connect_server( int fd, const struct sockaddr *serv_addr, socklen_t addrlen )
{
    int err = 0, flags = 0;
	
    fd_set rset;
    fd_set wset;
    socklen_t optlen;	
    struct timeval tv;
	
	int timeout = NET_CONNECT_TIME_OUT;
	
    /* make socket non-blocking */
    flags = fcntl( fd, F_GETFL, 0 );
    if( fcntl(fd, F_SETFL, flags|O_NONBLOCK) == RFAILED )
    {
        return RFAILED;
    }

    /* start connect */
    if( connect(fd, serv_addr, addrlen) < 0 )
    {
        if( errno != EINPROGRESS )
        {
            return RFAILED;
        }

        tv.tv_sec = timeout;
        tv.tv_usec = 0;
		
        FD_ZERO( &rset );
        FD_ZERO( &wset );
        FD_SET( fd, &rset );
        FD_SET( fd, &wset );

        /* wait for connect() to finish */
        if( (err = select(fd + 1, &rset, &wset, NULL, &tv)) <= 0 )
        {
            /* errno is already set if err < 0 */
            if (err == 0)
            {
                errno = ETIMEDOUT;
            }
            return RFAILED;
        }

        /* test for success, set errno */
        optlen = sizeof(int);
        if( getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &optlen) < 0 )
        {
            return RFAILED;
        }
        if( err != 0 )
        {
            errno = err;
            return RFAILED;
        }
    }

    /* restore blocking mode */
    if( fcntl(fd, F_SETFL, flags) == RFAILED )
    {
        return RFAILED;
    }	
	
    return ROK;
}

static int ru_open_socket( const char* hostname, int port )
{	
	int fd = -1;
	int error_code = 0;
	struct addrinfo  hints;
	struct addrinfo* res0;
	struct addrinfo* res;
	char port_string[12] = {0};

	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	if( 0 == port )
		port = 80;
	
	sprintf( port_string, "%d", port );

	/** DNS解析升级服务器地址 */
	error_code = getaddrinfo( hostname, port_string, &hints, &res0 );
	if( error_code )
	{
		fprintf( stderr, "getaddrinfo failed[%s,%s]: %s\n", hostname,port_string, gai_strerror(error_code) );
		return RFAILED;
	}

	for( res = res0; res; res = res->ai_next )
	{
		fd = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
		if( fd < 0 )
		{	
			perror("create socket failed");
			continue;
		}
		
		if( ru_connect_server(fd, res->ai_addr, res->ai_addrlen) < 0 )
		{
			close(fd);
			fd = -1;
			continue;
		}
		
		break;
	}

	freeaddrinfo(res0);

	if( fd < 0 )
	{
		return RFAILED;
	}

	/** set recv/send timeout */
	ru_set_io_timeout( fd, NET_CONNECT_TIME_OUT );
	
	return fd;

}

static int ru_send_req_to_server( int fd, char* http_req, int len )
{
	ssize_t ret;
	
	if( (NULL == http_req) || (len <= 0) )
	{
		printf("ru_send_req_to_server: Invalid param\n");
		return RFAILED;
	}

	printf("%s\n", http_req);
	
    if( (ret = send(fd, http_req, len, 0)) < 0 )
    {
        if (errno == EINTR)
        {
            fprintf( stderr, "ru_send_req_to_server: operation aborted!\n" );
        }
        else if (errno == EAGAIN)
        {
            fprintf( stderr, "ru_send_req_to_server: network write error,the operation timed out!\n" );
        }
        else
        {
            fprintf( stderr, "ru_send_req_to_server: network write error: %s\n", strerror(errno) );
        }
		
        return RFAILED;
    }
    else if( (size_t)ret == len )
    {
        return ROK;
    }
    else /* 0 <= error_code < len */
    {
        fprintf( stderr, "ru_send_req_to_server write error!" );
        return RFAILED;
    }
}

static int ru_recv_response_from_server( int fd, char* buff, size_t buffsize )
{
    int recv_len = 0, total_len = 0;	
	
	while( total_len < buffsize )
	{
		recv_len = recv( fd, buff + total_len, buffsize - total_len, 0 );
		if( recv_len < 0 )
		{
        	if( errno == EINTR )
            {
                fprintf( stderr, "ru_recv_response_from_server: operation aborted\n" );
            }
            else if( errno == EAGAIN )
            {
                fprintf( stderr, "ru_recv_response_from_server: network read error,the operation timed out\n" );
            }
            else
            {
                fprintf( stderr, "ru_recv_response_from_server: network read error: %s\n", strerror(errno) );
            }
			
            return total_len;
		}
		else if( 0 == recv_len ) /* socket close */
		{
			printf("ru_recv_response_from_server: socket closed!\n");
			return total_len;
		}
		else
		{
			total_len += recv_len;
		}
	}
	
    return total_len;
}

static int ru_create_check_update_info_req
( 
	char* http_req,
	const char* ubootversion,
	const char* kernelversion,
	const char* fsversion,
	const char* appversion
)
{
	const char* format = "GET /AppUpdateInfo?device_version=%s;%s;%s;%s HTTP/1.1\r\n"
						 "Accept: text/html, application/xhtml+xml, */*\r\n"
						 "Host: %s:%d\r\n"
						 "Connection: close\r\n\r\n";
	
	if( NULL == http_req )
	{
		printf("ru_create_check_update_info_req: Invalid param\n");
		return RFAILED;
	}

	snprintf( http_req, MAX_HTTP_REQ_LEN, format, ubootversion, kernelversion, 
				fsversion, appversion, ZMD_UPDATE_SERVER, RU_HTTP_PORT );

	return ROK;
	
}

static int ru_create_start_update_req( char* http_req, int offset )
{
	const char* format = "GET %s HTTP/1.1\r\n"
						 "Accept: text/html, application/xhtml+xml, */*\r\n"
						 "Accept-Language: zh-CN\r\n"
						 "Host: %s:%d\r\n"
						 "Range: bytes=%d-\r\n"
						 "Connection: Keep-Alive\r\n\r\n";
	
	if( NULL == http_req )
	{
		printf("ru_create_start_update_req: Invalid param\n");
		return RFAILED;
	}

	snprintf( http_req, MAX_HTTP_REQ_LEN, format, g_download_page, g_download_server, g_download_port, offset );

	return ROK;
}

static int ru_save_download_file( const char* filename, char* buff, int len )
{
	int fd, ret;

	fd = open( filename, O_RDWR|O_CREAT, 0666 );
	if( fd < 0 )
	{
		printf( "ru_save_download_file: open file %s faild\r\n", filename );
		return RFAILED;
	}
	
	lseek( fd, 0, SEEK_END );
	
	ret = write( fd, buff, len );
	if( ret != len )
	{
		printf( "ru_save_download_file: write file %s faild\r\n", filename );		
		close( fd );
		return RFAILED;
	}
	
	close( fd );
	return ROK;
	
}

static void ru_rm_download_file( const char* filename )
{
	int fd;

	fd = open( filename, O_RDWR );
	if( fd < 0 )
	{
		printf( "ru_rm_download_file: open file %s faild\r\n", filename );
		return;
	}

	ftruncate( fd, 0 );

	close( fd );
	
}

static int ru_check_download_file( const char* file_name, UPGRADECHECKSTRUCT* header )
{
	FILE* fp = NULL;
	
	fp = fopen( file_name, "rb" );
	if( fp != NULL )
	{
		if( CheckUpdateFileMD5(header, fp) < 0 )
		{
			fclose(fp);
			return RFAILED;
		}
		else
		{
			fclose(fp);
			return ROK;
		}
	}

	printf("ru_check_download_file: open file failed!\n");
	
	return RFAILED;
}

static int ru_resume_download_update_file( int sock_fd, const char* file_name )
{
	char* buff, *pos;
	int resp_len = 0, ret = 0;
	
	buff = (char* )malloc(20*1024);
	memset( buff, 0x00, sizeof(20*1024) );
	
	g_download_cmd = DOWNLOAD_CMD_START;
	
	/** 1. 续传的话要跳过HTTP头*/
	resp_len = ru_recv_response_from_server( sock_fd, buff, 2048);
	if( resp_len < 0 )
	{
		printf( "ru_resume_download_update_file: recv failed\n" );
		goto Failed;
	}

	/** 查找HTTP的BODY,即升级文件*/
	pos = strstr( buff, "\r\n\r\n" ); 
	if( NULL == pos )
	{
		printf( "ru_resume_download_update_file: can not find update file\n" );
		goto Failed;
	}
	
	/* 此时pos指向升级文件的头*/
	pos += strlen("\r\n\r\n"); 

	/** 跳过HTTP头后将后面的写入文件*/
	ret = ru_save_download_file( file_name, pos, 2048-(pos-buff) );
	if( ret < 0 )
	{
		printf( "ru_resume_download_update_file: save file failed!\n" );
		goto Failed;
	}

	g_download_file_len += 2048-(pos-buff);
	
	/** 3. 继续接收文件，同时写文件*/
	while( DOWNLOAD_CMD_START == g_download_cmd )
	{
		memset( buff, 0x00, 20*1024 );
		resp_len = ru_recv_response_from_server( sock_fd, buff, 20*1024 );
		if( 20*1024 != resp_len )
		{
			if( resp_len > 0 )
			{
				g_download_file_len += resp_len;
				ret = ru_save_download_file( file_name, buff, resp_len );
				if( ret < 0 )
				{
					printf( "ru_resume_download_update_file: save file failed!\n" );
					goto Failed;
				}
			}
			
			/** 可能是接收完成，socket断开，也可能是接收时发生错误*/
			printf( "ru_resume_download_update_file: recv %d data\n", g_download_file_len );
			break;
		}
		else
		{
			g_download_file_len += resp_len;
			ret = ru_save_download_file( file_name, buff, resp_len );
			if( ret < 0 )
			{
				printf( "ru_resume_download_update_file: save file failed!\n" );
				goto Failed;
			}
		}
	}


	/** 退出接收可能是收到暂停或者取消信令*/
	if( DOWNLOAD_CMD_CANCEL == g_download_cmd )
	{
		goto Failed;
	}
	else if( DOWNLOAD_CMD_PAUSE == g_download_cmd )
	{
		goto Failed;
	}
	
	/** 有可能是下载过程中无法连接了，需要重新下载*/
	if( g_download_file_len != g_update_file_len )
	{
		printf( "ru_download_update_file: recv file size %d is not same with http %d!\n", 
					g_download_file_len, g_update_file_len );

		if( g_download_file_len > g_update_file_len )
		{
			g_download_file_len = 0;
			ru_rm_download_file( g_save_file_name );
		}
		
		goto Failed;
	}

	/** 4. MD5校验*/
	ret = ru_check_download_file( file_name, &g_UpdateFileHeader );
	if( ret < 0 )
	{
		printf( "ru_resume_download_update_file: check file failed!\n" );
		g_download_file_len = 0;
		ru_rm_download_file( g_save_file_name );
		goto Failed;
	}
	
	free( buff );
	return ROK;

Failed:
	free( buff );
	return RFAILED;
}

/**
 * @brief 进行恢复系统升级的线程
 */
static void* ru_do_resume_update( void* arg )
{
	int sock_fd = -1, res;
	char http_req[MAX_HTTP_REQ_LEN] = {0};		
	pthread_t pid;
	
	/** 1. 设置状态以回应IE的状态查询*/
	g_update_stat = UPDATE_STAT_IN_DOWNLOAD;
	
	/** 2. 连接到升级服务器*/
	sock_fd = ru_open_socket( g_download_server, g_download_port );
	if( sock_fd < 0 )
	{
		printf( "ru_do_resume_update: connect to server failed\n" );
		if( DOWNLOAD_CMD_CANCEL == g_download_cmd )
		{
			g_update_stat = UPDATE_STAT_IDEL;
		}
		else
		{
			//g_update_stat = UPDATE_STAT_ERROR;
		}
		
		goto Failed;
	}
	
	/* 用户已经取消操作*/
	if( (DOWNLOAD_CMD_CANCEL == g_download_cmd) || (DOWNLOAD_CMD_PAUSE == g_download_cmd) )
	{
		goto Failed;
	}

	/** 3. 构造断点续传请求获取升级文件*/
	res = ru_create_start_update_req( http_req, g_download_file_len );
	if( RFAILED == res )
	{
		printf( "ru_do_resume_update: create start update requst failed\n" );
		goto Failed;
	}

	/** 4. 发送开始升级请求到升级服务器*/
	res = ru_send_req_to_server( sock_fd, http_req, strlen(http_req) );
	if( RFAILED == res )
	{
		printf( "ru_do_resume_update: send request to server failed\n" );
		goto Failed;
	}
	
	/* 用户已经取消操作*/
	if( (DOWNLOAD_CMD_CANCEL == g_download_cmd) || (DOWNLOAD_CMD_PAUSE == g_download_cmd) )
	{
		goto Failed;
	}

	/** 5. 继续接收升级文件*/
	res = ru_resume_download_update_file( sock_fd, g_save_file_name );
	if( RFAILED == res )
	{
		printf( "ru_do_start_update: download update file failed\n" );
		goto Failed;
	}

	printf("ru_do_resume_update: Download File Success!\n");

	/** 6. 进行升级*/
	g_update_stat = UPDATE_STAT_IN_UPDATE;
	close( sock_fd ); 
	
	sleep(2); 	/** 使能够及时向IE反馈进度*/
	UpdateToFlash( g_UpdateFileHeader.m_filetype );
	
	return 0;

Failed:
	/** 退出接收可能是收到暂停或者取消信令*/
	if( DOWNLOAD_CMD_CANCEL == g_download_cmd )
	{
		printf("ru_do_resume_update: download has been canceld!\n");
		ru_rm_download_file( g_save_file_name );
		g_update_stat = UPDATE_STAT_IDEL;		
	}
	else if( DOWNLOAD_CMD_PAUSE == g_download_cmd )
	{
		printf("ru_do_resume_update: download has been paused!\n");
		g_update_stat = UPDATE_STAT_PAUSED;
	}
	else
	{
		//ru_rm_download_file( g_save_file_name );
		//g_update_stat = UPDATE_STAT_ERROR;
		/** 断点续传*/
		sleep(2);
		if( 0 == g_download_file_len )
		{
			/** 创建线程负责重新进行远程自动升级*/
			if( pthread_create(&pid, NULL, &ru_do_start_update, NULL) < 0 )
			{
				printf("ru_do_start_update: create pthread for update failed!\n");
				g_update_stat = UPDATE_STAT_ERROR;
			}
		}
		else
		{
			/** 创建线程负责重新进行远程自动升级*/
			if( pthread_create(&pid, NULL, &ru_do_resume_update, NULL) < 0 )
			{
				printf("ru_do_start_update: create pthread for update failed!\n");
				g_update_stat = UPDATE_STAT_ERROR;
			}
		}
	}

	if( -1 != sock_fd)
		close( sock_fd );
	
	return 0;
}


static int ru_download_update_file( int sock_fd, const char* file_name )
{
	char* buff, *pos;
	int resp_len = 0, ret = 0, header_len = 0;
	
	buff = (char* )malloc(20*1024);
	memset( buff, 0x00, sizeof(20*1024) );
		
	/** 1. 首先要获取HTTP头以及升级文件的头*/
	header_len = 2048+sizeof(UPGRADECHECKSTRUCT);
	resp_len = ru_recv_response_from_server( sock_fd, buff, header_len );
	if( header_len != resp_len )
	{
		/** 升级文件连头都收不到，发生了错误*/
		printf( "ru_download_update_file: recv failed\n" );
		goto Failed;
	}
	
	/** 2. 头文件中包含了HTTP信息和升级文件头，及部分升级文件*/
	pos = strstr( buff, "Content-Length:" );
	if( NULL == pos )
	{
		printf( "ru_download_update_file: can not find Content-Length\n" );
		goto Failed;
	}

	g_update_file_len = atoi( pos+strlen("Content-Length:") );
	printf("ru_download_update_file:total file len = %d\n", g_update_file_len );

	/** 查找HTTP的BODY,即升级文件*/
	pos = strstr( buff, "\r\n\r\n" ); 
	if( NULL == pos )
	{
		printf( "ru_download_update_file: can not find update file\n" );
		goto Failed;
	}
	
	/* 此时pos指向升级文件的头*/
	pos += strlen("\r\n\r\n"); 
	memcpy( &g_UpdateFileHeader, pos, sizeof(UPGRADECHECKSTRUCT));
	
	if( CheckUpdateVersion(&g_UpdateFileHeader) < 0 )
	{
		/** 文件头校验失败*/
		printf( "ru_download_update_file: file version check failed!\n" );
		goto Failed;
	}

	g_download_file_len = header_len-(pos-buff);

	/** 将已接收到的2K文件中的出去HTTP和文件头为内容写入文件*/
	pos += sizeof(UPGRADECHECKSTRUCT);
	ret = ru_save_download_file( file_name, pos, header_len-(pos-buff) );
	if( ret < 0 )
	{
		printf( "ru_download_update_file: save file failed!\n" );
		goto Failed;
	}
	
	/** 3. 继续接收文件，同时写文件*/
	while( DOWNLOAD_CMD_START == g_download_cmd )
	{
		memset( buff, 0x00, 20*1024 );
		resp_len = ru_recv_response_from_server( sock_fd, buff, 20*1024 );
		if( 20*1024 != resp_len )
		{
			if( resp_len > 0 )
			{
				g_download_file_len += resp_len;
				ret = ru_save_download_file( file_name, buff, resp_len );
				if( ret < 0 )
				{
					printf( "ru_download_update_file: save file failed!\n" );
					goto Failed;
				}
			}
			
			/** 可能是接收完成，socket断开，也可能是接收时发生错误*/
			printf( "ru_download_update_file: recv %d data\n", g_download_file_len );
			break;
		}
		else
		{
			g_download_file_len += resp_len;
			ret = ru_save_download_file( file_name, buff, resp_len );
			if( ret < 0 )
			{
				printf( "ru_download_update_file: save file failed!\n" );
				goto Failed;
			}
		}
		
	}


	/** 退出接收可能是收到暂停或者取消信令*/
	if( DOWNLOAD_CMD_CANCEL == g_download_cmd )
	{
		goto Failed;
	}
	else if( DOWNLOAD_CMD_PAUSE == g_download_cmd )
	{
		goto Failed;
	}
	
	/** 有可能是下载过程中无法连接了，需要重新下载*/
	if( g_download_file_len != g_update_file_len )
	{
		printf( "ru_download_update_file: recv file size %d is not same with http %d!\n", 
					g_download_file_len, g_update_file_len );
		
		goto Failed;
	}

	/** 4. MD5校验*/
	ret = ru_check_download_file( file_name, &g_UpdateFileHeader );
	if( ret < 0 )
	{
		printf( "ru_download_update_file: check file failed!\n" );
		g_download_file_len = 0;
		ru_rm_download_file( g_save_file_name );
		goto Failed;
	}
	
	free( buff );
	return ROK;

Failed:
	free( buff );
	return RFAILED;
}

static int ru_parse_check_update_response
(
	int sock_fd,
	char* version,
	int* updateflag, 
	char* description, 
	int* des_len
)
{
	int resp_len = 0, ret = 0;
	char* buff, *pos, *end;
	char download_addr[1024] = {0};
	char download_url[128] = {0};
	
	buff = (char* )malloc(20*1024);
	memset( buff, 0x00, sizeof(20*1024) );

	/** 1. 接收检测版本更新的响应*/
	resp_len = ru_recv_response_from_server( sock_fd, buff, 20*1024 - 1 );
	if( resp_len <= 0 )
	{
		printf( "ru_parse_check_update_response: recv response failed!\n" );
		goto Failed;
 	}

	printf( "%s\n", buff );
	
	/** 2. 解析UpdateFlag，如果确定有版本更新再继续解析其他字段*/
	if( 200 != atoi(buff+strlen("HTTP/1.1")) )
	{
		printf( "ru_parse_check_update_response: get error response code %d!\n",  atoi(buff+strlen("HTTP/1.1")) );
		goto Failed;
	}
	
	pos = strstr( buff, "<UpdateFlag>" );
	if( NULL == pos)
	{
		printf( "ru_parse_check_update_response: parse UpdateFlag failed!\n" );
		goto Failed;
	}

	*updateflag = atoi( pos + strlen("<UpdateFlag>") );
	if( 0 == *updateflag )
	{
		printf("ru_parse_check_update_response: current version is newest!\n");
		free( buff );
		return ROK;
	}

	/* 3. 解析新版本信息AppVersion 字段*/
	pos = strstr( buff, "<AppVersion>" );
	if( NULL == pos)
	{
		printf( "ru_parse_check_update_response: parse AppVersion start failed!\n" );
		goto Failed;
	}

	sscanf( pos + strlen("<AppVersion>"), "%[^<]", version );


	/* 3. 解析描述信息Description */
	pos = strstr( buff, "<Description>" );
	if( NULL == pos)
	{
		printf( "ru_parse_check_update_response: parse Description start failed!\n" );
		goto Failed;
	}

	end = strstr( buff, "</Description>" );
	if( NULL == end )
	{
		printf( "ru_parse_check_update_response: parse Description end failed!\n" );
		goto Failed;
	}

	pos += strlen("<Description>");
	*des_len = end - pos;
	strncpy( description, pos, *des_len );

	/* 4. 解析下载新版本的地址*/
	pos = strstr( buff, "<DownloadAddress>" );
	if( NULL == pos)
	{
		printf( "ru_parse_check_update_response: parse DownloadAddress failed!\n" );
		goto Failed;
	}

	sscanf( pos + strlen("<DownloadAddress>"), "%[^<]", download_addr );

	/** 从下载URL中解析出主机地址和页面 */
	ret = sscanf( download_addr+strlen("http://"), "%[^/]%s", download_url, g_download_page );
	if( 2 != ret )
	{
		printf( "ru_parse_check_update_response: DownloadAddress url %s format is wrong!\n", download_addr );
		goto Failed;
	}

	/** 返回的地址可能使用的指定的端口*/
	if( strchr(download_url, ':') )
	{
		sscanf( download_url, "%[^:]:%d", g_download_server, &g_download_port );
	}
	else
	{
		strcpy( g_download_server, download_url );
		g_download_port = RU_HTTP_PORT;
	}
	
	printf("ru_parse_check_update_response:\n");
	printf("version= %s\n", version);
	printf("updateflag= %d\n", *updateflag);
	printf("description= %s\n", description);
	printf("des_len= %d\n", *des_len);
	printf("DownloadAddress= %s\n", download_addr);
	printf("server = %s\n", g_download_server);
	printf("port = %d\n", g_download_port);
	
	free( buff );
	return ROK;

Failed:
	free( buff );
	return RFAILED;
}

/**
 * @brief 进行系统升级的线程
 */
static void* ru_do_start_update( void* arg )
{
	int sock_fd = -1, res;
	char http_req[MAX_HTTP_REQ_LEN] = {0};		
	pthread_t pid;
	
	/** 1. 设置状态以回应IE的状态查询*/
	g_update_stat = UPDATE_STAT_IN_DOWNLOAD;
	ru_rm_download_file( g_save_file_name );
	
	/** 2. 连接到升级服务器*/
	sock_fd = ru_open_socket( g_download_server, g_download_port );
	if( sock_fd < 0 )
	{
		printf( "ru_do_start_update: connect to server failed\n" );
		
		if( DOWNLOAD_CMD_CANCEL == g_download_cmd )
		{
			g_update_stat = UPDATE_STAT_IDEL;
		}
		else
		{
			//g_update_stat = UPDATE_STAT_ERROR;
		}

		goto Failed;
	}

	/* 用户已经取消操作*/
	if( (DOWNLOAD_CMD_CANCEL == g_download_cmd) || (DOWNLOAD_CMD_PAUSE == g_download_cmd) )
	{
		goto Failed;
	}

	/** 3. 构造获取新版本信息HTTP请求*/
	res = ru_create_start_update_req( http_req, 0 );
	if( RFAILED == res )
	{
		printf( "ru_do_start_update: create start update requst failed\n" );
		goto Failed;
	}

	/** 4. 发送开始升级请求到升级服务器*/
	res = ru_send_req_to_server( sock_fd, http_req, strlen(http_req) );
	if( RFAILED == res )
	{
		printf( "ru_do_start_update: send request to server failed\n" );
		goto Failed;
	}
	
	/* 用户已经取消操作*/
	if( (DOWNLOAD_CMD_CANCEL == g_download_cmd) || (DOWNLOAD_CMD_PAUSE == g_download_cmd) )
	{
		goto Failed;
	}

	/** 5. 接收升级文件*/
	res = ru_download_update_file( sock_fd, g_save_file_name );
	if( RFAILED == res )
	{
		printf( "ru_do_start_update: download update file failed\n" );
		goto Failed;
	}

	printf("ru_do_start_update: Download File Success!\n");

	/** 6. 进行升级*/
	g_update_stat = UPDATE_STAT_IN_UPDATE;
	close( sock_fd ); 
	sleep(2); 	/** 使能够及时向IE反馈进度*/
	UpdateToFlash( g_UpdateFileHeader.m_filetype );
	
	return 0;

Failed:
	/** 退出接收可能是收到暂停或者取消信令*/
	if( DOWNLOAD_CMD_CANCEL == g_download_cmd )
	{
		printf("ru_do_start_update: download has been canceld!\n");
		ru_rm_download_file( g_save_file_name );
		g_update_stat = UPDATE_STAT_IDEL;		
	}
	else if( DOWNLOAD_CMD_PAUSE == g_download_cmd )
	{
		printf("ru_do_start_update: download has been paused!\n");
		g_update_stat = UPDATE_STAT_PAUSED;
	}
	else
	{
		//ru_rm_download_file( g_save_file_name );
		//g_update_stat = UPDATE_STAT_ERROR;
		/** 断点续传*/
		sleep(2);
		if( 0 == g_download_file_len )
		{
			/** 创建线程负责重新进行远程自动升级*/
			if( pthread_create(&pid, NULL, &ru_do_start_update, NULL) < 0 )
			{
				printf("ru_do_start_update: create pthread for update failed!\n");
				g_update_stat = UPDATE_STAT_ERROR;
			}
		}
		else
		{
			/** 创建线程负责重新进行远程自动升级*/
			if( pthread_create(&pid, NULL, &ru_do_resume_update, NULL) < 0 )
			{
				printf("ru_do_start_update: create pthread for update failed!\n");
				g_update_stat = UPDATE_STAT_ERROR;
			}
		}
	}

	if( -1 != sock_fd)
		close( sock_fd );
	
	return 0;
}

/**
 * @brief 向远程升级服务器获取软件版本更新信息
 *
 * @param version 	返回的新版本号
 * @param updateflag 	是否需要更新，1:是，0:否
 * @param description 	新版本描述信息，仅当updateflag为1时有意义
 * @param des_len 	描述信息的长度
 * @param ubootversion 	当前系统UBOOT版本
 * @param kernelversion 	当前系统内核版本
 * @param fsversion 	当前系统文件系统版本
 * @param appversion 	当前系统APP版本
 *
 * @return 0:检测更新信息成功，-1:检测更新信息失败
 */
int ru_check_update_info
( 
	char* version,
	int* updateflag, 
	char* description, 
	int* des_len,
	const char* ubootversion,
	const char* kernelversion,
	const char* fsversion,
	const char* appversion
)
{
	int sock_fd, res;
	char http_req[MAX_HTTP_REQ_LEN] = {0};
	
	if( (NULL == version) || (NULL == updateflag) || 
		(NULL == description) ||(NULL == des_len) )
	{
		printf( "ru_check_update_info: invalied argment\n" );
		return RFAILED;
	}

	/** 1. 连接到升级服务器*/
	sock_fd = ru_open_socket( ZMD_UPDATE_SERVER, RU_HTTP_PORT );
	if( sock_fd < 0 )
	{
		printf( "ru_check_update_info: connect to server failed\n" );
		return RFAILED;
	}

	/** 2. 构造获取新版本信息HTTP请求*/
	res = ru_create_check_update_info_req( http_req, ubootversion, kernelversion, fsversion, appversion );
	if( RFAILED == res )
	{
		printf( "ru_check_update_info: create check update info requst failed\n" );
		close( sock_fd );
		return RFAILED;
	}

	/** 3. 发送获取新版本信息请求到升级服务器*/
	res = ru_send_req_to_server( sock_fd, http_req, strlen(http_req) );
	if( RFAILED == res )
	{
		printf( "ru_check_update_info: send request to server failed\n" );		
		close( sock_fd );
		return RFAILED;
	}

	/** 4. 接收升级服务器的响应信息并解析*/
	res = ru_parse_check_update_response( sock_fd, version, updateflag, description, des_len );
	if( RFAILED == res )
	{
		printf( "ru_check_update_info: parse response failed\n" );		
		close( sock_fd );
		return RFAILED;
	}
	
	close( sock_fd );
	return ROK;
}

/**
 * @brief 开始系统升级
 */
void ru_start_update( const char* savefile )
{
	pthread_t pid;

	/* 清除可能的前一次升级过程信息*/
	g_update_file_len = 0;
	g_download_file_len = 0;
	g_retry_download_count = 0;
	g_update_stat = UPDATE_STAT_IDEL;
	g_download_cmd = DOWNLOAD_CMD_START;
	memset( &g_UpdateFileHeader, 0x00, sizeof(UPGRADECHECKSTRUCT) );
	
	strcpy( g_save_file_name, savefile );

	/** 创建线程负责远程自动升级*/
	if( pthread_create(&pid, NULL, &ru_do_start_update, NULL) < 0 )
	{
		printf("ru_start_update: create pthread for update failed!\n");
		g_update_stat = UPDATE_STAT_ERROR;
	}

	return;
}

/**
 * @brief 获取升级状态
 *
 * @param update_stat 升级状态
 * @param process 下载进度
 */
void ru_get_update_stat( int* update_stat, int* process )
{
	if( (NULL == update_stat) || (NULL == process) )
		return;
	
	*update_stat = g_update_stat;

	if( g_update_file_len != 0 )
		*process = (g_download_file_len*100)/g_update_file_len;
	else
		*process = 0;
	
	return;
}

/**
 * @brief 取消升级
 *
 * @return 0:取消成功，-1:无法取消
 */
int ru_cancel_update()
{
	/** 只有处于下载状态才能取消*/
	if( UPDATE_STAT_IN_UPDATE != g_update_stat )
	{
		g_download_cmd = DOWNLOAD_CMD_CANCEL;
		return ROK;
	}
	else
	{
		return RFAILED;
	}
}

/**
 * @brief 暂停升级
 *
 * @return 0:暂停成功，-1:无法暂停
 */
int ru_pause_update()
{
	/** 只有处于下载状态才能暂停*/
	if( UPDATE_STAT_IN_DOWNLOAD == g_update_stat )
	{
		g_download_cmd = DOWNLOAD_CMD_PAUSE;
		return ROK;
	}
	else
	{
		return RFAILED;
	}
}

/**
 * @brief 恢复升级
 *
 * @return 0:恢复成功，-1:无法恢复
 */
int ru_resume_update()
{
	pthread_t pid;
	
	/** 只有处于下载状态才能暂停*/
	if( UPDATE_STAT_PAUSED == g_update_stat )
	{
		printf("ru_resume_update: begin to resume download, has recv file %d\n", g_download_file_len );
		
		g_download_cmd = DOWNLOAD_CMD_START;

		/* 一点文件都没收到，相当于重新来*/
		if( 0 == g_download_file_len )
		{
			/** 创建线程负责重新进行远程自动升级*/
			if( pthread_create(&pid, NULL, &ru_do_start_update, NULL) < 0 )
			{
				printf("ru_resume_update: create pthread for update failed!\n");
				g_update_stat = UPDATE_STAT_ERROR;
			}
		}
		else
		{
			/** 创建线程负责重新进行远程自动升级*/
			if( pthread_create(&pid, NULL, &ru_do_resume_update, NULL) < 0 )
			{
				printf("ru_resume_update: create pthread for update failed!\n");
				g_update_stat = UPDATE_STAT_ERROR;
			}
		}
		
		return ROK;
	}
	else
	{
		return RFAILED;
	}
}

