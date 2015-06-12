.DEFAULT_GOAL := lc3as

help:
	@echo Runing make will generate the lexer and parser from their corresponding Flex and
	@echo Bison source files, as well as the binary lc3.

lc3.tab.c lc3.tab.h: lc3.y
	bison -d lc3.y

lex.yy.c: lc3.l lc3.tab.h
	flex lc3.l

lc3as: lex.yy.c lc3.tab.c lc3.tab.h assembly_printer.h assembly_printer.cpp ../utils/printer.h ../utils/printer.cpp tokens.h tokens.cpp lc3as.cpp assembler.h assembler.cpp
	clang++ -Wall -x c++ -std=c++11 tokens.cpp ../utils/printer.cpp assembly_printer.cpp lc3.tab.c lex.yy.c lc3as.cpp assembler.cpp -o lc3as

clean:
	rm lc3as lc3.tab.c lc3.tab.h lex.yy.c
