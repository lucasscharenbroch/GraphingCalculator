#ifndef BACKEND
#define BACKEND

#include "parser.h"

/* ~ ~ ~ ~ ~ Backend Structures ~ ~ ~ ~ ~ */

// Function base-class
struct Function {
    virtual double eval(vector<unique_ptr<TreeNode>>& args) = 0;
    virtual ~Function() { }
};

extern unordered_map<string, double> identifier_table; // stores values of all variables
extern bool param_override; // set to true during function calls for name substitution
extern unordered_map<string, double> param_id; // names to substitute w/ (index to substitute + 1)
extern vector<double> params; // values to substitute
extern unordered_map<string, unique_ptr<Function>> fn_table; // stores all functions

typedef function<unique_ptr<TreeNode>(unique_ptr<TreeNode>&&)> macro_function;
extern unordered_map<string, unique_ptr<macro_function>> macro_table;

void init_math_functions();
void init_macro_functions();

void init_math_constants();
void init_macro_constants();

/*
 * UserFunction: standard user-defined function.
 * eval() takes a vector of pointers to arguments' TreeNodes (whose size must
 * match the number of parameters), evaluates them, and sets up parameter
 * substitution (so variable lookups to the parameter names will evaluate
 * to the respective arguments). Then the function's tree (defined by the user)
 * is evauated, and its result is returned.
 */
struct UserFunction : Function {
    vector<string> arg_ids;
    unique_ptr<TreeNode> tree;

    double eval(vector<unique_ptr<TreeNode>>& args) override {
        if(arg_ids.size() != args.size()) throw invalid_function_call_error("wrong number of "
                                          "arguments (" + to_string(args.size()) + " given, " +
                                                          to_string(arg_ids.size()) + " expected)");

        params.clear();
        for(unique_ptr<TreeNode>& a : args) params.push_back(a->eval());

        param_override = true;
        for(int i = 0; i < arg_ids.size(); i++) param_id[arg_ids[i]] = i + 1;
        double return_val = tree->eval();
        for(int i = 0; i < arg_ids.size(); i++) param_id[arg_ids[i]] = 0;
        param_override = false;

        return return_val;
    }

    UserFunction(vector<string>&& a, unique_ptr<TreeNode>&& t) :
        arg_ids(std::move(a)),
        tree(std::move(t)) { }
};

/*
 * RawFunction: a library function that directly handles the argument trees.
 * The vector<unique_ptr<TreeNode>>& is passed directly to the function pointer,
 * which allows for non-mathematical behavior (like printing) and variadic
 * functions.
 */
struct RawFunction : Function {
    function<double(vector<unique_ptr<TreeNode>>&)> fn;

    double eval(vector<unique_ptr<TreeNode>>& args) override {
        for(unique_ptr<TreeNode>& arg : args) { // ensure no arguments have been moved / nullified
                                                // (this will occur on repeated calls of graph(),
                                                // because graph() consumes the unique_ptr.
            if(!arg) throw calculator_error("function call invalidated");
        }
        return fn(args);
    }

    RawFunction(function<double(vector<unique_ptr<TreeNode>>&)> f) : fn(f) { }
};

/*
 * NDoubleFunction: a library function that accepts exacly N floating-point arguments.
 */
template<unsigned int N>
struct NDoubleFunction : Function {
    function<double(vector<double>&)> fn;

    double eval(vector<unique_ptr<TreeNode>>& args) override {
        if(args.size() != N) throw invalid_function_call_error("wrong number of "
                                          "arguments (" + to_string(args.size()) + " given, " +
                                                          to_string(N) + " expected)");

        vector<double> arg_vals;
        for(auto& arg : args) arg_vals.push_back(arg->eval());

        return fn(arg_vals);
    }

    NDoubleFunction(function<double(vector<double>&)> f) : fn(f) { }
};

#endif // BACKEND
