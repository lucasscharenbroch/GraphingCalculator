#include "calculator.h"

/*
 * Grammar outline:
 *
 * Terminal symbols:
 *
 * "Literals":
 * - Variables
 * - Numeric Literals (decimal (floating-point and scientific notation), binary (integer) and hex (integer) literals)
 * - Function Calls "function-name(args, ...)"; support for variadic functions.
 *
 * "Operations"
 * - "+" addition
 * - "-" subtraction
 * - "-" unary negation
 * - "*" multiplication
 * - "/" float division
 * - "(" ")" grouping
 * - "=" assignment
 * - "==" equality
 * - "!=" inequality
 * - "<" less than
 * - "<=" less than or equal to
 * - ">" greater than
 * - ">=" greater than or equal to
 *
 * Nonterminal Symbols:
 *
 * S means "Statement"
 * E means "Expression" (parsed before () and comparison operators)
 * T means "Term" (parsed before + and -)
 * F means "Factor" (parsed before * and /)
 * ARGS means "Argument tuple"
 *
 * Terminal Symbols:
 * VAR means "Variable name" (this also encompasses function names)
 * NUM means "Numeric literal"
 * "" refers to an empty string
 * $ refers to the end of input
 *
 * Grammar Rules:
 *
 * S -> E$
 *
 * E -> VAR = E
 *   -> T == E
 *   -> T != E
 *   -> T < E
 *   -> T <= E
 *   -> T > E
 *   -> T >= E
 *   -> T {+|- T}
 *
 * T -> -F
 *   -> F {*|/ T}
 *
 * F -> (E)
 *   -> VAR
 *   -> NUM
 *   -> VAR(ARGS)
 *
 * ARGS -> E, ARGS
 *      -> E
 *      -> ""
 */

/* ~ ~ ~ ~ ~ Lexicographical Tokenizing ~ ~ ~ ~ ~ */

/* ~ ~ ~ Token Classes ~ ~ ~ */

enum class TokenType {
    VAR,      // variable or function name
    NUM,      // numeric literal
    OP,       // +, -, *, /, (, or )
    NONE      // represents $, NULL, end of string, etc.
};

// Token base-class (only used for inheritance)
struct Token {
    TokenType type;
    Token(TokenType tp): type(tp) { }
};

struct VarToken : Token {
    string identifier;
    VarToken(string id): Token(TokenType::VAR), identifier(id) { }
};

struct NumToken : Token {
    double value;
    NumToken(double val): Token(TokenType::NUM), value(val) { }
};

struct OpToken : Token {
    char op;
    OpToken(char o): Token(TokenType::OP), op(o) { }
};

struct NoneToken : Token {
    NoneToken() : Token(TokenType::NONE) { }
};

int main() { // TODO remove (test method)
    Token *t = new NumToken(66.666);

    switch(t->type) {
        case TokenType::VAR:
            cout << ((VarToken *)t)->identifier << endl;
            break;
        case TokenType::NUM:
            cout << ((NumToken *)t)->value << endl;
            break;
        case TokenType::OP:
            cout << ((OpToken *)t)->op << endl;
            break;
        case TokenType::NONE:
            cout << "NONE" << endl;
            break;
    }

    return 0;
}
