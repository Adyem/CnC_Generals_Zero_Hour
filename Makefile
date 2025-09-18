# Top-level Makefile for Command & Conquer Generals source tree
#
# This Makefile is intentionally POSIX/gmake compatible and is designed as a
# best-effort attempt to build the legacy project on modern toolchains.  The
# original project targets Microsoft Visual C++ 6.0 and makes heavy use of
# proprietary SDKs that are not included in this repository.  As a result the
# build produced by this Makefile will likely fail without significant manual
# configuration, but it provides a starting point for exploring how the build
# can be wired together on non-Visual Studio environments.

# -----------------------------------------------------------------------------
# Toolchain configuration
# -----------------------------------------------------------------------------
CXX ?= g++
CC  ?= gcc
AR  ?= ar
RC  ?= windres

CXXSTANDARD ?= c++03
CXXFLAGS   ?= -std=$(CXXSTANDARD) -Wall -Wextra -Wpedantic
CFLAGS     ?= -Wall -Wextra -Wpedantic
CPPFLAGS   ?=
LDFLAGS    ?=
LIBS       ?=

# Useful preprocessor definitions that roughly mirror the legacy MSVC project
CPPFLAGS   += -DWIN32 -D_WINDOWS -DNOMINMAX -D_CRT_SECURE_NO_DEPRECATE

# -----------------------------------------------------------------------------
# Source discovery
# -----------------------------------------------------------------------------
SRC_DIR     := Generals/Code
BUILD_DIR   := build
OBJ_DIR     := $(BUILD_DIR)/obj
BIN_DIR     := $(BUILD_DIR)/bin
TARGET      ?= $(BIN_DIR)/generals-sfml

# Automatically generate object lists and per-file build rules.  The helper
# script writes a makefile fragment to $(SOURCES_MK), which is then included
# below so the generated rules become part of the build graph.
SOURCES_MK := $(BUILD_DIR)/generated_sources.mk

$(shell python3 build_tools/generate_make_fragment.py $(SRC_DIR) $(OBJ_DIR) $(SOURCES_MK))

include $(SOURCES_MK)

# -----------------------------------------------------------------------------
# Include path configuration
# -----------------------------------------------------------------------------
# We collect every directory under $(SRC_DIR) and add it to the include search
# path.  Directories containing whitespace are supported by escaping the space
# characters with a backslash.  Consumers can extend INCLUDE_DIRS from the
# command line if additional external SDKs are available on the system.
ESCAPED_INCLUDE_DIRS := $(shell find $(SRC_DIR) -type d -print | sed 's/ /\\ /g' | sort)
INCLUDE_DIRS         := $(foreach dir,$(ESCAPED_INCLUDE_DIRS),-I$(dir))
CPPFLAGS             += $(INCLUDE_DIRS)

# STLport ships in a nested "stlport" include directory, so add a convenience
# include for it when present.
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
# Primary targets
# -----------------------------------------------------------------------------
.PHONY: all clean distclean print-config

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

# -----------------------------------------------------------------------------
# Per-file build rules
# -----------------------------------------------------------------------------
# Individual object build rules are generated dynamically by the
# generate_source_rules helper defined above.

# -----------------------------------------------------------------------------
# Utility targets
# -----------------------------------------------------------------------------
print-config:
	@echo "CXX        = $(CXX)"
	@echo "CC         = $(CC)"
	@echo "CXXFLAGS   = $(CXXFLAGS)"
	@echo "CFLAGS     = $(CFLAGS)"
	@echo "CPPFLAGS   = $(CPPFLAGS)"
	@echo "LDFLAGS    = $(LDFLAGS)"
	@echo "LIBS       = $(LIBS)"
	@echo "TARGET     = $(TARGET)"
	@echo "ENABLE_SFML = $(ENABLE_SFML)"
	@echo "SFML_MODULES = $(SFML_MODULES)"
	@echo "SFML_CFLAGS = $(SFML_CFLAGS)"
	@echo "SFML_LIBS   = $(SFML_LIBS)"
	@echo "# of C++ files = $(CPP_SOURCE_COUNT)"
	@echo "# of C files  = $(C_SOURCE_COUNT)"

clean:
	rm -rf $(OBJ_DIR)

# Remove the entire build directory, including the binary output.
distclean: clean
	rm -rf $(BIN_DIR)

