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

.PHONY: all lib main addon node clean format

all: lib main addon

# Standalone C++ library (static + shared)
lib: $(LIB_STATIC) $(LIB_SHARED)

# Native console application
main: $(MAIN_BIN)

# Node.js native addon
addon: $(ADDON_BIN)
node: addon

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

format: out/.format

out/.format: $(IMPL_SRCS) $(WRAPPER_SRCS) src/main.cpp $(INC_FILES) | out
	@echo "Formatting code..."
	@clang-format -i $^
	@touch $@

out:
	@mkdir -p out

clean:
	@rm -rf out
	@npx node-gyp clean 2>/dev/null || true
