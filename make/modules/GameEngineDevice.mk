# Static library module for Generals/Code/GameEngineDevice

GAME_ENGINE_DEVICE_SRC_DIR := $(SRC_DIR)/GameEngineDevice
GAME_ENGINE_DEVICE_OBJ_DIR := $(OBJ_DIR)/GameEngineDevice
GAME_ENGINE_DEVICE_SOURCES_MK := $(BUILD_DIR)/GameEngineDevice_sources.mk

$(shell python3 build_tools/generate_make_fragment.py \
        $(GAME_ENGINE_DEVICE_SRC_DIR) \
        $(GAME_ENGINE_DEVICE_OBJ_DIR) \
        $(GAME_ENGINE_DEVICE_SOURCES_MK) \
        --variable-prefix=GAME_ENGINE_DEVICE \
        --exclude=MilesAudioDevice)

include $(GAME_ENGINE_DEVICE_SOURCES_MK)

ifneq ($(strip $(GAME_ENGINE_DEVICE_OBJECTS)),)
GAME_ENGINE_DEVICE_LIB := $(LIB_DIR)/libGameEngineDevice.a
STATIC_LIBS += $(GAME_ENGINE_DEVICE_LIB)

$(GAME_ENGINE_DEVICE_LIB): $(GAME_ENGINE_DEVICE_OBJECTS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^
endif

TOTAL_CPP_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_CPP_SOURCE_COUNT) + $(GAME_ENGINE_DEVICE_CPP_SOURCE_COUNT) )))
TOTAL_C_SOURCE_COUNT := $(shell echo $$(( $(TOTAL_C_SOURCE_COUNT) + $(GAME_ENGINE_DEVICE_C_SOURCE_COUNT) )))
