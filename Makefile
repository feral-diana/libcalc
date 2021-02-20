CPP=g++
CXXFLAGS= -std=c++17 -Wall -Werror

all: tests_run

tests_run:
	$(MAKE) -C tests

calc.o: calc.hh calc_impl.hh calc.cc
	$(CPP) $(CXXFLAGS) -c -o calc.o calc.cc

clean:
	$(MAKE) -C tests clean
	rm -rf calc.o
