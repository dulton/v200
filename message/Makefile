CROSS_COMPILER:= arm-hisiv100nptl-linux-
CC:=$(CROSS_COMPILER)gcc	
AR:=$(CROSS_COMPILER)ar
STRIP:=$(CROSS_COMPILER)strip
EXE_NAME:=message
export PROJECT_ROOT=$(PWD)

INCLUDE:=-I$(PROJECT_ROOT)/

CFLAGS += -DPJ_IS_LITTLE_ENDIAN=1 -DPJ_IS_BIG_ENDIAN=0  
CFLAGS += -Wall  -Werror -fpic -O2 -fno-strict-aliasing  #-lstdc++

LDFLAGS+=-lpthread -lm -ldl

SRC+=$(wildcard $(PROJECT_ROOT)/*.c)	


#目标文件
TARGETOBJ  := $(SRC:%.c=%.o)

.PHONY:all
all: $(TARGETOBJ) $(EXE_NAME)	  
$(TARGETOBJ): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCLUDE)
	
	

$(EXE_NAME): $(TARGETOBJ) 
	$(CC) $(CFLAGS) $(TARGETOBJ)  -o $(EXE_NAME)  -Xlinker --start-group $(LDFLAGS) -Xlinker --end-group
	$(STRIP)	./$(EXE_NAME)		
	$(AR) -rcu libmessage.a AdapterMessage.o

.PHONY:clean
clean:
	@for srcfile in $(TARGETOBJ); do	\
	rm -rf $$srcfile ;		\
	done

	rm -rf $(EXE_NAME) 	
	rm -rf *.o 	
	
