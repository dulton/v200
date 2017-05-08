##################################################################
##编译器定义
##################################################################
CROSS_COMPILER:=arm-hismall-linux-
CC:=$(CROSS_COMPILER)g++-uc
GCC:=$(CROSS_COMPILER)gcc
STRIP:=$(CROSS_COMPILER)strip
CFLAGS:=-Wall  -Werror
LDFLAGS:= 
PROJECT_ROOT=$(PWD)
RELEASE_DIR:=$(PROJECT_ROOT)
PROJECT_INCLUDE_DIR:=$(PROJECT_ROOT)/include
EXE_NAME:=$(RELEASE_DIR)/App3518

ifeq ($(RELEASE), RELAPP)
	CFLAGS +=-DRELEASE_APP
	INCLUDE_DIR+=-DRELEASE_SOFTWARE=1
	INCLUDE_DIR+=-DSYSTEM_VER="\"${VERSION}\""	
else
	ifeq ($(RELEASE), RELFS)
		INCLUDE_DIR+=-DRELEASE_SOFTWARE=1
#		INCLUDE_DIR+=-DSYSTEM_VER="\"${VERSION}\""	
	else
#		INCLUDE_DIR+=-DSYSTEM_VER="\"0.00.01\""
	endif
endif

##################################################################
#子功能模块
##################################################################
NORMAL_API_DIR:=$(PROJECT_ROOT)/normal_api
#hi3511 API
HISAPI_DIR:=$(PROJECT_ROOT)/his_api
#GUI

#参数配置
CONFIG_DIR :=$(PROJECT_ROOT)/parameter
#videoencoder
VIDEOENCODER_DIR:=$(PROJECT_ROOT)/encode
#decode 
VIDEODECODE_DIR:=$(PROJECT_ROOT)/decode
#File Manage
FILEMANAGE_DIR:=$(PROJECT_ROOT)/filemanage
#record
RECORD_DIR:=$(PROJECT_ROOT)/record
#periphera
PERIPHERA_DIR:=$(PROJECT_ROOT)/peripheral
#ini profile parse
#INIPARSE_DIR 	:=$(PROJECT_ROOT)/iniparser

#Application file
APP_DIR:=$(PROJECT_ROOT)/App
#upgrade 
SYSUPGRADE_DIR:=$(PROJECT_ROOT)/upgrade

ENCRPT_DIR:=$(PROJECT_ROOT)/at88sc0104

TINYXML_DIR:=$(PROJECT_ROOT)/xmlResolve

LOG_DIR:=$(PROJECT_ROOT)/logmanage

#块设备管理
BLKMANAGE_DIR:=$(PROJECT_ROOT)/blockmanage

NET_DIR:=$(PROJECT_ROOT)/net

THTTPD_DIR:=$(PROJECT_ROOT)/httpd
PTZCTRL_DIR:=$(PROJECT_ROOT)/ptz

####################################################################
#在下面增加相关模块的源文件和头文件
####################################################################
INCLUDE_DIR:=-I$(PROJECT_INCLUDE_DIR)

INCLUDE_DIR+=-I../pub/include
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/normal_api
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/his_api
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/gui
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/parameter
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/record
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/encode
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/decode
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/filemanage
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/peripheral
#INCLUDE_DIR+=-I$(INIPARSE_DIR)/include
#INCLUDE_DIR+=-I$(LOG_DIR)/include
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/App
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/upgrade
INCLUDE_DIR+=-I$(PROJECT_INCLUDE_DIR)/at88sc0104
INCLUDE_DIR+= -I$(PROJECT_INCLUDE_DIR)/blockmanage
INCLUDE_DIR += -I$(PROJECT_INCLUDE_DIR)/zmdnetlib
INCLUDE_DIR += -I$(PROJECT_INCLUDE_DIR)/httpd
INCLUDE_DIR += -I$(PROJECT_INCLUDE_DIR)/ptz
INCLUDE_DIR += -I$(PROJECT_INCLUDE_DIR)/logmanage
INCLUDE_DIR += -I$(PROJECT_INCLUDE_DIR)/xmlResolve
