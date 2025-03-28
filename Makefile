#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)

include $(DEVKITPRO)/wups/share/wups_rules

WUMS_ROOT := $(DEVKITPRO)/wums
WUT_ROOT := $(DEVKITPRO)/wut

#-------------------------------------------------------------------------------
# PLUGIN_NAME sets the name of the plugin.
# PLUGIN_DESCRIPTION sets the description of the plugin.
# PLUGIN_VERSION sets the version of the plugin.
# PLUGIN_AUTHOR sets the author of the plugin.
# PLUGIN_LICENSE sets the license of the plugin.
#-------------------------------------------------------------------------------
PLUGIN_NAME    := Wii U Time Sync
PLUGIN_VERSION := v3.1.0+

#-------------------------------------------------------------------------------
# TARGET is the name of the output.
# BUILD is the directory where object files & intermediate files will be placed.
# SOURCES is a list of directories containing source code.
# DATA is a list of directories containing data files.
# INCLUDES is a list of directories containing header files.
#-------------------------------------------------------------------------------
TARGET   := Wii_U_Time_Sync
BUILD    := build
SOURCES  := source source/net external/libwupsxx/src
SOURCES_EXCLUDE := \
	external/libwupsxx/src/shortcut.cpp \
	external/libwupsxx/src/shortcut_item.cpp
DATA     := data
INCLUDES := include external/libwupsxx/include

#-------------------------------------------------------------------------------
# DEBUG sets the debug flag for the plugin.
# This should be 0 for release builds, and 1 for development/workflow builds.
# * The version string will be appended with the git hash.
# * Compiling will produce verbose logs.
#-------------------------------------------------------------------------------
DEBUG := 0

# This appends the git hash to the version string.
ifeq ($(DEBUG),1)
	GIT_HASH := $(shell git rev-parse --short HEAD)
	PLUGIN_VERSION := $(PLUGIN_VERSION)-$(GIT_HASH)
endif

#-------------------------------------------------------------------------------
# options for code generation
#-------------------------------------------------------------------------------
WARN_FLAGS := -Wall -Wextra -Wundef -Wpointer-arith -Wcast-align -Wno-odr

OPTFLAGS := -Os -fipa-pta -ffunction-sections -flto

CFLAGS := $(WARN_FLAGS) $(OPTFLAGS) $(MACHDEP)

CXXFLAGS := $(CFLAGS) -std=c++23

DEFINES := '-DPLUGIN_NAME="$(PLUGIN_NAME)"'                   \
           '-DPLUGIN_VERSION="$(PLUGIN_VERSION)"'

# Note: INCLUDE will be defined later, so CPPFLAGS has to be of the recursive flavor.
CPPFLAGS = $(INCLUDE) -D__WIIU__ -D__WUT__ -D__WUPS__  $(DEFINES)

ASFLAGS	:= -g $(ARCH)

LDFLAGS	= -g \
          $(ARCH) \
          $(RPXSPECS) \
          $(WUPSSPECS) \
          -Wl,-Map,$(notdir $*.map) \
          $(CXXFLAGS)

LIBS := -lcurlwrapper -lnotifications -lwups -lwut

#-------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level
# containing include and lib
#-------------------------------------------------------------------------------
LIBDIRS	:= $(WUMS_ROOT) $(WUPS_ROOT) $(WUT_ROOT) $(PORTLIBS)

#-------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#-------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#-------------------------------------------------------------------------------

export TOPDIR	:=	$(CURDIR)
export OUTPUT	:=	$(TOPDIR)/$(TARGET)
export VPATH	:=	$(TOPDIR)
export DEPSDIR	:=	$(TOPDIR)/$(BUILD)

CFILES		:=	$(filter-out $(SOURCES_EXCLUDE),$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c)))
CPPFILES	:=	$(filter-out $(SOURCES_EXCLUDE),$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp)))
SFILES		:=	$(filter-out $(SOURCES_EXCLUDE),$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.s)))
BINFILES	:=	$(filter-out $(SOURCES_EXCLUDE),$(foreach dir,$(DATA),$(wildcard $(dir)/*.*)))

#-------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#-------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#-------------------------------------------------------------------------------
	export LD	:=	$(CC)
#-------------------------------------------------------------------------------
else
#-------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES		:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(TOPDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

#-------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	mkdir -p $(addprefix build/,$(sort $(dir $(OFILES))))
	$(MAKE) -C $(BUILD) -f $(TOPDIR)/Makefile V=$(DEBUG)

#-------------------------------------------------------------------------------
clean:
	$(info clean ...)
	$(RM) -r $(BUILD) $(TARGET).wps $(TARGET).elf

#-------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#-------------------------------------------------------------------------------
# main targets
#-------------------------------------------------------------------------------
all	:	$(OUTPUT).wps

$(OUTPUT).wps	:	$(OUTPUT).elf
$(OUTPUT).elf	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES_BIN)

#-------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#-------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#-------------------------------------------------------------------------------
	$(info $(notdir $<))
	$(bin2o)

-include $(DEPENDS)

#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------
