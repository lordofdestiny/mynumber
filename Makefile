all: main

main: build/main

CPP_FILES=$(wildcard src/**/*.cpp)
OBJ_FILES=$(CPP_FILES:src/%.cpp=build/obj/%.o)
INC_FILES=$(wildcard include/**/*.hpp)

IMPL_LIB_OBJ=$(filter build/obj/impl/%.o, $(OBJ_FILES))

build/main: build/obj/main.o build/lib/libmynumber.dylib | build
	@mkdir -p $(@D)
	g++-15 -std=c++23 -O3 -g -Wall -Wextra -Werror -Iinclude -fvisibility=hidden -flto=auto $^ -o $@

build/lib/libmynumber.dylib: $(IMPL_LIB_OBJ)
	@mkdir -p $(@D)
	g++-15 -std=c++23 -O3 -g -Wall -Wextra -Werror -Iinclude -shared -fvisibility=hidden -flto=auto $^ -o $@

build/obj/%.o: src/%.cpp | build
	@mkdir -p $(@D)
	g++-15 -std=c++23 -O3 -g -Wall -Wextra -Werror -Iinclude -c -fPIC -fvisibility=hidden -flto=auto $< -o $@

build/obj/%.o: src/main.cpp | build
	@mkdir -p $(@D)
	g++-15 -std=c++23 -O3 -g -Wall -Wextra -Werror -Iinclude -c -fvisibility=hidden -flto=auto $< -o $@

format: build/.format

build/.format : $(CPP_FILES) $(INC_FILES) | build
	@echo "Formatting code..."
	@echo "$^" | tr ' ' '\n'
	@clang-format -i $^
	@touch build/.format

build:
	@mkdir build

clean:
	@rm -rf build

.PHONY: all main clean format