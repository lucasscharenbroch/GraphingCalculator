run: calculator
	./calculator

calculator: main.cpp lexer.cpp grammar_parser.cpp calc_backend.cpp math.cpp calculator.h parser.h backend.h
	g++ -std=c++17 -o calculator lexer.cpp grammar_parser.cpp main.cpp calc_backend.cpp math.cpp
