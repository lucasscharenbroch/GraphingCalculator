#ifndef PARSER
#define PARSER

#include "calculator.h"

/* ~ ~ ~ Token Classes ~ ~ ~ */

enum class TokenType {
    VAR,      // variable or function name
    NUM,      // numeric (float) literal
    OP        // +, -, *, /, (, ), etc.
};

/*
 * A generic container for storing a variety of types associated with
 * tokens - this allows easy polymorphic access to a value of one of these types.
 */
struct TokenVal {
    string s;
    double d;

    TokenVal(string str): s(str), d(NAN) { }
    TokenVal(double dub): s("None"), d(dub) { }
    TokenVal(): s("None"), d(NAN) { }
};

// Token base-class (abstract - only used for inheritance)
struct Token {
    TokenType type;
    TokenVal val;
    Token(TokenType tp, TokenVal vl): type(tp), val(vl) { }
    virtual string to_string() = 0;
};

struct VarToken : Token {
    string identifier;
    VarToken(string id): Token(TokenType::VAR, {id}), identifier(id) { }
    string to_string() override { return identifier; }
};

struct NumToken : Token {
    double value;
    NumToken(double val): Token(TokenType::NUM, {val}), value(val) { }
    string to_string() override { return std::to_string(value); }
};

struct OpToken : Token {
    string op;
    OpToken(string o): Token(TokenType::OP, {o}), op(o) { }
    string to_string() override { return op; }
};

/* ~ ~ ~ Lexing Functions ~ ~ ~ */

vector<unique_ptr<Token>> tokenize(const string& expr_str);

/* ~ ~ ~ Operation (Parsing Tree Node) Classes ~ ~ ~ */

struct TreeNode { // Abstract superclass for all other node types
    virtual string to_string() = 0;
    virtual double eval() = 0;
};

struct NumberNode : TreeNode {
    double val;

    NumberNode(double v) : val(v) { }

    string to_string() override {
        return std::to_string(val);
    }

    double eval() override {
        return val;
    }
};

struct VariableNode : TreeNode {
    string id;

    VariableNode(string i) : id(i) { }

    string to_string() override {
        return id;
    }

    double eval() override {
        return get_id_value(id);
    }
};

struct AdditionNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    AdditionNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                       right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " + " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() + right->eval();
    }
};

struct SubtractionNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    SubtractionNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                          right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " - " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() - right->eval();
    }
};

struct NegationNode : TreeNode {
    unique_ptr<TreeNode> node;

    NegationNode(unique_ptr<TreeNode>&& n) : node(move(n)) { }

    string to_string() override {
        return "(-" + node->to_string() + ')';
    }

    double eval() override {
        return -1 * node->eval();
    }
};

struct MultiplicationNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    MultiplicationNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                             right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " * " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() * right->eval();
    }
};

struct DivisionNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    DivisionNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                       right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " / " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() / right->eval();
    }
};

struct IntDivisionNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    IntDivisionNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                          right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " // " + right->to_string() + ')';
    }

    double eval() override {
        long long numerator = (long long)left->eval();
        long long denominator = (long long)right->eval();

        if(denominator == 0) return NAN;
        return numerator / denominator;
    }
};

struct ModulusNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    ModulusNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                      right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " // " + right->to_string() + ')';
    }

    double eval() override {
        long long numerator = (long long)left->eval();
        long long denominator = (long long)right->eval();

        if(denominator == 0) return NAN;
        return numerator % denominator;
    }
};

struct AssignmentNode : TreeNode {
    string lvalue_id;
    unique_ptr<TreeNode> rvalue;

    AssignmentNode(string l, unique_ptr<TreeNode>&& r) : lvalue_id(l), rvalue(move(r)) { }

    string to_string() override {
        return '(' + lvalue_id + " = " + rvalue->to_string() + ')';
    }

    double eval() override {
        return set_id_value(lvalue_id, rvalue->eval());
    }
};

struct PowerNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    PowerNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                    right(move(r)) { }
    string to_string() override {
        return '(' + left->to_string() + " ^ " + right->to_string() + ')';
    }

    double eval() override {
        return pow(left->eval(), right->eval());
    }
};

struct EqualityNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    EqualityNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                       right(move(r)) { }
    string to_string() override {
        return '(' + left->to_string() + " == " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() == right->eval();
    }
};

struct InequalityNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    InequalityNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                         right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " != " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() == right->eval();
    }
};

struct LessThanNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    LessThanNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                       right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " < " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() < right->eval();
    }
};

struct GreaterThanNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    GreaterThanNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                          right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " > " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() > right->eval();
    }
};

struct LessOrEqualNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    LessOrEqualNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                          right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " <= " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() <= right->eval();
    }
};

struct GreaterOrEqualNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    GreaterOrEqualNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) : left(move(l)), 
                                                                             right(move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " >= " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() >= right->eval();
    }
};

struct FunctionCallNode : TreeNode {
    string function_id;
    vector<unique_ptr<TreeNode>> args;

    FunctionCallNode(string i, vector<unique_ptr<TreeNode>>&& a) : function_id(i), args(move(a)) { }

    string to_string() override {
        string s = "(" + function_id + "(";

        for(int i = 0; i < args.size(); i++) {
            s += args[i]->to_string();
            if(i != args.size() - 1) s += ", ";
        }

        s += "))";
        return s;
    }

    double eval() override {
        return 0; // TODO decide how to approach this
    }
};

/* ~ ~ ~ Grammar Parsing Functions ~ ~ ~ */

unique_ptr<TreeNode> parseS(vector<unique_ptr<Token>>& tokens, int i);
unique_ptr<TreeNode> parseE(vector<unique_ptr<Token>>& tokens, int& i);
unique_ptr<TreeNode> parseT(vector<unique_ptr<Token>>& tokens, int& i);
unique_ptr<TreeNode> parseF(vector<unique_ptr<Token>>& tokens, int& i);
vector<unique_ptr<TreeNode>> parseARGS(vector<unique_ptr<Token>>& tokens, int& i);

#endif // PARSER
