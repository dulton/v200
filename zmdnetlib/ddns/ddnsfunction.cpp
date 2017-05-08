#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include "net_base64.h"

int dyndnshasstart = 0;

int ip88(char *name, char *passwd)
{
    int sockfd;
    struct servent *sent;
    struct sockaddr_in ser_addr;
    struct hostent *sh;
    struct in_addr **addrs;
    char HTTPCMD[1024];
    char DDNS_XML[512];
    char tmpbuf[512];
    char buf2[1024];
    char *p;
    int ret;
    int sread = -1;

    sent = getservbyname("http", "tcp");
    if(!sent)
    {
	    printf("Get info error!\n");
        return 0;
    }
    if(!(sh = gethostbyname("link.dipserver.com")))
    {
        printf("Can't resolve host.\n");
        return 0;
    }
    addrs = (struct in_addr **)sh->h_addr_list;

    //Set Post Data (XML)
    memset(DDNS_XML, 0, 512);
    p = DDNS_XML;
    strcat(p, "<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
    strcat(p, "<ELinkPacket>\r\n");
    strcat(p, "    <MsgType>ActiveTestReq</MsgType>\r\n");
    strcat(p, "    <Version>1.0</Version>\r\n");
    memset(tmpbuf, 0, 512);
    sprintf(tmpbuf, "        <UserName>%s</UserName>\r\n", name);
    //sprintf(tmpbuf,"        <UserName>%s</UserName>\r\n", "thesues");
    strcat(p, tmpbuf);
    memset(tmpbuf, 0, 512);
    sprintf(tmpbuf, "        <UserPwd>%s</UserPwd>\r\n", passwd);
    //sprintf(tmpbuf,"        <UserPwd>%s</UserPwd>\r\n", "19780214");
    strcat(p, tmpbuf);
    strcat(p, "</ELinkPacket>\r\n");
    // Set value for HTTP Protocol
    memset(HTTPCMD, 0, 1024);
    strcat(HTTPCMD, "POST /elink/elink.dll/ HTTP/1.1\r\n");
    strcat(HTTPCMD, "Host: link.dipserver.com\r\n");
    strcat(HTTPCMD, "Content-type: text/html;charset=gb2312\r\n");
    memset(tmpbuf, 0, 512);
    sprintf(tmpbuf, "Content-length: %d\r\n", strlen(DDNS_XML));
    strcat(HTTPCMD, tmpbuf);
    strcat(HTTPCMD, "Connection: Close\r\n");
    strcat(HTTPCMD, "\r\n");
    strcat(HTTPCMD, DDNS_XML);
    // Post Data END

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if( sockfd < 0 ) 							
    {
        return 0 ;
    }											

    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port =  sent->s_port;
    memcpy(&(ser_addr.sin_addr), *addrs, sizeof(struct in_addr));
    bzero(&(ser_addr.sin_zero), 8);
    if(connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Connect error\n");
        close( sockfd ) ;						
        return 0;
    }
    if(send(sockfd, HTTPCMD, strlen(HTTPCMD), 0) == -1)
    {
        printf("send data error\n");
        close( sockfd ) ;						
        return 0;
    }
    // receive
    bzero(buf2, sizeof(buf2));
    if ((sread = recv(sockfd, buf2, sizeof(buf2), MSG_WAITALL)) == -1)
    {
        printf("error recv\n");
        close( sockfd ) ;					
        return 0;
    }
    close(sockfd);

    p = strstr(buf2, "<Result>");
    if(p == NULL)
    {
        close( sockfd ) ;					
        return 0;
    }
    if(sscanf(p, "<Result>%d</Result>", &ret) != 1)
    {
        close( sockfd ) ;					
        return 0;
    }
    p = strstr(buf2, "<UserIP>");
    if(p == NULL)
    {
        close( sockfd ) ;					
        return 0;
    }
    if(sscanf(p, "<UserIP>%15[0-9.x]</UserIP>", &tmpbuf ) != 1)
    {
        close( sockfd ) ;					
        return 0;
    }
   // printf("Return String:(IP) %s\n",tmpbuf);
	close( sockfd ) ;
    return (0);
}

int noip(char *hostname, char *username, char *passwd)
{
    int sockfd = -1;
    int len;
    struct sockaddr_in address;
    int result;
    char urlbuf[1024] = "";
    char *encbuf = NULL;
    char namebuf[64] = "";
    const char *servername = "dynupdate.no-ip.com";
    //const char *hostname = "www.baidu.com";
    struct hostent *host;
    host = gethostbyname(servername);
    if (host == NULL)
    {
        perror("cannot get host by servername");
        return(EXIT_FAILURE);
    }
    const char *hostip = inet_ntoa(*((struct in_addr *)host->h_addr));
    printf("host ip = %s \n", hostip);

    sprintf(namebuf, "%s:%s", username, passwd);
    encbuf = net_base64_encode((char *)namebuf);
    //sprintf(urlbuf,"GET /dyndns/update?system=dyndns&hostname=%s&myip=%s HTTP/1.0\r\nHost: www.3322.org\r\nAuthorization: Basic %s\r\n\r\n",hostname,"",encbuf);

    sprintf(urlbuf, "GET /nic/update?hostname=%s&myip=%s HTTP/1.0\r\nHost: dynupdate.no-ip.com\r\nAuthorization: Basic %s\r\n\r\n", hostname, "", encbuf);

    if( encbuf )									
        free( encbuf );										
    char ch;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(hostip);
    address.sin_port = htons(8245);
    len = sizeof(address);
    result = connect(sockfd,  (struct sockaddr *)&address, len);
    //printf("result = %d\n", result);
    if(result == -1)
    {
        perror("oops: client1");
        close( sockfd ) ;										
        return -1;
    }

    write(sockfd, urlbuf, strlen(urlbuf));
    //printf("write length = %d\n", length);
    read(sockfd, &ch, 1);
    //printf("len_read = %d\n", len_read);
    while(read(sockfd, &ch, 1))
    {
        printf("%c", ch);
    }
    printf( "\r\n" ) ;
    close(sockfd);

    return 0;
}

int zmododns(char *hostname, char *username, char *passwd)
{
    int sockfd = -1;
    int len;
    struct sockaddr_in address;
    int result;
    char urlbuf[1024] = "";
    struct hostent *host;
    const char *servername = "zmododns.com";

    host = gethostbyname(servername);
    if (host == NULL)
    {
        perror("cannot get host by servername");
        return(EXIT_FAILURE);
    }
    const char *hostip = inet_ntoa(*((struct in_addr *)host->h_addr));
    printf("host ip = %s \n", hostip);

    sprintf(urlbuf, "GET http://zmododns.com/updateip.php?username=%s&password=%s&hostname=%s&myip=%s HTTP/1.0\r\n\r\n", username, passwd, hostname, "");//should two \r\n
    //sprintf(urlbuf, "GET http://zmododns.com/updateip.php?username=%s&password=%s&hostname=%s&myip=%s HTTP/1.0\r\n\r\n", username, passwd, hostname, "");
    char ch;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(hostip);
    address.sin_port = htons(80);
    len = sizeof(address);
    result = connect(sockfd,  (struct sockaddr *)&address, len);
    if(result == -1)
    {
        perror("oops: client1");
        close( sockfd ) ;							
        return -1;
    }

    len = write(sockfd, urlbuf, strlen(urlbuf));
    printf("write length = %d\n", len);
    read(sockfd, &ch, 1);
    while(read(sockfd, &ch, 1))
    {
        printf("%c", ch);
    }
    printf( "\r\n" ) ;
    close(sockfd);

    return 0;
}

int ddns(int server, char *hostname, char *username, char *passwd)
{
    int soket = -1;
    char urlbuf[1024] = "";
    char *encbuf = NULL;
    char namebuf[64] = "";
    struct hostent *hos = NULL;
    char *sIP = NULL;
    struct sockaddr_in addrServ;
	//printf("server:%d hostname:%s username:%s passwd:%s\n",server,hostname,username,passwd);

    if(hostname == NULL || username == NULL)
    {
        printf("hostname or username is NULL\n");
        return 0;
    }
    if(server == 1)
    {
        sprintf(namebuf, "%s:%s", username, passwd);
        encbuf = net_base64_encode((char *)namebuf);
        if(NULL == encbuf )
        {
            return -1;
        }												
        sprintf(urlbuf, "GET /dyndns/update?system=dyndns&hostname=%s&myip=%s HTTP/1.0\r\nHost: www.3322.org\r\nAuthorization: Basic %s\r\n\r\n", hostname, "", encbuf);
        free( encbuf ) ;

    }
    else if(server == 2)
    {
        if(dyndnshasstart == 0)
        {
            printf("dyndns start\n");
            dyndnshasstart = 1;
            memset(urlbuf, 0x0, sizeof(urlbuf));
            sprintf(urlbuf, "/sbin/dyndns -u %s -p %s -a %s&", username, passwd, hostname);
            system(urlbuf);
            printf("dyndns start complete\n");
        }
        else
            printf("dyddns has start\n");
        return 0;
    }
    else if(server == 3)
    {
        ip88(username, passwd);
        return 0;

    }
    else if(server == 4)
    {
        noip(hostname, username, passwd);
        return 0;
    }
    else if(server == 5)
    {
        zmododns(hostname, username, passwd);
        return 0;
    }
    soket = socket(AF_INET, SOCK_STREAM, 0);
    if(soket == -1)
    {
        printf("Create socket fail!\n");
    }
    if(server == 1)
    {
        hos   = gethostbyname("www.3322.org");
    }
    if (hos != NULL)
    {
        sIP = inet_ntoa(*(struct in_addr *)hos->h_addr_list[0]);
        printf("[DDNS]:get ipaddr = %s\n", sIP);
    }
    else
    {
        close( soket ) ;								
        printf("[DDNS]:Can't get host  IP\r\n");
        return 0;
    }

    addrServ.sin_family = AF_INET;
    addrServ.sin_port = htons(80);
    addrServ.sin_addr.s_addr = inet_addr(sIP);
    if(0 > connect(soket, (struct sockaddr *)&addrServ, sizeof(addrServ)))
    {
        printf("[DDNS] : can't connet serverI\r\n");
        addrServ.sin_port = htons(8080);
        if(0 > connect(soket, (struct sockaddr *)&addrServ, sizeof(addrServ)))
        {
            printf("[DDNS] : can't connet serverII\r\n");
        }
    }
    send(soket, urlbuf, strlen(urlbuf), 0);
    memset(urlbuf, 0, sizeof(urlbuf));
    recv(soket, urlbuf, 64, 0);
    printf("return urlbuf:%s\n", urlbuf);
    close(soket);
    return 0;
}
