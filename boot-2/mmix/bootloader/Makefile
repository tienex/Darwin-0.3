# MMIX Bootloader Makefile
# Copyright (c) 1999-2025 Apple Computer, Inc.

# Tools
CC = mmix-gcc
AS = mmix-as
LD = mmix-ld
OBJCOPY = mmix-objcopy

# Flags
CFLAGS = -O2 -Wall -fno-builtin -nostdinc -nostdlib -ffreestanding
ASFLAGS = -march=mmix
LDFLAGS = -nostdlib -Ttext=0x1000

# Source files
SRCS_C = main.c console.c disk.c mmu.c
SRCS_S = boot.s
OBJS = $(SRCS_S:.s=.o) $(SRCS_C:.c=.o)

# Output
TARGET = mmix_bootloader
TARGET_BIN = $(TARGET).bin

# Build rules
all: $(TARGET_BIN)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(TARGET_BIN): $(TARGET)
	$(OBJCOPY) -O binary $< $@

%.o: %.c boot.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET_BIN)

.PHONY: all clean
