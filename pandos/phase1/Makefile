# This makefile creates a kernel file from three source files:
ifneq ($(wildcard /usr/bin/umps3),)
	UMPS3_DIR_PREFIX = /usr
	uname = $(shell uname -m)
	LIBDIR = $(UMPS3_DIR_PREFIX)/lib/$(uname)-linux-gnu/umps3
else
	UMPS3_DIR_PREFIX = /usr/local
	LIBDIR = $(UMPS3_DIR_PREFIX)/lib/umps3
endif

INCDIR = $(UMPS3_DIR_PREFIX)/include/umps3/umps
SUPDIR = $(UMPS3_DIR_PREFIX)/share/umps3

BUILDOBJS = build/objs
BUILDMACHINE = build/machine
SOURCES = p1test.c pcb.c ns.c ash.c
OBJECTS := $(SOURCES:%.c=$(BUILDOBJS)/%.o)
CFLAGS = -ffreestanding -ansi -Wall -c -mips1 -mabi=32 -mfp32 \
-mno-gpopt -G 0 -fno-pic -mno-abicalls -I ./include -std=c17
LDCOREFLAGS = -G 0 -nostdlib -T $(SUPDIR)/umpscore.ldscript

CC = mipsel-linux-gnu-gcc
LD = mipsel-linux-gnu-ld
AS = mipsel-linux-gnu-as -KPIC
EF = umps3-elf2umps
UDEV = umps3-mkdev

#main target
all: kernel.core.umps disk0.umps

# use umps3-mkdev to create the disk0 device
disk0.umps:
	@echo "Creating disk device into '$(BUILDMACHINE)/$@'"
	@$(UDEV) -d $(BUILDMACHINE)/$@ > /dev/null 2>&1

# create the kernel.core.umps kernel executable file
kernel.core.umps: kernel
	@echo "Converting '$<' ELF object file to '$@'"
	@$(EF) -k $(BUILDMACHINE)/kernel -o $(BUILDMACHINE)/$@ > /dev/null 2>&1

kernel: $(OBJECTS)
	$(eval OBJECTSFORMATTED=$(shell echo $(OBJECTS) | sed -e "s/\s\+/', '/g"))
	@echo "Linking '$(OBJECTSFORMATTED)' into '$(BUILDMACHINE)/$@' ELF object file"
	@$(LD) $(LDCOREFLAGS) $(LIBDIR)/crtso.o $(OBJECTS) \
	$(LIBDIR)/libumps.o -o $(BUILDMACHINE)/$@ > /dev/null 2>&1

$(BUILDOBJS)/%.o: src/%.c
	@echo "Compiling '$<' into '$@'"
	@$(CC) $(CFLAGS) $< -o $@ > /dev/null 2>&1

clean:
	rm -rf $(BUILDOBJS)/*.o
	rm -f $(BUILDMACHINE)/kernel $(BUILDMACHINE)/*.umps