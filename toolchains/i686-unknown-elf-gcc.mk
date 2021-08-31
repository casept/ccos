TC_PREFIX = i686-unknown-elf-
TC_PATH = $(CCOS_PROJECT_ROOT)/toolchains/i686-unknown-elf/bin

AS = $(TC_PATH)/$(TC_PREFIX)as
CC = $(TC_PATH)/$(TC_PREFIX)gcc
CXX = $(TC_PATH)/$(TC_PREFIX)g++
LD = $(TC_PATH)/$(TC_PREFIX)ld
AR = $(TC_PATH)/$(TC_PREFIX)ar
OBJCOPY = $(TC_PATH)/$(TC_PREFIX)objcopy

CFLAGS = -nostdlib -ffreestanding -lgcc -Wall -Wextra -Wpedantic -Werror -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wunreachable-code -Wwrite-strings -fdiagnostics-color=always
CXXFLAGS += -nostdlib -ffreestanding -lgcc -fno-exceptions -fno-rtti -fno-unwind-tables -fno-use-cxa-atexit -fno-threadsafe-statics -Wall -Wextra -Wpedantic -Werror -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wunreachable-code -Wwrite-strings -Wctor-dtor-privacy -Wold-style-cast -Wstrict-null-sentinel -Woverloaded-virtual -Wsign-promo -Wnoexcept -Wzero-as-null-pointer-constant -Wuseless-cast -fdiagnostics-color=always
