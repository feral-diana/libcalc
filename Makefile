CPP=g++
CXXFLAGS= -std=c++17 -Wall -Werror

all: tests_run

tests_run:
	$(MAKE) -C tests

clean:
	$(MAKE) -C tests clean
