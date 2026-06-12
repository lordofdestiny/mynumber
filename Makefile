CXXFLAGS := -std=c++20 -O3 -g -Wall -Wextra -Werror

all: main build/lib/libmynumber.dylib addon

main: build/main

addon: build/Release/addon.node

CPP_FILES=$(wildcard src/**/*.cpp)
OBJ_FILES=$(CPP_FILES:src/%.cpp=build/obj/%.o)
INC_FILES=$(wildcard include/**/*.hpp)

IMPL_LIB_OBJ=$(filter build/obj/impl/%.o, $(OBJ_FILES))

build/Release/addon.node: build/Release/.configured binding.gyp $(CPP_FILES) $(INC_FILES)
	@npx node-gyp build

build/Release/.configured: build/lib/libmynumber.dylib
	@mkdir -p $(@D)
	@npx node-gyp configure
	@touch $@

build/main: build/obj/main.o build/lib/libmynumber.dylib | build
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iinclude -fvisibility=hidden -flto=auto $^ -o $@

build/lib/libmynumber.dylib: $(IMPL_LIB_OBJ)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iinclude -shared -fvisibility=hidden -flto=auto $^ -o $@

build/obj/%.o: src/%.cpp | build
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iinclude -c -fPIC -fvisibility=hidden -flto=auto $< -o $@

build/obj/%.o: src/main.cpp | build
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iinclude -c -fvisibility=hidden -flto=auto $< -o $@

format: build/.format

build/.format : $(CPP_FILES) $(INC_FILES) | build
	@echo "Formatting code..."
	@echo "$^" | tr ' ' '\n'
	@clang-format -i $^
	@touch build/.format

build:
	@mkdir -p build

clean:
	@rm -rf build

.PHONY: all main clean format addon emsc