TARGET := snake3ds
BUILD := build
SOURCES := .
INCLUDES := .

ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

LIBS := -lcitro2d -lcitro3d

include $(DEVKITPRO)/libctru/3ds_rules
