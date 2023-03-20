#include "parser.h"

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Lexicographical Tokenizing ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

/*
 * Terminal symbols: three (3) types.
 *
 * (1) (VAR)iable identifier:
 *     - A (letter (case sensitive) or underscore) followed by zero or more (letters, underscores, or 
 *       digits).
 *     - Regex: "[a-zA-Z_][a-zA-Z_0-9]*".
 *
 * (2) (NUM)eric literal
 *     - A "basic" decimal floating point literal in base-10.
 *         - (one or more decimal digits)
 *         - [(one or more decimal digits)].(one or more decimal digits)
 *         - Regex: "(([0-9]*\\.[0-9]+)|([0-9]+))".
 *     - A "scientific-notation" decimal floating point literal in base-10.
 *         - ("basic" floating point literal)(e or E)[-](one or more decimal digits)
 *         - Regex: "(([0-9]*\\.[0-9]+)|([0-9]+))((e|E)-?[0-9]+)".
 *     - Leading '-' symbols are ignored, as they are handled as a unary operator during gramatical
 *       parsing.
 *     - A binary literal
 *         - (a sequence of 1s and 0s with a leading "0b" or "0B")
 *         - Regex: "(0[bB][01]+)".
 *     - A hexidecimal literal
 *         - (a sequence of hex characters (not case-sensitive) with a leading "0x" or "0X")
 *         - Regex: "(0[xX][0-9a-fA-F]+)"
 *     - Regex: "((([0-9]*\\.[0-9]+)|([0-9]+))((e|E)-?[0-9]+)?)|(0[bB][01]+)|(0[xX][0-9a-fA-F]+)".
 *
 * (3) (OP)erator symbol
 *     - "+", "-", "/", "*", "//", "%", "^", "(", ")", "=", "==", "!=", ">", "<", ">=", "<=" ",", "'"
 *     - Regex: "\\+|-|\\*|//|/|%|\\^|\\(|\\)|=|==|!=|>|<|>=|<=|,|'"
 */

/* ~ ~ ~ ~ ~ Terminal Token Regular Expressions ~ ~ ~ ~ ~ */

const regex var_regex("^[a-zA-Z_][a-zA-Z_0-9]*", regex::extended);
const regex num_regex("^(((([0-9]*\\.[0-9]+)|([0-9]+))((e|E)-?[0-9]+)?)|(0[bB][01]+)|(0[xX][0-9a-fA-F]+))", regex::extended);
const regex op_regex("^(\\+|-|\\*|//|/|%|\\^|\\(|\\)|=|==|!=|>|<|>=|<=|,|')", regex::extended);

/* ~ ~ ~ ~ ~ Parsing Function ~ ~ ~ ~ ~ */

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
            string var_id = string(expr_str, i, match.length());
            token_vec.push_back(unique_ptr<Token> {new VarToken(var_id)});
        } else if(regex_search(&expr_str[i], match, num_regex)) { // matched a numeric literal
            double num_val = 0;
            string match_str = string(expr_str, i, match.length());

            if(match_str.size() >= 2 && match_str[0] == '0' && tolower(match_str[1]) == 'b') {
                // copy binary literal digit-by-digit
                for(int i = 2; i < match_str.size(); i++) {
                    num_val *= 2;
                    num_val += match_str[i] - '0';
                }
            } else num_val = stod(match_str); // read other (hex/decimal) literals with std::stod

            token_vec.push_back(unique_ptr<Token> {new NumToken(num_val)});
        } else if(regex_search(&expr_str[i], match, op_regex)) { // matched an operator
            string op = string(expr_str, i, match.length());
            token_vec.push_back(unique_ptr<Token> {new OpToken(op)});
        } else { // invalid token
            throw invalid_token_error("invalid token at char " + to_string(i) + " (" + expr_str[i] + ")");
        }

        i += match.length();
    }

    return token_vec;
}
