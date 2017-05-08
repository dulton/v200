#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h> 

#define BUFSIZE 1024*100
char oneconfig[8][32]={0};
int  SUPPORT_ZINK =0;
char *getRoorDir()
{
	static char curdir[128]={0};
	memset(curdir,0,sizeof(curdir));
	getcwd(curdir,sizeof(curdir));	
    return curdir;
}
/*
修改文件将关键字后面的start和end字符串中间的数据改成data
@name:文件名称
@keystr:关键字
@data:要修改的数据
*/
int Modifyfile(char *name,char *keystr,char *start,char *end,char *data)
{
	char *buf = (char *)malloc(BUFSIZE);
	char filename[128]={0};
	char *ptr = NULL;
	int  readlen = 0;
	if(buf==NULL)return -1;
	int fd = -1;
	printf("Modifyfile-----------{%s,%s}\n",name,data);
	printf("##########################################\n");
	printf("Modifyfile-----------{%s,%s,%s}\n",keystr,start,end);
	printf("##########################################\n");
	do
	{
		sprintf(filename,"%s/%s",getRoorDir(),name);
		
		if((fd=open(filename,O_RDWR)) <=0)
		{
			printf("open filename[%s] file failed\n",filename);
			break;
		}
		if((readlen=read(fd,buf,BUFSIZE))<=0)
		{
			printf("read filename:%s file failed\n",filename);
			break;
		}
		if( (ptr=strstr(buf,keystr)) == NULL)
		{
			printf("find keystr:%s  failed\n",keystr);
			break;
		}
		
		ptr+=strlen(keystr);

		char *pstart =strstr(ptr,start);		
		if(pstart ==NULL)break;
		char *pend =strstr(pstart,end);
		if(pend ==NULL)break;
		ftruncate(fd, 0);
		lseek(fd, 0, SEEK_SET);		
		int   index = pstart-buf+strlen(start);
		//printf("index:%d,%d,%d\n",index,strlen(start),strlen(data));
		write(fd , buf, index);//写要修改字符串之前的数据
		write(fd , data, strlen(data));//写修改的数据
		index=readlen-(pend-buf);	
		write(fd ,pend,index);//写后面的数据
		//printf("pend:%s\n",pend);
		fsync(fd);
		//printf("index:%d,len:%d\n",readlen-(pend-buf),readlen);
		

	}
	while(0);
	if(fd>0)
	{
		close(fd);
	}
	if(buf!=NULL)
	{
		free(buf);
	}
	printf("修改成功!!!!!\n");
	return 0;
}
int ComplieAndMakepacket()
{
	/*清除并编译*/
	char cmd[512]={0};
	sprintf(cmd,"make clean;make all -j");
	system(cmd);
	printf("run cmd :%s\n",cmd);


	
	/*创建版本目录*/
	sprintf(cmd,"mkdir output/%s",oneconfig[5]);
	system(cmd);	
	printf("run cmd :%s\n",cmd);


	/*拷贝对应的xml到对应的flash目录*/
	sprintf(cmd,"cp xml/%s flash/%s/versioninfo.xml",oneconfig[2],oneconfig[4]);
	system(cmd);
	printf("run cmd :%s\n",cmd);
#if 0
	/*按需拷贝wifi相关文件到指定打包目录*/
	if( strcmp(oneconfig[0],"H42SPOE_IPC")!=0&& strcmp(oneconfig[0],"SPOE_IPC")!=0)
	{
		sprintf(cmd,"cp -rf ./wifi/Zink/3518e/timeout ./AppReleaseDir/%s/",oneconfig[3]);
		system(cmd);
		printf("run cmd :%s\n",cmd);

		sprintf(cmd,"cp -rf ./wifi/Zink/3518e/www ./AppReleaseDir/%s/",oneconfig[3]);
		system(cmd);
		printf("run cmd :%s\n",cmd);

		sprintf(cmd,"cp -rf ./wifi/Zink/3518e/tools ./AppReleaseDir/%s/wifi/",oneconfig[3]);
		system(cmd);
		printf("run cmd :%s\n",cmd);
	}
#endif

	/*打包版本*/
	
	sprintf(cmd,"make app appversion=%s",oneconfig[5]);
	system(cmd);
	printf("run cmd :%s\n",cmd);


	/*将打包后的文件拷贝到对应版本目录*/
	sprintf(cmd,"cp flash/%s/* output/%s/ -rf",oneconfig[4],oneconfig[5]);
	system(cmd);	
	printf("run cmd :%s\n",cmd);
//  ./flash/3518e_ipc/makeprogrammingflashimg ./flash/3518e_ipc/u-boot-ok.bin ./flash/3518e_ipc/config ./flash/3518e_ipc/root_cramfs.img ./flash/3518e_ipc/app.img

	//	./makeprogrammingflashimg u-boot-ok.bin config root_cramfs.img app.img 
	/*制作烧片文件*/

	sprintf(cmd,"./output/%s/makeprogrammingflashimg output/%s/u-boot-ok.bin output/%s/hikernel output/%s/rootfs_64k.squashfs output/%s/app.img output/%s/config",oneconfig[5],oneconfig[5],oneconfig[5],oneconfig[5],oneconfig[5],oneconfig[5]);
	system(cmd);	
	printf("run cmd :%s\n",cmd);
	/*移动烧片文件到指定目录*/
	sprintf(cmd,"mv 3518E-SOFTWARE.bin output/%s/",oneconfig[5]);
	system(cmd);	
	printf("run cmd :%s\n",cmd);
	//删除不用的文件
	sprintf(cmd,"output/%s/app.img",oneconfig[5]);
	unlink(cmd);	
	printf("unlink :%s\n",cmd);
	
	sprintf(cmd,"output/%s/config",oneconfig[5]);
	unlink(cmd);	
	printf("unlink :%s\n",cmd);

	sprintf(cmd,"output/%s/makeprogrammingflashimg",oneconfig[5]);
	unlink(cmd); 
	printf("unlink :%s\n",cmd);


	sprintf(cmd,"output/%s/rootfs_64k.squashfs",oneconfig[5]);
	unlink(cmd); 
	printf("unlink :%s\n",cmd);

	sprintf(cmd,"output/%s/hikernel",oneconfig[5]);
	unlink(cmd); 
	printf("unlink :%s\n",cmd);

	sprintf(cmd,"output/%s/u-boot-ok.bin",oneconfig[5]);
	unlink(cmd); 
	printf("unlink :%s\n",cmd);

	sprintf(cmd,"output/%s/updatefile",oneconfig[5]);
	unlink(cmd); 
	printf("unlink :%s\n",cmd);


	sprintf(cmd,"output/%s/versioninfo.xml",oneconfig[5]);
	unlink(cmd); 
	printf("unlink :%s\n",cmd);

	

	
	

	
}
int ParserOneVersionInof(int description,char* buf)
{

	char tmp[128]={0};	
	printf("========================start============================\n");
	if(strlen(buf)>1023)
	{
		printf("ParserOneVersionInof error buf size!!!\n");
		return -1;
	}
	if(description&& oneconfig[6][0]=='1' )
	{
		//修改xml 版本记录
		sprintf(tmp,"xml/%s",oneconfig[2]);
		printf("修改xml 版本记录\n");
		Modifyfile(tmp,(char *)"</version>",(char *)"<description>",(char *)"	</description>",buf);
		printf("=========================end===========================\n");
		ComplieAndMakepacket();
		return 0;
	}
	char *ptr = strchr(buf,'[');
	if(ptr == NULL)
	{
		//printf("ParserOneVersionInof error return!!!\n");
		return -1;
	}	
	char data[1024]={0};
	char index=0;
	strcpy(data,ptr+1);
	printf("%s\n",data);
	char *line;
	char *outer_ptr = NULL;
	line = strtok_r(data, ",", &outer_ptr); 	
	if(line!=NULL)
	{
		printf("ParserOneVersionInof:%s\n",line);
		strcpy(oneconfig[index++],line);
	}
	while  (line != NULL)		
	{
			
		line =	strtok_r(NULL, ",", &outer_ptr);
		if(line!=NULL)
		{
			printf("ParserOneVersionInof:%s\n",line);
			strcpy(oneconfig[index++],line);
		}

	}
		//CARD_IPC,AR9271_IPC,card_ipc.xml,card_ipc,3518e_ipc,V7.9.0.18
	//char machine[32]={0};
	//char macro[32]={0};
	//char xml[32]={0};
	//char apprel[32]={0};
	//char appflash[32]={0};
	//char appver[32]={0};
	//char need[32]={0};
	if(oneconfig[6][0]!='0')//需要发布此版本
	{
		
		printf("------------------------------------------------------need make %s app\n",oneconfig[0]);
		
	}
	else
	{
		printf("-------------------------------------------------------not need make %s\n",oneconfig[0]);
		return -1;
		
	}
	//修改makefile
	printf("修改makefile\n");
	Modifyfile((char *)"Makefile",(char *)"MACHINE_TYPE_SWITCH",(char *)" = ",(char *)"\n",(char *)oneconfig[0]);
	
	//修改common.h
	printf("修改common.h\n");
	char keystr[64]={"defined "};
	strcat(keystr,oneconfig[1]);
	Modifyfile((char *)"include/common.h",keystr,(char *)"SOFTWAREVERSION		\"",(char *)"\"\r\n",(char *)oneconfig[5]);
	
	//修改xml 版本号
	sprintf(tmp,"xml/%s",oneconfig[2]);
	printf("修改xml 版本号\n");
	Modifyfile(tmp,(char *)"</rootfs>",(char *)"<app>",(char *)"</app>",(char *)oneconfig[5]);
	

	printf("=========================end===========================\n");
	return 0;
}

int ParserConfig(char* FileName)
{
	char configfile[128]={0};
	char buf[10240]={0};
	sprintf(configfile,"%s/v200makeconfig",getRoorDir());
	int configfd = -1;
	do
	{
		if((configfd=open(configfile,O_RDONLY)) <=0)
		{
			break;
		}
		if(read(configfd,buf,10240)<=0)
		{
			break;
		}
		char *decripe = strchr(buf,'{');	
		if(decripe==NULL)
		{
			break;			
		}
		int decripelen  = strlen(decripe); 
		decripe[decripelen-1] = 0;
		decripe[decripelen-2] = 0;
		decripe++;
		printf("@@@@@@@@@@@@@@%s@@@@@@@@@@@@@@\n",decripe);

		
		char *line;
		char *outer_ptr = NULL;
		line = strtok_r(buf, "]", &outer_ptr); 

		if(line!=NULL)
		{
			
			if(ParserOneVersionInof(0,line)==0)
			{
				ParserOneVersionInof(1,decripe);
			}
			//ParserOneVersionInof(1,decripe);
		}
		while  (line != NULL)		
		{
				
			line =	strtok_r(NULL, "]", &outer_ptr);
			
			if(line!=NULL)
			{
				//printf("222###%s###\n",line);
				if(ParserOneVersionInof(0,line)==0)
				{
					ParserOneVersionInof(1,decripe);
				}
				//ParserOneVersionInof(1,decripe);
			}
		}

	}
	while(0);
	if(configfd>0)
	{
		close(configfd);
	}
	
    return 0;
}


int main()
{
	//printf("==============[%s]\n",getRoorDir());
	system("rm output -rf");
	system("rm TmpRelease -rf");
	mkdir("output",S_IRUSR | S_IWUSR | S_IXUSR|S_IRGRP | S_IWGRP | S_IXGRP|S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("TmpRelease",S_IRUSR | S_IWUSR | S_IXUSR|S_IRGRP | S_IWGRP | S_IXGRP|S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	ParserConfig(NULL);
	//Modifyfile((char *)"Makefile",(char *)"MACHINE_TYPE_SWITCH",(char *)" = ",(char *)"\n",(char *)"SP_IPC");
	return 0;
}




