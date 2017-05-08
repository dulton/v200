/* extdrv/peripheral/keypad/hi_gpio.c
 *
 * Copyright (c) 2006 Hisilicon Co., Ltd. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * History: 
 *      10-April-2006 create this file
 */                                                   

#include <linux/fs.h>
//#include <linux/delay.h>
#include <linux/string.h>
//#include <linux/poll.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>

#include "GpioInterface.h"
//#include <asm/irq.h>
//#include<asm/io.h>

MMAP_NODE_T 	* pMMAPNode_yxl = NULL;
static int 			  mem_fd = -1;
static const char	 *mem_dev="/dev/mem";
static int  		HAVE_MAPED =0;
char				* GPIO_BASE = NULL;


int memunmap(void * addr_mapped)
{
	MMAP_NODE_T * pPre;
	MMAP_NODE_T * pTmp;

    if(pMMAPNode_yxl == NULL)
    {
        printf("memunmap(): address have not been mmaped!\n");
        return -1;
    }

	/* check if the physical memory space have been mmaped */
    pTmp = pMMAPNode_yxl;
    pPre = pMMAPNode_yxl;

    do
	{
		if( ((unsigned int)addr_mapped >= pTmp->Start_V) && 
			((unsigned int)addr_mapped <= (pTmp->Start_V + pTmp->length)) )
		{
	            pTmp->refcount--;   /* referrence count decrease by 1  */
	            if(0 == pTmp->refcount)
	            {
	                /* 引用计数变为0, 被map的内存空间不再使用,此时需要进行munmap回收 */
	 
	                printf("memunmap(): map node will be remove!\n");

	                /* delete this map node from pMMAPNode */
	                if(pTmp == pMMAPNode_yxl)
	                {
	                    pMMAPNode_yxl = NULL;
	                }
	                else
	                {
	                    pPre->next = pTmp->next;
	                }

	                /* munmap */
	                if(munmap((void *)pTmp->Start_V, pTmp->length) != 0 )
	                {
	                    printf("memunmap(): munmap failed!\n");
	                }

	                free(pTmp);
	            }
	            
	            return 0;
		}

        pPre = pTmp;
		pTmp = pTmp->next;
	}while(pTmp != NULL);

    printf("memunmap(): address have not been mmaped!\n");
    return -1;
}


char * memmap(unsigned int phy_addr, unsigned int size)
{
	unsigned int phy_addr_in_page;
	unsigned int page_diff;

	unsigned int size_in_page;

	MMAP_NODE_T * pTmp;
	MMAP_NODE_T * pNew;
	
	void *addr=NULL;

	if(size == 0)
	{
		printf("memmap():size can't be zero!\n");
		return NULL;
	}

	/* check if the physical memory space have been mmaped */
	pTmp = pMMAPNode_yxl;
	while(pTmp != NULL)
	{
		if( (phy_addr >= pTmp->Start_P) && 
			( (phy_addr + size) <= (pTmp->Start_P + pTmp->length) ) )
		{
            pTmp->refcount++;   /* referrence count increase by 1  */
			return (char *)(pTmp->Start_V + phy_addr - pTmp->Start_P);
		}

		pTmp = pTmp->next;
	}

	/* not mmaped yet */
	if(mem_fd < 0)
	{
		/* dev not opened yet, so open it */
		mem_fd= open (mem_dev, O_RDWR | O_SYNC);
		if (mem_fd < 0)
		{
			printf("memmap():open %s error!\n", mem_dev);
			return NULL;
		}
	}

	/* addr align in page_size(4K) */
	phy_addr_in_page = phy_addr & PAGE_SIZE_MASK;  //calculate the phyaddr start position in a page 
	page_diff = phy_addr - phy_addr_in_page;   //calculate the diff from the page start to the specified phyaddr

	/* size in page_size */
	size_in_page =((size + page_diff - 1) & PAGE_SIZE_MASK) + PAGE_SIZE_P;  //映射的最小单位为一页

	addr = mmap ((void *)0, size_in_page, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, phy_addr_in_page);
	if (addr == MAP_FAILED)
	{
		printf("memmap():mmap @ 0x%x error!\n", phy_addr_in_page);
		return NULL;
	}

	/* add this mmap to MMAP Node */
	pNew = (MMAP_NODE_T *)malloc(sizeof(MMAP_NODE_T));
    if(NULL == pNew)
    {
        printf("memmap():malloc new node failed!\n");
        return NULL;
    }
	pNew->Start_P = phy_addr_in_page;// 要映射的物理页起始地址
	pNew->Start_V = (unsigned int)addr; //映射后的虚拟地址空间页起始地址
	pNew->length = size_in_page; //映射后虚拟空间所映射的空间大小
       pNew->refcount = 1;
	pNew->next = NULL;
	
	if(pMMAPNode_yxl == NULL)
	{
		pMMAPNode_yxl = pNew;
	}
	else
	{
		pTmp = pMMAPNode_yxl;
		while(pTmp->next != NULL)
		{
			pTmp = pTmp->next;
		}

		pTmp->next = pNew;
	}

	return (char *)(addr)+page_diff;    //返回映射的虚拟空间起始地址

}


/*
 *	Free GPIO virtual addr 
 */
int gpio_unmap(void)
{
    	HAVE_MAPED--;
    	if(!HAVE_MAPED)
    		memunmap((void *)GPIO_BASE);
    	return 0;
}

/*
 *	Get virtual addr for GPIO
 */
 
int gpio_remap(void)
{
	if(!HAVE_MAPED)
	{
        	GPIO_BASE=memmap(GPIO_PHY_BASE,GPIO_MAP_SIZE);
        	if(!GPIO_BASE)
        	{
                	printf("No MEM allocat for GPIO\n");
                        return -1;
            	}
	}
    	HAVE_MAPED++;
	return 0;
}

/*
*  Gpio init
*/

int gpio_init()
{
	return gpio_remap();
}

/*
 *  	set the mode of the gpio path pins
 *  	1:hardware control;
 *  	0:software control,reset default.
 */
  
int gpio_modeset(unsigned int gpio_pathnum , unsigned int mode)
{ 
  	unsigned int modeValue;
  
  	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1; 
  	modeValue = mode;
  	gpio_user_write( modeValue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_AFSEL));
  	return 0;
}


/*
 *	Get the mode of the gpio path pins
 */
int gpio_modeget(unsigned int gpio_pathnum ,unsigned int * mode )
{
   
   	unsigned int returnmode;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
   
   	returnmode=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_AFSEL);
   	*mode = returnmode;	
  	return 0;
}


/*	
 *	Set the direction of the single bit for the specific path 
 * 	dirbit=1 means output
 * 	dirbit=0 means input
 */
int gpio_dirsetbit(unsigned int gpio_pathnum , unsigned int bitx ,unsigned int dirbit)
{
  	unsigned int dirvalue;

  	if (gpio_pathnum >= GPIO_GROUP_NUMBER) 
      		return -1;

  	dirvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_DIR);


   	if (dirbit == DIRECTION_OUTPUT)
   	{   
       		dirvalue |= (DIRECTION_OUTPUT << bitx);
       		gpio_user_write(dirvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_DIR));
	
			return 0;
			
   	}
   	else if(dirbit == DIRECTION_INPUT)
   	{
     		dirvalue &= (~(1 << bitx));	
     		gpio_user_write(dirvalue,(GPIO_BASE+(gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_DIR));
     		return 0;
   	}
   	else
   	   	return -1;  
}


/* 
 *	Set the input or output direction for the total path 
 */
int gpio_dirsetbyte(unsigned int gpio_pathnum , unsigned int dirbyte)
{
    	unsigned int dirvalue;
    
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
      
    	dirvalue = dirbyte;
   	gpio_user_write(dirvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_DIR));
    	return 0;
}


/* 
 *	Get the input or output direction for the total path 
 */
int gpio_dirgetbyte(unsigned int gpio_pathnum , unsigned int *dirbyte)
{
    	unsigned int dirvalue;
    
    	if (gpio_pathnum >= GPIO_GROUP_NUMBER )
      		return -1;
        
    	dirvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_DIR);
    	*dirbyte = dirvalue;
    
    	return 0;
}


/* 
 *	Get the input or output direction for a specific pin of a port 
 */
int gpio_dirgetbit(unsigned int gpio_pathnum , unsigned int bitx ,unsigned int *dirbit)
{
    	unsigned int dirvalue;
    
    	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
        
    	dirvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_DIR);
   	dirvalue &= (1 << bitx);
    	if(dirvalue == 0)
         	dirvalue = DIRECTION_INPUT;
    	else 
         	dirvalue = DIRECTION_OUTPUT;     
    	*dirbit = dirvalue;
    
    	return 0;
}    


/*
 *	Bit write the value from a specific bit of a gpio path
 */
int gpio_writebit(unsigned int gpio_pathnum , unsigned int bitx , unsigned int bitvalue)
{
    	unsigned int bitnum = bitx;
    	unsigned int dirbyte;

    	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
    	{
    		return -1;
    	}

    	if (bitnum > 7)
    	{
    		return -1;
    	}

    	if (gpio_dirgetbyte(gpio_pathnum , &dirbyte) == -1)
    	{
    		return -1;
    	}

	if((dirbyte & (1<<bitnum)) == DIRECTION_INPUT) 
    	{
    		return -1;   
    	}
    
    	if (bitvalue == 1)
      		gpio_user_write((1<<(bitnum)),(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + (4<<bitnum)));
    	else if(bitvalue == 0)
      		gpio_user_write( 0,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + (4<<bitnum)));
    	else 
    	{
    		return -1;
    	}
	
       return 0;

}


/*
 *	byte write the value of a specific gpio path
 */
int gpio_writebyte(unsigned int gpio_pathnum , unsigned int bytevalue)
{
  	unsigned int dirbyte;
  
  	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
  
  	if (gpio_dirgetbyte(gpio_pathnum , &dirbyte) == -1)
        	return -1;
     
  	if(dirbyte != 0xFF) 
       		return -1;   
        
  	gpio_user_write(bytevalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + 0x3FC));
  
  	return 0;
}


/*
 *	Bit read the value from a specific bit of a gpio path
 */
int gpio_readbit(unsigned int gpio_pathnum , unsigned int bitx , unsigned int * readbit)
{
   	unsigned int readvalue;
   	unsigned int bitnum = bitx;
   	unsigned int dirbyte;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
      
    	if (bitnum > 7)
      		return -1;
       
    	if (gpio_dirgetbyte(gpio_pathnum , &dirbyte) == -1)
        	return -1;
     
    	if((dirbyte & (1<<bitnum)) != DIRECTION_INPUT) 
       		return -1;  
      
    	readvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + (4<<bitnum));
    	readvalue >>= bitnum;
    	readvalue &= 0x01;
    	*readbit = readvalue;
    
    	return 0;
}


/*
 *	byte read the value of a specific gpio path
 */
int gpio_readbyte (unsigned int gpio_pathnum , unsigned int * readbyte)
{
  	unsigned int readvalue;
  	unsigned int dirbyte;
  
  	if (gpio_pathnum >= GPIO_GROUP_NUMBER) 
      		return -1;
   
   	if (gpio_dirgetbyte(gpio_pathnum , &dirbyte) == -1)
        	return -1;
     
   	if(dirbyte != 0x00) 
       		return -1;  
      
     	readvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + 0x3FC);
   	*readbyte = readvalue;
   	
   	return 0; 
}

//Begin add by zhuangjb, 20080124, 增加对一组GPIO的操作
/*
 *	byte write the value of a specific team gpio path
 */
int gpio_writeteambyte(unsigned int gpio_pathnum , unsigned int bytevalue)
{
  	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
  	{
      		return -1;
  	}
  
  	gpio_user_write(bytevalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + 0x3FC));
  
  	return 0;
}

/*
 *	byte read the value of a specific team gpio path
 */
int gpio_readteambyte (unsigned int gpio_pathnum , unsigned int * readbyte)
{
  	unsigned int readvalue;
  	//unsigned int dirbyte;
  
  	if (gpio_pathnum >= GPIO_GROUP_NUMBER) 
  	{
      		return -1;
  	}
   
     	readvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + 0x3FC);
   	*readbyte = readvalue;
   	
   	return 0; 
}
//End add by zhuangjb, 20080124

/* 
 *	Enable  single interrupt bit
 */
int gpio_interruptenable(unsigned int gpio_pathnum ,unsigned int bitx)
{
   	unsigned int regvalue;
   	int status;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
      
   	if (bitx > 7)
   	{
      		return -1;
   	}
   	else
   	{ 
     		status = gpio_dirsetbit(gpio_pathnum , bitx , DIRECTION_INPUT);  
     		regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IE);
     		regvalue |= (1<<(bitx));
     
     		gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IE));
		return 0;
   	}	
}



/* 
 *	Disable single interrupt bit
 */
int gpio_interruptdisable (unsigned int gpio_pathnum ,unsigned int bitx)
{
   	unsigned int regvalue;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
      
   	if (bitx > 7)
   	{
      		return -1;
        } 
   	else
   	{   
     		regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IE);   
     		regvalue &= (~(1<<(bitx)));
   
     		gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IE));

	     	return 0;
   	}		
}

/*
 *	Clear interrupt flags of gpio
 */
int gpio_interruptclear (unsigned int gpio_pathnum ,unsigned int bitx)
{
   	unsigned int regvalue=0;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
      
   	if (bitx > 7)
   	{
      		return -1;
   	}
   	else
   	{   
     		//regvalue=readw(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IC);   
     		regvalue = (1<<(bitx));
   
     		gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IC));

     		return 0;
   	}		
}



/*
 *	Select both edge(set to 1) or single edge(set to 0) to sense interrupt
 */
int gpio_interruptsenseboth(unsigned int gpio_pathnum ,unsigned int bitx ,unsigned int bothnsingle)
{
   	unsigned int regvalue;

   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
      
   	if (bitx > 7)
   	{
      		return -1;
   	}
   	else
   	{ 
    		regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IBE);
     		if (bothnsingle == SENSE_SINGLE)  
            		regvalue &= (~(1<<(bitx)));
     		else if (bothnsingle == SENSE_BOTH) 
            		regvalue |= (1<<(bitx));
     		else 
         		return -1;
                
     		gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IBE));
     		return 0;
   	}		
}


/*
 *	Select both edge(set to 1) or single edge(set to 0) to sense interrupt byte
 */
int gpio_interruptsenseboth_byte(unsigned int gpio_pathnum ,unsigned int byte ,unsigned int bothnsingle)
{
   	unsigned int regvalue;

   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
    	regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IBE);
    	if (bothnsingle == SENSE_SINGLE)  
            	regvalue &= ~byte;
     	else if (bothnsingle == SENSE_BOTH) 
            	regvalue |= byte;
     	else 
         	return -1;
     	
     	gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IBE));
     	return 0;
}


/*
 *	Select the single interrupt sense Edge(set bit to 0) 
 *	or sense level(set bit to 1) 
 */
int gpio_interruptsenseset(unsigned int gpio_pathnum ,unsigned int bitx ,unsigned int levelnedge)
{
   	unsigned int regvalue;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
      
   	if (bitx > 7)
   	{
      		return -1;
   	}
   	else
   	{ 
     		regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IS);
     		if (levelnedge == SENSE_EDGE)  
            		regvalue &= (~(1<<(bitx)));
     		else if (levelnedge == SENSE_LEVEL) 
            		regvalue |= (1<<(bitx));
     		else 
         		return -1;
                
     		gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IS));
     		return 0;
   	}		
}


/*
  Select the single interrupt  sense Edge(set bit to 0) 
  or sense level(set bit to 1) in a byte 
*/
int gpio_interruptsenseset_byte(unsigned int gpio_pathnum ,unsigned int byte ,unsigned int levelnedge)
{
   	unsigned int regvalue;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
     	regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IS);
     	if (levelnedge == SENSE_EDGE)  
            	regvalue &= ~byte ;
     	else if (levelnedge == SENSE_LEVEL) 
            	regvalue |= byte ;
     	else 
         	return -1;
                
     	gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IS));
     	return 0;
}


/*
 *	Select the single interrupt event rising Edge & high level (set bit to 1)
 *	or select the event falling edge or low level (set bit to 0);
 */
int gpio_interruptevenset(unsigned int gpio_pathnum ,unsigned int bitx , unsigned int risingnfalling)
{
   	unsigned int regvalue;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
      
   	if (bitx > 7)
   	{
      		return -1;
   	}
   	else
   	{  
     		regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IEV);
     		if ((risingnfalling == EVENT_RISING_EDGE)||(risingnfalling == EVENT_HIGH_LEVEL)) 
          		regvalue |= (1<<(bitx));
     		else if ((risingnfalling == EVENT_FALLING_EDGE)||(risingnfalling == EVENT_LOW_LEVEL))
          		regvalue &= (~(1<<(bitx)));
     		else 
         		return -1;     
     
     		gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IEV));
     		return 0;
   	}		
}


/*
 *	Select the single interrupt event rising Edge & high level (set bit to 1)
 *	or select the event falling edge or low level (set bit to 0) in a byte;
 */
int gpio_interruptevenset_byte(unsigned int gpio_pathnum ,unsigned int byte , unsigned int risingnfalling)
{
   	unsigned int regvalue;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
      
     	regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IEV);
     	if ((risingnfalling == EVENT_RISING_EDGE)||(risingnfalling == EVENT_HIGH_LEVEL)) 
        	regvalue |= byte;
     	else if ((risingnfalling == EVENT_FALLING_EDGE)||(risingnfalling == EVENT_LOW_LEVEL))
          	regvalue &= ~byte;
     	else 
         	return -1;     
    
     	gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IEV));
     	return 0;
}


/* 
 *	Interrupt mode set:include bothnsingle,levelnedge,risingnfalling
 */
int gpio_interruptset
   	(
    		unsigned int gpio_pathnum ,
    		unsigned int bitx ,
    		unsigned int bothnsingle,
    		unsigned int levelnedge,
    		unsigned int risingnfalling
    	)
{
    	int status;
    	status = gpio_interruptsenseboth(gpio_pathnum,bitx,bothnsingle);
    	if(status == -1)
      		return -1;
    	status = gpio_interruptsenseset(gpio_pathnum,bitx,levelnedge);
    	if(status == -1)
      		return -1;
    	status = gpio_interruptevenset(gpio_pathnum,bitx,risingnfalling);    
    	return status;
}


/* 
 *	Interrupt mode set:include bothnsingle,levelnedge,risingnfalling in a byte
 */
int gpio_interruptset_byte
   	(
    		unsigned int gpio_pathnum ,
    		unsigned int byte ,
    		unsigned int bothnsingle,
    		unsigned int levelnedge,
    		unsigned int risingnfalling
    	)
{
    	int status;
    	status = gpio_interruptsenseboth_byte(gpio_pathnum,byte,bothnsingle);
    	if(status == -1)
      		return -1;
    	status = gpio_interruptsenseset_byte(gpio_pathnum,byte,levelnedge);
    	if(status == -1)
      		return -1;
    	status = gpio_interruptevenset_byte(gpio_pathnum,byte,risingnfalling);    
    	return status;
}


/* 
 *	Disable  single interrupt byte
 */
int gpio_interruptdisable_byte (unsigned int gpio_pathnum ,unsigned int byte)
{
   	unsigned int regvalue;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
     	regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IE);   
     	regvalue &=  ~byte;
     	gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IE));
     	return 0;
}	

	
/* 
 *	Enable  single interrupt byte
 */
int gpio_interruptenable_byte (unsigned int gpio_pathnum ,unsigned int byte)
{
   	unsigned int regvalue;
   
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
     	regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IE);   
     	regvalue |=  byte;
     	gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IE));
     	return 0;
}		

int gpio_interruptclear_byte (unsigned int gpio_pathnum ,unsigned int byte)
{
   	unsigned int regvalue;
   	if (gpio_pathnum >= GPIO_GROUP_NUMBER)
      		return -1;
     	regvalue=gpio_user_read(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IC);   
     	regvalue |= byte;
     	gpio_user_write(regvalue,(GPIO_BASE + (gpio_pathnum) * GPIO_SPACE_SIZE + GPIO_IC));
     	return 0;
}








