
CROSS_COMPILER:= arm-hisiv100nptl-linux-
CC:=$(CROSS_COMPILER)g++	
GCC:=$(CROSS_COMPILER)gcc
AR:=$(CROSS_COMPILER)ar
CFLAGS:= -Wall  -Werror -fpic -O2 -fno-strict-aliasing  #-lstdc++

LIB = -static  -lpthread -L.

LIBOBJ_DIR=$(PROJECT_ROOT)/$(TARGET)
SRC +=$(wildcard $(LIBOBJ_DIR)/*.cpp)
SRC +=$(wildcard $(LIBOBJ_DIR)/*.c)
OBJ = $(SRC:%.cpp=%.o)
LIBSET_DIR=$(PROJECT_ROOT)/lib
OBJ_DIR=$(PROJECT_ROOT)/objects
OBJ_LIB=$(addprefix $(LIBOBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SRC)))))

LIBNAME		:= $(join lib,$(addsuffix .a,$(TARGET)))



.PHONY:all
all: $(OBJ) lib 	
$(OBJ): %.o:%.cpp	
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDE)
	
.PHONY:lib
lib: $(OBJ)
	$(AR) -rcu $(LIBNAME) $(OBJ)
	cp $(LIBOBJ_DIR)/$(LIBNAME) $(LIBSET_DIR)
	echo $(LIBOBJ_DIR)
	echo $(OBJ_DIR)
	cp $(LIBOBJ_DIR)/*.o $(OBJ_DIR)
.PHONY:clean
clean:	
	rm -f $(LIBNAME)
	rm -f $(OBJ)

