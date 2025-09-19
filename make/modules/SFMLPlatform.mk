# Static library module for Generals/Code/SFMLPlatform

SFML_PLATFORM_SRC_DIR := $(SRC_DIR)/SFMLPlatform
SFML_PLATFORM_OBJ_DIR := $(OBJ_DIR)/SFMLPlatform
SFML_PLATFORM_SOURCES_MK := $(BUILD_DIR)/SFMLPlatform_sources.mk

$(shell python3 build_tools/generate_make_fragment.py \
        $(SFML_PLATFORM_SRC_DIR) \
        $(SFML_PLATFORM_OBJ_DIR) \
        $(SFML_PLATFORM_SOURCES_MK) \
        --variable-prefix=SFML_PLATFORM)

include $(SFML_PLATFORM_SOURCES_MK)

ifneq ($(strip $(SFML_PLATFORM_OBJECTS)),)
SFML_PLATFORM_LIB := $(LIB_DIR)/libSFMLPlatform.a
STATIC_LIBS += $(SFML_PLATFORM_LIB)

$(SFML_PLATFORM_LIB): $(SFML_PLATFORM_OBJECTS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^
endif

TOTAL_CPP_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_CPP_SOURCE_COUNT) + $(SFML_PLATFORM_CPP_SOURCE_COUNT) )))
TOTAL_C_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_C_SOURCE_COUNT) + $(SFML_PLATFORM_C_SOURCE_COUNT) )))
