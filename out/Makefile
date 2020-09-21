TMPOBJS += $(patsubst %.c, %.o, $(wildcard *.c))
TMPOBJS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))

OBJDIR = .objs
OBJS += $(foreach it, $(TMPOBJS), $(OBJDIR)/$(it))

INCLUDE_DIR += -I./include
CFLAGS += -g -std=c++11
CC = gcc
CPP = g++
TARGET = test

all:all_echo $(TARGET)

all_echo:
	@mkdir -p $(OBJDIR)
	@echo OBJS:$(OBJS)

$(TARGET):$(OBJS)
	$(CPP) -o $@ $(OBJS) $(CFLAGS)

$(OBJDIR)/%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE_DIR)
	
$(OBJDIR)/%.o:%.cpp
	$(CPP) $(CFLAGS) -o $@ -c $< $(INCLUDE_DIR)

clean:
	rm -rf $(TARGET) $(OBJS)
