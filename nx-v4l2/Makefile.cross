CFLAGS = -Wall -fPIC
INCLUDES := -I./
LDFLAGS :=
LIBS :=

CROSS_COMPILE := aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

NAME := nx-v4l2
LIB_TARGET := lib$(NAME).so

.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $^

$(LIB_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(TARGET) -o $@ $^ $(LIBS)

install: $(LIB_TARGET)
	cp $^ ../sysroot/lib
	cp nx-v4l2.h ../sysroot/include
	cp media-bus-format.h ../sysroot/include

all: $(LIB_TARGET)

.PHONY: clean

clean:
	rm -f *.o
	rm -f $(LIB_TARGET)
