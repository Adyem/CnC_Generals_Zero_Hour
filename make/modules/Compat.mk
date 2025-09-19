# Static library module for Generals/Code/compat

COMPAT_SRC_DIR := $(SRC_DIR)/compat
COMPAT_OBJ_DIR := $(OBJ_DIR)/compat
COMPAT_SOURCES_MK := $(BUILD_DIR)/compat_sources.mk

$(shell python3 build_tools/generate_make_fragment.py \
        $(COMPAT_SRC_DIR) \
        $(COMPAT_OBJ_DIR) \
        $(COMPAT_SOURCES_MK) \
        --variable-prefix=COMPAT)

include $(COMPAT_SOURCES_MK)

ifneq ($(strip $(COMPAT_OBJECTS)),)
COMPAT_LIB := $(LIB_DIR)/libcompat.a
STATIC_LIBS += $(COMPAT_LIB)

$(COMPAT_LIB): $(COMPAT_OBJECTS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^
endif

TOTAL_CPP_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_CPP_SOURCE_COUNT) + $(COMPAT_CPP_SOURCE_COUNT) )))
TOTAL_C_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_C_SOURCE_COUNT) + $(COMPAT_C_SOURCE_COUNT) )))
