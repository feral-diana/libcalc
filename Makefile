CXXFLAGS= -std=c++11

all: calc.o tests_run

tests_run:
	$(MAKE) -C tests

clean:
	rm -rf calc.o
	$(MAKE) -C tests clean
