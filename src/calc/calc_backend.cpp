#include "backend.h"

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Calculator Backend ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

/* ~ ~ ~ ~ ~ Backend Structures ~ ~ ~ ~ ~ */

unordered_map<string, double> identifier_table; // stores values of all variables
unordered_map<string, unique_ptr<Function>> fn_table; // stores all functions
unordered_map<string, unique_ptr<macro_function>> macro_table;
bool param_override = false; // set to true during function evaluations so parameters can be
                             // distinguished and evaluated as such.
unordered_map<string, double> param_id; // if param_id[id] == 0, then id is not a parameter of the
                                        // currently executing function. Otherwise, param's value
                                        // should be substituted with params[param_id[id] - 1].
vector<double> params; // parameters of currently executing function

/* ~ ~ ~ ~ ~ Backend Functions ~ ~ ~ ~ ~ */

// returns floating-point number associated with given identifier; 0 by default.
double get_id_value(string id) {
    if(param_override && param_id[id]) return params[param_id[id] - 1];
    return identifier_table[id];
}

// assigns floating-point number to associate with given identifier.
double set_id_value(string id, double val) {
    return identifier_table[id] = val;
}

// sets up param_id, param, and param_override,
// then returns result of evaulating the function's tree.
double call_function(string id, vector<unique_ptr<TreeNode>>& args) {
    if(fn_table[id] == nullptr) throw invalid_function_call_error("no such function: '" + id + "'");
    return fn_table[id]->eval(args);
}

void assign_function(string id, vector<string>&& args, unique_ptr<TreeNode>&& tree) {
    // ensure arg ids aren't re-used
    unordered_map<string, bool> seen;
    for(const string& arg : args) {
        if(seen[arg]) throw invalid_expression_error("argument id `" + arg + "` used twice "
                                                      "in function assignment");
        seen[arg] = true;
    }

    // ensure that id doesn't conflict with a non-user function or macro
    if(macro_table[id] != nullptr)
        throw invalid_expression_error("can't assign function `" + id + "`: " +
                                       "macro with the same name exists");
    else if(fn_table[id] != nullptr && !fn_table[id]->is_user_fn())
        throw invalid_expression_error("can't assign function `" + id + "`: " +
                                       "built-in function with the same name exists");

    fn_table[id] = make_unique<UserFunction>(std::move(args), std::move(tree));
}

unique_ptr<TreeNode> execute_macro(string id, unique_ptr<TreeNode>&& node) {
    if(macro_table[id] == nullptr) return std::move(node);
    else return (*macro_table[id])(std::move(node));
}

void init_constants() {
    init_math_constants();
    init_macro_constants();
}

void init_functions() {
    init_math_functions();
    init_macro_functions();
}
