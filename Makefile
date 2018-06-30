CXXFLAGS= -std=c++14

all: calc.o tests_run

calc.o: calc.hh calc.cc
	g++ $(CXXFLAGS) -c -o calc.o calc.cc

tests_run:
	$(MAKE) -C tests

clean:
	rm -rf calc.o
	$(MAKE) -C tests clean
