#include "backend.h"

/* ~ ~ ~ ~ ~ Math Functions and Constants ~ ~ ~ ~ ~ */


/*
 * Math Constants:
 * - PI          =~ 3.141592...
 * - E           =~ 2.718281...
 * - NAN         == NAN
 * - RAND_MAX    == RAND_MAX
 *
 * Math Variables:
 * - DERIV_STEP : step-size for nderiv function, = 1e-6 by default
 * - INT_NUM_RECTS : the default number of rectangles to use when calculating an integral
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
 *     - deg(r) : converts r (a quantity in radians) to degrees
 *     - rad(d) : converts d (a quantity in degrees) to radians
 *     - sin(x) : sine (x a is quantity in radians)
 *     - cos(x) : cosine (x a is quantity in radians)
 *     - tan(x) : tangent (x a is quantity in radians)
 *     - csc(x) : cosecant (x a is quantity in radians)
 *     - sec(x) : secant (x a is quantity in radians)
 *     - cot(x) : cotangent (x a is quantity in radians)
 *     - asin(x) : arcsine (x a is quantity in radians)
 *     - acos(x) : arccosine (x a is quantity in radians)
 *     - atan(x) : arctangent (x a is quantity in radians)
 *
 * - (Note: all trigonometric functions accept arguments in radians)
 *
 * - Specialized Math:
 *     - nderiv(f, d, x) : f'(x), with respect to variable name d
 *                         where f is an expression (the argument's tree is parsed and evaluated
 *                         directly); d is the differential identifier (variable whose identifier
 *                         should be shifted), and x is the x coordinate to evalate f'(x) at.
 *     - nintegral(f, d, s, e, r = INT_NUM_RECTS) : approximates the integral of f with respect
 *                                                  variable name d on the interval [s, e], with
 *                                                  r rectangles of equal width
 */

void init_math_constants() {
    identifier_table["PI"] = M_PI;
    identifier_table["E"] = M_E;
    identifier_table["NAN"] = NAN;
    identifier_table["RAND_MAX"] = RAND_MAX;

    identifier_table["DERIV_STEP"] = 1e-6;
    identifier_table["INT_NUM_RECTS"] = 100;
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
double to_degrees(vector<double>& args);
double to_radians(vector<double>& args);
double sine(vector<double> & args);
double cosine(vector<double> & args);
double tangent(vector<double> & args);
double cosecant(vector<double> & args);
double secant(vector<double> & args);
double cotangent(vector<double> & args);
double arcsine(vector<double> & args);
double arccosine(vector<double> & args);
double arctangent(vector<double> & args);

double numeric_derivative(vector<unique_ptr<TreeNode>>& args);
double numeric_integral(vector<unique_ptr<TreeNode>>& args);

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
    fn_table["rand"] = make_unique<NDoubleFunction<0>>(random_int);
    fn_table["factorial"] = make_unique<NDoubleFunction<1>>(factorial);
    fn_table["perm"] = make_unique<NDoubleFunction<2>>(permutation);
    fn_table["comb"] = make_unique<NDoubleFunction<2>>(combination);
    fn_table["deg"] = make_unique<NDoubleFunction<1>>(to_degrees);
    fn_table["rad"] = make_unique<NDoubleFunction<1>>(to_radians);
    fn_table["sin"] = make_unique<NDoubleFunction<1>>(sine);
    fn_table["cos"] = make_unique<NDoubleFunction<1>>(cosine);
    fn_table["tan"] = make_unique<NDoubleFunction<1>>(tangent);
    fn_table["csc"] = make_unique<NDoubleFunction<1>>(cosecant);
    fn_table["sec"] = make_unique<NDoubleFunction<1>>(secant);
    fn_table["cot"] = make_unique<NDoubleFunction<1>>(cotangent);
    fn_table["asin"] = make_unique<NDoubleFunction<1>>(arcsine);
    fn_table["acos"] = make_unique<NDoubleFunction<1>>(arccosine);
    fn_table["atan"] = make_unique<NDoubleFunction<1>>(arctangent);

    // specialized:
    fn_table["nderiv"] = make_unique<RawFunction>(numeric_derivative);
    fn_table["nintegral"] = make_unique<RawFunction>(numeric_integral);
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
    for(int i = 1; i < args.size(); i++) result = min(result, args[i]->eval());
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
    return ceil(args[0]);
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

double factorial(long long arg) {
    if(arg > 100) return NAN; // hard-coded upper-limit for unreasonable calculations

    if(arg <= 1) return 1.0;
    return arg * factorial(arg - 1);
}

double factorial(vector<double>& args) {
    return factorial((long long)args[0]);
}

double permutation(vector<double>& args) {
    long long n = args[0], r = args[1];
    if(n - r > 100) return NAN; // hard-coded upper-limit for unreasonable calculations

    double result = 1;
    for(long long i = n - r + 1; i <= n; i++) result *= i;
    return result;
}

double combination(vector<double>& args) {
    double r = args[1];
    return permutation(args) / factorial((long long)r);
}

double to_degrees(vector<double>& args) {
    return args[0] * 180 / M_PI;
}

double to_radians(vector<double>& args) {
    return args[0] * M_PI / 180;
}

double sine(vector<double> & args) {
    return sin(args[0]);
}

double cosine(vector<double> & args) {
    return cos(args[0]);
}

double tangent(vector<double> & args) {
    return tan(args[0]);
}

double cosecant(vector<double> & args) {
    return 1 / sin(args[0]);
}

double secant(vector<double> & args) {
    return 1 / cos(args[0]);
}

double cotangent(vector<double> & args) {
    return 1 / tan(args[0]);
}

double arcsine(vector<double> & args) {
    return asin(args[0]);
}

double arccosine(vector<double> & args) {
    return acos(args[0]);
}

double arctangent(vector<double> & args) {
    return atan(args[0]);
}

/* ~ ~ ~ Specialized Math Functions ~ ~ ~ */

double numeric_derivative(vector<unique_ptr<TreeNode>>& args) {
    if(args.size() != 3) return NAN; // TODO throw error - wrong number of args
    if(!args[1]->is_var()) return NAN; // TODO throw an error - arg 2 is not an identifier

    string diff_id = ((VariableNode *) args[1].get())->id; // differential's identifier

    double old_diff_value = get_id_value(diff_id); // store diff_id's old value (to restore)
    double step = get_id_value("DERIV_STEP");
    double x = args[2]->eval();

    set_id_value(diff_id, x);
    double f_x = args[0]->eval();

    set_id_value(diff_id, x + step);
    double f_x_plus_step = args[0]->eval();

    set_id_value(diff_id, old_diff_value);
    return (f_x_plus_step - f_x) / step;
}

double numeric_integral(vector<unique_ptr<TreeNode>>& args) {
    if(args.size() != 4 && args.size() != 5) return NAN; // TODO throw error - wrong number of args

    if(!args[1]->is_var()) return NAN; // TODO throw an error - arg 2 is not an identifier

    string diff_id = ((VariableNode *) args[1].get())->id; // differential's identifier
    double num_rects = args.size() == 5 ? args[4]->eval() : get_id_value("INT_NUM_RECTS");
    double old_diff_value = get_id_value(diff_id); // store diff_id's old value (to restore)
    double s = args[2]->eval(), e = args[3]->eval();
    double rect_width = (e - s)  / num_rects;
    double sum = 0;

    // for each rectangle
    for(int i = 0; i < (int) num_rects; i++) {
        set_id_value(diff_id, s + i * rect_width + rect_width / 2); // set x coord to center of rect
        sum += args[0]->eval() * rect_width;
    }

    set_id_value(diff_id, old_diff_value);
    return sum;
}
