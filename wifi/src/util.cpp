#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>
#include<string.h>

void GerenalRandomMAC(unsigned char *MAC)
{
#define RANDOM(x) (rand()%x)
#define FMT_MAC_ADDR_LEN 17
	unsigned char MACAddr[FMT_MAC_ADDR_LEN]={'0','4',':','5','C',':','0','6',':',
	'0','0',':','0','0',':','0','0'};
	unsigned char HEXCHAR[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C',
'D','E','F'};
	
   unsigned short i=8;
   unsigned short n=0;
  
   if(NULL == MAC )
		return ;
   for(;i<FMT_MAC_ADDR_LEN;i++)
   {
       n=RANDOM(16);
       if(MACAddr[i]!=':')
		{
		  MACAddr[i]=HEXCHAR[n];
		} 
		else
		{
		  MACAddr[i]=':';
		}
    }
	memcpy(MAC,MACAddr,FMT_MAC_ADDR_LEN);
	printf("GerenalRandomMAC:%s\n",MAC);
}