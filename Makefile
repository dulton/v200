#makefile优化，精简makefile维护方便， harvey 2013.01.31
CROSS_COMPILER:= arm-hisiv300-linux-
#CROSS_COMPILER:= arm-hisiv100nptl-linux-
CC:=$(CROSS_COMPILER)g++	
STRIP:=$(CROSS_COMPILER)strip
#MINI_IPC POC_IPC NORMAL_IPC MININVR_IPC 3518C平台
EXE_NAME:=App3518
#1080P_IPC 3516C平台
#EXE_NAME:=App3516
export PROJECT_ROOT=$(PWD)
MAKEPACKET_APP=$(PROJECT_ROOT)/TmpRelease/

RELEASE_NFS:=~/nfs

HW_VERSION=V1.0
MACHINE_TYPE=HI3518V200
#VERSION=V7OR V8 
VERSION=V8
ifeq ($(VERSION), V8)	
	UOOT_VERSION=V8.0.0.0 
	KERNEL_VERSION=V8.0.0.0 
	ROOTFS_VERSION=V8.0.0.0 
	HARDWARE_VERSION=V8.0.0.0 
	APP_VERSION=V8.0.0.05001
	CFLAGS+=-DV8
else
ifeq ($(VERSION), V7)
	UOOT_VERSION=V7.0.0.0 
	KERNEL_VERSION=V7.0.0.0 
	ROOTFS_VERSION=V7.0.0.0 
	HARDWARE_VERSION=V7.0.0.0 
	CFLAGS+=-DV7
else
$(error ERROR:)
endif
endif
#0和2 代表mpp和mpp2  后续只用mpp2
MPP_VERSION=0
SUPPORT_VIDEO_AESENC = y



#设备型号，必须定义一种，定义后会指定AppReleaseDir 和打包的flash目录
#NORMAL_IPC  
#AR9271_IPC  
#PT_IPC 
#MINI_IPC 
#NEW_NORMAL_IPC 
#V74_NORMAL_IPC 
#SPOE_IPC 
#H42SPOE_IPC 
#H42PT_IPC 
#V200_NORMAL_IPC 
#V200_SPOE_IPC 
MACHINE_TYPE_SWITCH = V200_NORMAL_IPC

ifeq ($(MACHINE_TYPE_SWITCH), V200_NORMAL_IPC)	
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/v200_normal
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/v200_normal
WIFI_DIR:=$(PROJECT_ROOT)/wifi/ar9271
	CFLAGS+=-DWIFI_18E_IPC
	CFLAGS+=-DV200_NORMAL_IPC
	CFLAGS+=-DV200
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = y
	MACHINE_TYPE=HI3518V200
else
ifeq ($(MACHINE_TYPE_SWITCH), V200_SPOE_IPC)	
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/v200_spoe
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/v200_spoe
WIFI_DIR:=$(PROJECT_ROOT)/wifi/ar9271
	CFLAGS+=-DV200_SPOE_IPC
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = n
	CFLAGS+=-DV200
	MACHINE_TYPE=HI3518V200
else
ifeq ($(MACHINE_TYPE_SWITCH), AR9271_IPC)	
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/card_ipc
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/3518e_ipc
WIFI_DIR:=$(PROJECT_ROOT)/wifi/ar9271
	CFLAGS+=-DWIFI_18E_IPC
	CFLAGS+=-DAR9271_IPC
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = n
else
ifeq ($(MACHINE_TYPE_SWITCH), NORMAL_IPC)
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/normal_ipc
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/3518e_ipc
WIFI_DIR:=$(PROJECT_ROOT)/wifi/ar9271
	CFLAGS+=-DWIFI_18E_IPC
	CFLAGS+=-DNORMAL_IPC
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = y
else
ifeq ($(MACHINE_TYPE_SWITCH), V74_NORMAL_IPC)
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/normal_ipc
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/3518e_ipc
WIFI_DIR:=$(PROJECT_ROOT)/wifi/ar9271
	CFLAGS+=-DWIFI_18E_IPC
	CFLAGS+=-DV74_NORMAL_IPC
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = n
else
ifeq ($(MACHINE_TYPE_SWITCH), MINI_IPC)
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/mini_ipc
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/3518e_ipc
WIFI_DIR:=$(PROJECT_ROOT)/wifi/ar9271
	CFLAGS+=-DWIFI_18E_IPC
	CFLAGS+=-DMINI_IPC
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = y
else
ifeq ($(MACHINE_TYPE_SWITCH), PT_IPC)
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/pt_ipc
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/3518e_ipc
WIFI_DIR:=$(PROJECT_ROOT)/wifi/ar9271
	CFLAGS+=-DWIFI_18E_IPC
	CFLAGS+=-DPT_IPC
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = y
else
ifeq ($(MACHINE_TYPE_SWITCH), NEW_NORMAL_IPC)
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/new_normal_ipc
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/3518e_ipc_new
WIFI_DIR:=$(PROJECT_ROOT)/wifi/ar9271
	CFLAGS+=-DWIFI_18E_IPC
	CFLAGS+=-DNEW_NORMAL_IPC
	CFLAGS+=-DNEW_MPP
	MPP_VERSION=2
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = y
else
ifeq ($(MACHINE_TYPE_SWITCH), H42PT_IPC)
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/pth42_ipc
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/3518e_ipc_new
WIFI_DIR:=$(PROJECT_ROOT)/wifi/ar9271
	CFLAGS+=-DWIFI_18E_IPC
	CFLAGS+=-DH42PT_IPC
	CFLAGS+=-DNEW_MPP
	MPP_VERSION=2
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = y
else
ifeq ($(MACHINE_TYPE_SWITCH), SPOE_IPC)
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/spoe_ipc
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/3518e_spoe
	#CFLAGS+=-DCONFIG_VAR
	CFLAGS+=-DSPOE_IPC
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = n
else
ifeq ($(MACHINE_TYPE_SWITCH), H42SPOE_IPC)
RELEASE_APP=$(PROJECT_ROOT)/AppReleaseDir/spoeh42_ipc
UPGRADE_DIR:=$(PROJECT_ROOT)/flash/3518eh42-spoe
	CFLAGS+=-DH42SPOE_IPC
	CFLAGS+=-DNEW_MPP
	MPP_VERSION=2
	SUPPORT_P2P_FUNC=y
	SUPPORT_ZINK = n
else
$(error ERROR:You must define MACHINE_TYPE_SWITCH = CARD_IPC or MACHINE_TYPE_SWITCH = NORMAL_IPC )
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
#包含的头文件，将要包含的头文件放在指定目录
PROINC:=-I$(PROJECT_ROOT)/include/
export INCLUDE+=$(PROINC)
#INCLUDE+=-I$(PROJECT_ROOT)/zmdnetlib/include
INCLUDE+=-I$(PROJECT_ROOT)/record/include
INCLUDE+=-I$(PROJECT_ROOT)/message
INCLUDE+=$(PROINC)alarm $(PROINC)at88sc0104 $(PROINC)blockmanage $(PROINC)encode $(PROINC)decode $(PROINC)filemanage $(PROINC)zbar $(PROINC)wifi\
$(PROINC)g3modem $(PROINC)gui  $(PROINC)jpg_include $(PROINC)logmanage $(PROINC)normal_api $(PROINC)parameter  \
$(PROINC)peripheral $(PROINC)parameter $(PROINC)ptz $(PROINC)record $(PROINC)upgrade $(PROINC)xmlResolve $(PROINC)App $(PROINC)zmdnetlib $(PROINC)led $(PROINC)mbedtls

ifeq ($(MACHINE_TYPE), HI3518V100)
	ifeq ($(MPP_VERSION), 2)
	INCLUDE+=$(PROINC)his_api/mpp2
	else
	INCLUDE+=$(PROINC)his_api/mpp
	endif
else
	INCLUDE+=$(PROINC)his_api/v200mpp
endif
#编译库目录
normal_src :=  normal_api  encode    memwatch  parameter \
	ptz App upgrade   peripheral  record   
normal_lib_make+=$(addprefix $(PROJECT_ROOT)/,$(normal_src))
#链接库，可以不分先后顺序
linklib+=$(wildcard $(PROJECT_ROOT)/lib/*.a)


ifeq ($(MACHINE_TYPE), HI3518V100)
	ifeq ($(MPP_VERSION), 2)
		linklib+=$(wildcard $(PROJECT_ROOT)/his_api/mpp2/lib.rel/*.a)
		
		ifeq ($(MACHINE_TYPE_SWITCH), 1080P_IPC)
		linklib+=$(wildcard $(PROJECT_ROOT)/his_api/mpp2/lib.rel/3516sns/*.a)
		else
		linklib+=$(wildcard $(PROJECT_ROOT)/his_api/mpp2/lib.rel/3518sns/*.a)
		endif	
	else
		linklib+=$(wildcard $(PROJECT_ROOT)/his_api/mpp/lib.rel/*.a)
		ifeq ($(MACHINE_TYPE_SWITCH), 1080P_IPC)
			linklib+=$(wildcard $(PROJECT_ROOT)/his_api/mpp/lib.rel/3516sns/*.a)
		else
			linklib+=$(wildcard $(PROJECT_ROOT)/his_api/mpp/lib.rel/3518sns/*.a)
		endif	
	endif
else
	linklib+=$(wildcard $(PROJECT_ROOT)/his_api/v200mpp/lib.rel/*.a)
	linklib+=$(wildcard $(PROJECT_ROOT)/his_api/v200mpp/lib.rel/3518sns/*.a)
endif




ifeq ($(SUPPORT_ZINK), y)
	linklib+=$(wildcard $(PROJECT_ROOT)/wifi/Zink/3518e/lib/*.a)	
	CFLAGS+=-DZINK
endif

ifeq ($(MACHINE_TYPE_SWITCH), AR9271_IPC)	
linklib+=$(wildcard $(PROJECT_ROOT)/wifi/ar9271/lib/*.a)
endif

ifeq ($(MACHINE_TYPE_SWITCH), V74_NORMAL_IPC)	
linklib+=$(wildcard $(PROJECT_ROOT)/wifi/8188/3518/lib/*.a)
endif
###############################
ifeq ($(MACHINE_TYPE_SWITCH), SPOE_IPC)	
linklib+=$(wildcard $(PROJECT_ROOT)/wifi/8188/3518/lib/*.a)
endif
ifeq ($(MACHINE_TYPE_SWITCH), H42SPOE_IPC)	
linklib+=$(wildcard $(PROJECT_ROOT)/wifi/8188/3518/lib/*.a)
endif
ifeq ($(MACHINE_TYPE_SWITCH), V200_SPOE_IPC)	
linklib+=$(wildcard $(PROJECT_ROOT)/wifi/8188/3518/lib/*.a)
endif
############################
ifeq ($(SUPPORT_P2P_FUNC), y)
#linklib+=$(wildcard $(PROJECT_ROOT)/lib/sip/*.a)

#新平台
linklib+=$(wildcard $(PROJECT_ROOT)/lib/meshare/*.a)
CFLAGS+=-DSUPPORT_P2P
endif




ifeq ($(SUPPORT_VIDEO_AESENC), y)
CFLAGS+=-DVIDEO_AES_ENC
endif

#linklib+=$(wildcard $(PROJECT_ROOT)/zmdnetlib/lib/*.a)
#动态链接库
#linklib+=-L$(PROJECT_ROOT)/lib/onvif -lnvt -lssl -lcrypto -dl
#linklib+=-L$(PROJECT_ROOT)/lib/ -lAbnormalVoice -dl
#linklib+=-L$(PROJECT_ROOT)/his_api/lib.rel -lisp  -dl
# support timer lib
linklib+= -lrt 
#编译链接参数，-O2 是必须的参数，
CFLAGS += -DPJ_IS_LITTLE_ENDIAN=1 -DPJ_IS_BIG_ENDIAN=0 -D_BARCODE_SUP_	 -DAPP3518 -DSUPPORT_WIFI
CFLAGS += -Wall   -fpic -O2 -fno-strict-aliasing 
LDFLAGS+=$(linklib)
LDFLAGS+=-lpthread -lm -ldl
SRC+=$(wildcard $(PROJECT_ROOT)/encode/bitrate_adjust/*.cpp)
INCLUDE+=-I$(PROJECT_ROOT)/encode/bitrate_adjust
#编译源文件
SRC+=$(wildcard $(PROJECT_ROOT)/alarm/*.cpp) 
SRC+=$(wildcard $(PROJECT_ROOT)/at88sc0104/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/normal_api/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/encode/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/logmanage/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/memwatch/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/parameter/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/ptz/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/App/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/upgrade/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/peripheral/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/peripheral/mcu/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/peripheral/realtime/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/peripheral/serial/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/record/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/filemanage/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/blockmanage/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/led/*.cpp)
SRC+=$(wildcard $(PROJECT_ROOT)/*.cpp)
#目标文件
TARGETOBJ  := $(SRC:%.cpp=%.o)

ifeq ($(RELEASE), RELAPP)
	CFLAGS +=-DRELEASE_APP
	INCLUDE_DIR+=-DRELEASE_SOFTWARE=1
	INCLUDE_DIR+=-DSYSTEM_VER="\"${VERSION}\""	
else	
		INCLUDE_DIR+=-DRELEASE_SOFTWARE=1
endif


.PHONY:all
all: $(TARGETOBJ) $(EXE_NAME)	  
$(TARGETOBJ): %.o: %.cpp 
	$(CC) -c $(CFLAGS) $< -o $@ $(INCLUDE)

$(EXE_NAME): $(TARGETOBJ) 
	#make -C zmdnetlib/build	
	$(CC) $(CFLAGS) $(TARGETOBJ)  -o $(EXE_NAME)   -Xlinker --start-group $(LDFLAGS) -Xlinker --end-group
	$(STRIP)	./$(EXE_NAME)
	#@- cp -rf   $(PROJECT_ROOT)/cab/IPCWeb.cab $(RELEASE_APP)/dvr/	

	#cp -rf   $(WIFI_DIR)/driver $(RELEASE_APP)/wifi/
	#cp -rf   $(WIFI_DIR)/tools $(RELEASE_APP)/wifi/
	cp -rf	 $(EXE_NAME) $(RELEASE_APP)/
	cp -rf	 $(EXE_NAME) $(RELEASE_NFS)/	
#注意，在发布版本时检测WIFI是否为最新
app:
	#./tools/CreateSoftVersion  $(RELEASE_APP)/   2   ${VERSION} $(HW_VERSION) $(MACHINE_TYPE) 
	#./tools/mkapp_32m $(RELEASE_APP) ${UPGRADE_DIR}/app.img	
	rm -rf $(MAKEPACKET_APP)/*
	cp -rf	$(RELEASE_APP)/* $(MAKEPACKET_APP)
	@-find $(MAKEPACKET_APP) -name *.svn -exec rm -rf {} \;
	./tools/mkfs.jffs2 -d $(MAKEPACKET_APP) -l -e 0x10000 -o ${UPGRADE_DIR}/app.img
	rm -f ${UPGRADE_DIR}/IPC-APP
	./tools/makeZmdImage ${UPGRADE_DIR}/app.img ${UPGRADE_DIR}/IPC-APP ${UOOT_VERSION} ${KERNEL_VERSION} ${ROOTFS_VERSION} ${appversion} ${HARDWARE_VERSION} 4
	@chmod 777 ${UPGRADE_DIR}/IPC-APP
	@- cp ${UPGRADE_DIR}/IPC-APP ${UPGRADE_DIR}/updatefile
	@- cd ${UPGRADE_DIR};tar cvzf updatefile.tar.gz updatefile versioninfo.xml;	
	cd ${UPGRADE_DIR}; md5sum updatefile.tar.gz | cut -d ' ' -f1 > MD5.txt
	@- cd ${UPGRADE_DIR}; ./makeprogrammingflashimg u-boot-ok.bin hikernel rootfs_64k.squashfs app.img config 
rootfs:

#	./tools/CreateSoftVersion  $(RELEASE_ROOTFS)/var/   1   ${VERSION} $(HW_VERSION) $(MACHINE_TYPE) 
	@-rm -f ${UPGRADE_DIR}/IPC-RTFS
	./tools/makeZmdImage ${UPGRADE_DIR}/rootfs.img ${UPGRADE_DIR}/IPC-RTFS ${UOOT_VERSION} ${KERNEL_VERSION} ${ROOTFS_VERSION} ${APP_VERSION} ${HARDWARE_VERSION} 3
	@chmod 777 ${UPGRADE_DIR}/IPC-RTFS

kernell:
#	./tools/CreateSoftVersion  $(RELEASE_ROOTFS)/var/   1   ${VERSION} $(HW_VERSION) $(MACHINE_TYPE) 
	#@-rm -f ${UPGRADE_DIR}/IPC-KERNEL
	./tools/makeZmdImage ${UPGRADE_DIR}/hikernel ${UPGRADE_DIR}/IPC-KERNEL ${UOOT_VERSION} ${KERNEL_VERSION} ${ROOTFS_VERSION} ${APP_VERSION} ${HARDWARE_VERSION} 2
	@chmod 777 ${UPGRADE_DIR}/IPC-KERNEL
uboot:
#	./tools/CreateSoftVersion  $(RELEASE_ROOTFS)/var/   1   ${VERSION} $(HW_VERSION) $(MACHINE_TYPE) 
	#@-rm -f ${UPGRADE_DIR}/IPC-UBOOT
	./tools/makeZmdImage ${UPGRADE_DIR}/u-boot-ok.bin ${UPGRADE_DIR}/IPC-UBOOT ${UOOT_VERSION} ${KERNEL_VERSION} ${ROOTFS_VERSION} ${APP_VERSION} ${HARDWARE_VERSION} 1
	@chmod 777 ${UPGRADE_DIR}/IPC-UBOOT
	
#伪目标lib来编译lib   make lib 如果服务器下载代码链接不通过就上传库即可
.PHONY:lib
lib:$(normal_lib_make)
	@for srcfile in $(normal_lib_make); do	\
	echo $$srcfile;	$(MAKE) -C $$srcfile all;	\
	done
.PHONY:allclean
allclean:clean
	rm -rf $(PROJECT_ROOT)/lib/*.a 
.PHONY:clean
clean:
	@for srcfile in $(TARGETOBJ); do	\
	rm -rf $$srcfile ;		\
	done

	rm -rf $(EXE_NAME) 	
	rm -rf *.o 	
	
