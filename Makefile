all: main

main: build/main

build/main: main.cpp | build
	g++-15 -std=c++23 -O0 -g -Wall -Wextra -Werror $< -o $@

format: build/.format

build/.format : main.cpp | build
	clang-format -i $<
	@touch build/.format

build:
	@mkdir build

clean:
	@rm -rf build

.PHONY: all main clean format