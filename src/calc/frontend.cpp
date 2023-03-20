#include "parser.h"

double last_answer = NAN;

// initializes backend constants and functions
void init() {
    init_math_constants();
    init_math_functions();
}

// evaluates the (user provided) string, and returns the result as a string
string calculate_text(string text) {
    try {
        vector<unique_ptr<Token>> token_vec = tokenize(text);
        unique_ptr<TreeNode> tree = parseS(token_vec);
        if(tree == nullptr) throw invalid_expression_error("error while parsing");
        last_answer = tree->eval();
        return to_string(last_answer);
    } catch(calculator_error& err) {
        return err.to_string();
    }
}

// C-style wrapper for calculate_text - returns a c-style string
char *calculate_text(const char *text) {
    string result = calculate_text(string(text));

    char *cstr = (char *) malloc(result.size() + 1);
    strcpy(cstr, result.c_str());

    return cstr;
}
