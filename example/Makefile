CXXFLAGS=-c -Wall -std=c++11

build:
	@$(CXX) $(CXXFLAGS) main.cpp -o main.o
	@$(CXX) $(CXXFLAGS) ../ssde/ssde_x86.cpp -o ssde_x86.o
	@$(CXX) main.o ssde_x86.o -o main

clean:
	@rm -f *.o