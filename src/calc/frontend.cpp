#include "parser.h"

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Frontend Interface (with webpage) ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

double last_answer = NAN;

// initializes backend constants and functions
void init() {
    init_constants();
    init_functions();
}

// evaluates the (user provided) string, and returns the result as a string
string calculate_text(string text) {
    try {
        string ret = "";
        vector<unique_ptr<Token>> token_vec = tokenize(text);
        unique_ptr<TreeNode> tree = parseS(std::move(token_vec));

        if(get_id_value("ECHO")) {
            string before_macros = tree->to_string();
            tree = tree->exe_macros(std::move(tree));
            string after_macros = tree->to_string();

            if(before_macros == after_macros) ret += ">   " + after_macros + "\n";
            else ret = ">   " + before_macros + "\n=>  " + after_macros + "\n";
        } else tree = tree->exe_macros(std::move(tree));

        last_answer = tree->eval();
        return ret + to_string(last_answer);
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
