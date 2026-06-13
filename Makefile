CXXFLAGS := -std=c++20 -O3 -g -Wall -Wextra -Werror

JOBS ?= $(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

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

IMPL_OBJS := $(IMPL_SRCS:src/%.cpp=out/obj/%.o)
MAIN_OBJ := out/obj/main.o

LIB_STATIC := out/lib/libmynumber.a
LIB_SHARED := out/lib/libmynumber.$(LIB_EXT)
MAIN_BIN := out/main
CONFIGURED := build/Release/.configured
ADDON_BIN := build/Release/addon.node

EMXX ?= em++
EMSCRIPTEN_DIR := out/emscripten
EM_JS := $(EMSCRIPTEN_DIR)/mojbroj.js
EM_BINDING_SRC := src/emscripten/bindings.cpp
EM_IMPL_OBJS := $(IMPL_SRCS:src/%.cpp=$(EMSCRIPTEN_DIR)/obj/%.o)
EM_BINDING_OBJ := $(EMSCRIPTEN_DIR)/obj/emscripten/bindings.o

EMXXFLAGS := -std=c++20 -O3 -Wall -Wextra -Werror -Iinclude
EMLDFLAGS := --bind -sMODULARIZE=1 -sEXPORT_NAME=createMojbrojModule -sALLOW_MEMORY_GROWTH=1 \
	-sFILESYSTEM=0 -sENVIRONMENT=web,node

.PHONY: all lib main addon node emscripten emscripten-check wasm clean format

all: lib main addon

# Standalone C++ library (static + shared)
lib: $(LIB_STATIC) $(LIB_SHARED)

# Native console application
main: $(MAIN_BIN)

# Node.js native addon
addon: $(ADDON_BIN)
node: addon

# WebAssembly build (requires em++ on PATH — https://emscripten.org/docs/getting_started/downloads.html)
emscripten-check:
	@command -v $(EMXX) >/dev/null 2>&1 || { \
		echo "em++ not found. Install Emscripten and add em++ to PATH." >&2; \
		echo "See https://emscripten.org/docs/getting_started/downloads.html" >&2; \
		exit 1; \
	}

emscripten: emscripten-check $(EM_JS)
wasm: emscripten

$(LIB_STATIC): $(IMPL_OBJS) | out
	@mkdir -p $(@D)
	ar rcs $@ $^

$(LIB_SHARED): $(IMPL_OBJS) | out
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iinclude -shared -fvisibility=hidden -flto=auto $^ -o $@

$(MAIN_BIN): $(MAIN_OBJ) $(LIB_STATIC) | out
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iinclude -fvisibility=hidden -flto=auto $^ -o $@

$(CONFIGURED): binding.gyp $(LIB_STATIC)
	@npx node-gyp configure
	@mkdir -p $(@D)
	@touch $@

$(ADDON_BIN): $(CONFIGURED) $(LIB_STATIC) $(WRAPPER_SRCS) $(INC_FILES)
	@npx node-gyp build -j $(JOBS)

out/obj/impl/%.o: src/impl/%.cpp | out
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iinclude -c -fPIC -fvisibility=hidden -flto=auto $< -o $@

out/obj/main.o: src/main.cpp | out
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iinclude -c -fvisibility=hidden -flto=auto $< -o $@

$(EM_JS): emscripten-check $(EM_IMPL_OBJS) $(EM_BINDING_OBJ) | out
	@mkdir -p $(EMSCRIPTEN_DIR)
	$(EMXX) $(EMXXFLAGS) $(EM_IMPL_OBJS) $(EM_BINDING_OBJ) -o $@ $(EMLDFLAGS)

$(EMSCRIPTEN_DIR)/obj/%.o: src/%.cpp emscripten-check | out
	@mkdir -p $(@D)
	$(EMXX) $(EMXXFLAGS) -c $< -o $@

format: out/.format

out/.format: $(IMPL_SRCS) $(WRAPPER_SRCS) src/main.cpp $(EM_BINDING_SRC) $(INC_FILES) | out
	@echo "Formatting code..."
	@clang-format -i $^
	@touch $@

out:
	@mkdir -p out

clean:
	@rm -rf out
	@npx node-gyp clean 2>/dev/null || true
