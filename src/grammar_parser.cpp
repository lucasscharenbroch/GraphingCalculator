#include "calculator.h"

/* ~ ~ ~ ~ ~ Gramatical Parsing ~ ~ ~ ~ ~ */

/*
 * Grammar outline:
 *
 * Terminal symbols:
 *
 * "Literals":
 * - Variables
 * - Numeric Literals (decimal (floating-point and scientific notation), binary (integer) and hex (integer) literals)
 * - Function Calls "function-name(args, ...)"; support for variadic functions.
 *
 * "Operations"
 * - "+" addition
 * - "-" subtraction
 * - "-" unary negation
 * - "*" multiplication
 * - "/" float division
 * - "(" ")" grouping
 * - "=" assignment
 * - "==" equality
 * - "!=" inequality
 * - "<" less than
 * - "<=" less than or equal to
 * - ">" greater than
 * - ">=" greater than or equal to
 *
 * Nonterminal Symbols:
 *
 * S means "Statement"
 * E means "Expression" (parsed before () and comparison operators)
 * T means "Term" (parsed before + and -)
 * F means "Factor" (parsed before * and /)
 * ARGS means "Argument tuple"
 *
 * Terminal Symbols:
 * VAR means "Variable name" (this also encompasses function names)
 * NUM means "Numeric literal"
 * "" refers to an empty string
 * $ refers to the end of input
 *
 * Grammar Rules:
 *
 * S -> E$
 *
 * E -> VAR = E
 *   -> T == E
 *   -> T != E
 *   -> T < E
 *   -> T <= E
 *   -> T > E
 *   -> T >= E
 *   -> T {+|- T}
 *
 * T -> -F
 *   -> F {*|/ T}
 *
 * F -> (E)
 *   -> VAR
 *   -> NUM
 *   -> VAR(ARGS)
 *
 * ARGS -> E, ARGS
 *      -> E
 *      -> ""
 */

int main() { // TODO remove (test method)
    return 0;
}
