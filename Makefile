CXXFLAGS= -std=c++17 -Wall -Werror

all: calc.o tests_run

calc.o: calc.hh calc.cc
	g++-7 $(CXXFLAGS) -c -o calc.o calc.cc -O3

tests_run:
	$(MAKE) -C tests

clean:
	rm -rf calc.o
	$(MAKE) -C tests clean
