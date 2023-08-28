#include "cas.h"

string diff_id;
bool is_partial;

unique_ptr<TreeNode> symb_deriv(unique_ptr<TreeNode>&& node) {

    unique_ptr<TreeNode> result, left, right, arg, resl, resr, reslr, resll, resrl, resrr;

    if(is_binary_op(node->type())) {
        left = std::move(((BinaryOpNode *)node.get())->left);
        right = std::move(((BinaryOpNode *)node.get())->right);
    } else if(is_unary_op(node->type())) {
        arg = std::move(((UnaryOpNode *)node.get())->arg);
    }

    switch(node->type()) {
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
            resl = make_unique<BinaryOpNode>(symb_deriv(std::move(left->copy())),
                                             std::move(right->copy()),
                                             "*");

            resr = make_unique<BinaryOpNode>(symb_deriv(std::move(right)),
                                             std::move(left),
                                             "*");

            result = make_unique<BinaryOpNode>(std::move(left), std::move(right), "+");
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
            unique_ptr<FunctionCallNode> fn = unique_ptr<FunctionCallNode>((FunctionCallNode *)node.release());

            // all of the following functions are unary: ensure that only one arg is supplied
            if(fn->args.size() != 1) throw invalid_expression_error("expected 1 argument for `" +
                                                                    fn->function_id + "`; got " +
                                                                    to_string(fn->args.size()));

            unique_ptr<TreeNode> arg = std::move(fn->args[0]);

            if(fn->function_id == "ln") { // d(ln(u)) = d(u)/u
                result = make_unique<BinaryOpNode>(symb_deriv(std::move(arg)), arg->copy(), "/");
            } // TODO add trig functions
            else {
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
            string id = ((VariableNode *)node.get())->id;
            if(id == diff_id) result = make_unique<NumberNode>(1);
            else if(is_partial) result = make_unique<NumberNode>(0);
            else throw invalid_expression_error("can't take non-partial derivative of `" + id + "` "
                                                "with respect to " + diff_id);
            break;
        }
        default: {
            throw invalid_expression_error("cannot differentiate expression: `" +
                                           node->to_string() + "`");
        }
    }

    return result;
}
