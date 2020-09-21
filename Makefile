SRCSp += gjson.cpp
SRCSc += base/cJSON.c
SRCSp += base/json_base.cpp

OBJS += $(patsubst %.c, %.o, $(SRCSc))
OBJS += $(patsubst %.cpp, %.o, $(SRCSp))

INCLUDE_DIR += -I./base
CFLAGS += -g -std=c++11
CC = gcc
CPP = g++
TARGET = gjson

all:all_echo $(TARGET)

all_echo:
	echo OBJS:$(OBJS)

$(TARGET):$(OBJS)
	$(CPP) -o $@ $(OBJS) $(CFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE_DIR)
	
%.o:%.cpp
	$(CPP) $(CFLAGS) -o $@ -c $< $(INCLUDE_DIR)

clean:
	rm -rf $(TARGET) $(OBJS)
