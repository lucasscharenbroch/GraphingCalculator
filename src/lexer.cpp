#include "calculator.h"

/* ~ ~ ~ ~ ~ Lexicographical Tokenizing ~ ~ ~ ~ ~ */

/*
 * Terminal symbols: three (3) types.
 *
 * (1) (VAR)iable identifier:
 *     - A (letter (case sensitive) or underscore) followed by zero or more (letters, underscores, or 
 *       digits).
 *     - Regex: "[a-zA-Z_][a-zA-Z_1-9]*".
 *
 * (2) (NUM)eric literal
 *     - A "basic" floating point literal in base-10.
 *         - (one or more decimal digits)
 *         - [(one or more decimal digits)].(one or more decimal digits)
 *         - Regex: "(([0-9]*\\.[0-9]+)|([1-9]+))".
 *     - A "scientific-notation" floating point literal in base-10.
 *         - ("basic" floating point literal)(e or E)[-](one or more decimal digits)
 *         - Regex: "(([0-9]*\\.[0-9]+)|([1-9]+))((e|E)-?[0-9]+)".
 *     - Leading '-' symbols are ignored, as they are handled as a unary operator during gramatical
 *       parsing.
 *     - Regex: "(([0-9]*\\.[0-9]+)|([1-9]+))((e|E)-?[0-9]+)?".
 *
 * (3) (OP)erator symbol
 *     - "+", "-", "/", "*", "//", "%", "(", or ")".
 *     - Regex: "\\+|-|\\*|//|/|%|\\(|\\)""
 */

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

/* ~ ~ ~ Terminal Token Regular Expressions ~ ~ ~ */

const regex var_regex("^[a-zA-Z_][a-zA-Z_1-9]*", regex::extended);
const regex num_regex("^((([0-9]*\\.[0-9]+)|([1-9]+))((e|E)-?[0-9]+)?)", regex::extended);
const regex op_regex("^(\\+|-|\\*|//|/|%|\\(|\\))", regex::extended);

/* ~ ~ ~ Parsing Function ~ ~ ~ */

vector<unique_ptr<Token>> tokenize(const string& expr_str) {
    vector<unique_ptr<Token>> token_vec;

    for(int i = 0; i < expr_str.length();) {
        if(isspace(expr_str[i])) { // skip whitespace
            i++; 
            continue;
        }

        cmatch match;

        if(regex_search(&expr_str[i], match, var_regex)) {
            // matched a variable
            cout << "(V): " << string(expr_str, i, match.length()) << endl; // TODO remove (debug)

            string var_id = string(expr_str, i, match.length());
            token_vec.push_back(unique_ptr<Token> {new VarToken(var_id)});
        } else if(regex_search(&expr_str[i], match, num_regex)) {
            // matched a numeric literal
            cout << "(N): " << string(expr_str, i, match.length()) << endl; // TODO remove (debug)

            double num_val = stod(string(expr_str, i, match.length()));
            token_vec.push_back(unique_ptr<Token> {new NumToken(num_val)});
        } else if(regex_search(&expr_str[i], match, op_regex)) {
            // matched an operator
            cout << "(O): " << string(expr_str, i, match.length()) << endl; // TODO remove (debug)

            string op = string(expr_str, i, match.length());
            token_vec.push_back(unique_ptr<Token> {new OpToken(op)});
        } else {
            // invalid token
            cout << "Invalid token! @ " << i << endl; // TODO remove (debug)
            return vector<unique_ptr<Token>>(); // return an empty token vector
        }

        i += match.length();
    }

    return token_vec;
}

int main() { // TODO remove (test method)
    string in = "my_var_3// 32 * -1 // % * ( ) / / // // / / // //";

    vector<unique_ptr<Token>> token_vec = tokenize(in);

    for (auto& token_ptr : token_vec) {
        cout << token_ptr->to_string() << " " << endl;
    }

    return 0;
}
