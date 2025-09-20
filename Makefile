# Top-level Makefile (single-path build, no fallback, loud-failing)
# - Scans Generals/Code for *.cpp/*.c
# - Compiles to build/obj/
# - Links executable in repo root: ./generals-sfml(.exe)
# - Fails loudly on missing sources or missing final binary

# -----------------------------------------------------------------------------
# Strict shell & make settings (fail loudly)
# -----------------------------------------------------------------------------
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables

# -----------------------------------------------------------------------------
# Toolchain configuration
# -----------------------------------------------------------------------------
CXX ?= g++
CC  ?= gcc

CXXSTANDARD ?= c++17
CXXFLAGS   ?= -std=$(CXXSTANDARD) -Wall -Wextra -Wpedantic
CFLAGS     ?= -Wall -Wextra -Wpedantic
CPPFLAGS   ?=
LDFLAGS    ?=
LIBS       ?=

APT_GET := $(shell command -v apt-get 2>/dev/null)
BREW    := $(shell command -v brew 2>/dev/null)
USER_ID := $(shell id -u 2>/dev/null)

ifeq ($(USER_ID),0)
SUDO :=
else
SUDO := sudo
endif

# Useful preprocessor definitions similar to the legacy MSVC project
CPPFLAGS   += -DWIN32 -D_WINDOWS -DNOMINMAX -D_CRT_SECURE_NO_DEPRECATE
CPPFLAGS   += -DBX_CONFIG_DEBUG=0
CPPFLAGS   += -DWW3D_BGFX_AVAILABLE=1
WW3D_ENABLE_LEGACY_DX8 ?= 0
CPPFLAGS   += -DWW3D_ENABLE_LEGACY_DX8=$(WW3D_ENABLE_LEGACY_DX8)

# -----------------------------------------------------------------------------
# Paths & target (EXE in repo root)
# -----------------------------------------------------------------------------
EXEEXT :=
ifeq ($(OS),Windows_NT)
EXEEXT := .exe
endif

SRC_DIR   := Generals/Code
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj

# Put the executable at the root of the repo:
TARGET ?= ./generals-sfml$(EXEEXT)

# Core include search paths (extend freely if needed)
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

# STLport convenience include if present
STLPORT_DIR := $(wildcard $(SRC_DIR)/Libraries/STLport-4.5.3/stlport)
ifneq ($(STLPORT_DIR),)
CPPFLAGS   += -I$(STLPORT_DIR)
endif

# -----------------------------------------------------------------------------
# SFML bootstrap configuration
# -----------------------------------------------------------------------------
ENABLE_SFML ?= 1
PKG_CONFIG  ?= pkg-config
SFML_MODULES ?= sfml-graphics sfml-window sfml-system sfml-audio
SFML_CFLAGS ?=
SFML_LIBS   ?=

ifeq ($(ENABLE_SFML),1)
SFML_PKGCONFIG_CFLAGS := $(strip $(shell $(PKG_CONFIG) --silence-errors --cflags $(SFML_MODULES)))
SFML_PKGCONFIG_LIBS   := $(strip $(shell $(PKG_CONFIG) --silence-errors --libs $(SFML_MODULES)))

ifneq ($(SFML_PKGCONFIG_CFLAGS),)
SFML_CFLAGS += $(SFML_PKGCONFIG_CFLAGS)
endif

ifeq ($(strip $(SFML_LIBS)),)
SFML_LIBS := $(SFML_PKGCONFIG_LIBS)
endif

ifeq ($(strip $(SFML_LIBS)),)
SFML_LIBS := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
endif

CPPFLAGS += $(SFML_CFLAGS)
LIBS     += $(SFML_LIBS)
endif

# -----------------------------------------------------------------------------
# libVLC detection (optional)
# -----------------------------------------------------------------------------
LIBVLC_MODULE ?= libvlc
LIBVLC_CFLAGS ?=
LIBVLC_LIBS   ?=

LIBVLC_PKGCONFIG_CFLAGS := $(strip $(shell $(PKG_CONFIG) --silence-errors --cflags $(LIBVLC_MODULE)))
LIBVLC_PKGCONFIG_LIBS   := $(strip $(shell $(PKG_CONFIG) --silence-errors --libs $(LIBVLC_MODULE)))

ifneq ($(LIBVLC_PKGCONFIG_CFLAGS),)
LIBVLC_CFLAGS += $(LIBVLC_PKGCONFIG_CFLAGS)
endif

ifeq ($(strip $(LIBVLC_LIBS)),)
LIBVLC_LIBS := $(LIBVLC_PKGCONFIG_LIBS)
endif

ifeq ($(strip $(LIBVLC_LIBS)),)
LIBVLC_LIBS := -lvlc
endif

CPPFLAGS += $(LIBVLC_CFLAGS)
LIBS     += $(LIBVLC_LIBS)

# -----------------------------------------------------------------------------
# Source discovery (single path; no fallback logic—this IS the build)
# -----------------------------------------------------------------------------
# Exclude problem areas ad-hoc from the command line if needed:
#   make EXCLUDE_PATTERNS='%/DX8/% %/DirectX% %/win32/% %/Windows/%'
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

# -----------------------------------------------------------------------------
# Build rules
# -----------------------------------------------------------------------------
.PHONY: all clean distclean print-config install-deps relink

# Loud error if nothing to build
ifeq ($(strip $(TOTAL_CPP_SOURCE_COUNT) $(TOTAL_C_SOURCE_COUNT)),)
$(error No source files found under '$(SRC_DIR)'. Check SRC_DIR or EXCLUDE_PATTERNS.)
endif

all: $(TARGET)

# Compile (create folders on demand)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Link (executable in repo root)
$(TARGET): $(CPP_OBJS) $(C_OBJS)
	@mkdir -p $(dir $(TARGET))
	@echo "[LINK] $@"
	$(CXX) $(CPP_OBJS) $(C_OBJS) $(LDFLAGS) $(LIBS) -o $@
	@test -f "$@" || { echo "ERROR: link step did not produce $@"; exit 1; }

# Utilities
print-config:
	@echo "TARGET              = $(TARGET)"
	@echo "EXEEXT              = $(EXEEXT)"
	@echo "SRC_DIR             = $(SRC_DIR)"
	@echo "OBJ_DIR             = $(OBJ_DIR)"
	@echo "Fallback?           = NO (single-path build)"
	@echo "C++ files           = $(TOTAL_CPP_SOURCE_COUNT)"
	@echo "C files             = $(TOTAL_C_SOURCE_COUNT)"
	@echo "EXCLUDE_PATTERNS    = $(EXCLUDE_PATTERNS)"
	@echo "CPPFLAGS            = $(CPPFLAGS)"
	@echo "CXXFLAGS            = $(CXXFLAGS)"
	@echo "CFLAGS              = $(CFLAGS)"
	@echo "LDFLAGS             = $(LDFLAGS)"
	@echo "LIBS                = $(LIBS)"

relink:
	@rm -f "$(TARGET)"
	$(MAKE) $(MAKEFLAGS) $(TARGET)

clean:
	rm -rf $(OBJ_DIR)

distclean: clean
	@rm -f "$(TARGET)"

# Best-effort dependency installer for common development environments.
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

