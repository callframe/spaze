.SILENT:
.DEFAULT_GOAL := all

WORK_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

PRINT ?= printf

RM ?= rm
RM_FLAGS := -f

CC ?= clang
CC_FLAGS := \
	-std=c17 \
	-Wall -Wextra -Werror \
	-MMD -MP

LD_FLAGS :=

SOURCES := \
	$(WORK_DIR)/spaze.c

OBJECTS := $(SOURCES:.c=.o)
DEPENDS := $(SOURCES:.c=.d)

SPAZE := $(WORK_DIR)/spaze

.PHONY: all
all: $(SPAZE)

$(SPAZE): $(OBJECTS)
	$(PRINT) " LD $(notdir $@)\n"
	$(CC) $(CC_FLAGS) -o $@ $^ $(LD_FLAGS)

%.o: %.c
	$(PRINT) " CC $(notdir $@)\n"
	$(CC) $(CC_FLAGS) -c -o $@ $<

.PHONY: clean
clean:
	$(RM) $(RM_FLAGS) $(OBJECTS) $(DEPENDS) $(SPAZE)

-include $(DEPENDS)
