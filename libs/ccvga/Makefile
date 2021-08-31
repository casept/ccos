SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

.PHONY: clean run


include $(CCOS_PROJECT_ROOT)/toolchains/$(TARGET_TRIPLE).mk

BUILD_DIR = ./build-$(TARGET_TRIPLE)
SRC_DIRS = ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.asm)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_DIRS += ../cclibc/include ../ccnonstd/include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
ASFLAGS +=
CFLAGS  += -O0 -g

MKDIR_P ?= mkdir -p

.DEFAULT_GOAL := $(BUILD_DIR)/ccvga.a

# External deps
.PHONY: cclibc ccnonstd

cclibc:
	$(MAKE) -C ../cclibc

ccnonstd:
	$(MAKE) -C ../ccnonstd

$(BUILD_DIR)/ccvga.a: $(OBJS) cclibc ccnonstd
	$(AR) -rcs $(BUILD_DIR)/ccvga.a $(OBJS)

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

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
