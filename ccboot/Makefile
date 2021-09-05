SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

.PHONY: clean run

include $(CCOS_PROJECT_ROOT)/toolchains/i686-unknown-elf-gcc.mk

BUILD_DIR = ./build
SRC_DIRS = ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.asm)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_DIRS += ../libs/cclibc/include ../libs/ccnonstd/include ../libs/ccelf/include ../libs/ccvga/include/
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
LDFLAGS += -nostdlib -static-libgcc -L../libs/cclibc/build-i686-unknown-elf-gcc -L../libs/ccnonstd/build-i686-unknown-elf-gcc  -L../libs/ccvga/build-i686-unknown-elf-gcc  -L../libs/ccelf/build-i686-unknown-elf-gcc -l:ccvga.a -l:ccelf.a -l:ccnonstd.a -l:cclibc.a -lgcc -T ccboot.lds


ASFLAGS +=
CFLAGS  += -std=c18
CFLAGS  += -O0 -g

MKDIR_P ?= mkdir -p

# External libs
.PHONY: ccelf ccvga cclibc ccnonstd

cclibc:
	$(MAKE) TARGET_TRIPLE=i686-unknown-elf-gcc -C ../libs/cclibc

ccnonstd:
	$(MAKE) TARGET_TRIPLE=i686-unknown-elf-gcc -C ../libs/ccnonstd

ccvga:
	$(MAKE) TARGET_TRIPLE=i686-unknown-elf-gcc -C ../libs/ccvga

ccelf:
	$(MAKE) TARGET_TRIPLE=i686-unknown-elf-gcc -C ../libs/ccelf

.DEFAULT_GOAL := $(BUILD_DIR)/ccboot.img

$(BUILD_DIR)/ccboot.elf: $(OBJS) cclibc ccvga ccelf ccnonstd
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

$(BUILD_DIR)/ccboot_boot0.bin: $(BUILD_DIR)/ccboot.elf
	$(OBJCOPY) -O binary --strip-all --only-section=.boot0.text $(BUILD_DIR)/ccboot.elf $(BUILD_DIR)/ccboot_boot0.bin

$(BUILD_DIR)/ccboot_boot1.bin: $(BUILD_DIR)/ccboot.elf
	$(OBJCOPY) -O binary --strip-all --only-section=.boot1.text $(BUILD_DIR)/ccboot.elf $(BUILD_DIR)/ccboot_boot1.bin


$(BUILD_DIR)/ccboot.bin: $(BUILD_DIR)/ccboot_boot0.bin $(BUILD_DIR)/ccboot_boot1.bin
	cat $(BUILD_DIR)/ccboot_boot0.bin $(BUILD_DIR)/ccboot_boot1.bin > $(BUILD_DIR)/ccboot.bin

$(BUILD_DIR)/ccboot.img: $(BUILD_DIR)/ccboot.bin
	dd conv=sync if=/dev/zero of=$(BUILD_DIR)/ccboot.img bs=512 count=2880
	dd conv=sync,notrunc if=$(BUILD_DIR)/ccboot.bin of=$(BUILD_DIR)/ccboot.img bs=512

clean:
	$(RM) -r $(BUILD_DIR)

clean_recursive:
	$(RM) -r $(BUILD_DIR)
	$(MAKE) TARGET_TRIPLE=i686-unknown-elf-gcc -C ../libs/cclibc clean
	$(MAKE) TARGET_TRIPLE=i686-unknown-elf-gcc -C ../libs/ccnonstd clean
	$(MAKE) TARGET_TRIPLE=i686-unknown-elf-gcc -C ../libs/ccvga clean
	$(MAKE) TARGET_TRIPLE=i686-unknown-elf-gcc -C ../libs/ccelf clean

-include $(DEPS)

run: $(BUILD_DIR)/ccboot.img
	qemu-system-i386 -machine q35 -fda $(BUILD_DIR)/ccboot.img

run_debug: $(BUILD_DIR)/ccboot.img
	qemu-system-i386 -machine q35 -fda $(BUILD_DIR)/ccboot.img -s -S
