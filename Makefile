ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment.")
endif

include $(DEVKITARM)/3ds_rules

TARGET		:=	snake3ds
BUILD		:=	build

# main.cpp liegt in source/
SOURCES		:=	source

# Header liegen optional in include/
INCLUDES	:=	include

ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mfpu=vfp -mtp=soft

CFLAGS	:=	-g -Wall -O2 $(ARCH)

CXXFLAGS := $(CFLAGS) \
			-fno-rtti \
			-fno-exceptions \
			-std=gnu++11

LIBDIRS := \
	-L$(DEVKITPRO)/libctru/lib \
	-L$(DEVKITPRO)/portlibs/3ds/lib

LDFLAGS := \
	-specs=3dsx.specs \
	-g \
	$(ARCH) \
	$(LIBDIRS)

LIBS := \
	-lcitro2d \
	-lcitro3d \
	-lctru \
	-lm

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH := \
	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR := $(CURDIR)/$(BUILD)

CPPFILES := \
	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

CFILES := \
	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))

SFILES := \
	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export OFILES := \
	$(CPPFILES:.cpp=.o) \
	$(CFILES:.c=.o) \
	$(SFILES:.s=.o)

export INCLUDE := \
	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
	-I$(DEVKITPRO)/libctru/include \
	-I$(DEVKITPRO)/portlibs/3ds/include

.PHONY: all clean

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	rm -rf $(BUILD)
	rm -f *.3dsx
	rm -f *.elf
	rm -f *.cia
	rm -f *.smdh

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).3dsx: $(OUTPUT).elf

$(OUTPUT).elf: $(OFILES)
	@echo linking $(notdir $@)
	$(CXX) $(LDFLAGS) $(OFILES) $(LIBS) -o $@

%.o: %.cpp
	@echo compiling $<
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d \
	$(CXXFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.c
	@echo compiling $<
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d \
	$(CFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.s
	@echo assembling $<
	$(AS) $(ARCH) -c $< -o $@

-include $(DEPENDS)

endif
