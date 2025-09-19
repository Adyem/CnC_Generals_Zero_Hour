# Static library module for Generals/Code/GameEngine

GAME_ENGINE_SRC_DIR := $(SRC_DIR)/GameEngine
GAME_ENGINE_OBJ_DIR := $(OBJ_DIR)/GameEngine
GAME_ENGINE_SOURCES_MK := $(BUILD_DIR)/GameEngine_sources.mk

$(shell python3 build_tools/generate_make_fragment.py \
        $(GAME_ENGINE_SRC_DIR) \
        $(GAME_ENGINE_OBJ_DIR) \
        $(GAME_ENGINE_SOURCES_MK) \
        --variable-prefix=GAME_ENGINE)

include $(GAME_ENGINE_SOURCES_MK)

ifneq ($(strip $(GAME_ENGINE_OBJECTS)),)
GAME_ENGINE_LIB := $(LIB_DIR)/libGameEngine.a
STATIC_LIBS += $(GAME_ENGINE_LIB)

$(GAME_ENGINE_LIB): $(GAME_ENGINE_OBJECTS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^
endif

TOTAL_CPP_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_CPP_SOURCE_COUNT) + $(GAME_ENGINE_CPP_SOURCE_COUNT) )))
TOTAL_C_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_C_SOURCE_COUNT) + $(GAME_ENGINE_C_SOURCE_COUNT) )))
