CXX = g++-8
EXE = iplC
CXXDEBUG = -g -Wall
CXXSTD = -std=c++11


.PHONY: all
all: parser lexer 	
	$(CXX) $(CXXDEBUG) -o $(EXE) driver.cpp symbtab.cpp ast.cpp type.cpp parser.o scanner.o 

parser: parser.yy scanner.hh
	bison -d -v $<
	$(CXX) $(CXXDEBUG) -c parser.tab.cc -o parser.o 

lexer: scanner.l scanner.hh parser.tab.hh parser.tab.cc	
	flex++ --outfile=scanner.yy.cc  $<
	$(CXX)  $(CXXDEBUG) -c scanner.yy.cc -o scanner.o

clean: 
	rm location.hh
	rm parser.o
	rm parser.output
	rm parser.tab.cc
	rm parser.tab.hh
	rm position.hh
	rm scanner.o
	rm scanner.yy.cc
	rm stack.hh
	rm iplC
