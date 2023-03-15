#include "backend.h"

/* ~ ~ ~ ~ ~ Math Functions and Constants ~ ~ ~ ~ ~ */


/*
 * Math Constants:
 * - PI          =~ 3.141592...
 * - E           =~ 2.718281...
 * - NAN         == NAN
 * - RAND_MAX    == RAND_MAX
 *
 * Math Functions:
 * - Debug Functions:
 *     - print_tree(args) : prints the to_string of all arguments' trees
 * - Variadic Functions:
 *     - min(n, ...) : minimum of arguments (1 or more)
 *     - max(n, ...) : maximum of arguments (1 or more)
 *     - gcd(n, ...) : greatest common divisor of all arguments (1 or more); rounds all args to int.
 * - Fundamental Math:
 *     - floor(n) : round down to int
 *     - ceil(n) : round up to int
 *     - abs(n) : absolute value
 *     - pow(b, e) : exponentiation; analogous to "(b ^ e)"
 *     - rand() : random integer
 *     - factorial(n) : n!
 *     - perm(n, r) : n! / (n - r)!
 *     - comb(n, r) : n! / ((n - r)! * r!)
 * - Specialized Math:
 *     - nderiv(f, d, x) : f'(x), with respect to variable name "d"
 *                         where f is a function (the argument's tree is parsed and evaluated
 *                         directly; d is the differential identifier (variable whose identifier
 *                         should be shifted), and x is the x coordinate to evalate f'(x) at.
 *     - TODO add nintegral?
 */

void init_math_constants() {
    identifier_table["PI"] = M_PI;
    identifier_table["E"] = M_E;
    identifier_table["NAN"] = NAN;
    identifier_table["RAND_MAX"] = RAND_MAX;
}

double print_tree(vector<unique_ptr<TreeNode>>& args);

double vararg_max(vector<unique_ptr<TreeNode>>& args);
double vararg_min(vector<unique_ptr<TreeNode>>& args);
double vararg_gcd(vector<unique_ptr<TreeNode>>& args);

double float_floor(vector<double>& args);
double float_ceil(vector<double>& args);
double power(vector<double>& args);
double absolute_val(vector<double>& args);
double random_int(vector<double>& args);
double factorial(vector<double>& args);
double permutation(vector<double>& args);
double combination(vector<double>& args);

double numeric_derivative(vector<unique_ptr<TreeNode>>& args);


void init_math_functions() {
    // debug:
    fn_table["print_tree"] = make_unique<RawFunction>(print_tree);

    // variadic:
    fn_table["max"] = make_unique<RawFunction>(vararg_max);
    fn_table["min"] = make_unique<RawFunction>(vararg_min);
    fn_table["gcd"] = make_unique<RawFunction>(vararg_gcd);

    // fundamental:
    fn_table["floor"] = make_unique<NDoubleFunction<1>>(float_floor);
    fn_table["ceil"] = make_unique<NDoubleFunction<1>>(float_ceil);
    fn_table["abs"] = make_unique<NDoubleFunction<1>>(absolute_val);
    fn_table["pow"] = make_unique<NDoubleFunction<2>>(power);
    fn_table["rand"] = make_unique<NDoubleFunction<1>>(random_int);
    fn_table["factorial"] = make_unique<NDoubleFunction<1>>(factorial);
    fn_table["perm"] = make_unique<NDoubleFunction<2>>(permutation);
    fn_table["comb"] = make_unique<NDoubleFunction<2>>(combination);

    // specialized:
    fn_table["nderiv"] = make_unique<RawFunction>(numeric_derivative);
}

/* ~ ~ ~ Debug Functions ~ ~ ~ */

// print_tree: debug function - prints out the parsed grammar tree of
// each of the passed arguments.
// TODO modify this for webpage
double print_tree(vector<unique_ptr<TreeNode>>& args) {
    for(auto& arg : args) cout << arg->to_string() << endl;
    return NAN;
}

/* ~ ~ ~ Vararg Functions ~ ~ ~ */

double vararg_max(vector<unique_ptr<TreeNode>>& args) {
    if(args.size() == 0) return NAN; // TODO throw exception: not enough args

    double result = args[0]->eval();
    for(int i = 1; i < args.size(); i++) result = max(result, args[i]->eval());
    return result;
}

double vararg_min(vector<unique_ptr<TreeNode>>& args) {
    if(args.size() == 0) return NAN; // TODO throw exception: not enough args

    double result = args[0]->eval();
    for(int i = 1; i < args.size(); i++) result = max(result, args[i]->eval());
    return result;
}

double vararg_gcd(vector<unique_ptr<TreeNode>>& args) {
    if(args.size() == 0) return NAN; // TODO throw exception: not enough args

    double result = args[0]->eval();
    for(int i = 1; i < args.size(); i++)
        result = gcd((long long) result, (long long) args[i]->eval());
    return result;
}

/* ~ ~ ~ Fundamental Math Functions ~ ~ ~ */

double float_floor(vector<double>& args) {
    return floor(args[0]);
}

double float_ceil(vector<double>& args) {
    return floor(args[0]);
}

double absolute_val(vector<double>& args) {
    return abs(args[0]);
}

double power(vector<double>& args) {
    return pow(args[0], args[1]);
}

double random_int(vector<double>& args) {
    return (double) rand();
}

double factorial(vector<double>& args) {
    return NAN; // TODO multiply 1 to n
}

double permutation(vector<double>& args) {
    return NAN; // TODO multiply r to n
}

double combination(vector<double>& args) {
    return NAN // TODO implement
}

/* ~ ~ ~ Specialized Math Functions ~ ~ ~ */

double numeric_derivative(vector<unique_ptr<TreeNode>>& args) {
    return NAN; // TODO implement
}
