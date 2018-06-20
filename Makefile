
# ------------------------------------------------------------------------------
#
# Command line macros:
#
# INSTALL_TOP ... place to put includes and library files
# LUA_HOME    ... where to find lua header and library files 
#
# If this is a local build, then you should set the environment variable
# LUA_CPATH to the local directory. On bash, this will work:
#
# export LUA_CPATH="$LUA_CPATH;`pwd`/lib/?.so"
#
# Absolute paths may be used instead, which will be useful if you are doing a
# remote install.
#
# ------------------------------------------------------------------------------



# System-specific things.
# ------------------------------------------------------------------------------

# C compiler icc, gcc, etc if other than system default
# CC = cc
CC = arm-epoc-pe-gcc -Znoemx

# C++ compiler icpc, g++, etc if other than system default
# CXX = c++
CXX = arm-epoc-pe-gcc -Znoemx

# sometimes only -fpic works
# FPIC = -fPIC # but not in epoc32-pe

# Warning level flags
WARN = -Wall

# robust optimazation level
OPTIM = -O2 -fomit-frame-pointer

# debug flags, use -g for debug symbols
DEBUG =

# location of Lua install on this system
$(eval LUA_INC ?= $(PWD)/../..)

# where to install lunum library and include
INSTALL_TOP = $(PWD)

# C Flags
CFLAGS = $(WARN) $(OPTIM) $(DEBUG) $(FPIC) -DLUNUM_API_NOCOMPLEX


# Configuration for common platforms. If you need to use a different linker,
# archiver, or C libraries then uncomment the UNAME = Custom line below, and
# edit the custom first block following.
# ------------------------------------------------------------------------------
#UNAME = $(shell uname)
UNAME = EPOC32
# ------------------------------------------------------------------------------
#
#
ifeq ($(UNAME), Custom)
# common for library links on Linux
CLIBS = -lm -ldl
# command for building shared libraries (this works for most Linux systems)
SO = $(CC) -O -shared
# command for generating static archives
AR = ar rcu
endif

ifeq ($(UNAME), Linux)
SO     = $(CC) -O -shared
AR     = ar rcu
CLIBS  = -lm -ldl
endif

ifeq ($(UNAME), Darwin)
SO     = $(CC) -O -bundle -undefined dynamic_lookup
AR     = ar rcu
CLIBS  =
endif

ifeq ($(UNAME), EPOC32)
SO     = $(CC) -shared \
	 -L../../.. -uid2 0x4c756121 -uid3  "0x10`echo lunum | md5sum | cut -c1-6`"
CLIBS  = -llua53 -lestlib
DLLTOOL= arm-epoc-pe-dlltool
endif


# -------------------------------------------------
# Ensure these values are passed to child Makefiles
# -------------------------------------------------
export CC
export CXX
export CFLAGS
export LUA_INC
export SO
export AR
export CLIBS
export DLLTOOL
# -------------------------------------------------


BUILD_TOP   = $(shell pwd)
LIB_SO      = lunum.dll

export LUNUM_SO = $(BUILD_TOP)/src/$(LIB_SO)

INSTALL_SO  = $(INSTALL_TOP)/lib/$(LIB_SO)

H1 = lunum.h
H2 = numarray.h

HEADERS = \
	$(INSTALL_TOP)/include/$(H1) \
	$(INSTALL_TOP)/include/$(H2)


default : $(LUNUM_SO)

config : 
	@echo "CC           = $(CC)"
	@echo "CXX          = $(CXX)"
	@echo "FPIC         = $(FPIC)"
	@echo "WARN         = $(WARN)"
	@echo "OPTIM        = $(OPTIM)"
	@echo "DEBUG        = $(DEBUG)"
	@echo "AR           = $(AR)"
	@echo "SO           = $(SO)"
	@echo "LUA_INC      = $(LUA_INC)"
	@echo "INSTALL_TOP  = $(INSTALL_TOP)"

test : $(LUNUM_SO)

all : default test

install : $(INSTALL_SO) $(HEADERS)

$(INSTALL_TOP)/include/$(H1) : 
	mkdir -p $(INSTALL_TOP)/include
	cp src/$(H1) $(INSTALL_TOP)/include

$(INSTALL_TOP)/include/$(H2) : 
	mkdir -p $(INSTALL_TOP)/include
	cp src/$(H2) $(INSTALL_TOP)/include

$(LUNUM_SO) : FORCE
	@make -C src $(LUNUM_SO)

test : FORCE
	@make -C test

$(INSTALL_SO) : $(LUNUM_SO)
	mkdir -p $(INSTALL_TOP)/lib
	cp $(LUNUM_SO) $(INSTALL_TOP)/lib

clean :
	make -C test clean
	make -C src clean
	rm -rf lib include

FORCE :
