# Makefile for DLXSIM - DLX Simulator
# Copyright (C) 1999 Apple Computer, Inc.

.PHONY: all clean install installhdrs installsrc

# Installation directories
d = $(DSTROOT)
INSTALL_DIR = /usr/local/bin
MAN_DIR = /usr/local/share/man/man1

# Compiler and flags
CC = cc
CFLAGS = -O2 -Wall -Wno-unused
LDFLAGS =

# Source files
SRCS = src/dlxsim.c src/memory.c src/device.c
OBJS = $(SRCS:.c=.o)
HEADERS = src/dlx.h

# Target
TARGET = dlxsim

# Default target
all: $(TARGET)

# Build simulator
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

# Compile object files
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies
src/dlxsim.o: src/dlxsim.c $(HEADERS)
src/memory.o: src/memory.c $(HEADERS)
src/device.o: src/device.c $(HEADERS)

# Clean build artifacts
clean:
	$(RM) $(OBJS) $(TARGET)
	find . -name \*~ -print0 | xargs -0 $(RM)

# Install binary
install: $(TARGET)
	install -d $(d)$(INSTALL_DIR)
	install -c -s -m 755 $(TARGET) $(d)$(INSTALL_DIR)/$(TARGET)
	@if [ -d $(d)$(MAN_DIR) ]; then \
		install -c -m 644 dlxsim.1 $(d)$(MAN_DIR)/dlxsim.1 2>/dev/null || true; \
	fi

# Install headers (none for this project)
installhdrs:
	@echo "No headers to install"

# Install source
installsrc:
	install -d $(SRCROOT)
	cp -r src Makefile README.md $(SRCROOT)/

# Test
test: $(TARGET)
	@echo "Running basic tests..."
	@./$(TARGET) -h
	@echo "Build successful!"
