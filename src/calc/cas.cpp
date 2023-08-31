#include "cas.h"

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Misc/Utils ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

// replaces VariableNode leaves according to the given (id -> node) mapping
// (used for manually applying function calls to trees to calculate the derivative)
unique_ptr<TreeNode> tree_var_sub(unique_ptr<TreeNode>&& tree, vector<string>& sub_ids,
                                  vector<unique_ptr<TreeNode>>& sub_vals) {
    if(tree->type() == nt_num) {
        return tree;
    } else if(tree->type() == nt_id) {
        unique_ptr<VariableNode> vn = unique_ptr<VariableNode>((VariableNode*)tree.release());
        string id = vn->id;
        for(int i = 0; i < sub_ids.size(); i++) {
            if(id == sub_ids[i]) return sub_vals[i]->copy();
        }
        return vn;
    } if(is_binary_op(tree->type())) {
        unique_ptr<BinaryOpNode> bo = unique_ptr<BinaryOpNode>((BinaryOpNode *)tree.release());
        bo->left = tree_var_sub(std::move(bo->left), sub_ids, sub_vals);
        bo->right = tree_var_sub(std::move(bo->right), sub_ids, sub_vals);
        return bo;
    } else if(is_unary_op(tree->type())) {
        unique_ptr<UnaryOpNode> uo = unique_ptr<UnaryOpNode>((UnaryOpNode *)tree.release());
        uo->arg = tree_var_sub(std::move(uo->arg), sub_ids, sub_vals);
        return uo;
    } else if(tree->type() == nt_fn_call){
        unique_ptr<FunctionCallNode> fn = unique_ptr<FunctionCallNode>((FunctionCallNode *)tree.release());
        vector<unique_ptr<TreeNode>>& args = fn->args;

        for(int i = 0; i < args.size(); i++) {
            args[i] = tree_var_sub(std::move(args[i]), sub_ids, sub_vals);
        }

        return fn;
    } else if(tree->type() == nt_deriv) {
        unique_ptr<DerivativeNode> der = unique_ptr<DerivativeNode>((DerivativeNode *)tree.release());
        vector<unique_ptr<TreeNode>>& args = der->args;

        for(int i = 0; i < args.size(); i++) {
            args[i] = tree_var_sub(std::move(args[i]), sub_ids, sub_vals);
        }

        return der;
    } else {
        throw invalid_expression_error("error during tree substitution: can't "
                                       "substitute in node-type: " + to_string(tree->type()));
    }
}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Symbolic Differentiation ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

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
            result = make_unique<BinaryOpNode>(symb_deriv(std::move(left)),
                                               symb_deriv(std::move(right)),
                                               "+");
            break;
        }
        case nt_difference: { // d(u - v) = d(u) + d(-v)
            right = make_unique<UnaryOpNode>(std::move(right), "-"); // negate right
            result = make_unique<BinaryOpNode>(symb_deriv(std::move(left)),
                                               symb_deriv(std::move(right)),
                                               "+");
            break;
        }
        case nt_negation: { // d(-u) = -d(u)
            result = make_unique<UnaryOpNode>(symb_deriv(std::move(arg)), "-");
            break;
        }
        case nt_product: { // d(u * v) => d(u) * v + d(v) * u
            resl = make_unique<BinaryOpNode>(symb_deriv(left->copy()),
                                             right->copy(),
                                             "*");

            resr = make_unique<BinaryOpNode>(symb_deriv(std::move(right)),
                                             std::move(left),
                                             "*");

            result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "+");
            break;
        }
        case nt_quotient: { // d(u / v) => d(u * v^-1)
            resrr = make_unique<NumberNode>(-1);
            resr = make_unique<BinaryOpNode>(std::move(right), std::move(resrr), "^");
            result = make_unique<BinaryOpNode>(std::move(left), std::move(resr), "*");
            result = symb_deriv(std::move(result));
            break;
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


            result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
            break;
        }
        case nt_fn_call: {
            unique_ptr<FunctionCallNode> fn = unique_ptr<FunctionCallNode>((FunctionCallNode *)tree.release());

            if(fn_table[fn->function_id] == nullptr) {
                throw invalid_expression_error("no such function: `" + fn->function_id + "`");
            } else if(fn_table[fn->function_id]->is_user_fn()) {
                // careful using raw pointer! (I can't figure out how to do this with unique_ptr)
                UserFunction *usr_fn = (UserFunction *)fn_table[fn->function_id].get();
                if(usr_fn->arg_ids.size() != fn->args.size())
                    throw invalid_expression_error("expected " + to_string(usr_fn->arg_ids.size()) +
                                                   " argument(s) for `" + fn->function_id + "`; "
                                                   "got " + to_string(fn->args.size()));
                result = symb_deriv(std::move(tree_var_sub(usr_fn->tree->copy(),
                                                           usr_fn->arg_ids, fn->args)));
                break;
            }

            // built-in function
            // all of the following functions are unary: ensure that only one arg is supplied
            if(fn->args.size() != 1) throw invalid_expression_error("expected 1 argument for `" +
                                                                    fn->function_id + "`; got " +
                                                                    to_string(fn->args.size()));

            unique_ptr<TreeNode> arg = std::move(fn->args[0]);

            if(fn->function_id == "ln") { // d(ln(u)) = d(u)/u
                result = make_unique<BinaryOpNode>(symb_deriv(std::move(arg)), arg->copy(), "/");
            } else if(fn->function_id == "sin") { // d(sin(u)) = cos(u) * d(u)
                vector<unique_ptr<TreeNode>> cos_args;
                cos_args.push_back(arg->copy());

                resl = make_unique<FunctionCallNode>("cos", std::move(cos_args));
                resr = symb_deriv(std::move(arg));
                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
            } else if(fn->function_id == "cos") { // d(cos(u)) = -(sin(u) * d(u))
                vector<unique_ptr<TreeNode>> sin_args;
                sin_args.push_back(arg->copy());

                resl = make_unique<FunctionCallNode>("sin", std::move(sin_args));
                resr = symb_deriv(std::move(arg));
                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
                result = make_unique<UnaryOpNode>(std::move(result), "-");
            } else if(fn->function_id == "tan") { // d(tan(u)) = sec(u)^2 * d(u)
                vector<unique_ptr<TreeNode>> sec_args;
                sec_args.push_back(arg->copy());

                resll = make_unique<FunctionCallNode>("sec", std::move(sec_args));
                reslr = make_unique<NumberNode>(2);

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "^");
                resr = symb_deriv(std::move(arg));

                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
            } else if(fn->function_id == "csc") { // d(csc(u) = -(csc(u) * cot(u) * d(u))
                vector<unique_ptr<TreeNode>> csc_args, cot_args;
                csc_args.push_back(arg->copy());
                cot_args.push_back(arg->copy());

                resll = make_unique<FunctionCallNode>("csc", std::move(csc_args));
                reslr = make_unique<FunctionCallNode>("cot", std::move(cot_args));

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "*");
                resr = symb_deriv(std::move(arg));

                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
                result = make_unique<UnaryOpNode>(std::move(result), "-");
            } else if(fn->function_id == "sec") { // d(sec(u)) = sec(u) * tan(u) * d(u)
                vector<unique_ptr<TreeNode>> sec_args, tan_args;
                sec_args.push_back(arg->copy());
                tan_args.push_back(arg->copy());

                resll = make_unique<FunctionCallNode>("sec", std::move(sec_args));
                reslr = make_unique<FunctionCallNode>("tan", std::move(tan_args));

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "*");
                resr = symb_deriv(std::move(arg));

                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
            } else if(fn->function_id == "cot") { // d(cot(u)) = -(csc(u)^2 * d(u))
                vector<unique_ptr<TreeNode>> csc_args;
                csc_args.push_back(arg->copy());

                resll = make_unique<FunctionCallNode>("csc", std::move(csc_args));
                reslr = make_unique<NumberNode>(2);

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "^");
                resr = symb_deriv(std::move(arg));

                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
                result = make_unique<UnaryOpNode>(std::move(result), "-");
            } else if(fn->function_id == "asin") { // d(asin(u)) = (1 - u^2)^(-1/2) * d(u)
                unique_ptr<TreeNode> two = make_unique<NumberNode>(2);
                resll = make_unique<BinaryOpNode>(make_unique<NumberNode>(1),
                                                  make_unique<BinaryOpNode>(arg->copy(),
                                                                            std::move(two),
                                                                            "^"),
                                                  "-");
                reslr = make_unique<NumberNode>(-1.0/2.0);

                resl = make_unique<BinaryOpNode>(std::move(resll), std::move(reslr), "^");

                resr = symb_deriv(std::move(arg));

                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "*");
            } else if(fn->function_id == "acos") { // d(acos(u)) = -((1 - u^2)^(-1/2) * d(u))
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
                result = make_unique<UnaryOpNode>(std::move(result), "-");
            } else if(fn->function_id == "atan") { // d(atan(u)) = d(u) / (1 + u^2)
                resl = symb_deriv(arg->copy());

                resrl = make_unique<NumberNode>(1);
                resrr = make_unique<BinaryOpNode>(std::move(arg),
                                                  make_unique<NumberNode>(2),
                                                  "^");

                resr = make_unique<BinaryOpNode>(std::move(resrl), std::move(resrr), "+");

                result = make_unique<BinaryOpNode>(std::move(resl), std::move(resr), "/");
            } else {
                throw invalid_expression_error("can't differentiate function `" +
                                                fn->function_id + "`");
            }
            break;
        }
        case nt_num: {
            result =  make_unique<NumberNode>(0);
            break;
        }
        case nt_id: {
            string id = ((VariableNode *)tree.get())->id;
            if(id == diff_id) result = make_unique<NumberNode>(1);
            else if(is_partial) result = make_unique<NumberNode>(0);
            else throw invalid_expression_error("can't take non-partial derivative of `" + id + "` "
                                                "with respect to " + diff_id);
            break;
        }
        default: {
            throw invalid_expression_error("cannot differentiate expression: `" +
                                           tree->to_string() + "`");
        }
    }

    return result;
}

unique_ptr<TreeNode> symb_simp(unique_ptr<TreeNode>&& tree) {
    return nullptr;
}
