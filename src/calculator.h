#ifndef CALCULATOR
#define CALCULATOR

#include <bits/stdc++.h> // TODO reduce dependencies

using namespace std;

/* ~ ~ ~ Parsing Tree Class ~ ~ ~ */

struct TreeNode { // Abstract superclass for all other node types
    virtual string to_string() = 0;
    virtual double eval() = 0;
    virtual ~TreeNode() { }
    virtual bool is_var() { return false; }
};


/* ~ ~ ~ Calculator Backend Functions ~ ~ ~ */

double get_id_value(string id);
double set_id_value(string id, double val);
double call_function(string id, vector<unique_ptr<TreeNode>>& args);
void assign_function(string id, vector<string>&& args, unique_ptr<TreeNode>&& tree);

void init_math_constants();
void init_math_functions();

const double DERIV_STEP = 1e-6;

#endif // CALCULATOR
