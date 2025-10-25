#
# Makefile for DLX Architecture Support
#

VPATH = .:$(SRCROOT)/machdep/dlx

# C Sources
CFILES = \
	pmap_complete.c \
	trap.c \
	machdep.c \
	clock.c \
	vm_machdep.c \
	dlx_init.c

# Assembly Sources
SFILES = \
	start.s \
	locore.s

OBJS = $(CFILES:.c=.o) $(SFILES:.s=.o)

# Compiler flags
CFLAGS = -O2 -Wall -I$(SRCROOT) -I$(SRCROOT)/mach -I$(SRCROOT)/machdep/dlx \
	-DKERNEL -D__DLX__ -fno-builtin

ASFLAGS = -I$(SRCROOT)

# Build rules
all: $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $<

.s.o:
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -f $(OBJS)

pmap_complete.o: pmap_complete.c pmap.h
trap.o: trap.c trap.h pmap.h
machdep.o: machdep.c pmap.h
clock.o: clock.c trap.h
vm_machdep.o: vm_machdep.c pmap.h
dlx_init.o: dlx_init.c pmap.h

start.o: start.s
locore.o: locore.s
