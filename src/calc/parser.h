#ifndef PARSER
#define PARSER

#include "../calculator.h"

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Token Classes ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

// Token base-class (abstract - only used for inheritance)
struct Token {
    virtual string to_string() = 0;
    virtual string str_val() { return ""; }
    virtual double dbl_val() { return NAN; }
    virtual bool is_var() { return false; }
    virtual bool is_num() { return false; }
    virtual bool is_op() { return false; }
    virtual ~Token() { }
};

struct VarToken : Token {
    string identifier;
    VarToken(string id): identifier(id) { }
    string to_string() override { return identifier; }
    string str_val() override { return identifier; }
    bool is_var() override { return true; }
};

struct NumToken : Token {
    double value;
    NumToken(double val): value(val) { }
    string to_string() override { return std::to_string(value); }
    double dbl_val() override { return value; }
    bool is_num() override { return true; }
};

struct OpToken : Token {
    string operand;
    OpToken(string op): operand(op) { }
    string to_string() override { return operand; }
    string str_val() override { return operand; }
    bool is_op() override { return true; }
};

/* ~ ~ ~ ~ ~ Lexing Functions ~ ~ ~ ~ ~ */

vector<unique_ptr<Token>> tokenize(const string& expr_str);

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Operation (Parsing Tree Node) Classes ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

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

    bool is_var() override { return true; }
};

struct AdditionNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    AdditionNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " + " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() + right->eval();
    }
};

struct SubtractionNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    SubtractionNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " - " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() - right->eval();
    }
};

struct NegationNode : TreeNode {
    unique_ptr<TreeNode> node;

    NegationNode(unique_ptr<TreeNode>&& n) : node(std::move(n)) { }

    string to_string() override {
        return "(-" + node->to_string() + ')';
    }

    double eval() override {
        return -1 * node->eval();
    }
};

struct MultiplicationNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    MultiplicationNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " * " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() * right->eval();
    }
};

struct DivisionNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    DivisionNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " / " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() / right->eval();
    }
};

struct IntDivisionNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    IntDivisionNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

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

    ModulusNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " % " + right->to_string() + ')';
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

    AssignmentNode(string l, unique_ptr<TreeNode>&& r) :
        lvalue_id(l),
        rvalue(std::move(r)) { }

    string to_string() override {
        return '(' + lvalue_id + " = " + rvalue->to_string() + ')';
    }

    double eval() override {
        return set_id_value(lvalue_id, rvalue->eval());
    }
};

struct PowerNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    PowerNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " ^ " + right->to_string() + ')';
    }

    double eval() override {
        return pow(left->eval(), right->eval());
    }
};

struct EqualityNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    EqualityNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " == " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() == right->eval();
    }
};

struct InequalityNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    InequalityNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " != " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() != right->eval();
    }
};

struct LessThanNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    LessThanNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " < " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() < right->eval();
    }
};

struct GreaterThanNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    GreaterThanNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " > " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() > right->eval();
    }
};

struct LessOrEqualNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    LessOrEqualNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

    string to_string() override {
        return '(' + left->to_string() + " <= " + right->to_string() + ')';
    }

    double eval() override {
        return left->eval() <= right->eval();
    }
};

struct GreaterOrEqualNode : TreeNode {
    unique_ptr<TreeNode> left, right;

    GreaterOrEqualNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        left(std::move(l)),
        right(std::move(r)) { }

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

    FunctionCallNode(string i, vector<unique_ptr<TreeNode>>&& a) :
        function_id(i),
        args(std::move(a)) { }

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
        return call_function(function_id, args);
    }
};

struct FunctionAssignmentNode : TreeNode {
    unique_ptr<FunctionCallNode> lhs;
    unique_ptr<TreeNode> rhs;

    FunctionAssignmentNode(unique_ptr<TreeNode>&& l, unique_ptr<TreeNode>&& r) :
        lhs((FunctionCallNode *) l.release()),
        rhs(std::move(r)) { }

    string to_string() override {
        return '(' + lhs->to_string() + " = " + rhs->to_string() + ')';
    }

    double eval() override {
        string function_id = lhs->function_id;
        vector<string> arg_ids;

        for(unique_ptr<TreeNode>& arg_node : lhs->args) {
            if(!arg_node->is_var()) throw invalid_expression_error("cannot assign to a function with"
                                                                   " a non-identifier parameter");
            arg_ids.push_back(((VariableNode *) arg_node.get())->id);
        }

        assign_function(function_id, std::move(arg_ids), std::move(rhs));

        return NAN;
    }
};

struct DerivativeNode : TreeNode {
    int nth_deriv;
    string fn_id;
    vector<unique_ptr<TreeNode>> args;


    DerivativeNode(string f, vector<unique_ptr<TreeNode>>&& a, int n) :
        fn_id(f),
        args(std::move(a)),
        nth_deriv(n) { }

    string to_string() override {
        string s = fn_id;
        for(int i = 0; i < nth_deriv; i++) s += "'";
        s += "(";
        s += args.size() == 0 ? "" : args[0]->to_string();
        s += ")";
        return s;
    }

    // returns a vector with a single tree-node pointer (NumberNode)
    vector<unique_ptr<TreeNode>> to_arg_vec(double val) {
        vector<unique_ptr<TreeNode>> vec;
        vec.push_back(make_unique<NumberNode>(val));
        return vec;
    }

    // calculates the n'th derivative of fn_id at %at%
    double nderiv(int n, double at) {
        if(n == 0) {
            vector<unique_ptr<TreeNode>> arg_vec = to_arg_vec(at);
            return call_function(fn_id, arg_vec);
        }
        return (nderiv(n - 1, at + DERIV_STEP) - nderiv(n - 1, at - DERIV_STEP)) / (2 * DERIV_STEP);
    }

    double eval() override {
        if(args.size() == 0) throw invalid_expression_error("can't implicitlly differentiate a "
                                                            "function with no arguments");
        if(args.size() > 1) throw invalid_expression_error("can't implicitlly differentiate a "
                                                           "function with more than one argument "
                                                           "(consider using nderiv)");
        return nderiv(nth_deriv, args[0]->eval());
    }
};

/* ~ ~ ~ ~ ~ Grammar Parsing Functions ~ ~ ~ ~ ~ */

unique_ptr<TreeNode> parseS(vector<unique_ptr<Token>>& tokens, int i = 0);
unique_ptr<TreeNode> parseE(vector<unique_ptr<Token>>& tokens, int& i);
unique_ptr<TreeNode> parseT(vector<unique_ptr<Token>>& tokens, int& i);
unique_ptr<TreeNode> parseF(vector<unique_ptr<Token>>& tokens, int& i);
unique_ptr<TreeNode> parseX(vector<unique_ptr<Token>>& tokens, int& i);
unique_ptr<TreeNode> parseFN(vector<unique_ptr<Token>>& tokens, int& i);
vector<unique_ptr<TreeNode>> parseARGS(vector<unique_ptr<Token>>& tokens, int& i);

#endif // PARSER
