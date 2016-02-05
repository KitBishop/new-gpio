# This is the only thing to change in this Makefile
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# You NEED to and MUST change TOOL_BIN_DIR to the "bin" directory of your OpenWrt uClibc toolchain
TOOL_BIN_DIR=/home/kit/OmegaCross/OpenWrt-Toolchain-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

# C++ compiler
CC=$(TOOL_BIN_DIR)/mips-openwrt-linux-uclibc-g++
CCFLAGS_A=-c -O2
CCFLAGS_SO=-c -O2 -fPIC
INC=-I./hdr

# Archiver for .a library
AR=$(TOOL_BIN_DIR)/mips-openwrt-linux-uclibc-ar
ARFLAGS=-rv

# Ranlib for .a library
RANLIB=$(TOOL_BIN_DIR)/mips-openwrt-linux-uclibc-ranlib
RANLIBFLAGS=

# Flags for linking .so library
LDFLAGS=-dynamic -shared -fPIC

# Source files
SOURCES=$(shell find ./src/ -name "*.cpp" | sed s:./src/:: | tr "\n" " ")
OBJECTS_A=$(patsubst %.cpp,build/static/%.o,$(SOURCES))
OBJECTS_SO=$(patsubst %.cpp,build/dynamic/%.o,$(SOURCES))

# Main build targets
TARGET_BASE=dist/libnew-gpio
TARGET_A=$(TARGET_BASE).a
TARGET_SO=$(TARGET_BASE).so

# Default target - build everything
all: static dynamic

# Build just static .a library
static: precomp $(TARGET_A)

#Build just dynamic .so library
dynamic: precomp $(TARGET_SO)

# Create and process .a library
$(TARGET_A): $(OBJECTS_A)
	$(AR) $(ARFLAGS) $(TARGET_A) $(OBJECTS_A)
	$(RANLIB) $(TARGET_A)

# Create .so library
$(TARGET_SO): $(OBJECTS_SO)
	$(CC) $(LDFLAGS) -o $(TARGET_SO) $(OBJECTS_SO)
	
# Compile sources for .a library
build/static/%.o: ./src/%.cpp
	$(CC) $(CCFLAGS_A) $(INC) $< -o $@

# Compile sources for .so library
build/dynamic/%.o: ./src/%.cpp
	$(CC) $(CCFLAGS_SO) $(INC) $< -o $@

# Remove all created files
clean-static:
	rm -rf build/static/*
	rm -rf $(TARGET_A)

clean-dynamic:
	rm -rf build/dynamic/*
	rm -rf $(TARGET_SO)

clean: clean-static clean-dynamic
	rm -rf build/*
	rm -rf dist/*

# Ensure required sub-directories are created
precomp:
	@mkdir -p build
	@mkdir -p build/static
	@mkdir -p build/dynamic
	@mkdir -p dist
	
