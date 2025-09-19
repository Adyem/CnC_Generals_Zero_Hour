# Static library module for Generals/Code/Main

MAIN_SRC_DIR := $(SRC_DIR)/Main
MAIN_OBJ_DIR := $(OBJ_DIR)/Main
MAIN_SOURCES_MK := $(BUILD_DIR)/Main_sources.mk

$(shell python3 build_tools/generate_make_fragment.py \
        $(MAIN_SRC_DIR) \
        $(MAIN_OBJ_DIR) \
        $(MAIN_SOURCES_MK) \
        --variable-prefix=MAIN)

include $(MAIN_SOURCES_MK)

ifneq ($(strip $(MAIN_OBJECTS)),)
MAIN_LIB := $(LIB_DIR)/libMain.a
STATIC_LIBS += $(MAIN_LIB)

$(MAIN_LIB): $(MAIN_OBJECTS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^
endif

TOTAL_CPP_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_CPP_SOURCE_COUNT) + $(MAIN_CPP_SOURCE_COUNT) )))
TOTAL_C_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_C_SOURCE_COUNT) + $(MAIN_C_SOURCE_COUNT) )))
