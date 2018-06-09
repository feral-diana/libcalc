CXXFLAGS= -std=c++11

all: test_build test_run

basic_tests.o: ../calc.hh basic_tests.cc
	g++ $(CXXFLAGS) -c -o basic_tests.o basic_tests.cc

test_build: ../calc.o ../calc.hh basic_tests.o
	g++ -std=c++11 -o basic_tests basic_tests.o ../calc.o

test_run:
	./basic_tests

clean:
	rm -rf basic_tests basic_tests.o
