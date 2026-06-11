all: main

main: build/main

build/main: main.cpp | build
	g++-15 -std=c++23 -O3 -g -Wall -Wextra -Werror $< -o $@

format: .format

.format : main.cpp
	clang-format -i $<
	@touch .format

build:
	@mkdir build

clean:
	@rm -rf build .format

.PHONY: all main clean format