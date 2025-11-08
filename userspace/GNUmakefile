CC      := gcc
CFLAGS  := -ffreestanding -O2 -Wall -Wextra -nostdlib -m64
LDFLAGS := -Ttext 0x00400000 -nostdlib -static

SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:.c=.o)
TARGET  := init.bin

SYSROOT_BIN := ../sysroot/bin

all: $(TARGET)
	mkdir -p $(SYSROOT_BIN)
	cp $(TARGET) $(SYSROOT_BIN)/

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)
