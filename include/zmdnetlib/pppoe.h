#ifndef PPPOE_H_
#define PPPOE_H_
#include<stdio.h>
#include <stdlib.h>
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
#define DHCP_IP_PRIFIX		"Lease of"
#define DHCP_IP_SUFFIX		"obtained"
#define DHCP_DNS_PRIFIX	"adding dns"
#define PPPOE_IP_PRIFIX		"remote IP address"
int SetPppoeConfigFile(unsigned char eth_id, char *username, char *password);
int PppoeConnect(int pppoeswitch);
void DhcpConfig(int flag);
#endif

