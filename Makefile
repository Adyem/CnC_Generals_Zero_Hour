# --- snip: your existing header/toolchain unchanged ---

# Choose executable extension on Windows
EXEEXT :=
ifeq ($(OS),Windows_NT)
EXEEXT := .exe
endif

SRC_DIR     := Generals/Code
BUILD_DIR   := build
OBJ_DIR     := $(BUILD_DIR)/obj
BIN_DIR     := $(BUILD_DIR)/bin
LIB_DIR     := $(BUILD_DIR)/lib
TARGET      ?= $(BIN_DIR)/generals-sfml$(EXEEXT)

# Core include search paths shared by every module
INCLUDE_DIRS := \
        $(SRC_DIR) \
        $(SRC_DIR)/GameEngine/Include \
        $(SRC_DIR)/GameEngine/Include/Precompiled \
        $(SRC_DIR)/GameEngineDevice/Include \
        $(SRC_DIR)/Libraries/Include \
        $(SRC_DIR)/Libraries/Source/WWVegas/WWLib \
        $(SRC_DIR)/Libraries/Source/WWVegas/WWMath \
        $(SRC_DIR)/Libraries/Source/WWVegas \
        $(SRC_DIR)/Main \
        $(SRC_DIR)/Main/Include

CPPFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

# --- snip: your include-path setup unchanged ---

# -----------------------------------------------------------------------------
# Static library modules
# -----------------------------------------------------------------------------
TOTAL_CPP_SOURCE_COUNT := 0
TOTAL_C_SOURCE_COUNT   := 0
STATIC_LIBS            :=

# Make module includes non-fatal and allow forcing fallback
-include make/modules/SFMLPlatform.mk
-include make/modules/Compat.mk
-include make/modules/Main.mk
-include make/modules/GameEngine.mk
-include make/modules/GameEngineDevice.mk
-include make/modules/Libraries.mk
-include make/modules/Tools.mk

# Set FORCE_FALLBACK=1 to ignore STATIC_LIBS and build monolithically
FORCE_FALLBACK ?= 0

# --- snip: SFML and libVLC detection blocks unchanged ---

# -----------------------------------------------------------------------------
# Monolithic fallback build (auto-scans sources)
# -----------------------------------------------------------------------------
EXCLUDE_PATTERNS ?= %/DirectX% %/DX8% %/dx8% %/win32/% %/Windows/%

RAW_CPP_SOURCES := $(shell find $(SRC_DIR) -type f -name '*.cpp' -print)
RAW_C_SOURCES   := $(shell find $(SRC_DIR) -type f -name '*.c'   -print)

CPP_SOURCES := $(RAW_CPP_SOURCES)
C_SOURCES   := $(RAW_C_SOURCES)
ifneq ($(strip $(EXCLUDE_PATTERNS)),)
  CPP_SOURCES := $(filter-out $(EXCLUDE_PATTERNS),$(CPP_SOURCES))
  C_SOURCES   := $(filter-out $(EXCLUDE_PATTERNS),$(C_SOURCES))
endif

REL_CPP_SOURCES := $(patsubst $(SRC_DIR)/%,%,$(CPP_SOURCES))
REL_C_SOURCES   := $(patsubst $(SRC_DIR)/%,%,$(C_SOURCES))

CPP_OBJS := $(addprefix $(OBJ_DIR)/,$(REL_CPP_SOURCES:.cpp=.o))
C_OBJS   := $(addprefix $(OBJ_DIR)/,$(REL_C_SOURCES:.c=.o))

TOTAL_CPP_SOURCE_COUNT := $(words $(CPP_SOURCES))
TOTAL_C_SOURCE_COUNT   := $(words $(C_SOURCES))

# Pattern rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# -----------------------------------------------------------------------------
# Primary targets
# -----------------------------------------------------------------------------
.PHONY: all clean distclean print-config install-deps

all: $(TARGET)

# Guard: If there is literally nothing to build, stop with a helpful error.
ifeq ($(strip $(FORCE_FALLBACK)),1)
  WANT_FALLBACK := 1
else ifeq ($(strip $(STATIC_LIBS)),)
  WANT_FALLBACK := 1
else
  WANT_FALLBACK := 0
endif

ifeq ($(WANT_FALLBACK),1)
  ifeq ($(strip $(TOTAL_CPP_SOURCE_COUNT) $(TOTAL_C_SOURCE_COUNT)),)
    $(error No source files found under '$(SRC_DIR)'. Fix SRC_DIR or EXCLUDE_PATTERNS, or set FORCE_FALLBACK=1 after adding sources)
  endif

# Fallback: link discovered objects into a real executable
$(TARGET): $(CPP_OBJS) $(C_OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "[LINK] $@"
	$(CXX) $(CPP_OBJS) $(C_OBJS) $(LDFLAGS) $(LIBS) -o $@
else
# Modular: link produced static libs
$(TARGET): $(STATIC_LIBS)
	@mkdir -p $(BIN_DIR)
	@echo "[LINK] $@ (modular)"
	$(CXX) $(LDFLAGS) -Wl,--start-group $(STATIC_LIBS) -Wl,--end-group $(LIBS) -o $@
endif

# -----------------------------------------------------------------------------
# Utilities
# -----------------------------------------------------------------------------
print-config:
	@echo "TARGET            = $(TARGET)"
	@echo "OS                = $(OS)"
	@echo "EXEEXT            = $(EXEEXT)"
	@echo "FORCE_FALLBACK    = $(FORCE_FALLBACK)"
	@echo "Static libs       = $(STATIC_LIBS)"
	@echo "Fallback C++ files= $(TOTAL_CPP_SOURCE_COUNT)"
	@echo "Fallback C files  = $(TOTAL_C_SOURCE_COUNT)"
	@echo "EXCLUDE_PATTERNS  = $(EXCLUDE_PATTERNS)"
	@echo "CPPFLAGS          = $(CPPFLAGS)"
	@echo "LIBS              = $(LIBS)"

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(LIB_DIR)
	rm -f $(BUILD_DIR)/*_sources.mk

distclean: clean
	rm -rf $(BIN_DIR)

install-deps:
ifdef APT_GET
	@echo "Installing dependencies via apt-get"
	$(SUDO) apt-get update
	$(SUDO) apt-get install -y build-essential pkg-config libsfml-dev
else ifdef BREW
	@echo "Installing dependencies via Homebrew"
	brew update
	brew install pkg-config sfml
else
	@echo "No supported package manager detected. Please install build essentials, pkg-config, and SFML manually."
endif

