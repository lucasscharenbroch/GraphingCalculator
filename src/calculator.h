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
#include <emscripten.h>
#include <cassert>

using namespace std;

extern double last_answer; // holds result of last computation

/* ~ ~ ~ ~ ~ Parsing Tree Class ~ ~ ~ ~ ~ */

struct TreeNode { // Abstract superclass for all other node types
    virtual string to_string() = 0;
    virtual double eval() = 0;
    virtual unique_ptr<TreeNode> copy() = 0;
    virtual ~TreeNode() { }
    virtual bool is_var() { return false; }
    virtual bool is_fn_call() { return false; }
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

/* ~ ~ ~ ~ ~ Graping Functions ~ ~ ~ ~ ~ */

bool add_to_graph(unique_ptr<TreeNode>&& expr);
void draw_axes();
void undraw_axes();

/* ~ ~ ~ ~ ~ Exported Functions ~ ~ ~ ~ ~ */

extern "C" {
    /* ~ Calculator ~ */
    void init();
    char *calculate_text(const char *);

    /* ~ Graphing ~ */
    int *get_graph_buffer();
    bool remove_from_graph(int);
    void resize_graph(int, int, double, double, double, double);
    void draw_trace_line(int x_c);
}

#endif // CALCULATOR
