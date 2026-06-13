CXXFLAGS := -std=c++20 -O3 -g -Wall -Wextra -Werror

JOBS ?= $(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
CMAKE_BUILD_TYPE ?= Release
CMAKE_BUILD_DIR := build/native
CMAKE_ARGS := -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

ifndef MAKELEVEL
  MAKEFLAGS += --output-sync=line -j$(JOBS)
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  LIB_EXT := dylib
else
  LIB_EXT := so
endif

IMPL_SRCS := $(wildcard src/impl/*.cpp)
WRAPPER_SRCS := $(wildcard src/wrapper/*.cpp)
INC_FILES := $(wildcard include/**/*.hpp)

LIB_STATIC := $(CMAKE_BUILD_DIR)/libmynumber.a
LIB_SHARED := $(CMAKE_BUILD_DIR)/libmynumber.$(LIB_EXT)
NATIVE_LIB := native-lib/libmynumber.a
MAIN_BIN := $(CMAKE_BUILD_DIR)/mynumber
CONFIGURED := build/Release/.configured
ADDON_BIN := build/Release/mynumber.node

EMXX ?= em++
EM_BINDING_SRC := src/emscripten/bindings.cpp
EM_IMPL_OBJS := $(IMPL_SRCS:src/%.cpp=build/wasm/obj/%.o)
EM_BINDING_OBJ := build/wasm/obj/emscripten/bindings.o

DIST_WASM_DIR := dist/wasm
DIST_NODE_DIR := dist/node
DIST_DEMO_DIR := dist/demo
DIST_NATIVE_DIR := dist/native

EMXXFLAGS := -std=c++20 -O3 -Wall -Wextra -Werror -Iinclude
EMLDFLAGS := --bind -sMODULARIZE=1 -sEXPORT_NAME=createMynumberModule -sALLOW_MEMORY_GROWTH=1 \
	-sFILESYSTEM=0 -sENVIRONMENT=web,node

.PHONY: all native lib main addon node emscripten emscripten-check wasm \
	dist-wasm dist-node dist-demo dist-native dist-npm clean format

all: native dist-node

# CMake static + shared library and CPack packages
native: lib
lib: $(LIB_STATIC) $(LIB_SHARED)

$(LIB_STATIC): CMakeLists.txt $(IMPL_SRCS) $(INC_FILES)
	cmake -B $(CMAKE_BUILD_DIR) $(CMAKE_ARGS)
	cmake --build $(CMAKE_BUILD_DIR) --target mynumber_static --parallel $(JOBS)

$(LIB_SHARED): $(LIB_STATIC)
	cmake --build $(CMAKE_BUILD_DIR) --target mynumber_shared --parallel $(JOBS)

dist-native: native
	@mkdir -p $(DIST_NATIVE_DIR)
	cmake --build $(CMAKE_BUILD_DIR) --target package
  cp $(CMAKE_BUILD_DIR)/mynumber-*.tar.gz $(DIST_NATIVE_DIR)/ 2>/dev/null || true

# Native console application
main: $(MAIN_BIN)

$(MAIN_BIN): $(LIB_STATIC) src/main.cpp
	cmake --build $(CMAKE_BUILD_DIR) --target mynumber_cli

# Node.js native addon
addon: dist-node
node: dist-node

dist-node: $(ADDON_BIN)
	@mkdir -p $(DIST_NODE_DIR)
	cp $(ADDON_BIN) $(DIST_NODE_DIR)/mynumber.node
	cp src/node/index.js $(DIST_NODE_DIR)/index.js
	cp src/node/index.d.ts $(DIST_NODE_DIR)/index.d.ts

$(CONFIGURED): binding.gyp $(LIB_STATIC)
	@mkdir -p native-lib
	@test -f $(LIB_STATIC) || (echo "missing $(LIB_STATIC); run make lib first" >&2; exit 1)
	@cp $(LIB_STATIC) $(NATIVE_LIB)
	@npx node-gyp configure
	@mkdir -p $(@D)
	@touch $@

$(NATIVE_LIB): $(LIB_STATIC)
	@mkdir -p native-lib
	@test -f $(LIB_STATIC) || (echo "missing $(LIB_STATIC); run make lib first" >&2; exit 1)
	@cp $(LIB_STATIC) $(NATIVE_LIB)

$(ADDON_BIN): $(CONFIGURED) $(LIB_STATIC) $(WRAPPER_SRCS) $(INC_FILES)
	@mkdir -p native-lib
	@test -f $(LIB_STATIC) || (echo "missing $(LIB_STATIC); run make lib first" >&2; exit 1)
	@cp $(LIB_STATIC) $(NATIVE_LIB)
	@npx node-gyp build -j $(JOBS)

# WebAssembly build (requires em++ on PATH)
emscripten-check:
	@command -v $(EMXX) >/dev/null 2>&1 || { \
		echo "em++ not found. Install Emscripten and add em++ to PATH." >&2; \
		echo "See https://emscripten.org/docs/getting_started/downloads.html" >&2; \
		exit 1; \
	}

emscripten: dist-wasm
wasm: dist-wasm

dist-wasm: emscripten-check $(DIST_WASM_DIR)/mynumber.js
	@mkdir -p $(DIST_WASM_DIR)
	cp src/emscripten/index.js $(DIST_WASM_DIR)/index.js
	cp src/emscripten/index.d.ts $(DIST_WASM_DIR)/index.d.ts

$(DIST_WASM_DIR)/mynumber.js: $(EM_IMPL_OBJS) $(EM_BINDING_OBJ)
	@mkdir -p $(DIST_WASM_DIR)
	$(EMXX) $(EMXXFLAGS) $(EM_IMPL_OBJS) $(EM_BINDING_OBJ) -o $@ $(EMLDFLAGS)

build/wasm/obj/%.o: src/%.cpp emscripten-check
	@mkdir -p $(@D)
	$(EMXX) $(EMXXFLAGS) -c $< -o $@

# Self-contained static demo for GitHub Pages
dist-demo: dist-wasm
	@mkdir -p $(DIST_DEMO_DIR)
	cp demo/index.html demo/style.css demo/app.js $(DIST_DEMO_DIR)/
	cp $(DIST_WASM_DIR)/mynumber.js $(DIST_WASM_DIR)/mynumber.wasm $(DIST_DEMO_DIR)/

# Staged npm publish packages (mynumber + mynumber-wasm)
dist-npm: dist-node dist-wasm
	node scripts/sync-version.mjs
	node scripts/stage-npm-packages.mjs

format: build/.format

build/.format: $(IMPL_SRCS) $(WRAPPER_SRCS) src/main.cpp $(EM_BINDING_SRC) $(INC_FILES)
	@echo "Formatting code..."
	@clang-format -i $^
	@mkdir -p build
	@touch $@

clean:
	@rm -rf build/wasm dist native-lib
	@rm -rf $(CMAKE_BUILD_DIR)
	@npx node-gyp clean 2>/dev/null || true
