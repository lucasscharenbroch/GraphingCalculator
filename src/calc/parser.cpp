#include "parser.h"

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Gramatical Parsing ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

/*
 * Grammar outline:
 *
 * Terminal symbols:
 *
 * "Literals":
 * - Variables
 * - Numeric Literals (see lexer.cpp)
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
 * ARGS means "Argument List"
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
 * E -> T = E                        // T must be a variable or function
 *   -> T {(+|-) T} [(==|!=|<|<=|>|>=) E]
 *
 * T -> F {(*|/|//|%|<nothing>) F}
 *
 * F -> [-]X [{(^|**) F]             // don't parse negation when parsing implicit multiplication
 *
 * X -> (E)
 *   -> NUM
 *   -> VAR[{'}(ARGS)]
 *
 * ARGS -> {E {, E {, ...}}}
 */

/* ~ ~ ~ ~ ~ Parser State ~ ~ ~ ~ ~ */

// push/pop saves/restores the values of the following global variables
// (just parsing_impl_mult for now...)

bool parsing_impl_mult = false;

struct parser_state {
    bool parsing_impl_mult;

    parser_state(bool parsing_impl_mult) :
        parsing_impl_mult(parsing_impl_mult) { }
};

vector<parser_state> state_stack;

void push_parser_state() {
    state_stack.push_back(parser_state(parsing_impl_mult));
}

void pop_parser_state() {
    parser_state popped_state = state_stack.back();

    parsing_impl_mult = popped_state.parsing_impl_mult;

    state_stack.pop_back();
}

void reset_parser_state() {
    parsing_impl_mult = false;
}

void reset_state_stack() {
    while(state_stack.size()) state_stack.pop_back(); // empty the state stack
    reset_parser_state();
}

/* ~ ~ ~ ~ ~ Token Fetching ~ ~ ~ ~ ~ */

int i;
vector<unique_ptr<Token>> tokens;
unique_ptr<Token> eos = unique_ptr<Token> {new NumToken(0)}; // dummy variable


unique_ptr<Token>& next_tok() {
    return i == tokens.size() ? eos : tokens[i++];
}

void unget_tok() {
    i--;
}


/* ~ ~ ~ ~ ~ Grammar Parsing ~ ~ ~ ~ ~ */

unique_ptr<TreeNode> parseS(vector<unique_ptr<Token>>&& token_vec) {
    i = 0;
    tokens = std::move(token_vec);
    reset_state_stack();

    unique_ptr<TreeNode> e = parseE();
    if(&next_tok() != &eos) throw invalid_expression_error("error while parsing");
    return e;
}

unique_ptr<TreeNode> _parseE() {
    unique_ptr<TreeNode> lhs = parseT();
    if(lhs == nullptr) return nullptr;

    while(true) {
        unique_ptr<Token>& op = next_tok();
        if(&op == &eos) return lhs;

        if(!op->is_op() ||
            op->str_val() != "="  &&
            op->str_val() != "+"  &&
            op->str_val() != "-"  &&
            op->str_val() != "==" &&
            op->str_val() != "!=" &&
            op->str_val() != "<"  &&
            op->str_val() != "<=" &&
            op->str_val() != ">"  &&
            op->str_val() != ">=") {
            unget_tok();
            return lhs;
        }

        if(op->str_val() == "=" && !lhs->is_var() && !lhs->is_fn_call())
            throw invalid_expression_error("invalid lhs in assignment");

        bool is_sum = op->str_val() == "+" || op->str_val() == "-";
        unique_ptr<TreeNode> rhs = is_sum ?  parseT() : parseE();
        if(rhs == nullptr) throw invalid_expression_error("expected operand after `" + op->str_val() + "`");
        lhs = unique_ptr<TreeNode> {new BinaryOpNode(std::move(lhs), std::move(rhs), op->str_val())};

        if(!is_sum) return lhs;
    }
}

unique_ptr<TreeNode> parseE() {
    unique_ptr<TreeNode> ret;

    push_parser_state();
    reset_parser_state();
    ret = _parseE();
    pop_parser_state();

    return ret;
}

unique_ptr<TreeNode> parseT() {
    unique_ptr<TreeNode> lhs = parseF();
    if(lhs == nullptr) return nullptr;

    while(true) {
        unique_ptr<Token>& op = next_tok();
        if(&op == &eos) return lhs;
        bool is_implicit = false;

        if(!op->is_op() ||
           op->str_val() != "*"  &&
           op->str_val() != "/"  &&
           op->str_val() != "//" &&
           op->str_val() != "%") {
            unget_tok();
            is_implicit = true;
        }

        if(is_implicit) parsing_impl_mult = true;
        unique_ptr<TreeNode> rhs = parseF();
        parsing_impl_mult = false;

        if(rhs == nullptr) {
            if(is_implicit) return lhs;
            else throw invalid_expression_error("expected operand after `" +  op->str_val() + "`");
        }
        lhs = unique_ptr<TreeNode> {new BinaryOpNode(std::move(lhs), std::move(rhs),
                                                     is_implicit ? "*" : op->str_val())};
    }
}

unique_ptr<TreeNode> parseF() {
    unique_ptr<Token>& tok = next_tok();
    if(&tok == &eos) return nullptr;

    bool negated = false;

    if(tok->is_op() && tok->str_val() == "-" && !parsing_impl_mult) {
        negated = true;
    } else unget_tok();

    unique_ptr<TreeNode> lhs = parseX();
    if(lhs == nullptr) {
        if(!negated) return nullptr;
        else throw invalid_expression_error("unexpected negation");
    }

    unique_ptr<Token>& op = next_tok();
    if(&op == &eos); // do nothing
    else if(!op->is_op() ||
        op->str_val() != "^"  &&
        op->str_val() != "**") {
        unget_tok();
    } else {
        unique_ptr<TreeNode> rhs = parseF();
        if(rhs == nullptr) throw invalid_expression_error("expected operand after `" + op->str_val() + "`");
        lhs = unique_ptr<TreeNode> {new BinaryOpNode(std::move(lhs), std::move(rhs), op->str_val())};
    }

    if(!negated) return lhs;
    return unique_ptr<TreeNode> {new UnaryOpNode(std::move(lhs), "-")};
}

unique_ptr<TreeNode> parseX() {
    unique_ptr<Token>& tok = next_tok();
    if(&tok == &eos) return nullptr;
    if(tok->is_num()) return unique_ptr<TreeNode> {new NumberNode(tok->dbl_val())};
    if(tok->is_op() && tok->str_val() == "(") {
        unique_ptr<TreeNode> exp = parseE();
        if(exp == nullptr) throw invalid_expression_error("empty or invalid parenthetical");
        unique_ptr<Token>& closer = next_tok();
        if(&closer == &eos || !closer->is_op() || closer->str_val() != ")")
            throw invalid_expression_error("unclosed/mismatched parenthesis");
        return exp;
    } else if(!tok->is_var()) {
        unget_tok();
        return nullptr;
    }

    string id_val = tok->str_val();

    // VAR[{'}(ARGS)]
    int deriv_degree = 0;
    while(true) { // read {'}
        unique_ptr<Token>& next = next_tok();
        if(&next != &eos && next->is_op() && next->str_val() == "'") deriv_degree++;
        else {
            if(&next != &eos) unget_tok();
            break;
        }
    }

    unique_ptr<Token>& opener = next_tok();
    if(&opener == &eos) {
        if(deriv_degree) throw invalid_expression_error("trailing apostrophe");
        return unique_ptr<TreeNode> {new VariableNode(id_val)};
    } else if(!opener->is_op() || opener->str_val() != "(") {
        unget_tok();
        return unique_ptr<TreeNode> {new VariableNode(id_val)};
    }

    vector<unique_ptr<TreeNode>> arg_list = parseARGS();

    unique_ptr<Token>& closer = next_tok();
    if(&closer == &eos || !closer->is_op() || closer->str_val() != ")")
        throw invalid_expression_error("unclosed/mismatched parenthesis");

    if(!deriv_degree)
        return unique_ptr<TreeNode> {new FunctionCallNode(id_val, std::move(arg_list))};
    else
        return unique_ptr<TreeNode> {new DerivativeNode(id_val, std::move(arg_list), deriv_degree)};
}

vector<unique_ptr<TreeNode>> parseARGS() {
    vector<unique_ptr<TreeNode>> result;
    unique_ptr<TreeNode> node;

    while((node = parseE())) {
        result.push_back(std::move(node));
        unique_ptr<Token>& comma = next_tok();
        if(&comma == &eos || !comma->is_op() || comma->str_val() != ",") {
            if(&comma != &eos) unget_tok();
            break;
        }
    }

    return result;
}
