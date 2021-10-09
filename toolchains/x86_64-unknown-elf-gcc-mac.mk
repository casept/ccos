TC_PREFIX = x86_64-unknown-elf-
TC_PATH = $(CCOS_PROJECT_ROOT)/toolchains/x86_64-unknown-elf/bin

PREFIX_TOOLCHAIN=x86_64-elf-
AS = $(PREFIX_TOOLCHAIN)as
CC = $(PREFIX_TOOLCHAIN)gcc
CXX = $(PREFIX_TOOLCHAIN)g++
LD = $(PREFIX_TOOLCHAIN)ld
AR = $(PREFIX_TOOLCHAIN)ar
OBJCOPY = $(PREFIX_TOOLCHAIN)objcopy

AS ?= $(TC_PATH)/$(TC_PREFIX)as
CC ?= $(TC_PATH)/$(TC_PREFIX)gcc
CXX ?= $(TC_PATH)/$(TC_PREFIX)g++
LD ?= $(TC_PATH)/$(TC_PREFIX)ld
AR ?= $(TC_PATH)/$(TC_PREFIX)ar
OBJCOPY ?= $(TC_PATH)/$(TC_PREFIX)objcopy

CFLAGS = -nostdlib -ffreestanding -Wall -Wextra -Wpedantic -Werror -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wunreachable-code -Wwrite-strings -fdiagnostics-color=always -mno-red-zone -mcmodel=kernel
CXXFLAGS += -nostdlib -ffreestanding -fno-exceptions -fno-rtti -fno-unwind-tables -fno-use-cxa-atexit -fno-threadsafe-statics -Wall -Wextra -Wpedantic -Werror -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wunreachable-code -Wwrite-strings -Wctor-dtor-privacy -Wold-style-cast -Wstrict-null-sentinel -Woverloaded-virtual -Wsign-promo -Wnoexcept -Wzero-as-null-pointer-constant -Wuseless-cast -fdiagnostics-color=always -mno-red-zone -mcmodel=kernel
