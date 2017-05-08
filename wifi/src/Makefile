
IPNC_INSTALL_DIR = ../../

CROSS_COMPILER:= arm-hisiv100nptl-linux-
CC:=$(CROSS_COMPILER)g++	
GCC:=$(CROSS_COMPILER)gcc
AR:=$(CROSS_COMPILER)ar
CFLAGS:= -Wall   -fpic -O2 -fno-strict-aliasing  #-Werror -lstdc++

CFLAGS+= -D $(WIFI_CHIP) 
CFLAGS+= -D $(PLATFORM_TYPE) 

#引用工程头文件

INCLUDE_DIR	:= parameter normal_api alarm App record xmlResolve encode blockmanage upgrade at88sc0104 \
				peripheral filemanage logmanage  ptz neti  snapProcess gui g3modem  wifi \
				encode decode mediaplayer buffermanage his_api zmdnetlib  led his_api/mpp\
				sip/pjsip/include sip/xmlrpc-epi/include   \
				

CFLAGS		+= -I$(IPNC_INSTALL_DIR)/include $(foreach INCLUDE_DIR,$(INCLUDE_DIR),$(addprefix -I$(IPNC_INSTALL_DIR)/include/,$(INCLUDE_DIR))) 

#LIB = -static  -lpthread -L.
# support timer lib
#LIB = -lrt 

LIBOBJ_DIR=.
SRC +=$(wildcard $(LIBOBJ_DIR)/*.cpp)
SRC +=$(wildcard $(LIBOBJ_DIR)/*.c)
OBJ = $(SRC:%.cpp=%.o)
LIBSET_DIR=$(IPNC_INSTALL_DIR)/lib
OBJ_DIR=$(PROJECT_ROOT)/objects
OBJ_LIB=$(addprefix $(LIBOBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SRC)))))

LIBNAME		=libwifi.a



.PHONY:all
all: $(OBJ) lib 	
$(OBJ): %.o:%.cpp	
	$(CC) $(CFLAGS)  -c -o $@ $< $(INCLUDE)  
	
.PHONY:lib
lib: $(OBJ)
	$(AR) -rcu $(LIBNAME) $(OBJ) 
#	cp $(LIBOBJ_DIR)/$(LIBNAME) $(LIBSET_DIR) -r
#	echo $(LIBOBJ_DIR)
#	echo $(OBJ_DIR)
#	cp $(LIBOBJ_DIR)/*.o $(OBJ_DIR)
.PHONY:clean
clean:	
	rm -f $(LIBNAME)
	rm -f $(OBJ)

