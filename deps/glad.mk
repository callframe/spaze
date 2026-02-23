GLAD_DIR := $(DEPS_DIR)/glad

GLAD_SOURCE := $(GLAD_DIR)/glad.c
GLAD_OBJECT := $(GLAD_SOURCE:.c=.o)
GLAD_DEPEND := $(GLAD_SOURCE:.c=.d)

GLAD_CC_FLAGS := \
	-I$(GLAD_DIR)

$(GLAD_OBJECT): CC_FLAGS += $(GLAD_CC_FLAGS)

$(GLAD_OBJECT): $(GLAD_SOURCE)
	$(PRINT) " CC $(notdir $@)\n"
	$(CC) $(CC_FLAGS) -c -o $@ $<

-include $(GLAD_DEPEND)

.PHONY: clean
clean::
	$(RM) $(RM_FLAGS) $(GLAD_OBJECT) $(GLAD_DEPEND)