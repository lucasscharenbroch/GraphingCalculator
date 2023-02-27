#include "calculator.h"

/* ~ ~ ~ Token Classes ~ ~ ~ */

enum class TokenType {
    VAR,      // variable or function name
    NUM,      // numeric (float) literal
    OP,       // +, -, *, /, (, or )
    NONE      // represents $, NULL, end of string, etc.
};

// Token base-class (abstract - only used for inheritance)
struct Token {
    TokenType type;
    Token(TokenType tp): type(tp) { }
    virtual string to_string() = 0;
};

struct VarToken : Token {
    string identifier;
    VarToken(string id): Token(TokenType::VAR), identifier(id) { }
    string to_string() override { return identifier; }
};

struct NumToken : Token {
    double value;
    NumToken(double val): Token(TokenType::NUM), value(val) { }
    string to_string() override { return std::to_string(value); }
};

struct OpToken : Token {
    string op;
    OpToken(string o): Token(TokenType::OP), op(o) { }
    string to_string() override { return op; }
};

struct NoneToken : Token { // TODO remove this type if unused.
    NoneToken() : Token(TokenType::NONE) { }
    string to_string() override { return "null"; }
};

/* ~ ~ ~ Lexing Functions ~ ~ ~ */

vector<unique_ptr<Token>> tokenize(const string& expr_str);

/* ~ ~ ~ Operation (Parsing Tree Node) Classes ~ ~ ~ */


/* ~ ~ ~ Grammar Parsing Functions ~ ~ ~ */





