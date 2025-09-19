# Static library module for Generals/Code/Tools

TOOLS_SRC_DIR := $(SRC_DIR)/Tools
TOOLS_OBJ_DIR := $(OBJ_DIR)/Tools
TOOLS_SOURCES_MK := $(BUILD_DIR)/Tools_sources.mk

$(shell python3 build_tools/generate_make_fragment.py \
        $(TOOLS_SRC_DIR) \
        $(TOOLS_OBJ_DIR) \
        $(TOOLS_SOURCES_MK) \
        --variable-prefix=TOOLS)

include $(TOOLS_SOURCES_MK)

ifneq ($(strip $(TOOLS_OBJECTS)),)
TOOLS_LIB := $(LIB_DIR)/libTools.a
STATIC_LIBS += $(TOOLS_LIB)

$(TOOLS_LIB): $(TOOLS_OBJECTS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^
endif

TOTAL_CPP_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_CPP_SOURCE_COUNT) + $(TOOLS_CPP_SOURCE_COUNT) )))
TOTAL_C_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_C_SOURCE_COUNT) + $(TOOLS_C_SOURCE_COUNT) )))
