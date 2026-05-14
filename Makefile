ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment.")
endif

include $(DEVKITARM)/3ds_rules

TARGET		:=	snake3ds
BUILD		:=	build
SOURCES		:=	.
INCLUDES	:=	.

ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mfpu=vfp -mtp=soft

CFLAGS	:=	-g -Wall -O2 $(ARCH)

CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

LDFLAGS := -specs=3dsx.specs -g $(ARCH)

LIBS	:= -lcitro2d -lcitro3d -lctru -lm

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(CURDIR)

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CPPFILES	:=	$(notdir $(wildcard *.cpp))

export OFILES	:=	$(CPPFILES:.cpp=.o)

export INCLUDE	:=	-I$(CURDIR)

.PHONY: all clean

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	rm -rf $(BUILD) *.3dsx *.elf *.smdh

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).3dsx: $(OUTPUT).elf

$(OUTPUT).elf: $(OFILES)
	$(CXX) $(LDFLAGS) $(OFILES) $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@

-include $(DEPENDS)

endif
