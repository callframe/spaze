.SILENT:
.DEFAULT_GOAL := all

WORK_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
DEPS_DIR := $(WORK_DIR)/deps

DEPS_MK := $(DEPS_DIR)/deps.mk
include $(DEPS_MK)

PRINT ?= printf

RM ?= rm
RM_FLAGS := -f

CC ?= clang
CC_FLAGS := \
	-std=c17 \
	-Wall -Wextra -Werror \
	-MMD -MP -fPIC

ifeq ($(RELEASE), 1)
	CC_FLAGS += -O3
else
	CC_FLAGS += -g
endif

SOURCES := \
	$(WORK_DIR)/spaze.c

OBJECTS := $(SOURCES:.c=.o)
DEPENDS := $(SOURCES:.c=.d)

SPAZE := $(WORK_DIR)/spaze

SPAZE_LD_FLAGS := 

SPAZE_CC_FLAGS := \
	-I$(MIMALLOC_INCLUDE_DIR)

.PHONY: all
all: $(SPAZE)

$(SPAZE): CC_FLAGS += $(SPAZE_CC_FLAGS)
$(SPAZE): $(OBJECTS) $(MIMALLOC_OBJECT)
	$(PRINT) " LD $(notdir $@)\n"
	$(CC) $(CC_FLAGS) -o $@ $^ $(SPAZE_LD_FLAGS)

%.o: %.c
	$(PRINT) " CC $(notdir $@)\n"
	$(CC) $(CC_FLAGS) -c -o $@ $<

.PHONY: clean
clean::
	$(RM) $(RM_FLAGS) $(OBJECTS) $(DEPENDS) $(SPAZE)

-include $(DEPENDS)
