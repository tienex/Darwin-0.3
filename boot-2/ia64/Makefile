# Makefile for IA64 EFI Bootloader

# Cross-compiler settings
CC = ia64-linux-gnu-gcc
LD = ia64-linux-gnu-ld
OBJCOPY = ia64-linux-gnu-objcopy

# Compiler flags
CFLAGS = -O2 -Wall -Wextra -fno-strict-aliasing \
         -fno-stack-protector -fno-builtin \
         -nostdinc -nostdlib \
         -DEFI_FUNCTION_WRAPPER \
         -I./efi -I../gen

# Linker flags for EFI application
LDFLAGS = -nostdlib -znocombreloc -T efi/efi_ia64.lds \
          -shared -Bsymbolic

# Source files
EFI_SRCS = efi/efi_main.c \
           efi/pal_sal.c \
           efi/ia64_vm.c \
           efi/efi_support.c

EFI_OBJS = $(EFI_SRCS:.c=.o)

# Target
TARGET = BOOTIA64.EFI

all: $(TARGET)

# Build EFI application
$(TARGET): $(EFI_OBJS)
	$(LD) $(LDFLAGS) -o bootia64.so $(EFI_OBJS)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic \
	           -j .dynsym -j .rel -j .rela -j .reloc \
	           --target=efi-app-ia64 bootia64.so $(TARGET)
	@echo "Built $(TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EFI_OBJS) bootia64.so $(TARGET)

install: $(TARGET)
	@echo "Copy $(TARGET) to EFI System Partition:"
	@echo "  mkdir -p /boot/efi/EFI/DARWIN"
	@echo "  cp $(TARGET) /boot/efi/EFI/DARWIN/"

.PHONY: all clean install
