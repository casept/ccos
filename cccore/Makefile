SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

.PHONY: clean run

include $(CCOS_PROJECT_ROOT)/toolchains/x86_64-unknown-elf-gcc.mk

BUILD_DIR = ./build
SRC_DIRS = ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.asm)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
# FIXME: Recompile with libgcc without redzone
LDFLAGS += -nostdlib -L../libs/cclibc/build-x86_64-unknown-elf-gcc -L../libs/ccnonstd/build-x86_64-unknown-elf-gcc -L../libs/ccelf/build-x86_64-unknown-elf-gcc -L../libs/ccvga/build-x86_64-unknown-elf-gcc -l:ccelf.a -l:ccvga.a -l:cclibc.a -l:ccnonstd.a -T cccore.lds
ASFLAGS +=

MKDIR_P ?= mkdir -p

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

# Append the kernel to the bootloader (starting at second cylinder)
$(BUILD_DIR)/cccore.img: $(BUILD_DIR)/cccore.elf ccboot
	cp ../ccboot/build/ccboot.img $(BUILD_DIR)/cccore.img
	dd conv=sync,notrunc if=$(BUILD_DIR)/cccore.elf of=$(BUILD_DIR)/cccore.img bs=512 seek=19

clean:
	$(RM) -r $(BUILD_DIR)

clean_recursive:
	$(RM) -r $(BUILD_DIR)
	make TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/cclibc clean
	make TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/ccnonstd clean
	make TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/ccvga clean
	make TARGET_TRIPLE=x86_64-unknown-elf-gcc -C ../libs/ccelf clean

-include $(DEPS)

run: $(BUILD_DIR)/cccore.img
	qemu-system-x86_64 -machine q35 -fda $(BUILD_DIR)/cccore.img

run_debug: $(BUILD_DIR)/cccore.img
	qemu-system-x86_64 -machine q35 -fda $(BUILD_DIR)/cccore.img -s -S
