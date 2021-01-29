

CROSS_COMPILE 	?= arm-none-linux-gnueabihf-
TARGET		  	?= wayz_linux_kit

CC 				:= $(CROSS_COMPILE)gcc


INCDIRS 		:= include
				   			   
SRCDIRS			:= src\
				   examples
				   
				   
INCLUDE			:= $(patsubst %,-I %, $(INCDIRS))

CFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

CFILENDIR		:= $(notdir  $(CFILES))

COBJS			:= $(patsubst %,%, $(CFILENDIR:.c=.o))
OBJS			:= $(COBJS)

VPATH			:= $(SRCDIRS)

.PHONY: clean

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ -lm -lpthread
	rm -rf $(OBJS)

$(COBJS) : %.o : %.c
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDE) -o $@ $<
	
clean:
	rm -rf $(TARGET) $(OBJS)

	
