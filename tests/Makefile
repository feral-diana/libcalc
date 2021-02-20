CPP=g++
CXXFLAGS= -std=c++17 -Wall -Werror

all: test_build test_run

basic_tests.o: ../calc.hh ../calc_impl.hh basic_tests.cc
	$(CPP) $(CXXFLAGS) -c -o basic_tests.o basic_tests.cc

test_build: basic_tests.o ../calc.o
	$(CPP) $(CXXFLAGS) -o basic_tests basic_tests.o ../calc.o

test_run:
	./basic_tests

clean:
	rm -rf basic_tests basic_tests.o
