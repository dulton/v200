#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <setjmp.h>
#include <errno.h>
#include <stdio.h>

/*两个timeval结构相减*/
struct timeval tvSub(struct timeval timeval1,struct timeval timeval2);
/*校验和算法*/
unsigned short getChksum(unsigned short *addr,int len);
int  packIcmp(int pack_no, struct icmp* icmp);
bool getsockaddr(const char * hostOrIp, struct sockaddr_in* sockaddr) ;
int  unpackIcmp(char *buf,int len/*, struct IcmpEchoReply *icmpEchoReply*/);
int  recvPacket(int m_sockfd ,char *ip);
int  ping(char * host);
