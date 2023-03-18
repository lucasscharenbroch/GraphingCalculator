#include "parser.h"

/* ~ ~ ~ ~ ~ Gramatical Parsing ~ ~ ~ ~ ~ */

/*
 * Grammar outline:
 *
 * Terminal symbols:
 *
 * "Literals":
 * - Variables
 * - Numeric Literals
 *      - decimal (floating-point and scientific notation)
 *      - binary (integer) and hex (integer) literals) (leading "0b" and "0x", respectively)
 *
 * "Operations"
 * - "+" addition
 * - "-" subtraction
 * - "-" unary negation
 * - "*" multiplication
 * - "/" float division
 * - "//" int division
 * - "%" modulus (converts arguments to int)
 * - "^" exponentiation
 * - "(" ")" grouping, function calls
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
 * X means "Exponential" (parsed before ^)
 * FN means "Function Call"
 * ARGS means "Argument Tuple"
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
 *   -> FN = E
 *   -> T == E
 *   -> T != E
 *   -> T < E
 *   -> T <= E
 *   -> T > E
 *   -> T >= E
 *   -> T {(+|-) T}
 *
 * T -> F {(*|/|//|%|^) F}
 *
 * F -> X {^ X}
 *
 * X -> -X
 *   -> (E)
 *   -> NUM
 *   -> VAR
 *   -> FN
 *
 * FN -> VAR{'}(ARGS)
 *       VAR(ARGS)
 *
 * ARGS -> {E {, E {, ...}}}
 */

/* ~ ~ ~ ~ ~ Grammar Parsing Functions ~ ~ ~ ~ ~ */

/*
 * NOTE: it is critical that the i reference returns to its original position
 * when parsing a nonterminal fails (as errors in parsing is regular behavior,
 * and no tokens shold be skipped). To avoid this, i's value is copied into j
 * at the beginning of each parsing function, and is only copied back before a
 * successful return.
 */

/*
 * S -> E$
 */
unique_ptr<TreeNode> parseS(vector<unique_ptr<Token>>& tokens, int i) {
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    unique_ptr<TreeNode> tree = parseE(tokens, j);

    if(j != tokens.size()) return nullptr; // expression doesn't span all tokens

    i = j;
    return tree;
}

/*
 * E -> VAR = E
 *   -> FN = E
 *   -> T == E
 *   -> T != E
 *   -> T < E
 *   -> T <= E
 *   -> T > E
 *   -> T >= E
 *   -> T ^ E
 *   -> T {(+|-) T}
 */
unique_ptr<TreeNode> parseE(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    // VAR = E
    if(tokens[j]->is_var() && j + 1 != tokens.size() &&
       tokens[j + 1]->is_op() && tokens[j + 1]->str_val() == "=") {
        string var_id = tokens[j]->str_val();
        j += 2; // jump over VAR and =
        unique_ptr<TreeNode> rhs = parseE(tokens, j);

        if(rhs != nullptr) {
            i = j;
            return make_unique<AssignmentNode>(var_id, move(rhs));
        } else j = i; // reset j to i, and try other rules
    }

    // -> FN = E
    unique_ptr<TreeNode> fn = parseFN(tokens, j);
    if(fn != nullptr && j != tokens.size() && tokens[j]->is_op() &&
                                              tokens[j]->str_val() == "=") {
        j++; // jump over =
        unique_ptr<TreeNode> e = parseE(tokens, j);

        if(e != nullptr){
            i = j;
            return make_unique<FunctionAssignmentNode>(move(fn), move(e));
        }
    }
    j = i; // reset j to i, and try other rules

    // -> T == E
    // -> T != E
    // -> T < E
    // -> T <= E
    // -> T > E
    // -> T >= E
    // -> T {(+|-) T}
    unique_ptr<TreeNode> lhs = parseT(tokens, j);
    if(lhs == nullptr) return nullptr; // failed to read term

    // T$ or T? (where ? isn't one of the above options) => skip loop
    // ==, !=, <, <=, >, >=, +, or -                     => loop
    while(j != tokens.size() && tokens[j]->is_op()) {
        string op = tokens[j]->str_val();
        if(op == "==" || op == "!=" || op == "<" || op == "<=" ||
           op == ">" || op == ">=" || op == "+" || op == "-") j++;
        else break;

        unique_ptr<TreeNode> rhs = (op == "+" || op == "-") ? parseT(tokens, j) : parseE(tokens, j);
        if(rhs == nullptr) {
            i = j - 1; // token before op
            return lhs;
        }

        // continue loop for + and - to maintain order of operations
        if(op == "+") lhs = make_unique<AdditionNode>(move(lhs), move(rhs));
        else if(op == "-") lhs = make_unique<SubtractionNode>(move(lhs), move(rhs));
        else {
            i = j;
            if(op =="==") return make_unique<EqualityNode>(move(lhs), move(rhs));
            else if(op =="!=") return make_unique<InequalityNode>(move(lhs), move(rhs));
            else if(op =="<") return make_unique<LessThanNode>(move(lhs), move(rhs));
            else if(op ==">") return make_unique<GreaterThanNode>(move(lhs), move(rhs));
            else if(op =="<=") return make_unique<LessOrEqualNode>(move(lhs), move(rhs));
            else if(op ==">=") return make_unique<GreaterOrEqualNode>(move(lhs), move(rhs));
        }
    }

    i = j;
    return lhs;
}

/*
 * T -> F {(*|/|//|%) F}
 */
unique_ptr<TreeNode> parseT(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    // -> F {(*|/|//|%) F}
    unique_ptr<TreeNode> lhs = parseF(tokens, j);
    if(lhs == nullptr) return nullptr;

    while(j != tokens.size() && tokens[j]->is_op()) {
        string op = tokens[j]->str_val();
        if(op == "*" || op == "/" || op == "//" || op == "%") j++;
        else break;

        unique_ptr<TreeNode> rhs = parseF(tokens, j);
        if(rhs == nullptr) return nullptr;

        if(op == "*") lhs = make_unique<MultiplicationNode>(move(lhs), move(rhs));
        else if(op == "/") lhs = make_unique<DivisionNode>(move(lhs), move(rhs));
        else if(op == "//") lhs = make_unique<IntDivisionNode>(move(lhs), move(rhs));
        else if(op == "%") lhs = make_unique<ModulusNode>(move(lhs), move(rhs));
    }

    i = j;
    return lhs;
}

/*
 * F -> X {^ X}
 */
unique_ptr<TreeNode> parseF(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    if(j == tokens.size()) return nullptr;

    unique_ptr<TreeNode> base = parseX(tokens, j);
    if(base == nullptr) return nullptr;

    while(j != tokens.size() && tokens[j]->is_op() && tokens[j]->str_val() == "^") {
        j++;
        unique_ptr<TreeNode> exp = parseX(tokens, j);
        if(exp == nullptr) {
            j--; // set j back to '^' token
            break;
        }

        base = make_unique<PowerNode>(move(base), move(exp));
    }

    i = j;
    return base;
}

/*
 * X -> -X
 *   -> (E)
 *   -> NUM
 *   -> FN
 *   -> VAR
 */
unique_ptr<TreeNode> parseX(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    // X -> -X
    if(tokens[j]->is_op() && tokens[j]->str_val() == "-") {
        j++;
        unique_ptr<TreeNode> x = parseX(tokens, j);
        if(x == nullptr) return nullptr;
        i = j;
        return make_unique<NegationNode>(move(x));
    }


    // -> (E)
    if(tokens[j]->is_op() && tokens[j]->str_val() == "(") {
        unique_ptr<TreeNode> e = parseE(tokens, ++j);
        if(j == tokens.size() || !tokens[j]->is_op() || tokens[j]->str_val() != ")")
            return nullptr;
        i = j + 1;
        return e;
    }

    // -> NUM
    if(tokens[j]->is_num()) {
        i = j + 1;
        return make_unique<NumberNode>(tokens[j]->dbl_val());
    }

    // -> FN
    unique_ptr<TreeNode> fn = parseFN(tokens, j);
    if(fn != nullptr) {
        i = j;
        return fn;
    }

    // -> VAR
    if(tokens[j]->is_var()) {
        i = j + 1;
        return make_unique<VariableNode>(tokens[j]->str_val());
    }

    return nullptr;
}

/*
 * FN -> VAR{'}(ARGS)
 *       VAR(ARGS)
 */
unique_ptr<TreeNode> parseFN(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    if(!tokens[j]->is_var()) return nullptr;
    string var_id = tokens[j++]->str_val();

    // parse {'}
    int nth_deriv = 0;
    while(j != tokens.size() && tokens[j]->is_op() && tokens[j]->str_val() == "'") nth_deriv++, j++;

    if(j == tokens.size() || !tokens[j]->is_op() || tokens[j]->str_val() != "(") return nullptr;
    vector<unique_ptr<TreeNode>> args = parseARGS(tokens, ++j);
    if(j == tokens.size() || !tokens[j]->is_op() || tokens[j]->str_val() != ")") return nullptr;

    i = j + 1; // i = char after ')'

    if(nth_deriv == 0) return make_unique<FunctionCallNode>(var_id, move(args));
    return make_unique<DerivativeNode>(var_id, move(args), nth_deriv);
}

/*
 * ARGS -> {E {, E {, ...}}}
 */
vector<unique_ptr<TreeNode>> parseARGS(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    vector<unique_ptr<TreeNode>> args;

    unique_ptr<TreeNode> current;

    while((current = parseE(tokens, j)) != nullptr) {
        args.push_back(move(current));
        i = j; // i is only updated after adding valid expresion so trailing commas aren't allowed
        if(j == tokens.size() || !tokens[j]->is_op() || tokens[j]->str_val() != ",") break;
        j++; // skip over comma, and try to parse another.
    }

    return args;
}
