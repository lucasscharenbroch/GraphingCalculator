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

        string before_macros = tree->to_string();
        tree = tree->exe_on_children(std::move(tree), tree_node_exe_macro);
        string after_macros = tree->to_string();

        if(get_id_value("ECHO_TREE")) {
            ret += "~>  " + before_macros + "\n" +
                   "->  " + after_macros + "\n";
        }

        last_answer = tree->eval();

        if(get_id_value("ECHO_AUTO")) {
            ret += "=>  " + (before_macros != after_macros ? after_macros :
                                                             to_string(last_answer)) + "\n";
        }

        if(get_id_value("ECHO_ANS")) {
            ret += "+>  " + to_string(last_answer) + "\n";
        }

        return ret;
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
