-include ./../common/common.mak


CPPSRC = $(wildcard *.cpp)  
CPPOBJ = $(CPPSRC:%.cpp=%.o) 

CSRC = $(wildcard *.c)  
COBJ = $(CSRC:%.c=%.o) 

DFLAGS=-DHI_DEBUG -g -DARM
#CFG_CFLAGS  =-Wall -mlittle-endian

CFLAGS += $(CFG_CFLAGS) $(DFLAGS)

LIBNAME=libtemp000000000000000.a


all: $(COBJ) $(CPPOBJ) lib

$(CPPOBJ): %.o:%.cpp	
	$(CC) $(CFLAGS) $(INCLUDE_DIR) $(LDFLAGS) -c -o $@ $< 

$(COBJ): %.o:%.c	
	$(CC) $(CFLAGS) $(INCLUDE_DIR) $(LDFLAGS) -c -o $@ $< 

	
lib: $(COBJ) $(CPPOBJ)
	$(AR) -rcu $(LIBNAME) *.o	
	cp $(LIBNAME) $(PROJECT_ROOT)/lib/
	rm *.o
	
clean:
	rm -f *.a
	rm -f *.o
	