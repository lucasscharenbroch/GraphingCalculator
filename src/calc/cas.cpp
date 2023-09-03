#include "cas.h"

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Symbolic Differentiation ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

// replaces VariableNode leaves according to the given (id -> node) mapping
// (used for manually applying function calls to trees to calculate the derivative)
unique_ptr<TreeNode> tree_var_sub(unique_ptr<TreeNode>&& tree, vector<string>& sub_ids,
                                  vector<unique_ptr<TreeNode>>& sub_vals) {

    return tree->exe_on_children(std::move(tree), [&](auto node) {
            if(node->type() != nt_id) return node;

            unique_ptr<VariableNode> vn = unique_ptr<VariableNode>((VariableNode*)tree.release());
            string id = vn->id;
            for(int i = 0; i < sub_ids.size(); i++) {
                if(id == sub_ids[i]) return sub_vals[i]->copy();
            }
            return unique_ptr<TreeNode>((TreeNode*)vn.release());
        });
}

string diff_id;
bool is_partial;

unique_ptr<TreeNode> symb_deriv(unique_ptr<TreeNode>&& tree) {

    unique_ptr<TreeNode> result, left, right, arg, resl, resr, reslr, resll, resrl, resrr;

    if(is_binary_op(tree->type())) {
        left = std::move(((BinaryOpNode *)tree.get())->left);
        right = std::move(((BinaryOpNode *)tree.get())->right);
    } else if(is_unary_op(tree->type())) {
        arg = std::move(((UnaryOpNode *)tree.get())->arg);
    }

    switch(tree->type()) {
        case nt_sum: { // d(u + v) => d(u) + d(v)
            return make_unique<BinaryOpNode>(symb_deriv(std::move(left)),
                                             symb_deriv(std::move(right)),
                                             "+");
        }
        case nt_difference: { // d(u - v) = d(u) + d(-v)
            right = make_unique<UnaryOpNode>(std::move(right), "-"); // negate right
            return make_unique<BinaryOpNode>(symb_deriv(std::move(left)),
                                             symb_deriv(std::move(right)),
                                             "+");
        }
        case nt_negation: { // d(-u) = -d(u)
            return make_unique<UnaryOpNode>(symb_deriv(std::move(arg)), "-");
        }
        case nt_product: { // d(u * v) => d(u) * v + d(v) * u
            resl = make_unique<BinaryOpNode>(symb_deriv(left->copy()),
                                             right->copy(),
                                             "*");

            resr = make_unique<BinaryOpNode>(symb_deriv(std::move(right)),
                                             std::move(left),
                                             "*");

            return make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "+");
        }
        case nt_quotient: { // d(u / v) => d(u * v^-1)
            resrr = make_unique<NumberNode>(-1);
            resr = make_unique<BinaryOpNode>(std::move(right), std::move(resrr), "^");
            result = make_unique<BinaryOpNode>(std::move(left), std::move(resr), "*");
            return symb_deriv(std::move(result));
        }
        case nt_exponentiation: {// d(u ^ v) => u^v * (d(v) * ln(u) + (d(u) / u) * v)
            resl = make_unique<BinaryOpNode>(left->copy(), right->copy(), "^");

            vector<unique_ptr<TreeNode>> ln_args;
            ln_args.push_back(left->copy());

            resrl = make_unique<BinaryOpNode>(symb_deriv(right->copy()),
                    make_unique<FunctionCallNode>("ln", std::move(ln_args)),
                    "*");

            resrr = make_unique<BinaryOpNode>(make_unique<BinaryOpNode>(symb_deriv(left->copy()), std::move(left), "/"),
                    std::move(right),
                    "*");

            resr = make_unique<BinaryOpNode>(std::move(resrl), std::move(resrr), "+");

            return make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
        }
        case nt_fn_call: {
            unique_ptr<FunctionCallNode> fn = unique_ptr<FunctionCallNode>((FunctionCallNode *)tree.release());

            if(fn_table[fn->fn_id] == nullptr) {
                throw invalid_expression_error("no such function: `" + fn->fn_id + "`");
            } else if(fn_table[fn->fn_id]->is_user_fn()) {
                // careful using raw pointer! (I can't figure out how cast&borrow with unique_ptr)
                UserFunction *usr_fn = (UserFunction *)fn_table[fn->fn_id].get();
                if(usr_fn->arg_ids.size() != fn->args.size())
                    throw invalid_expression_error("expected " + to_string(usr_fn->arg_ids.size()) +
                            " argument(s) for `" + fn->fn_id + "`; "
                            "got " + to_string(fn->args.size()));
                return symb_deriv(std::move(tree_var_sub(usr_fn->tree->copy(),
                                  usr_fn->arg_ids, fn->args)));
            }

            // built-in function
            // all of the following functions are unary: ensure that only one arg is supplied
            if(fn->args.size() != 1) throw invalid_expression_error("expected 1 argument for `" +
                    fn->fn_id + "`; got " +
                    to_string(fn->args.size()));

            unique_ptr<TreeNode> arg = std::move(fn->args[0]);

            if(fn->fn_id == "ln") { // d(ln(u)) = d(u)/u
                return make_unique<BinaryOpNode>(symb_deriv(arg->copy()), std::move(arg), "/");
            } else if(fn->fn_id == "sin") { // d(sin(u)) = cos(u) * d(u)
                vector<unique_ptr<TreeNode>> cos_args;
                cos_args.push_back(arg->copy());

                resl = make_unique<FunctionCallNode>("cos", std::move(cos_args));
                resr = symb_deriv(std::move(arg));
                return make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
            } else if(fn->fn_id == "cos") { // d(cos(u)) = -(sin(u) * d(u))
                vector<unique_ptr<TreeNode>> sin_args;
                sin_args.push_back(arg->copy());

                resl = make_unique<FunctionCallNode>("sin", std::move(sin_args));
                resr = symb_deriv(std::move(arg));
                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
                return make_unique<UnaryOpNode>(std::move(result), "-");
            } else if(fn->fn_id == "tan") { // d(tan(u)) = sec(u)^2 * d(u)
                vector<unique_ptr<TreeNode>> sec_args;
                sec_args.push_back(arg->copy());

                resll = make_unique<FunctionCallNode>("sec", std::move(sec_args));
                reslr = make_unique<NumberNode>(2);

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "^");
                resr = symb_deriv(std::move(arg));

                return make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
            } else if(fn->fn_id == "csc") { // d(csc(u) = -(csc(u) * cot(u) * d(u))
                vector<unique_ptr<TreeNode>> csc_args, cot_args;
                csc_args.push_back(arg->copy());
                cot_args.push_back(arg->copy());

                resll = make_unique<FunctionCallNode>("csc", std::move(csc_args));
                reslr = make_unique<FunctionCallNode>("cot", std::move(cot_args));

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "*");
                resr = symb_deriv(std::move(arg));

                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
                return make_unique<UnaryOpNode>(std::move(result), "-");
            } else if(fn->fn_id == "sec") { // d(sec(u)) = sec(u) * tan(u) * d(u)
                vector<unique_ptr<TreeNode>> sec_args, tan_args;
                sec_args.push_back(arg->copy());
                tan_args.push_back(arg->copy());

                resll = make_unique<FunctionCallNode>("sec", std::move(sec_args));
                reslr = make_unique<FunctionCallNode>("tan", std::move(tan_args));

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "*");
                resr = symb_deriv(std::move(arg));

                return make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
            } else if(fn->fn_id == "cot") { // d(cot(u)) = -(csc(u)^2 * d(u))
                vector<unique_ptr<TreeNode>> csc_args;
                csc_args.push_back(arg->copy());

                resll = make_unique<FunctionCallNode>("csc", std::move(csc_args));
                reslr = make_unique<NumberNode>(2);

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "^");
                resr = symb_deriv(std::move(arg));

                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
                return make_unique<UnaryOpNode>(std::move(result), "-");
            } else if(fn->fn_id == "asin") { // d(asin(u)) = (1 - u^2)^(-1/2) * d(u)
                unique_ptr<TreeNode> two = make_unique<NumberNode>(2);
                resll = make_unique<BinaryOpNode>(make_unique<NumberNode>(1),
                        make_unique<BinaryOpNode>(arg->copy(),
                            std::move(two),
                            "^"),
                        "-");
                reslr = make_unique<NumberNode>(-1.0/2.0);

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "^");

                resr = symb_deriv(std::move(arg));

                return make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
            } else if(fn->fn_id == "acos") { // d(acos(u)) = -((1 - u^2)^(-1/2) * d(u))
                unique_ptr<TreeNode> two = make_unique<NumberNode>(2);
                resll = make_unique<BinaryOpNode>(make_unique<NumberNode>(1),
                        make_unique<BinaryOpNode>(arg->copy(),
                            std::move(two),
                            "^"),
                        "-");
                reslr = make_unique<NumberNode>(-1/2);

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "^");

                resr = symb_deriv(std::move(arg));

                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
                return make_unique<UnaryOpNode>(std::move(result), "-");
            } else if(fn->fn_id == "atan") { // d(atan(u)) = d(u) / (1 + u^2)
                resl = symb_deriv(arg->copy());

                resrl = make_unique<NumberNode>(1);
                resrr = make_unique<BinaryOpNode>(std::move(arg),
                        make_unique<NumberNode>(2),
                        "^");

                resr = make_unique<BinaryOpNode>(std::move(resrl), std::move(resrr), "+");

                return make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "/");
            } else {
                throw invalid_expression_error("can't differentiate function `" +
                        fn->fn_id + "`");
            }
        }
        case nt_num: {
            return make_unique<NumberNode>(0);
        }
        case nt_id: {
            string id = ((VariableNode *)tree.get())->id;
            if(id == diff_id) return make_unique<NumberNode>(1);
            else if(is_partial) return make_unique<NumberNode>(0);
            else throw invalid_expression_error("can't take non-partial derivative of `" + id + "` "
                    "with respect to " + diff_id);
        }
        default: {
            throw invalid_expression_error("cannot differentiate expression: `" +
                    tree->to_string() + "`");
        }
    }
}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Simplification ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

// This structure is only used during symbolic simplification;
// binarize(tree) should be called to convert this back to binary
// operations on a simplified tree before further operations
struct NaryOpNode : TreeNode {
    vector<unique_ptr<TreeNode>> args;
    string op;

    NaryOpNode(vector<unique_ptr<TreeNode>>&& a, string o):
        args(std::move(a)),
        op(o) {
            assert(o == "+" || o == "*"); // only addition or multiplication
        }

    enum node_type type() override {
        if(op == "+") return nt_nary_sum;
        else if(op == "*") return nt_nary_product;
        else throw calculator_error("internal error: invalid operator for nary operator");
    }

    unique_ptr<TreeNode> exe_on_children(unique_ptr<TreeNode>&& self, macro_fn fn) override {
        for(auto& arg : args) arg = arg->exe_on_children(std::move(arg), fn);
        return fn(std::move(self));
    }

    unique_ptr<TreeNode> copy() override {
        vector<unique_ptr<TreeNode>> args_copy;
        for(auto& arg : args) {
            args_copy.push_back(arg->copy());
        }

        return make_unique<NaryOpNode>(std::move(args_copy), op);
    }

    string to_string(enum node_type parent_type = nt_none) override {
        if(args.size() == 0) return "[empty n-ary " + op + "]";
        string result = "";
        result += "(";
        for(int i = 0; i < args.size(); i++) {
            result += "(" + args[i]->to_string() + ")";
            if(i + 1 != args.size()) result += " " + op + " ";
        }
        result += ")";
        return result;
    }

    double eval() override {
        double result = op == "+" ? 0 : 1;
        for(int i = 0; i < args.size(); i++) {
            if(op == "+") result += args[i]->eval();
            else result *= args[i]->eval();
        }
        return result;
    }
};

// converts any NaryOpNode subtrees back to BinaryOpNode
unique_ptr<TreeNode> binarize(unique_ptr<TreeNode>&& tree) {
    return tree->exe_on_children(std::move(tree), [](auto node) {
            if(!is_nary_op(node->type())) return node;
            unique_ptr<NaryOpNode> nn = unique_ptr<NaryOpNode>((NaryOpNode*)node.release());

            assert(nn->args.size() >= 2); // this should be ensured by simplification

            unique_ptr<TreeNode> lhs = std::move(nn->args[0]);

            for(int i = 1; i < nn->args.size(); i++) {
                lhs = make_unique<BinaryOpNode>(std::move(lhs), std::move(nn->args[i]), nn->op);
            }

            return lhs;
        });
}

// performs a "lexicographical" comparison of two trees: this is used hevily to
// establish a well-defined order for nodes during simplification in order to
// accurately identify matching node-lists. The return-value is 0 for a match,
// 1 for >, and -1 for <.
int lex_cmp(unique_ptr<TreeNode>& a, unique_ptr<TreeNode>& b) {
    enum node_type at = a->type(), bt = b->type();

    // the following is mostly taken from Joel S. Cohen's book

    if(at == nt_num && bt == nt_num) {
        return a->eval() == b->eval() ? 0 : a->eval() > b->eval() ? 1 : -1;
    } else if(at == nt_num) {
        return -1;
    } else if(bt == nt_num) {
        return 1;
    } else if(at == nt_id & bt == nt_id) {
        string aid = a->to_string(), bid = b->to_string();
        return aid == bid ? 0 : aid > bid ? 1 : -1;
    } else if(at == bt && at == nt_nary_sum || at == nt_nary_product) {
        unique_ptr<NaryOpNode> an = unique_ptr<NaryOpNode>((NaryOpNode *)a->copy().release());
        unique_ptr<NaryOpNode> bn = unique_ptr<NaryOpNode>((NaryOpNode *)b->copy().release());

        if(an->args.size() != bn->args.size()) return an->args.size() > bn->args.size() ? 1 : -1;

        for(int i = 0; i < an->args.size(); i++) {
            int cmp = lex_cmp(an->args[i], bn->args[i]);
            if(cmp != 0) return cmp;
        }

        return 0;
    } else if(at == bt && at == nt_exponentiation) {
        auto [ba, ea] = base_and_exp(a);
        auto [bb, eb] = base_and_exp(b);

        int cmp = lex_cmp(ba, bb);
        if(cmp != 0) return cmp;
        else return lex_cmp(ea, eb);
    } else if(at == bt && at == nt_fn_call) {
        unique_ptr<FunctionCallNode> an = unique_ptr<FunctionCallNode>((FunctionCallNode *)a->copy().release());
        unique_ptr<FunctionCallNode> bn = unique_ptr<FunctionCallNode>((FunctionCallNode *)b->copy().release());

        if(an->fn_id != bn->fn_id) return an->fn_id > bn->fn_id ? 1 : -1;
        else if(an->args.size() != bn->args.size()) return an->args.size() > bn->args.size() ? 1 : -1;
        for(int i = 0; i < an->args.size(); i++) {
            int cmp = lex_cmp(an->args[i], bn->args[i]) ;
            if(cmp != 0) return cmp;
        }

        return 0;
    } else if(at == bt && at == nt_deriv) {
        unique_ptr<DerivativeNode> an = unique_ptr<DerivativeNode>((DerivativeNode *)a->copy().release());
        unique_ptr<DerivativeNode> bn = unique_ptr<DerivativeNode>((DerivativeNode *)b->copy().release());

        if(an->fn_id != bn->fn_id) return an->fn_id > bn->fn_id ? 1 : -1;
        else if(an->args.size() != bn->args.size()) return an->args.size() > bn->args.size() ? 1 : -1;
        for(int i = 0; i < an->args.size(); i++) {
            int cmp = lex_cmp(an->args[i], bn->args[i]);
            if(cmp != 0) return cmp;
        }

        return 0;
    } else if(at == nt_nary_product) {
        // cast b into a unary product, and compare from there
        vector<unique_ptr<TreeNode>> uprod;
        uprod.push_back(b->copy());
        unique_ptr<TreeNode> bprod = make_unique<NaryOpNode>(std::move(uprod), "*");
        return lex_cmp(a, bprod);
    } else if(at == nt_exponentiation) {
        // cast b into an exponentiation, compare from there
        unique_ptr<TreeNode> bexp = make_unique<BinaryOpNode>(b->copy(),
                                                                  make_unique<NumberNode>(1),
                                                                  "^");
        return lex_cmp(a, bexp);
    } else if(at == nt_nary_sum) {
        // cast b into a unary sum, and compare from there
        vector<unique_ptr<TreeNode>> usum;
        usum.push_back(b->copy());
        unique_ptr<TreeNode> bsum = make_unique<NaryOpNode>(std::move(usum), "+");
        return lex_cmp(a, bsum);
    }

    if(at != bt) return at > bt ? 1 : -1;

    switch(a->type()) {
        // unary operators
        case nt_negation: {
            unique_ptr<TreeNode>& arga = ((UnaryOpNode *)a.get())->arg;
            unique_ptr<TreeNode>& argb = ((UnaryOpNode *)b.get())->arg;

            return lex_cmp(arga, argb);
        }
        // binary operators
        case nt_exponentiation:
        case nt_sum:
        case nt_product:
        case nt_difference:
        case nt_quotient:
        case nt_int_quotient:
        case nt_modulus:
        case nt_eq:
        case nt_ne:
        case nt_lt:
        case nt_le:
        case nt_gt:
        case nt_ge:
        case nt_assignment: {
            unique_ptr<TreeNode>& lefta = ((BinaryOpNode *)a.get())->left;
            unique_ptr<TreeNode>& leftb = ((BinaryOpNode *)b.get())->left;

            unique_ptr<TreeNode>& righta = ((BinaryOpNode *)a.get())->right;
            unique_ptr<TreeNode>& rightb = ((BinaryOpNode *)b.get())->right;

            int cmp;

            if((cmp = lex_cmp(lefta, leftb))) return cmp;
            else return lex_cmp(righta, rightb);
        }
        default: {
            // other types should have been handled by now
            assert(false);
            return -1;
        }
    }
}

unique_ptr<TreeNode> symb_simp(unique_ptr<TreeNode>&& tree) {
    unique_ptr<TreeNode> left, right, arg;

    if(is_binary_op(tree->type())) {
        left = symb_simp(std::move(((BinaryOpNode *)tree.get())->left));
        right = symb_simp(std::move(((BinaryOpNode *)tree.get())->right));
    } else if(is_unary_op(tree->type())) {
        arg = symb_simp(std::move(((UnaryOpNode *)tree.get())->arg));
    }

    switch(tree->type()) {
        /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Unary Operators ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
        case nt_negation: { // -u => simp(-1 * u)
            vector<unique_ptr<TreeNode>> ops;
            ops.push_back(make_unique<NumberNode>(-1));
            ops.push_back(std::move(arg));

            return symb_simp(make_unique<NaryOpNode>(std::move(ops), "*"));
        }
                          /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Binary Operators ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
        case nt_sum: { // u + v => simp(u + v)
            vector<unique_ptr<TreeNode>> ops;
            ops.push_back(std::move(left));
            ops.push_back(std::move(right));

            return symb_simp(make_unique<NaryOpNode>(std::move(ops), "+"));
        }
        case nt_difference: { // u - v => simp(u + -v)
            vector<unique_ptr<TreeNode>> terms;
            terms.push_back(std::move(left));
            terms.push_back(make_unique<UnaryOpNode>(std::move(right), "-"));

            return symb_simp(make_unique<NaryOpNode>(std::move(terms), "+"));
        }
        case nt_product: { // u * v => simp(u * v)
            vector<unique_ptr<TreeNode>> ops;
            ops.push_back(std::move(left));
            ops.push_back(std::move(right));

            return symb_simp(make_unique<NaryOpNode>(std::move(ops), "*"));
        }
        case nt_quotient: { // u / v => simp(u * simp(v^-1))
            vector<unique_ptr<TreeNode>> ops;
            ops.push_back(std::move(left));
            ops.push_back(symb_simp(make_unique<BinaryOpNode>(std::move(right),
                                    make_unique<NumberNode>(-1),
                                    "^")));

            return symb_simp(make_unique<NaryOpNode>(std::move(ops), "*"));
        }
        case nt_exponentiation: {
            if(left->type() == nt_num && right->type() == nt_num) {
                return make_unique<NumberNode>(pow(left->eval(), right->eval()));
            } else if(left->type() == nt_num && left->eval() == 0) { // 0^u = 0, assuming u > 0
                return make_unique<NumberNode>(0);
            } else if(left->type() == nt_num && left->eval() == 1) { // 1^u = 1
                return make_unique<NumberNode>(1);
            } else if(right->type() == nt_num && right->eval() == 1) { // u^1 = u
                return std::move(left);
            } else if(right->type() == nt_num && right->eval() == 0) { // u^0 = 1
                return make_unique<NumberNode>(1);
            } else if(left->type() == nt_exponentiation) { // (u^v)^w = u^vw
                unique_ptr<BinaryOpNode> en = unique_ptr<BinaryOpNode>((BinaryOpNode *)left.release());

                vector<unique_ptr<TreeNode>> exp_factors;
                exp_factors.push_back(std::move(en->right));
                exp_factors.push_back(std::move(right));

                unique_ptr<TreeNode> exp = symb_simp(make_unique<NaryOpNode>(std::move(exp_factors), "*"));

                return symb_simp(make_unique<BinaryOpNode>(std::move(en->left), std::move(exp), "^"));
            } else {
                return make_unique<BinaryOpNode>(std::move(left),
                                                 std::move(right),
                                                 "^");
            }
        }
        // u ? v => simp(u) ? simp(v)
        case nt_assignment:
        case nt_int_quotient:
        case nt_modulus:
        case nt_eq:
        case nt_ne:
        case nt_lt:
        case nt_le:
        case nt_gt:
        case nt_ge: {
            return make_unique<BinaryOpNode>(std::move(left),
                                             std::move(right),
                                             ((BinaryOpNode *)tree.get())->op);
        }
        /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ N-ary Operators ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
        case nt_deriv: {
            unique_ptr<DerivativeNode> der = unique_ptr<DerivativeNode>((DerivativeNode *)tree.release());
            for(auto& arg : der->args) {
                arg = symb_simp(std::move(arg));
            }
            return unique_ptr<TreeNode>((TreeNode *)der.release());
        }
        case nt_fn_call: {
            unique_ptr<FunctionCallNode> fn = unique_ptr<FunctionCallNode>((FunctionCallNode *)tree.release());
            for(auto& arg : fn->args) {
                arg = symb_simp(std::move(arg));
            }
            return unique_ptr<TreeNode>((TreeNode *)fn.release());
        }
        case nt_nary_sum: {
            // assume that given terms are already simplified, but not necessarily in order
            // (unless there are more than 2)
            unique_ptr<NaryOpNode> nn = unique_ptr<NaryOpNode>((NaryOpNode *)tree.release());
            if(nn->args.size() == 0) {
                return make_unique<NumberNode>(0);
            } else if(nn->args.size() == 1) {
                return symb_simp(std::move(nn->args[0]));
            } else if(nn->args.size() == 2) {
                /* ~ ~ ~ ~ ~ Term Combination ~ ~ ~ ~ ~ */
                auto [c0, b0] = const_and_base(nn->args[0]);
                auto [c1, b1] = const_and_base(nn->args[1]);

                int cmp;
                if((cmp = lex_cmp(b0, b1)) == 0) {
                    vector<unique_ptr<TreeNode>> args;
                    args.push_back(make_unique<NumberNode>(c0->eval() + c1->eval()));
                    args.push_back(std::move(b0));
                    return symb_simp(make_unique<NaryOpNode>(std::move(args), "*"));
                } else if(cmp == -1) { // terms are out of order
                    std::swap(nn->args[0], nn->args[1]);
                }

                /* ~ ~ ~ ~ ~ Numeric Simplifications ~ ~ ~ ~ ~ */
                if(nn->args[0]->type() == nt_num && nn->args[0]->eval() == 0) { // 0 + u = u
                    return std::move(nn->args[1]);
                } else if(nn->args[1]->type() == nt_num && nn->args[1]->eval() == 0) { // u + 0 = u
                    return std::move(nn->args[0]);
                    /* ~ ~ ~ ~ ~ Nested Sums ~ ~ ~ ~ ~ */
                } else if(nn->args[0]->type() == nt_nary_sum && nn->args[1]->type() == nt_nary_sum) {
                    return merge_sums(std::move(nn->args[0]), std::move(nn->args[1]));
                } else if(nn->args[0]->type() == nt_nary_sum) {
                    vector<unique_ptr<TreeNode>> usum; // make args[1] a unary sum to be merged
                    usum.push_back(std::move(nn->args[1]));
                    return merge_sums(std::move(nn->args[0]), make_unique<NaryOpNode>(std::move(usum), "+"));
                } else if(nn->args[1]->type() == nt_nary_sum) {
                    vector<unique_ptr<TreeNode>> usum; // make args[0] a unary sum to be merged
                    usum.push_back(std::move(nn->args[0]));
                    return merge_sums(std::move(nn->args[1]), make_unique<NaryOpNode>(std::move(usum), "+"));
                } else return std::move(nn);
            } else {
                // since the original input tree only has binary sums, we can assume that
                // any nary-sum with more than 2 elements is in lexicographical order
                unique_ptr<TreeNode> first = std::move(nn->args[0]);
                nn->args.erase(nn->args.begin());

                if(first->type() != nt_nary_sum) { // ensure first is a sum: if not, make it unary
                    vector<unique_ptr<TreeNode>> usum;
                    usum.push_back(std::move(first));
                    first = make_unique<NaryOpNode>(std::move(usum), "+");
                }

                unique_ptr<TreeNode> rest = symb_simp(std::move(nn));
                if(rest->type() != nt_nary_sum) { // do the same for rest
                    vector<unique_ptr<TreeNode>> usum;
                    usum.push_back(std::move(rest));
                    rest = make_unique<NaryOpNode>(std::move(usum), "+");
                }

                return merge_sums(std::move(first), std::move(rest));
            }
        }
        case nt_nary_product: {
            unique_ptr<NaryOpNode> nn = unique_ptr<NaryOpNode>((NaryOpNode *)tree.release());

            if(nn->args.size() == 0) {
                return make_unique<NumberNode>(1);
            } else if(nn->args.size() == 1) {
                return symb_simp(std::move(nn->args[0]));
            } else if(nn->args.size() == 2) {
                /* ~ ~ ~ ~ ~ Term Combination ~ ~ ~ ~ ~ */
                auto [b0, e0] = base_and_exp(nn->args[0]);
                auto [b1, e1] = base_and_exp(nn->args[1]);

                int cmp;

                if((cmp = lex_cmp(b0, b1)) == 0) {
                    vector<unique_ptr<TreeNode>> exp_terms;
                    exp_terms.push_back(std::move(e0));
                    exp_terms.push_back(std::move(e1));

                    unique_ptr<TreeNode> exp = symb_simp(make_unique<NaryOpNode>(std::move(exp_terms), "+"));
                    return symb_simp(make_unique<BinaryOpNode>(std::move(b0), std::move(exp), "^"));
                } else if(cmp == 1) {
                    std::swap(nn->args[0], nn->args[1]);
                }

                /* ~ ~ ~ ~ ~ Numeric Simplifications ~ ~ ~ ~ ~ */
                if(nn->args[0]->type() == nt_num && nn->args[1]->type() == nt_num) { // c * k = eval(c * k)
                    return make_unique<NumberNode>(nn->args[0]->eval() * nn->args[1]->eval());
                } else if(nn->args[0]->type() == nt_num && nn->args[0]->eval() == 1) { // 1 * u = u
                    return std::move(nn->args[1]);
                } else if(nn->args[1]->type() == nt_num && nn->args[1]->eval() == 1) { // u * 1 = u
                    return std::move(nn->args[0]);
                } else if(nn->args[0]->type() == nt_num && nn->args[0]->eval() == 0) { // 0 * u = 0
                    return std::move(nn->args[0]);
                } else if(nn->args[1]->type() == nt_num && nn->args[1]->eval() == 0) { // u * 0 = 0
                    return std::move(nn->args[1]);
                    /* ~ ~ ~ ~ ~ Nested Products ~ ~ ~ ~ ~ */
                } else if(nn->args[0]->type() == nt_nary_product && nn->args[1]->type() == nt_nary_product) {
                    return merge_products(std::move(nn->args[0]), std::move(nn->args[1]));
                } else if(nn->args[0]->type() == nt_nary_product) {
                    vector<unique_ptr<TreeNode>> uprod; // make args[1] a unary product to be merged
                    uprod.push_back(std::move(nn->args[1]));

                    return merge_products(std::move(nn->args[0]), make_unique<NaryOpNode>(std::move(uprod), "*"));
                } else if(nn->args[1]->type() == nt_nary_product) {
                    vector<unique_ptr<TreeNode>> uprod; // make args[0] a unary product to be merged
                    uprod.push_back(std::move(nn->args[0]));

                    return merge_products(std::move(nn->args[1]), make_unique<NaryOpNode>(std::move(uprod), "*"));
                } else return std::move(nn);
            } else {
                unique_ptr<TreeNode> first = std::move(nn->args[0]);
                nn->args.erase(nn->args.begin());

                if(first->type() != nt_nary_product) {
                    vector<unique_ptr<TreeNode>> uprod;
                    uprod.push_back(std::move(first));
                    first = make_unique<NaryOpNode>(std::move(uprod), "*");
                }

                unique_ptr<TreeNode> rest = symb_simp(std::move(nn));
                if(rest->type() != nt_nary_product) {
                    vector<unique_ptr<TreeNode>> uprod;
                    uprod.push_back(std::move(rest));
                    rest = make_unique<NaryOpNode>(std::move(uprod), "*");
                }

                return merge_products(std::move(first), std::move(rest));
            }
        }
        default: {
            return std::move(tree);
        }
    }
}

// separates the constant factor from a term, and returns a (constant_factor, rest) pair
pair<unique_ptr<TreeNode>, unique_ptr<TreeNode>> const_and_base(unique_ptr<TreeNode>& node) {
    if(node->type() == nt_num)
        return pair<unique_ptr<TreeNode>, unique_ptr<TreeNode>>(node->copy().release(),
                unique_ptr<TreeNode>(new NumberNode(1)));
    if(node->type() != nt_nary_product)
        return pair<unique_ptr<TreeNode>, unique_ptr<TreeNode>>(unique_ptr<TreeNode>(new NumberNode(1)),
                node->copy().release());

    unique_ptr<NaryOpNode> nn = unique_ptr<NaryOpNode>((NaryOpNode *)node->copy().release());
    if(nn->args.size() == 0)
        return pair<unique_ptr<TreeNode>, unique_ptr<TreeNode>>(unique_ptr<TreeNode>(new NumberNode(1)),
                                                                unique_ptr<TreeNode>(new NumberNode(1)));
    else if(nn->args[0]->type() != nt_num) // assume that first is constant, because node is simplified
        return pair<unique_ptr<TreeNode>, unique_ptr<TreeNode>>(unique_ptr<TreeNode>(new NumberNode(1)),
                                                                unique_ptr<TreeNode>(nn.release()));

    // first factor is the constant factor
    unique_ptr<TreeNode> factor = std::move(nn->args[0]);
    nn->args.erase(nn->args.begin());

    return pair<unique_ptr<TreeNode>, unique_ptr<TreeNode>>(std::move(factor),
                                                            symb_simp(unique_ptr<TreeNode>(nn.release())));
}

pair<unique_ptr<TreeNode>, unique_ptr<TreeNode>> base_and_exp(unique_ptr<TreeNode>& node) {
    if(node->type() != nt_exponentiation)
        return pair<unique_ptr<TreeNode>, unique_ptr<TreeNode>>(node->copy().release(),
                unique_ptr<TreeNode>(new NumberNode(1)));

    unique_ptr<BinaryOpNode> bn = unique_ptr<BinaryOpNode>((BinaryOpNode *)node->copy().release());

    return pair<unique_ptr<TreeNode>, unique_ptr<TreeNode>>(std::move(bn->left), std::move(bn->right));
}

// merges and simplifies two lists of nodes under a single nary operator
vector<unique_ptr<TreeNode>> merge_nary_lists(vector<unique_ptr<TreeNode>>&& a,
                                              vector<unique_ptr<TreeNode>>&& b,
                                              string op,
                                              enum node_type op_type) {
    vector<unique_ptr<TreeNode>> out_list;
    int ai = 0, bi = 0;

    if(a.size() == 0) return b;
    if(b.size() == 0) return a;

    // do merge-sort-like routine to maintain sorted order of lists and
    // attempt to simplify matching terms along the way (hopefully combinable
    // nodes are adjacent when placed in order)

    while(ai < a.size() && bi < b.size()) {
        // try to merge a[ai] and b[bi]
        vector<unique_ptr<TreeNode>> args;
        args.push_back(a[ai]->copy());
        args.push_back(b[bi]->copy());

        unique_ptr<TreeNode> _c = symb_simp(make_unique<NaryOpNode>(std::move(args), op));

        if(_c->type() != op_type) {
            ai++, bi++;

            if(_c->type() == nt_num && op_type == nt_nary_sum && _c->eval() == 0);
            else if(_c->type() == nt_num && op_type == nt_nary_product && _c->eval() == 1);
            else if(_c->type() == nt_num && op_type == nt_nary_product && _c->eval() == 0)
                return vector<unique_ptr<TreeNode>>();
            else {
                // _c fall anywhere in out_list in lexicographical order, and it might
                // be combinable with any element of out-list; recursively call merge_nary_lists
                // on _c and out_list to get an updated out_list.
                vector<unique_ptr<TreeNode>> c_list;
                c_list.push_back(std::move(_c));
                out_list = merge_nary_lists(std::move(c_list), std::move(out_list), op, op_type);
            }
        } else {
            unique_ptr<NaryOpNode> c = unique_ptr<NaryOpNode>((NaryOpNode *)_c.release());
            assert(c->args.size() == 2); // c must be exactly equal to {a[ai], b[bi]} or
                                         // {b[bi], a[ai]}, because neither element can be a
                                         // product (both are simplfied, and simplified nodes cannot
                                         // contain nested nary operators of the same type)

            // take the smaller element of c and add it into out_list, leaving the other
            // one in its respective list (this is following the merge-sort routine)

            unique_ptr<TreeNode>& to_ins = c->args[0];

            if(lex_cmp(to_ins, a[ai]) == 0) {
                ai++;
            } else if(lex_cmp(to_ins, b[bi]) == 0) {
                bi++;
            } else assert(false);

            out_list.push_back(std::move(to_ins));
        }
    }

    while(ai != a.size()) out_list.push_back(std::move(a[ai++]));
    while(bi != b.size()) out_list.push_back(std::move(b[bi++]));

    return out_list;
}

unique_ptr<TreeNode> merge_sums(unique_ptr<TreeNode>&& _a, unique_ptr<TreeNode>&& _b) {
    assert(_a->type() == nt_nary_sum && _b->type() == nt_nary_sum);
    unique_ptr<NaryOpNode> a = unique_ptr<NaryOpNode>((NaryOpNode *)_a.release());
    unique_ptr<NaryOpNode> b = unique_ptr<NaryOpNode>((NaryOpNode *)_b.release());

    vector<unique_ptr<TreeNode>> out_list = merge_nary_lists(std::move(a->args),
                                                             std::move(b->args),
                                                             "+", nt_nary_sum);

    if(out_list.size() == 0) return make_unique<NumberNode>(0);
    if(out_list.size() == 1) return std::move(out_list[0]);
    else return make_unique<NaryOpNode>(std::move(out_list), "+");
}

unique_ptr<TreeNode> merge_products(unique_ptr<TreeNode>&& _a, unique_ptr<TreeNode>&& _b) {
    assert(_a->type() == nt_nary_product && _b->type() == nt_nary_product);
    unique_ptr<NaryOpNode> a = unique_ptr<NaryOpNode>((NaryOpNode *)_a.release());
    unique_ptr<NaryOpNode> b = unique_ptr<NaryOpNode>((NaryOpNode *)_b.release());

    vector<unique_ptr<TreeNode>> out_list = merge_nary_lists(std::move(a->args),
                                                             std::move(b->args),
                                                             "*", nt_nary_product);

    if(out_list.size() == 0) return make_unique<NumberNode>(1);
    if(out_list.size() == 1) return std::move(out_list[0]);
    else return make_unique<NaryOpNode>(std::move(out_list), "*");
}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Pretty Tree ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

// is tree a negative number or negation?
bool tree_is_negative(unique_ptr<TreeNode>& tree) {
    return tree->type() == nt_num && tree->eval() < 0 ||
           tree->type() == nt_negation;
}

// negate a negative-number/negation tree in-place
void negate_negated_tree(unique_ptr<TreeNode>& tree) {
    if(tree->type() == nt_num) {
        ((NumberNode *)tree.get())->val *= -1;
    } else if(tree->type() == nt_negation) {
        tree = std::move(((UnaryOpNode *)tree.get())->arg);
    } else assert(false);
}

// converts simplified trees into "readable" trees (adds
// quotients/negations/subtractions where appropriate)
unique_ptr<TreeNode> pretty_tree(unique_ptr<TreeNode>&& tree) {
    return tree->exe_on_children(std::move(tree), [](auto node) {
            if(node->type() != nt_product        &&
               node->type() != nt_exponentiation &&
               node->type() != nt_sum) return node;

            unique_ptr<BinaryOpNode> bn = unique_ptr<BinaryOpNode>((BinaryOpNode *)node.release());
            unique_ptr<TreeNode> result;

            if(bn->type() == nt_product) {
                if(bn->left->type() == nt_num && bn->left->eval() == -1) { // negation
                    result = make_unique<UnaryOpNode>(std::move(bn->right), "-");
                } else if(bn->left->type() == nt_quotient && bn->right->type() == nt_quotient) {
                    unique_ptr<TreeNode> numer, denom, ll, lr, rl, rr;
                    unique_ptr<BinaryOpNode> left, right;

                    left = unique_ptr<BinaryOpNode>((BinaryOpNode *)bn->left.release());
                    right = unique_ptr<BinaryOpNode>((BinaryOpNode *)bn->right.release());
                    ll = unique_ptr<TreeNode>(left->left.release());
                    lr = unique_ptr<TreeNode>(left->right.release());
                    rl = unique_ptr<TreeNode>(right->left.release());
                    rr = unique_ptr<TreeNode>(right->right.release());

                    if(ll->type() == nt_num && ll->eval() == 1)
                        numer = std::move(rl);
                    else if(rl->type() == nt_num && rl->eval() == 1)
                        numer = std::move(ll);
                    else
                        numer = make_unique<BinaryOpNode>(std::move(ll), std::move(rl), "*");

                    denom = make_unique<BinaryOpNode>(std::move(lr), std::move(rr), "*");
                    result = make_unique<BinaryOpNode>(std::move(numer), std::move(denom), "/");
                } else if(bn->left->type() == nt_quotient || bn->right->type() == nt_quotient) {
                    if(bn->right->type() == nt_quotient) std::swap(bn->left, bn->right);
                    unique_ptr<BinaryOpNode> left = unique_ptr<BinaryOpNode>((BinaryOpNode *)bn->left.release());
                    unique_ptr<TreeNode> numer, denom;

                    if(left->left->type() == nt_num && left->left->eval() == 1)
                        numer = std::move(bn->right);
                    else if(bn->right->type() == nt_num && bn->right->eval() == 1)
                        numer = std::move(left->left);
                    else
                        numer = make_unique<BinaryOpNode>(std::move(left->left), std::move(bn->right), "*");

                    denom = std::move(left->right);
                    result = make_unique<BinaryOpNode>(std::move(numer), std::move(denom), "/");
                } else {
                    result = unique_ptr<TreeNode>((TreeNode *)bn.release());
                }
            } else if(bn->type() == nt_exponentiation) {
                if(tree_is_negative(bn->right)) {
                    unique_ptr<TreeNode> resl = make_unique<NumberNode>(1), resr;

                    if(bn->right->type() == nt_num && bn->right->eval() == -1) {
                        resr = std::move(bn->left);
                    } else {
                        negate_negated_tree(bn->right);
                        resr = make_unique<BinaryOpNode>(std::move(bn->left),
                                                         std::move(bn->right),
                                                         "^");
                    }

                    result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "/");

                } else {
                    result = unique_ptr<TreeNode>((TreeNode *)bn.release());
                }
            } else if(bn->type() == nt_sum) {
                if(tree_is_negative(bn->left) && tree_is_negative(bn->right)) {
                    negate_negated_tree(bn->left);
                    negate_negated_tree(bn->right);
                    result = make_unique<UnaryOpNode>(std::move(bn), "-");
                } else if(tree_is_negative(bn->left) || tree_is_negative(bn->right)) {
                    if(tree_is_negative(bn->left)) std::swap(bn->left, bn->right);
                    negate_negated_tree(bn->right);
                    result = make_unique<BinaryOpNode>(std::move(bn->left), std::move(bn->right), "-");
                } else {
                    result = unique_ptr<TreeNode>((TreeNode *)bn.release());
                }
            } else assert(false);

            return result;
        });
}
