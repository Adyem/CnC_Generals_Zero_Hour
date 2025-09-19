# Static library module for Generals/Code/Libraries

LIBRARIES_SRC_DIR := $(SRC_DIR)/Libraries
LIBRARIES_OBJ_DIR := $(OBJ_DIR)/Libraries
LIBRARIES_SOURCES_MK := $(BUILD_DIR)/Libraries_sources.mk

$(shell python3 build_tools/generate_make_fragment.py \
        $(LIBRARIES_SRC_DIR) \
        $(LIBRARIES_OBJ_DIR) \
        $(LIBRARIES_SOURCES_MK) \
        --variable-prefix=LIBRARIES)

include $(LIBRARIES_SOURCES_MK)

ifneq ($(strip $(LIBRARIES_OBJECTS)),)
LIBRARIES_LIB := $(LIB_DIR)/libLibraries.a
STATIC_LIBS += $(LIBRARIES_LIB)

$(LIBRARIES_LIB): $(LIBRARIES_OBJECTS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^
endif

TOTAL_CPP_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_CPP_SOURCE_COUNT) + $(LIBRARIES_CPP_SOURCE_COUNT) )))
TOTAL_C_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_C_SOURCE_COUNT) + $(LIBRARIES_C_SOURCE_COUNT) )))
