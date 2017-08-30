# The name of your project (used to name the compiled .hex file)
TARGET = $(notdir $(CURDIR))

# The teensy version to use, 30, 31, or LC
TEENSY = 31

# Set to 24000000, 48000000, or 96000000 to set CPU core speed
TEENSY_CORE_SPEED = 96000000

# configurable options
OPTIONS = -DUSB_RAWHID -DLAYOUT_US_ENGLISH

# directory to build in
BUILDDIR = $(abspath $(CURDIR)/build)

TEENSYDUINODIR = /usr/share/arduino/hardware

#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

# path location for Teensy Loader, teensy_post_compile and teensy_reboot
TOOLSPATH = $(TEENSYDUINODIR)/tools

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	TOOLSPATH = /Applications/Arduino.app/Contents/Java/hardware/tools/
endif

# path location for Teensy 3 core
COREPATH = $(TEENSYDUINODIR)/teensy/avr/cores/teensy3

# path location for Arduino libraries
LIBRARYPATH = $(TEENSYDUINODIR)/teensy/avr/libraries

# path location for the arm-none-eabi compiler
COMPILERPATH = $(TOOLSPATH)/arm/bin

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -Os -mthumb -ffunction-sections -fdata-sections -MMD $(OPTIONS) -DTEENSYDUINO=124 -DF_CPU=$(TEENSY_CORE_SPEED) -Isrc -I$(COREPATH) -mfloat-abi=soft #-nostdlib

# compiler options for C++ only
CXXFLAGS = -std=gnu++11 -felide-constructors -fno-exceptions -fno-rtti

# compiler options for C only
CFLAGS =

# linker options
LDFLAGS = -Os -Wl,--gc-sections -mthumb

# additional libraries to link
LIBS = -lm -lstdc++

# compiler options specific to teensy version
ifeq ($(TEENSY), 30)
	CPPFLAGS += -D__MK20DX128__ -mcpu=cortex-m4
	LDSCRIPT = $(COREPATH)/mk20dx128.ld
	LDFLAGS += -mcpu=cortex-m4 -T$(LDSCRIPT)
else
	ifeq ($(TEENSY), 31)
		CPPFLAGS += -D__MK20DX256__ -mcpu=cortex-m4
		LDSCRIPT = $(COREPATH)/mk20dx256.ld
		LDFLAGS += -mcpu=cortex-m4 -T$(LDSCRIPT)
	else
		ifeq ($(TEENSY), LC)
			CPPFLAGS += -D__MKL26Z64__ -mcpu=cortex-m0plus
			LDSCRIPT = $(COREPATH)/mkl26z64.ld
			LDFLAGS += -mcpu=cortex-m0plus -T$(LDSCRIPT)
			LIBS += -larm_cortexM0l_math
		else
			$(error Invalid setting for TEENSY)
		endif
	endif
endif

# set arduino define if given
ifdef ARDUINO
		CPPFLAGS += -DARDUINO=$(ARDUINO)
else
		CPPFLAGS += -DUSING_MAKEFILE
endif

# names for the compiler programs
CC = $(abspath $(COMPILERPATH))/arm-none-eabi-gcc
CXX = $(abspath $(COMPILERPATH))/arm-none-eabi-g++
OBJCOPY = $(abspath $(COMPILERPATH))/arm-none-eabi-objcopy
SIZE = $(abspath $(COMPILERPATH))/arm-none-eabi-size

# automatically create lists of the sources and objects
LC_FILES := $(wildcard $(LIBRARYPATH)/SPI/*.c)
LCPP_FILES := $(wildcard $(LIBRARYPATH)/SPI/*.cpp)
TC_FILES := $(wildcard $(COREPATH)/*.c)
TCPP_FILES := $(wildcard $(COREPATH)/*.cpp)
# Remove default main.cpp included in newer Teensyduino versions
TCPP_FILES := $(filter-out $(COREPATH)/main.cpp, $(TCPP_FILES))
C_FILES := $(wildcard src/*.c) $(wildcard src/*/*.c)
CPP_FILES := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
INO_FILES := $(wildcard src/*.ino)

# include paths for libraries
L_INC := $(foreach lib,$(filter %/, $(wildcard $(LIBRARYPATH)/*/)), -I$(lib))

SOURCES := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o) $(INO_FILES:.ino=.o) $(TC_FILES:.c=.o) $(TCPP_FILES:.cpp=.o) $(LC_FILES:.c=.o) $(LCPP_FILES:.cpp=.o)
OBJS := $(foreach src,$(SOURCES), $(BUILDDIR)/$(src))

all: hex

build: $(TARGET).elf

hex: $(TARGET).hex

post_compile: $(TARGET).hex
		@$(abspath $(TOOLSPATH))/teensy_post_compile -file="$(basename $<)" -path=$(CURDIR) -tools="$(abspath $(TOOLSPATH))"

reboot:
		@-$(abspath $(TOOLSPATH))/teensy_reboot

upload: post_compile reboot

$(BUILDDIR)/%.o: %.c
		@echo "[CC]\t$<"
		@mkdir -p "$(dir $@)"
		@$(CC) $(CPPFLAGS) $(CFLAGS) $(L_INC) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.cpp
		@echo "[CXX]\t$<"
		@mkdir -p "$(dir $@)"
		@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.ino
		@echo "[CXX]\t$<"
		@mkdir -p "$(dir $@)"
		@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -x c++ -include Arduino.h -c "$<"

$(TARGET).elf: $(OBJS) $(LDSCRIPT)
		@echo "[LD]\t$@"
		@$(CC) $(LDFLAGS) -o "$@" $(OBJS) $(LIBS)

%.hex: %.elf
		@echo "[HEX]\t$@"
		@$(SIZE) "$<"
		@$(OBJCOPY) -O ihex -R .eeprom "$<" "$@"

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
		@echo Cleaning...
		@rm -rf "$(BUILDDIR)"
		@rm -f "$(TARGET).elf" "$(TARGET).hex"
