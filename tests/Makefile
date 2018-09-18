CPP=g++
CXXFLAGS= -std=c++17 -Wall -Werror

all: test_build test_run

basic_tests.o: ../calc.hh basic_tests.cc
	$(CPP) $(CXXFLAGS) -c -o basic_tests.o basic_tests.cc

test_build: ../calc.hh basic_tests.o
	$(CPP) $(CXXFLAGS) -o basic_tests basic_tests.o

test_run:
	./basic_tests

clean:
	rm -rf basic_tests basic_tests.o
