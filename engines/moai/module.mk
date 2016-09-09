MODULE := engines/moai

MODULE_OBJS = \
	detection.o \
	moai.o

# This module can be built as a plugin
ifeq ($(ENABLE_MOAI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

