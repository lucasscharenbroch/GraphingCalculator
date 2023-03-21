#ifndef CALCULATOR
#define CALCULATOR

#include <vector>
#include <regex>
#include <cmath>
#include <iostream>
#include <numeric>
#include <memory>
#include <utility>
#include <cstring>
#include <cstdlib>
#include<emscripten.h>

using namespace std;

extern double last_answer; // holds result of last computation

/* ~ ~ ~ ~ ~ Parsing Tree Class ~ ~ ~ ~ ~ */

struct TreeNode { // Abstract superclass for all other node types
    virtual string to_string() = 0;
    virtual double eval() = 0;
    virtual ~TreeNode() { }
    virtual bool is_var() { return false; }
};


/* ~ ~ ~ ~ ~ Calculator Backend Functions ~ ~ ~ ~ ~ */

double get_id_value(string id);
double set_id_value(string id, double val);
double call_function(string id, vector<unique_ptr<TreeNode>>& args);
void assign_function(string id, vector<string>&& args, unique_ptr<TreeNode>&& tree);

void init_math_constants();
void init_math_functions();

const double DERIV_STEP = 1e-6;

/* ~ ~ ~ ~ ~ Calculator Errors ~ ~ ~ ~ ~ */

struct calculator_error : public runtime_error {
    calculator_error(const string& what) : runtime_error(what) { }
    virtual string error_type() { return "Calculator Error"; }
    string to_string() {
        return error_type() + ": " + runtime_error::what() + ".";
    }
};

struct invalid_function_call_error : public calculator_error {
    invalid_function_call_error(const string& what) : calculator_error(what) { }
    string error_type() override { return "Invalid Function Call"; }
};

struct invalid_argument_error : public calculator_error {
    invalid_argument_error(const string& what) : calculator_error(what) { }
    string error_type() override { return "Invalid Argument"; }
};

struct invalid_token_error : public calculator_error {
    invalid_token_error(const string& what) : calculator_error(what) { }
    string error_type() override { return "Invalid Token"; }
};

struct invalid_expression_error : public calculator_error {
    invalid_expression_error(const string& what) : calculator_error(what) { }
    string error_type() override { return "Invalid Expression"; }
};

/* ~ ~ ~ ~ ~ Exported Functions ~ ~ ~ ~ ~ */

extern "C" {
    void init();
    char *calculate_text(const char *);
}

#endif // CALCULATOR
