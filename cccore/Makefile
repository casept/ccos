SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

.PHONY: clean run run_limine run_debug run_limine_debug

include $(CCOS_PROJECT_ROOT)/toolchains/x86_64-unknown-elf-gcc.mk

BUILD_DIR = ./build
SRC_DIRS = ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.asm)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_DIRS += ../libs/ccvga/include/
INC_DIRS += ../libs/ccnonstd/include/
INC_DIRS += ../libs/cclibc/include/
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS += -std=gnu2x
CFLAGS += -O0 -g
CFLAGS += -mcmodel=kernel
CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
# FIXME: Recompile with libgcc without redzone
LDFLAGS += -nostdlib -L../libs/cclibc/build-x86_64-unknown-elf-gcc
LDFLAGS += -L../libs/ccnonstd/build-x86_64-unknown-elf-gcc
LDFLAGS +=  -L../libs/ccelf/build-x86_64-unknown-elf-gcc
LDFLAGS += -L../libs/ccvga/build-x86_64-unknown-elf-gcc
LDFLAGS += -l:ccelf.a -l:ccvga.a -l:ccnonstd.a -l:cclibc.a
LDFLAGS += -T cccore.lds -zmax-page-size=0x1000 -static -ztext -mcmodel=kernel
ASFLAGS +=

MKDIR_P ?= mkdir -p

.DEFAULT_GOAL: $(BUILD_DIR)/cccore.img $(BUILD_DIR)/cccore_limine.img

# External libs
.PHONY: ccelf ccvga cclibc ccnonstd

cclibc:
	$(MAKE) TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/cclibc

ccnonstd:
	$(MAKE) TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/ccnonstd

ccvga:
	$(MAKE) TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/ccvga

ccelf:
	$(MAKE) TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/ccelf

.DEFAULT_GOAL := $(BUILD_DIR)/cccore.img

$(BUILD_DIR)/cccore.elf: $(OBJS) cclibc ccnonstd ccvga ccelf
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.asm.o: %.asm
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


# Bootloader built using recursive make
.PHONY: ccboot
ccboot:
	$(MAKE) -C ../ccboot/

# For the homegrown ccboot bootloader
$(BUILD_DIR)/cccore.img: $(BUILD_DIR)/cccore.elf ccboot
	cp ../ccboot/build/ccboot.img $(BUILD_DIR)/cccore.img
	dd conv=sync,notrunc if=$(BUILD_DIR)/cccore.elf of=$(BUILD_DIR)/cccore.img bs=512 seek=19

# This currently requires root because the filesystem has to be mounted for files to be placed on it.
# I tried to bypass this with the e2tools utilities, but these can't deal with offsets into the filesystem
# (a requirement because the MBR at the start of the image is not part of the partition).
# This could probably be worked around by formatting the file,
# using dd to copy only the partition to a separate file,
# manipulating that using e2tools and then merging it back together.
$(BUILD_DIR)/cccore_limine.img: $(BUILD_DIR)/cccore.elf limine/limine.cfg limine/limine.sys
	dd conv=sync if=/dev/zero of=$(BUILD_DIR)/cccore_limine.img bs=512 count=20480
	echo 'type=83' | sfdisk $(BUILD_DIR)/cccore_limine.img
	limine-install $(BUILD_DIR)/cccore_limine.img
	mkfs.ext2 -F -E offset=1048576 $(BUILD_DIR)/cccore_limine.img
	sudo $(MKDIR_P) /cccore
	sudo mount -o loop,offset=1048576 $(BUILD_DIR)/cccore_limine.img /cccore
	sudo $(MKDIR_P) /cccore/boot
	sudo cp limine/limine.cfg /cccore/boot/
	sudo cp limine/limine.sys /cccore/boot/
	sudo cp $(BUILD_DIR)/cccore.elf /cccore/boot/
	sudo umount /cccore

clean:
	$(RM) -r $(BUILD_DIR)

clean_recursive:
	$(RM) -r $(BUILD_DIR)
	$(MAKE) TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/cclibc clean
	$(MAKE) TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/ccnonstd clean
	$(MAKE) TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/ccvga clean
	$(MAKE) TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/ccelf clean

-include $(DEPS)

run: $(BUILD_DIR)/cccore.img
	qemu-system-x86_64 -machine q35 -fda $(BUILD_DIR)/cccore.img

run_debug: $(BUILD_DIR)/cccore.img
	qemu-system-x86_64 -machine q35 -fda $(BUILD_DIR)/cccore.img -s -S

run_limine: $(BUILD_DIR)/cccore_limine.img
	qemu-system-x86_64 -machine q35 -d guest_errors,cpu_reset -no-reboot -hda $(BUILD_DIR)/cccore_limine.img

run_limine_debug: $(BUILD_DIR)/cccore_limine.img
	qemu-system-x86_64 -machine q35 -hda $(BUILD_DIR)/cccore_limine.img -s -S

run_limine_bochs: $(BUILD_DIR)/cccore_limine.img
	bochs -q
