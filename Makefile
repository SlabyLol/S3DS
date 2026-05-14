# =============================================
# S3DS Snake - Nintendo 3DS Homebrew
# =============================================

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=/path/to/devkitARM")
endif

include $(DEVKITARM)/3ds_rules

# ==================== Projekt-Infos ====================
APP_TITLE     := S3DS Snake
APP_AUTHOR    := DarkFox Co.
APP_VERSION   := 1.2.0
APP_PRODUCT_CODE := CTR-P-SNAK
APP_UNIQUE_ID := 0x534E414B  # SNAK

TARGET        := snake
BUILD         := build
SOURCES       := source
INCLUDES      := include

# ==================== Flags ====================
ARCH      := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mfpu=vfp -mtp=soft

CFLAGS    := -g -Wall -O2 $(ARCH)
CXXFLAGS  := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17

LIBDIRS   := -L$(DEVKITPRO)/libctru/lib

LIBS      := -lctru -lcitro2d -lcitro3d -lndsp -lm

LDFLAGS   := -specs=3dsx.specs -g $(ARCH) $(LIBDIRS)

# ==================== Dateien ====================
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export TOPDIR := $(CURDIR)
export VPATH  := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  -I$(DEVKITPRO)/libctru/include

.PHONY: all clean

all: $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

$(BUILD):
	@[ -d $@ ] || mkdir -p $@

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD) $(TARGET).3dsx $(TARGET).elf $(TARGET).smdh *.cia

else

# ==================== Build-Regeln ====================
DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).3dsx : $(OUTPUT).elf
	@echo "Creating 3DSX..."
	@3dsxtool $< $@ --smdh=$(TOPDIR)/$(TARGET).smdh

$(OUTPUT).elf : $(OFILES)
	@echo "Linking $(notdir $@)"
	@$(CXX) $(LDFLAGS) $(OFILES) $(LIBS) -o $@

%.o: %.cpp
	@echo "Compiling $<"
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.c
	@echo "Compiling $<"
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.s
	@echo "Assembling $<"
	@$(AS) $(ARCH) -c $< -o $@

-include $(DEPENDS)

endif
